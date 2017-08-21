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
#include "YamlSP.h"

namespace osg_material_manager {

  YamlSP::YamlSP(string res_path) : IShaderProvider(res_path) {
  }

  int YamlSP::getMinVersion() {
    if (function.get() == nullptr) {
      return -1; //TODO: -1 as error return value okay?
    }
    return function.get()->getMinVersion();
  }

  const set<GLSLUniform> &YamlSP::getUniforms() const {
    if (function.get() == nullptr) {
      return set<GLSLUniform>();
    }
    return function.get()->getUniforms();
  }

  const set<GLSLAttribute> &YamlSP::getVaryings() const {
    if (function.get() == nullptr) {
      return set<GLSLAttribute>();
    }
    return function.get()->getVaryings();
  }

  const std::set<GLSLConstant> &YamlSP::getConstants() const {
    if (function.get() == nullptr) {
      return set<GLSLConstant>();
    }
    return function.get()->getConstants();
  }

  const std::set<GLSLAttribute> &YamlSP::getAttributes() const {
    if (function.get() == nullptr) {
      return set<GLSLAttribute>();
    }
    return function.get()->getAttributes();
  }

  string YamlSP::generateMainSource() {
    if (function.get() == nullptr) {
      return "";
    }
    ShaderFunc *u = function.get();

    stringstream code;
    code << "void main()" << endl;
    code << "{" << endl;

    for (set<GLSLAttribute>::const_iterator it = u->getMainVarDecs().begin();
         it != u->getMainVarDecs().end(); ++it)
      code << "    " << *it << ";" << endl;

    std::priority_queue<PrioritizedLine> lines;
    for (list<MainVar>::const_iterator it = u->getMainVars().begin();
         it != u->getMainVars().end(); ++it)
      lines.push(PrioritizedLine((*it).toString(), (*it).priority, lines.size()));

    for (vector<FunctionCall>::const_iterator it = u->getFunctionCalls().begin();
         it != u->getFunctionCalls().end(); ++it)
      lines.push(PrioritizedLine((*it).toString(), (*it).priority, lines.size()));

    for (vector<PrioritizedLine>::const_iterator it = u->getSnippets().begin(); it != u->getSnippets().end(); ++it) {
      lines.push(*it);
    }

    while (!lines.empty()) {
      code << "    " << lines.top().line << ";" << " //Priority: " << lines.top().priority << endl;
      lines.pop();
    }

    for (vector<GLSLExport>::const_iterator it = u->getExports().begin();
         it != u->getExports().end(); ++it)
      code << "    " << *it << ";" << endl;
    for (set<GLSLSuffix>::iterator it = u->getSuffixes().begin();
         it != u->getSuffixes().end(); ++it)
      code << "    " << *it << ";" << endl;

    code << "}" << endl;

    return code.str();
  }

  string YamlSP::generateDefinitions() {
    if (function.get() == nullptr) {
      return "";
    }
    return function.get()->generateFunctionCode();
  }

  const set<string> &YamlSP::getEnabledExtensions() const {
    if (function.get() == nullptr) {
      cout << "Sadly nullptr.." << endl;
      return set<string>();
    }
    return function.get()->getEnabledExtensions();
  }

  const set<string> &YamlSP::getDisabledExtensions() const {
    if (function.get() == nullptr) {
      return set<string>();
    }
    return function.get()->getDisabledExtensions();
  }

  const vector<pair<string, string> > &YamlSP::getDependencies() const {
    return function.get()->getDeps();
  }

  void YamlSP::addShaderFunction(ShaderFunc *func) {
    if (function.get() == nullptr) {
      function = unique_ptr<ShaderFunc>(func);
    } else {
      function.get()->merge(func);
      delete (func);
    }
  }

  void YamlSP::setupShaderEnv(ShaderType shader_type, ConfigMap material, bool has_texture, bool use_world_tex_coords) {
    if (shader_type == SHADER_TYPE_VERTEX) {
      ShaderFunc *vertexShader = new ShaderFunc;
      if (material.hasKey("instancing")) {
        vertexShader->addUniform((GLSLUniform)
                                         {"sampler2D", "NoiseMap"});
        vertexShader->enableExtension("GL_ARB_draw_instanced");
        vertexShader->addUniform((GLSLUniform)
                                         {"float", "sin_"});
        vertexShader->addUniform((GLSLUniform)
                                         {"float", "cos_"});
        vertexShader->addMainVar((GLSLVariable)
                                         {"vec4", "offset",
                                          "vec4(10.0*rnd(float(gl_InstanceIDARB)*0.1, float(gl_InstanceIDARB)*0.2), 10.0*rnd(float(gl_InstanceIDARB)*0.2, float(gl_InstanceIDARB)*0.3), rnd(float(gl_InstanceIDARB)*0.1, float(gl_InstanceIDARB)*0.9), rnd(float(gl_InstanceIDARB)*0.7, float(gl_InstanceIDARB)*0.7))"},
                                 -1);
        vertexShader->addMainVar((GLSLVariable)
                                         {"vec4", "fPos",
                                          "vec4(gl_Vertex.xy + offset.xy, gl_Vertex.z/**(0.5+offset.z)*/, gl_Vertex.w)"},
                                 -1);
        vertexShader->addMainVar((GLSLVariable)
                                         {"vec3", "sc",
                                          "vec3(normalize(vec2(fPos.x*0.1+0.5*rnd(float(gl_InstanceIDARB)*0.16, float(gl_InstanceIDARB)*0.8), fPos.y*0.1+0.5*rnd(float(gl_InstanceIDARB)*0.8, float(gl_InstanceIDARB)*0.16))), rnd(float(gl_InstanceIDARB)*0.4, float(gl_InstanceIDARB)*0.4))-0.5"},
                                 -1);
        vertexShader->addMainVar((GLSLVariable)
                                         {"vec4", "vWorldPos",
                                          "fPos + vec4(0.1*sc.z*(sin_*gl_Vertex.z*sc.x + cos_*gl_Vertex.z*sc.y), 0.1*sc.z*(sin_*gl_Vertex.z*sc.y + cos_*gl_Vertex.z*sc.x), 0, 0)"},
                                 -1);
        vertexShader->addMainVar((GLSLVariable)
                                         {"vec4", "vModelPos", "vWorldPos"}, -1);
        vertexShader->addMainVar((GLSLVariable)
                                         {"vec4", "vViewPos", "gl_ModelViewMatrix * vModelPos "}, -1);
        vertexShader->addExport((GLSLExport)
                                        {"gl_TexCoord[2].xy", "gl_TexCoord[2].xy + vec2(-offset.y, offset.x)"});
        vertexShader->addExport((GLSLExport)
                                        {"diffuse[0]", "vec4(0.5)+diffuse[0] * (1+offset.x)"});
        vertexShader->addMainVar((GLSLVariable)
                                         {"vec4", "specularCol", "gl_FrontMaterial.specular*(0.5+offset.w)"}, -1);
      } else {
        vertexShader->addMainVar((GLSLVariable)
                                         {"vec4", "vModelPos", "gl_Vertex"}, -120);
        vertexShader->addMainVar((GLSLVariable)
                                         {"vec4", "vViewPos", "gl_ModelViewMatrix * vModelPos "}, -110);
        vertexShader->addMainVar((GLSLVariable)
                                         {"vec4", "vWorldPos", "osg_ViewMatrixInverse * vViewPos "}, -100);
        vertexShader->addMainVar((GLSLVariable)
                                         {"vec4", "specularCol", "gl_FrontMaterial.specular"}, -90);
      }
      vertexShader->addExport((GLSLExport)
                                      {"gl_Position", "gl_ModelViewProjectionMatrix * vModelPos"});
      vertexShader->addExport((GLSLExport) {"gl_ClipVertex", "vViewPos"});
      if (has_texture) {
        vertexShader->addExport((GLSLExport) {"gl_TexCoord[0].xy", "gl_MultiTexCoord0.xy"});
      }
      addShaderFunction(vertexShader);
    } else if (shader_type == SHADER_TYPE_FRAGMENT) {
      ShaderFunc *fragmentShader = new ShaderFunc;
      if (has_texture) {
        fragmentShader->addUniform((GLSLUniform) {"float", "texScale"});
        if (use_world_tex_coords) {
          fragmentShader->addMainVar((GLSLVariable) {"vec2", "texCoord", "positionVarying.xy*texScale+vec2(0.5, 0.5)"},
                                     -200);
        } else {
          fragmentShader->addMainVar((GLSLVariable) {"vec2", "texCoord", "gl_TexCoord[0].xy*texScale"}, -200);
        }
      }
      if (material["insetancing"]) {
        fragmentShader->addMainVar(
                (GLSLVariable) {"vec4", "col", "vec4(1.0, 1.0, 1.0, texture2D(normalMap, texCoord).a)"}, -200);
      } else {
        fragmentShader->addMainVar((GLSLVariable) {"vec4", "col", "vec4(1.0)"}, -200);
      }
      fragmentShader->addExport((GLSLExport) {"gl_FragColor", "col"});
      addShaderFunction(fragmentShader);
    }
  }
}