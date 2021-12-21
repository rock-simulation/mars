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

#include <queue>
#include <fstream>
#include "DRockGraphSP.h"
#include <mars/utils/misc.h>

extern "C" {
#include "../tsort/tsort.h"
}

namespace osg_material_manager {
  using namespace std;

  using namespace mars::utils;
  DRockGraphSP::DRockGraphSP(string res_path, ConfigMap model, ConfigMap options, string shadowTechnique) : IShaderProvider(res_path) {
    this->model = model;
    this->options = options;
    this->minVersion = 100;
    this->shadowTechnique = shadowTechnique;
    parseGraph();
  }

  int DRockGraphSP::getMinVersion() {
    return minVersion;
  }

  const set<GLSLUniform> &DRockGraphSP::getUniforms() const {
    return uniforms;
  }

  const set<GLSLAttribute> &DRockGraphSP::getVaryings() const {
    return varyings;
  }

  string DRockGraphSP::generateMainSource() {
    return main_source;
  }

  const set<string> &DRockGraphSP::getEnabledExtensions() const {
    return enabledExtensions;
  }

  const set<string> &DRockGraphSP::getDisabledExtensions() const {
    return disabledExtensions;
  }

  const std::set<GLSLConstant> &DRockGraphSP::getConstants() const {
    return constants;
  }

  const std::set<GLSLAttribute> &DRockGraphSP::getAttributes() const {
    return attributes;
  }

  const vector<pair<string, string> > DRockGraphSP::getDependencies() const {
    return dependencies;
  }

  string DRockGraphSP::generateDefinitions() {
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

  void DRockGraphSP::parseGraph() {
    stringstream code;
    std::map<unsigned long, ConfigMap> nodeMap;
    std::vector<ConfigMap *> sortedNodes;
    std::map<std::string, unsigned long> nodeNameId;
    std::map<unsigned long, ConfigMap>::iterator nodeIt;
    std::vector<ConfigMap *>::iterator sNodeIt;
    std::vector<std::string> add; // lines to add after function call generation
    std::vector<GLSLAttribute> vars; //definitions of main variables
    std::vector<GLSLVariable> defaultVars;
    std::vector<std::string> function_calls;
    ConfigItem graph = model["versions"][0]["components"];
    ConfigMap nodeConfig;
    ConfigMap iOuts; //Map containing already created edge variables for output interfaces
    ConfigMap filterMap;
    filterMap["int"] = 1;
    filterMap["float"] = 1;
    filterMap["vec2"] = 1;
    filterMap["vec3"] = 1;
    filterMap["vec4"] = 1;
    filterMap["sampler2D"] = 1;
    filterMap["samplerCube"] = 1;
    filterMap["outColor"] = 1;


    ConfigVector::iterator it;
    ConfigMap::iterator mit;

    // first store options as const variables:
    for(auto optIt: options) {
      constants.insert((GLSLConstant) {"int", optIt.first, optIt.second.toString()});
    }

    // Making default values of nodes easily accessible
    for (it = graph["configuration"]["nodes"].begin(); it != graph["configuration"]["nodes"].end(); ++it) {
      std::string name = replaceString((std::string)(*it)["name"], "::", "_");
      ConfigMap data = ConfigMap::fromYamlString((*it)["data"].getString());
      nodeConfig[name] = data["data"];
    }

    // create node ids for tsort
    unsigned long id = 1;
    for (it = graph["nodes"].begin(); it != graph["nodes"].end(); ++it) {
      string function = (*it)["model"]["name"];
      // replace abstract shadow node
      if(function == "shadow") {
        function = "shadow_"+shadowTechnique;
        (*it)["model"]["name"] = function;
      }
      std::string name = replaceString((std::string)(*it)["name"], "::", "_");
      (*it)["name"] = name;
      nodeMap[id] = (*it);
      nodeNameId[name] = id++;
      if (nodeConfig.hasKey(name) && nodeConfig[name].hasKey("type")) {
        string type = nodeConfig[name]["type"].getString();
        if(nodeConfig[name].hasKey("loadName")) {
          function << nodeConfig[name]["loadName"];
        }
        if (type == "uniform") {
          if(!options.hasKey(name)) {
            uniforms.insert((GLSLUniform) {function, name});
          }
        } else if (type == "varying") {
          varyings.insert((GLSLAttribute) {function, name});
        }
      }
    }

    for (it = graph["edges"].begin(); it != graph["edges"].end(); ++it) {
      string fromNodeName = replaceString((std::string)(*it)["from"]["name"], "::", "_");
      string toNodeName = replaceString((std::string)(*it)["to"]["name"], "::", "_");
      string fromInterfaceName = (*it)["from"]["interface"];
      string toInterface = (*it)["to"]["interface"];
      string fromVar = "";
      // create relations for tsort
      if(nodeNameId.count(fromNodeName) > 0 && nodeNameId.count(toNodeName) > 0) {
        add_relation(nodeNameId[fromNodeName], nodeNameId[toNodeName]);
      }
      ConfigMap fromNode = nodeMap[nodeNameId[fromNodeName]];
      string name = fromNode["model"]["name"];
      string function = name;
      if(nodeConfig[fromNodeName].hasKey("loadName")) {
        function << nodeConfig[name]["loadName"];
      }
      if (filterMap.hasKey(function)) {
        fromVar = fromNodeName;
      } else {
        fromVar = fromNodeName + "_at_" + fromInterfaceName;
      }
      // Create Mappings from mainVar names to node input interfaces
      nodeConfig[toNodeName]["toParams"][toInterface] = fromVar;
      ConfigMap toNode = nodeMap[nodeNameId[toNodeName]];
      if (filterMap.hasKey(toNode["model"]["name"]) && nodeConfig[toNodeName].hasKey("type")) {
        if ( nodeConfig[toNodeName]["type"].getString() == "varying") {
          // We have to output into a varying
          add.push_back(toNodeName + " = " + fromVar + ";\n");
        }
      }
    }

    tsort();
    unsigned long *ids = get_sorted_ids();
    while (*ids != 0) {
      sortedNodes.push_back(&(nodeMap[*ids]));
      ids++;
    }
    for (nodeIt = nodeMap.begin(); nodeIt != nodeMap.end(); ++nodeIt) {
      if (find(sortedNodes.begin(), sortedNodes.end(), &(nodeIt->second)) == sortedNodes.end()) {
        sortedNodes.push_back(&(nodeIt->second));
      }
    }

    for (sNodeIt = sortedNodes.begin(); sNodeIt != sortedNodes.end(); ++sNodeIt) {
      ConfigMap &nodeMap = **sNodeIt;
      string nodeFunction = nodeMap["model"]["name"];
      string nodeName = nodeMap["name"];
      priority_queue<PrioritizedLine> incoming, outgoing;
      bool first = true;
      if (nodeConfig.hasKey(nodeName)) {
        if(nodeConfig[nodeName].hasKey("loadName")) {
          nodeFunction << nodeConfig[nodeName]["loadName"];
        }
      }
      if (!filterMap.hasKey(nodeFunction)) {
        ConfigMap functionInfo = ConfigMap::fromYamlFile(resPath + "/graph_shader/" + nodeFunction + ".yaml");
        parse_functionInfo(nodeFunction, functionInfo);
        stringstream call;
        call.clear();
        call << nodeFunction << "(";
        if (functionInfo["params"].hasKey("out")) {
          for (mit = functionInfo["params"]["out"].beginMap(); mit != functionInfo["params"]["out"].endMap(); mit++) {
            string varName = nodeName + "_at_" + mit->first;
            string varType = (mit->second)["type"];
            outgoing.push((PrioritizedLine) {varName, (int) functionInfo["params"]["out"][mit->first]["index"], 0});
            vars.push_back((GLSLAttribute) {varType, varName}); // Add main var
          }
        }
        if (functionInfo["params"].hasKey("in")) {
          for (mit = functionInfo["params"]["in"].beginMap(); mit != functionInfo["params"]["in"].endMap(); mit++) {
            string varName = "";
            if (nodeConfig[nodeName]["toParams"].hasKey(mit->first)) {
              varName = nodeConfig[nodeName]["toParams"][mit->first].getString();
            } else if (nodeConfig[nodeName].hasKey("inputs") && nodeConfig[nodeName]["inputs"].hasKey(mit->first)) {
              varName = "default_" + mit->first + "_for_" + nodeName;
              string varType = (mit->second)["type"];
              defaultVars.push_back(GLSLVariable {varType, varName, nodeConfig[nodeName]["inputs"][mit->first]});
            }
            else {
              varName = "default_" + mit->first + "_for_" + nodeName;
              string varType = (mit->second)["type"].toString();
              string value = "1.0";
              if(varType == "vec2") {
                value = "vec2(0, 0)";
              }
              if(varType == "vec3") {
                value = "vec2(0, 0, 0)";
              }
              if(varType == "vec4") {
                value = "vec4(0, 0, 0, 1)";
              }
              else {
                fprintf(stderr, "Shader ERROR: input not connected: node %s input %lu\n", nodeName.c_str(), incoming.size());
              }
              defaultVars.push_back(GLSLVariable {varType, varName, value});
            }
            incoming.push((PrioritizedLine) {varName, (int) functionInfo["params"]["in"][mit->first]["index"], 0});
          }
        }
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
      }
    }

    // Compose code
    code << "void main() {" << endl;
    for (size_t i = 0; i < vars.size(); ++i) {
      code << "  " << vars[i] << ";" << endl;
    }
    code << endl;
    for (size_t i = 0; i < defaultVars.size(); ++i) {
      code << "  " << defaultVars[i] << ";" << endl;
    }
    code << endl;
    for (size_t i = 0; i < function_calls.size(); ++i) {
      code << function_calls[i] << endl;
    }
    for (size_t i = 0; i < add.size(); ++i) {
      code << add[i];
    }
    code << "}" << endl;
    main_source = code.str();
  }

  void DRockGraphSP::parse_functionInfo(string functionName, ConfigMap functionInfo) {
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
        for (vit=list["enabled"].begin();vit!=list["enabled"].end();vit++) {
          enabledExtensions.insert(vit.base()->getString());
        }
      }
      if (list.hasKey("disabled")) {
        for (vit=list["disabled"].begin();vit!=list["disabled"].end();vit++) {
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
          if(!options.hasKey(name)) {
            uniforms.insert((GLSLUniform) {type_name, name});
          }
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
