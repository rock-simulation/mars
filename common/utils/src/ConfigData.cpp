#include "ConfigData.h"

#include <yaml-cpp/yaml.h>
#include <fstream>


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



    ConfigMap ConfigMap::fromYamlFile(const string &filename) {
      std::ifstream fin(filename.c_str());
      YAML::Parser parser(fin);
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


    void ConfigMap::toYamlFile(const std::string &filename) const {
      YAML::Emitter emitter;
      dumpConfigMapToYaml(emitter, *this);
      if(!emitter.good()) {
        fprintf(stderr, "ERROR: CFGManager::dumpConfigMapToYaml failed!\n");
        return;
      }
      std::ofstream f(filename.c_str());
      if(!f.good()) {
        fprintf(stderr,
                "ERROR: CFGManager::dumpConfigMapToYaml failed! "
                "could not open output file \"%s\"\n", filename.c_str());
        return;
      }
      f << emitter.c_str() << endl;
      f.close();
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
      if(!item.getUnparsedString().empty())
        emitter << item.getUnparsedString();
      if(item.children.size())
        dumpConfigMapToYaml(emitter, item.children);
    }
    
    static void dumpConfigVectorToYaml(YAML::Emitter &emitter,
                                       const ConfigVector &vec) {
      if(vec.size() > 1) {
        emitter << YAML::BeginSeq;
      }
      assert(emitter.good() && 1);
      for(unsigned int i = 0; i < vec.size(); ++i) {
        dumpConfigItemToYaml(emitter, vec[i]);
      }
      if(vec.size() > 1) {
        emitter << YAML::EndSeq;
      }
    }

    static void dumpConfigMapToYaml(YAML::Emitter &emitter,
                                    const ConfigMap &configMap) {
      emitter << YAML::BeginMap;
      ConfigMap::const_iterator it;
      for(it = configMap.begin(); it != configMap.end(); ++it) {
        emitter << YAML::Key << it->first;
        emitter << YAML::Value;
        dumpConfigVectorToYaml(emitter, it->second);
      }
      emitter << YAML::EndMap;
    }

  } // end of namespace utils
} // end of namespace mars
