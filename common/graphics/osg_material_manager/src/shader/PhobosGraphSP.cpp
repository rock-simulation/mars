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
#include <queue>
#include "PhobosGraphSP.h"

namespace osg_material_manager {
  using namespace std;

  PhobosGraphSP::PhobosGraphSP(string res_path, ConfigMap graph, ConfigMap options) : IShaderProvider(res_path) {
    this->model = graph;
    this->options = options;
    this->minVersion = 120;
    parseGraph();
  }

  void PhobosGraphSP::parseGraph() {
    cout << "parsing!!!!!!!!!" << endl;
    vector<GLSLAttribute> vars;
    vector<string> function_calls;
    ConfigVector::iterator vit;
    ConfigMap::iterator mit;
    ConfigMap functionInfo;
    string customPath = options["loadPath"].getString() + options["customPath"].getString() + "/";
    string defaultPath = resPath + "/graph_shader/";
    string legacyPath = resPath + "/shader/";
    string actualPath = "";
    for (vit = model["sort"].begin(); vit != model["sort"].end(); ++vit) { // iterate over sorted node names
      string nodeName = vit.base()->getString();
      string nodeType = model["nodes"][nodeName]["type"].getString();
      if (nodeType == "uniform") {
        if (!(bool)(model["nodes"][nodeName]["uniform_builtin"].getOrCreateAtom())) {
          uniforms.insert({model["nodes"][nodeName]["uniform_type"].getString(),
                           model["nodes"][nodeName]["uniform_name"].getString()});
        }
        continue;
      } else if (nodeType == "varying_fragment") {
        varyings.insert({model["nodes"][nodeName]["varying_type"].getString(),
                         model["nodes"][nodeName]["varying_name"].getString()});
        continue;
      } else if (nodeType == "varying_vertex") {
        varyings.insert({model["nodes"][nodeName]["varying_type"].getString(),
                         model["nodes"][nodeName]["varying_name"].getString()});
        stringstream call;
        call.clear();
        call << model["nodes"][nodeName]["varying_name"].getString() << " = "
             << model["nodes"][nodeName]["incoming"]["input"].getString() << ";" << endl;
        function_calls.push_back(call.str());
        continue;
      }
      if (file_exists(customPath + nodeType + ".yaml")) {
        actualPath = customPath + nodeType + ".yaml";
      } else if (file_exists(defaultPath + nodeType + ".yaml")) {
        actualPath = defaultPath + nodeType + ".yaml";
      } else if (file_exists(legacyPath + nodeType + ".yaml")) {
        actualPath = legacyPath + nodeType + ".yaml";
      } else {
        cout << "ERROR: Could not find node definition for " << nodeType << "!" << endl;
        // TODO: How to handle an error like that?
        return;
      }
      functionInfo = ConfigMap::fromYamlFile(actualPath);
      parse_functionInfo(nodeType, functionInfo);
      // CREATE VAR DEFINITIONS
      for (mit = model["nodes"][nodeName]["outgoing"].beginMap();
           mit != model["nodes"][nodeName]["outgoing"].endMap(); ++mit) {
        vars.push_back({(mit->second)["type"], (mit->second)["name"]});
      }
      //Create function calls
      priority_queue<PrioritizedLine> incoming, outgoing;
      if (functionInfo["params"].hasKey("in")) {
        for (mit = functionInfo["params"]["in"].beginMap(); mit != functionInfo["params"]["in"].endMap(); ++mit) {
          incoming.push((PrioritizedLine) {model["nodes"][nodeName]["incoming"][mit->first],
                                           (int) functionInfo["params"]["in"][mit->first]["index"], 0});
        }
      }
      if (functionInfo["params"].hasKey("out")) {
        for (mit = functionInfo["params"]["out"].beginMap(); mit != functionInfo["params"]["out"].endMap(); ++mit) {
          outgoing.push((PrioritizedLine) {model["nodes"][nodeName]["outgoing"][mit->first]["name"].getString(),
                                           (int) functionInfo["params"]["out"][mit->first]["index"], 0});
        }
      }
      stringstream call;
      bool first = true;
      call.clear();
      call << nodeType << "(";
      while (!incoming.empty()) {
        if (!first) {
          call << ", ";
        }
        first = false;
        call << incoming.top().line;
        incoming.pop();
      }
      while (!outgoing.empty()) {
        if (!first) {
          call << ", ";
        }
        first = false;
        call << outgoing.top().line;
        outgoing.pop();
      }
      call << ");" << endl;
      function_calls.push_back(call.str());
    } // end iteration over sorted node names
    // Create main function code
    stringstream code;
    code.clear();
    code << "void main() {" << endl;
    for (size_t i = 0; i < vars.size(); ++i) {
      code << "  " << vars[i] << ";" << endl;
    }
    code << endl;
    for (size_t i = 0; i < function_calls.size(); ++i) {
      code << function_calls[i] << endl;
    }
    code << "}" << endl;
    main_source = code.str();
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
      ifstream t((string) mit->second);
      stringstream buffer;
      buffer << t.rdbuf();
      code << buffer.str() << endl;
    }
    return code.str();
  }

  void PhobosGraphSP::parse_functionInfo(string functionName, ConfigMap functionInfo) {
    ConfigMap::iterator mit;
    ConfigVector::iterator vit;
    if (source_files.count(functionName) == 0) {
      string suffix = "";
      if (functionInfo.hasKey("source")) {
        suffix = functionInfo["source"].getString();
      } else {
        suffix = functionName + ".c";
      }
      if (file_exists(options["loadPath"].getString() + options["customPath"].getString() + "/" + suffix)) {
        source_files[functionName] = options["loadPath"].getString() + options["customPath"].getString() + "/" + suffix;
      } else if (file_exists(resPath + "/" + suffix)) {
        source_files[functionName] = resPath + "/" + suffix;
      }
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
        std::cout << "Having enabled extensions" <<
                  std::endl;
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