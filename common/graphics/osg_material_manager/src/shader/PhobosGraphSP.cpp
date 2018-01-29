/*
 *  Copyright 2017, DFKI GmbH Robotics Innovation Center
 *
 *  This file is part of the MARS simulation framework.
 *
 *  MARS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3
 *  of the License, or (at your option) any later version.
 *
 *  MARS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with MARS.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <fstream>
#include "PhobosGraphSP.h"
#include <boost/filesystem.hpp>

namespace osg_material_manager {
  using namespace std;

  PhobosGraphSP::PhobosGraphSP(string res_path, ConfigMap graph, ConfigMap options) : IShaderProvider(res_path) {
    this->model = model;
    this->options = options;
    this->minVersion = 100;
    parseGraph();
  }

  void PhobosGraphSP::parseGraph() {
    vector<GLSLAttribute> vars;
    vector<string> function_calls;
    ConfigVector::iterator vit;
    ConfigMap::iterator mit;
    string customPath = model["loadPath"] + model["customPath"] + "/";
    string defaultPath = resPath + "/graph_shader/";
    string legacyPath = resPath + "/shader/";
    string actualPath = "";
    for (vit = model["sort"].begin(); vit != model["sort"].end(); ++vit) { // iterate over sorted node names
      string nodeName = vit.base()->getString();
      string nodeType = model["nodes"][nodeName]["type"].getString();
      if (nodeType == "uniform") {
        uniforms.insert({model["nodes"][nodeName]["uniform_type"].getString(),
                         model["nodes"][nodeName]["uniform_name"].getString()});
        continue;
      } else if (nodeType == "varying_fragment") {
        varyings.insert({model["nodes"][nodeName]["varying_type"].getString(),
                         model["nodes"][nodeName]["varying_name"].getString()});
        continue;
      }
      if (boost::filesystem::exists(customPath + nodeType + ".yaml")) {
        actualPath = customPath + nodeType + ".yaml";
      } else if (boost::filesystem::exists(defaultPath + nodeType + ".yaml")) {
        actualPath = defaultPath + nodeType + ".yaml";
      } else if (boost::filesystem::exists(legacyPath + nodeType + ".yaml")) {
        actualPath = legacyPath + nodeType + ".yaml";
      } else {
        cout << "ERROR: Could not find node definition for " << nodeType << "!" << endl;
        // TODO: How to handle an error like that?
        return;
      }
      parse_functionInfo(nodeType, ConfigMap::fromYamlFile(actualPath));
      // CREATE VAR DEFINITIONS
      for (mit = model["nodes"][nodeName]["outgoing"].beginMap();
           mit != model["nodes"][nodeName]["outgoing"].endMap(); ++mit) {
        vars.push_back({(mit->second)["type"], (mit->second)["name"]});
      }
      // TODO: Create function calls, Create main function code
    } // end iteration over sorted node names
  }

  int PhobosGraphSP::getMinVersion() {
    return minVersion;
  }

  const set<GLSLUniform> &PhobosGraphSP::getUniforms() const {
    return uniforms;
  }

  const set<GLSLAttribute> &PhobosGraphSP::getVaryings() const {
    return varyings;
  }

  string PhobosGraphSP::generateMainSource() {
    return main_source;
  }

  const set<string> &PhobosGraphSP::getEnabledExtensions() const {
    return enabledExtensions;
  }

  const set<string> &PhobosGraphSP::getDisabledExtensions() const {
    return disabledExtensions;
  }

  const std::set<GLSLConstant> &PhobosGraphSP::getConstants() const {
    return constants;
  }

  const std::set<GLSLAttribute> &PhobosGraphSP::getAttributes() const {
    return attributes;
  }

  const vector<pair<string, string> > PhobosGraphSP::getDependencies() const {
    return dependencies;
  }

  string PhobosGraphSP::generateDefinitions() {
    stringstream code;
    map<string, string>::iterator mit;
    for (mit = source_files.begin(); mit != source_files.end(); ++mit) {
      string path = resPath + (string) mit->second;
      ifstream t(path);
      stringstream buffer;
      buffer << t.rdbuf();
      code << buffer.str() << endl;
    }
    return code.str();
  }

  void PhobosGraphSP::parse_functionInfo(string functionName, ConfigMap functionInfo) {
    ConfigMap::iterator mit;
    ConfigVector::iterator vit;
    if (functionInfo.hasKey("source") && source_files.count(functionName) == 0) {
      source_files[functionName] = functionInfo["source"].getString();
    }

    if (functionInfo.hasKey("minVersion")) {
      int version = functionInfo["minVersion"];
      if (version > minVersion) {
        minVersion = version;
      }
    }

    if (functionInfo.hasKey("extensions")) {
      ConfigItem list = functionInfo["extensions"];
      if (list.hasKey("enabled")) {
        std::cout << "Having enabled extensions" << std::endl;
        for (vit = list["enabled"].begin(); vit != list["enabled"].end(); vit++) {
          enabledExtensions.insert(vit.base()->getString());
        }
      }
      if (list.hasKey("disabled")) {
        for (vit = list["disabled"].begin(); vit != list["disabled"].end(); vit++) {
          disabledExtensions.insert(vit.base()->getString());
        }
      }
    }

    if (functionInfo.hasKey("uniforms")) {
      ConfigItem list = functionInfo["uniforms"];
      for (mit = list.beginMap(); mit != list.endMap(); ++mit) {
        string type = mit->first;
        string type_name = "";
        std::size_t a_pos = type.find("[]");
        bool is_array = false;
        if (a_pos != string::npos) {
          type_name = type.substr(0, a_pos);
          is_array = true;
        } else {
          type_name = type;
        }
        for (vit = list[type].begin(); vit != list[type].end(); ++vit) {
          ConfigItem &item = *vit;
          string name = item["name"].getString();
          if (is_array) {
            stringstream s;
            int num = 1;
            if (item.hasKey("arraySize")) {
              num = (int) options[item["arraySize"].getString()];
            }
            s << "[" << num << "]";
            name.append(s.str());
          }
          uniforms.insert((GLSLUniform) {type_name, name});
        }
      }
    }

    if (functionInfo.hasKey("varyings")) {
      ConfigItem list = functionInfo["varyings"];
      for (mit = list.beginMap(); mit != list.endMap(); ++mit) {
        string type = mit->first;
        string type_name = "";
        std::size_t a_pos = type.find("[]");
        bool is_array = false;
        if (a_pos != string::npos) {
          type_name = type.substr(0, a_pos);
          is_array = true;
        } else {
          type_name = type;
        }
        for (vit = list[type].begin(); vit != list[type].end(); ++vit) {
          ConfigItem &item = *vit;
          string name = item["name"].getString();
          if (is_array) {
            stringstream s;
            int num = 1;
            if (item.hasKey("arraySize")) {
              num = (int) options[item["arraySize"].getString()];
            }
            s << "[" << num << "]";
            name.append(s.str());
          }
          varyings.insert((GLSLAttribute) {type_name, name});
        }
      }
    }

    if (functionInfo.hasKey("attributes")) {
      ConfigItem list = functionInfo["attributes"];
      for (mit = list.beginMap(); mit != list.endMap(); ++mit) {
        string type = mit->first;
        string type_name = "";
        std::size_t a_pos = type.find("[]");
        bool is_array = false;
        if (a_pos != string::npos) {
          type_name = type.substr(0, a_pos);
          is_array = true;
        } else {
          type_name = type;
        }
        for (vit = list[type].begin(); vit != list[type].end(); ++vit) {
          ConfigItem &item = *vit;
          string name = item["name"].getString();
          if (is_array) {
            stringstream s;
            int num = 1;
            if (item.hasKey("arraySize")) {
              num = (int) options[item["arraySize"].getString()];
            }
            s << "[" << num << "]";
            name.append(s.str());
          }
          attributes.insert((GLSLAttribute) {type_name, name});
        }
      }
    }
  }
}