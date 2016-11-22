/*
 *  Copyright 2011, 2016, DFKI GmbH Robotics Innovation Center
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

#include <sstream>
#include <iostream>
#include <fstream>
#include "yaml-shader.h"

namespace osg_material_manager {

    using namespace std;
    using namespace configmaps;

    YamlShader::YamlShader(string name, vector<std::string> &args, ConfigMap &map, string resPath)
            : ShaderFunc(name, args) {
      if (map.hasKey("source")) {
        resPath += (string)map["source"];
        ifstream t(resPath.c_str());
        stringstream buffer;
        buffer << t.rdbuf();
        source = buffer.str();
      } else {
        source = "";
      }

      if (map.hasKey("priority")) {
        funcs[0].setPriority((unsigned int)map["priority"]);
      }

      if (map.hasKey("params")) {
        ConfigVector::iterator it = map["params"].begin();
        for (;it!=map["params"].end();it++) {
          funcs[0].addArgument(it.base()->getString());
        }
      }

      if (map.hasKey("varyings")) {
        ConfigMap::iterator it = map["varyings"].beginMap();
        for (;it!=map["varyings"].endMap();it++) {
          string type = it->first;
          std::size_t a_pos = type.find("[]"); // to determine if value is array or not
          ConfigVector::iterator it2 = map["varyings"][type].begin();
          for (;it2!=map["varyings"][type].end();it2++) {
            ConfigItem &item = *it2;
            std::stringstream s;
            string typeName; // should contain type without [] if present at the end
            if (a_pos != string::npos) {
              typeName = type.substr(0,a_pos);
              if (item.hasKey("arraySize")) {
                string size = map["mappings"][(string)item["arraySize"]];
                s << "[" << size << "]";
              } else {
                s << "[1]"; // fallback arraySize of 1
              }
            } else {
              typeName = type;
              s << "";
            }
            addVarying( (GLSLVarying) { typeName, (string)item["name"] + s.str() } );
          }
        }
      }

      if (map.hasKey("uniforms")) {
        ConfigMap::iterator it = map["uniforms"].beginMap();
        for (;it!=map["uniforms"].endMap();it++) {
          string type = it->first;
          std::size_t a_pos = type.find("[]"); // to determine if value is array or not
          ConfigVector::iterator it2 = map["uniforms"][type].begin();
          for (;it2!=map["uniforms"][type].end();it2++) {
            ConfigItem &item = *it2;
            std::stringstream s;
            string typeName; // should contain type without [] if present at the end
            if (a_pos != string::npos) {
              typeName = type.substr(0,a_pos);
              if (item.hasKey("arraySize")) {
                string size = map["mappings"][(string)item["arraySize"]];
                s << "[" << size << "]";
              } else {
                s << "[1]"; // fallback arraySize of 1
              }
            } else {
              typeName = type;
              s << "";
            }
            addUniform( (GLSLUniform) { typeName, (string)item["name"] + s.str() } );
          }
        }
      }

      if (map.hasKey("attributes")) {
        ConfigMap::iterator it = map["attributes"].beginMap();
        for (;it!=map["attributes"].endMap();it++) {
          string type = it->first;
          std::size_t a_pos = type.find("[]"); // to determine if value is array or not
          ConfigVector::iterator it2 = map["attributes"][type].begin();
          for (;it2!=map["attributes"][type].end();it2++) {
            ConfigItem &item = *it2;
            std::stringstream s;
            string typeName; // should contain type without [] if present at the end
            if (a_pos != string::npos) {
              typeName = type.substr(0,a_pos);
              if (item.hasKey("arraySize")) {
                string size = map["mappings"][(string)item["arraySize"]];
                s << "[" << size << "]";
              } else {
                s << "[1]"; // fallback arraySize of 1
              }
            } else {
              typeName = type;
              s << "";
            }
            addUniform( (GLSLUniform) { typeName, (string)item["name"] + s.str() } );
          }
        }
      }

      if (map.hasKey("exports")) {
        ConfigVector::iterator it = map["exports"].begin();
        for (;it!=map["exports"].end();it++) {
          ConfigItem &item = *it;
          addExport( (GLSLExport) { (string)item["name"], (string)item["value"] } );
        }
      }

      if (map.hasKey("mainVarDecs")) {
        ConfigMap::iterator it = map["mainVarDecs"].beginMap();
        for (;it!=map["mainVarDecs"].endMap();it++) {
          string type = it->first;
          std::size_t a_pos = type.find("[]"); // to determine if value is array or not
          ConfigVector::iterator it2 = map["mainVarDecs"][type].begin();
          for (;it2!=map["mainVarDecs"][type].end();it2++) {
            ConfigItem &item = *it2;
            std::stringstream s;
            string typeName;
            if (a_pos != string::npos) {
              typeName = type.substr(0,a_pos);
              if (item.hasKey("arraySize")) {
                string size = map["mappings"][(string)item["arraySize"]];
                s << "[" << size << "]";
              } else {
                s << "[1]"; // fallback arraySize of 1
              }
            } else {
              typeName = type;
              s << "";
            }
            addMainVarDec((GLSLAttribute) {typeName, (string)item["name"]});
          }
        }
      }

      if (map.hasKey("mainVars")) {
        ConfigMap::iterator it = map["mainVars"].beginMap();
        for (;it!=map["mainVars"].endMap();it++) {
          string type = it->first;
          std::size_t a_pos = type.find("[]"); // to determine if value is array or not
          ConfigVector::iterator it2 = map["mainVars"][type].begin();
          for (;it2!=map["mainVars"][type].end();it2++) {
            ConfigItem &item = *it2;
            std::stringstream s;
            string typeName; // should contain type without [] if present at the end
            if (a_pos != string::npos) {
              typeName = type.substr(0,a_pos);
              if (item.hasKey("arraySize")) {
                string size = map["mappings"][(string)item["arraySize"]];
                s << "[" << size << "]";
              } else {
                s << "[1]"; // fallback arraySize of 1
              }
            } else {
              typeName = type;
              s << "";
            }
            if (item.hasKey("if")) {
              if (map["mappings"][(string)item["if"]]) {
                addMainVar( (GLSLVariable) { typeName, (string)item["name"], (string)item["value"] } );
              }
            } else {
              addMainVar( (GLSLVariable) { typeName, (string)item["name"], (string)item["value"] } );
            }
          }
        }
      }
    }

    std::string YamlShader::code() const {
      return source;
    }

}
