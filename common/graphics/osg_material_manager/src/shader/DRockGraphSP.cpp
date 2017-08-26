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

extern "C" {
#include "../tsort/tsort.h"
}

namespace osg_material_manager {

  DRockGraphSP::DRockGraphSP(string res_path, ConfigMap model, ConfigMap options) : IShaderProvider(res_path) {
    this->model = model;
    this->options = options;
    this->minVersion = 100;
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
    vector<string>::iterator vit;
    for (vit = source_files.begin(); vit != source_files.end(); ++vit) {
      string path = resPath + (string) *vit;
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
    std::vector<GLSLVariable> defaultInputs;
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
    filterMap["outColor"] = 1;


    ConfigVector::iterator it, et;

    // Making default values of nodes easily accessible
    for (it = graph["configuration"]["nodes"].begin(); it != graph["configuration"]["nodes"].end(); ++it) {
      ConfigMap data = ConfigMap::fromYamlString((*it)["data"].getString());
      nodeConfig[(*it)["name"]] = data["data"];
    }
    // create node ids for tsort
    unsigned long id = 1;
    for (it = graph["nodes"].begin(); it != graph["nodes"].end(); ++it) {
      string function = (*it)["model"]["name"].getString();
      string name = (*it)["name"].getString();
      if (!filterMap.hasKey(function)) {
        nodeMap[id] = (*it);
        nodeNameId[(*it)["name"].getString()] = id++;
      } else {
        if (nodeConfig.hasKey(name) && nodeConfig[name].hasKey("type")) {
          string type = nodeConfig[name]["type"].getString();
          if (type == "uniform") {
            uniforms.insert((GLSLUniform) {function, name});
          } else if (type == "varying") {
            varyings.insert((GLSLAttribute) {function, name});
          }
        }
      }
    }

    // create relations for tsort
    for (et = graph["edges"].begin(); et != graph["edges"].end(); ++et) {
      std::string from = (*et)["from"]["name"];
      std::string to = (*et)["to"]["name"];
      if (nodeNameId.find(from) != nodeNameId.end() &&
          nodeNameId.find(to) != nodeNameId.end()) {
        add_relation(nodeNameId[from], nodeNameId[to]);
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

    // create edge variables
    for (et = graph["edges"].begin(); et != graph["edges"].end(); ++et) {
      ConfigMap data = ConfigMap::fromYamlString((*et)["data"].getString());
      std::string from = (*et)["from"]["name"];
      std::string from_I = (*et)["from"]["interface"];
      std::string to = (*et)["to"]["name"];
      std::string dataType = data["dataType"];
      // Necessary to non unique edge names in DRockGui
      std::string name = (*et)["from"]["name"].getString() + "_at_" + (*et)["from"]["interface"].getString();
      (*et)["name"] = name;
      bool print = true;
      for (it = graph["nodes"].begin(); it != graph["nodes"].end(); ++it) {
        if ((*it)["name"].getString() == from) {
          if (filterMap.hasKey((*it)["model"]["name"].getString())) {
            (*et)["name"] = (*it)["name"];
            print = false;
          } else if (iOuts.hasKey(from + from_I)) {
            print = false;
            (*et)["name"] = iOuts[from + from_I]["name"];
          }
        }
        if ((*it)["name"].getString() == to) {
          if ((*it)["model"]["name"].getString() == "outColor") {
            std::string t = "  gl_FragColor = " + (*et)["name"].getString() + ";\n";
            add.push_back(t);
          } else if (filterMap.hasKey((*it)["model"]["name"].getString())) {
            std::string t = "  " + to + " = " + (*et)["name"].getString() + ";\n";
            add.push_back(t);
            //(*et)["name"] = (*it)["name"].getString();
          }
        }
      }
      if (print) {
        vars.push_back((GLSLAttribute) {dataType, name});
        iOuts[from + from_I]["name"] = name;
        iOuts[from + from_I]["connected"] = 0;
      }
    }

    // create function calls
    for (sNodeIt = sortedNodes.begin(); sNodeIt != sortedNodes.end(); ++sNodeIt) {
      ConfigMap &nodeMap = **sNodeIt;
      std::string function = nodeMap["model"]["name"];
      std::stringstream call;
      call.clear();
      if (!filterMap.hasKey(function)) {
        // todo: make shader-type sensitive!
        ConfigMap functionInfo = ConfigMap::fromYamlFile(resPath + "/graph_shader/" + function + ".yaml");
        parse_functionInfo(functionInfo);
        call << "  " << function << "(";
        std::priority_queue<PrioritizedLine> incoming, outgoing;
        bool first = true;
        for (et = graph["edges"].begin(); et != graph["edges"].end(); ++et) {
          std::string paramName;
          std::string varName = (*et)["name"];
          if ((*et)["to"]["name"].getString() == nodeMap["name"].getString()) {
            paramName = (*et)["to"]["interface"].getString();
            incoming.push((PrioritizedLine) {varName, (int) functionInfo["params"]["in"][paramName]["index"], 0});
            functionInfo["params"]["in"][paramName]["connected"] = 1;
          } else if ((*et)["from"]["name"].getString() == nodeMap["name"].getString()) {
            paramName = (*et)["from"]["interface"].getString();
            std::string iOuts_key = (*et)["from"]["name"].getString() + paramName;
            if (iOuts.hasKey(iOuts_key) && (int) iOuts[iOuts_key]["connected"] == 0) {
              outgoing.push((PrioritizedLine) {varName, (int) functionInfo["params"]["out"][paramName]["index"], 0});
              functionInfo["params"]["out"][paramName]["connected"] = 1;
              iOuts[iOuts_key]["connected"] = 1;
            }
          }
        }

        ConfigMap::iterator m_it = functionInfo["params"]["out"].beginMap();
        for (; m_it != functionInfo["params"]["out"].endMap(); m_it++) {
          if (!m_it->second.hasKey("connected")) {
            std::string varName = "unused_" + m_it->first + "_" + nodeMap["name"].getString();
            outgoing.push((PrioritizedLine) {varName, (int) m_it->second["index"], 0});
            vars.push_back((GLSLAttribute) {m_it->second["type"], varName});
          }
        }

        m_it = functionInfo["params"]["in"].beginMap();
        for (; m_it != functionInfo["params"]["in"].endMap(); m_it++) {
          if (!m_it->second.hasKey("connected")) {
            std::string varName = "default_" + m_it->first + "_" + nodeMap["name"].getString();
            std::string value = nodeConfig[nodeMap["name"]]["inputs"][m_it->first];
            incoming.push((PrioritizedLine) {varName, (int) m_it->second["index"], 0});
            defaultInputs.push_back((GLSLVariable) {m_it->second["type"], varName, value});
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
        // search for outgoing edges
        call << ");" << endl;
      }
      function_calls.push_back(call.str());
    }
    // Compose code
    code << "void main() {" << endl;
    for (size_t i = 0; i < vars.size(); ++i) {
      code << "  " << vars[i] << ";" << endl;
    }
    code << endl;
    for (size_t i = 0; i < defaultInputs.size(); ++i) {
      code << "  " << defaultInputs[i] << ";" << endl;
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

  void DRockGraphSP::parse_functionInfo(ConfigMap functionInfo) {
    ConfigMap::iterator mit;
    ConfigVector::iterator vit;
    bool is_array;
    if (functionInfo.hasKey("source")) {
      source_files.push_back(functionInfo["source"].getString());
    }

    if (functionInfo.hasKey("minVersion")) {
      int version = functionInfo["minVersion"];
      if (version > minVersion) {
        minVersion = version;
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