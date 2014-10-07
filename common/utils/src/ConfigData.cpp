#include "ConfigData.h"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <sstream>


namespace mars {
  namespace utils {

    using std::string;
    using std::endl;

    /************************
     * Implementation
     ************************/

    static ConfigItem parseConfigItemFromYamlNode(const YAML::Node &n);
    static ConfigVector parseConfigVectorFromYamlNode(const YAML::Node &n);
    static ConfigMap parseConfigMapFromYamlNode(const YAML::Node &n);
    static void dumpConfigItemToYaml(YAML::Emitter &emitter,
                                     const ConfigItem &item);
    static void dumpConfigVectorToYaml(YAML::Emitter &emitter,
                                       const ConfigVector &vec);
    static void dumpConfigMapToYaml(YAML::Emitter &emitter,
                                    const ConfigMap &configMap);



    ConfigMap ConfigMap::fromYamlStream(std::istream &in) {
      YAML::Parser parser(in);
      YAML::Node doc, node;
      while(parser.GetNextDocument(doc)) {
        if(doc.Type() == YAML::NodeType::Map) {
          return parseConfigMapFromYamlNode(doc);
        } else {
          fprintf(stderr,
                  "CFGManager::ConfigMapFromYaml currently only supports "
                  "mapping types at the root level. please contact the "
                  "developers if you need support for other types.\n");
          return ConfigMap();
        }
      }
      // if there is no valid document return a empty ConfigMap
      return ConfigMap();
    }

    ConfigMap ConfigMap::fromYamlFile(const string &filename, bool loadURI) {
      std::ifstream fin(filename.c_str());
      ConfigMap map = fromYamlStream(fin);
      if(loadURI) {
        std::string pathToFile = getPathOfFile(filename);
        recursiveLoad(&map, pathToFile);
      }
      return map;
    }

    void ConfigMap::recursiveLoad(ConfigMap *map, std::string path) {
      ConfigMap::iterator it = map->begin();
      for(; it!=map->end(); ++it) {
        if(it->first == "URI") {
          fprintf(stderr, "ConfigMap::recursiveLoad: found uri: %s\n",
                  it->second[0].getString().c_str());
          std::string subPath, file = (std::string)it->second[0];
          subPath = getPathOfFile(file);
          ConfigMap m2 = fromYamlFile(file, true);
          recursiveLoad(&m2, subPath);
          map->append(m2);
        }
        else {
          ConfigVector::iterator vIt = it->second.begin();
          for(;vIt!=it->second.end(); ++vIt) {
            recursiveLoad(&vIt->children, path);
          }
        }
      }
    }

    ConfigMap ConfigMap::fromYamlString(const string &s) {
      std::istringstream sin(s);
      return fromYamlStream(sin);
    }


    void ConfigMap::toYamlStream(std::ostream &out) const {
      YAML::Emitter emitter;
      dumpConfigMapToYaml(emitter, *this);
      if(!emitter.good()) {
        fprintf(stderr, "ERROR: ConfigMap::toYamlStream failed!\n");
        return;
      }
      out << emitter.c_str() << endl;
    }

    void ConfigMap::toYamlFile(const std::string &filename) const {
      std::ofstream f(filename.c_str());
      if(!f.good()) {
        fprintf(stderr,
                "ERROR: ConfigMap::toYamlFile failed! "
                "Could not open output file \"%s\"\n", filename.c_str());
        return;
      }
      toYamlStream(f);
    }

    std::string ConfigMap::toYamlString() const {
      std::ostringstream sout;
      toYamlStream(sout);
      return sout.str();
    }


    /***************************
     * static helper functions *
     ***************************/


    static ConfigItem parseConfigItemFromYamlNode(const YAML::Node &n) {
      ConfigItem item;
      if(n.Type() == YAML::NodeType::Scalar) {
        std::string s;
        n.GetScalar(s);
        item.setUnparsedString(s);
      }
      return item;
    }

    static ConfigVector parseConfigVectorFromYamlNode(const YAML::Node &n) {
      ConfigVector vec;
      if(n.Type() == YAML::NodeType::Sequence) {
        YAML::Iterator it;
        for(it = n.begin(); it != n.end(); ++it) {
          ConfigItem item;
          if(it->Type() == YAML::NodeType::Scalar) {
            item = parseConfigItemFromYamlNode(*it);
          } else if(it->Type() == YAML::NodeType::Sequence) {
            item[""] = parseConfigVectorFromYamlNode(*it);
          } else if(it->Type() == YAML::NodeType::Map) {
            item.children = parseConfigMapFromYamlNode(*it);
          }
          vec.push_back(item);
        }
      }
      return vec;
    }
    
    static ConfigMap parseConfigMapFromYamlNode(const YAML::Node &n) {
      ConfigMap configMap;
      for(YAML::Iterator it = n.begin(); it != n.end(); ++it) {
        if(it.second().Type() == YAML::NodeType::Scalar) {
          configMap[it.first().to<std::string>()].push_back(parseConfigItemFromYamlNode(it.second()));
        } else if(it.second().Type() == YAML::NodeType::Sequence) {
          configMap[it.first().to<std::string>()] = parseConfigVectorFromYamlNode(it.second());
        } else if(it.second().Type() == YAML::NodeType::Map) {
          ConfigItem item;
          item.children = parseConfigMapFromYamlNode(it.second());
          configMap[it.first().to<std::string>()].push_back(item);
        } else if(it.second().Type() == YAML::NodeType::Null) {
          continue;
        } else {
          fprintf(stderr, "Unknown YAML::NodeType: %d\n", it.second().Type());
          continue;
        }
      }
      return configMap;
    }

    static void dumpConfigItemToYaml(YAML::Emitter &emitter,
                                     const ConfigItem &item) {
      std::string s = item.toString();

      if(!s.empty() && item.children.size()) {
        fprintf(stderr, "%s: To dump to yaml file it is not allowed to have a item value and map at the same time.\n",
                item.getParentName().c_str());
        assert(false);
      }
      else {
        if(s.empty() && item.children.size() == 0) {
          emitter << "";
        }
      }

      if(!s.empty())
        emitter << s;
      if(item.children.size())
        dumpConfigMapToYaml(emitter, item.children);
    }
    
    static void dumpConfigVectorToYaml(YAML::Emitter &emitter,
                                       const ConfigVector &vec) {
      bool do_sequence = false;
      if(vec.size() == 1){
        if (vec.begin()->children.size() > 1){
          do_sequence = true;
        }
      }
      if(vec.size() > 1 || do_sequence) {
        emitter << YAML::BeginSeq;
      }
      if(!(emitter.good() && 1)) {
        std::string s = vec.getParentName();
        fprintf(stderr, "problem with ConfigVector for: %s\n",
                s.c_str());
      }
      assert(emitter.good() && 1);
      for(unsigned int i = 0; i < vec.size(); ++i) {
        dumpConfigItemToYaml(emitter, vec[i]);
      }
      if(vec.size() > 1 || do_sequence) {
        emitter << YAML::EndSeq;
      }
    }

    static void dumpConfigMapToYaml(YAML::Emitter &emitter,
                                    const ConfigMap &configMap) {
      emitter << YAML::BeginMap;
      ConfigMap::const_iterator it;
      for(it = configMap.begin(); it != configMap.end(); ++it) {
        emitter << YAML::Key << it->first;
        if(!(emitter.good())) {
          fprintf(stderr, "problem with ConfigMap for: %s\n", it->first.c_str());
        }
        emitter << YAML::Value;
        dumpConfigVectorToYaml(emitter, it->second);
      }
      emitter << YAML::EndMap;
    }

  } // end of namespace utils
} // end of namespace mars
