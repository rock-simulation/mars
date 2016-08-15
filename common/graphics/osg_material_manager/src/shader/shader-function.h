/*
 *  Copyright 2016, DFKI GmbH Robotics Innovation Center
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


#ifndef OSG_MATERIAL_MANAGER_SHADER_FUNC_H
#define OSG_MATERIAL_MANAGER_SHADER_FUNC_H

#include <cstdio>
#include <set>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "shader-types.h"

namespace osg_material_manager {

  class ShaderFunc {
  public:
    ShaderFunc(std::string name, std::vector<std::string> args) {
      funcs.push_back( std::pair< std::string, std::vector<std::string> >(name,args) );
      this->name = name;
      // minimum gl version
      minVersion = 120;
      shaderCode = code();
    }

    ShaderFunc() {
      this->name = "";
      // minimum gl version
      minVersion = 120;
      shaderCode = code();
    }

    void setMinVersion(int minVersion) {
      if (minVersion>this->minVersion)
        this->minVersion = minVersion;
    }

    int getMinVersion() {
      return minVersion;
    }

    void addDependencyCode(std::string codeId, std::string code) {
      deps[codeId] = code;
    }

    std::vector< std::pair<std::string,std::string> > getDeps() const {
      std::vector< std::pair<std::string,std::string> > v;
      v.reserve(deps.size());
      v.insert(v.begin(), deps.begin(), deps.end());
      return v;
    }

    void addVarying(GLSLVarying varying) {
      varyings.insert(varying);
    }
    const std::set<GLSLVarying>& getVaryings() const {
      return varyings;
    }

    void addUniform(GLSLUniform uniform) {
      uniforms.insert(uniform);
    }
    const std::set<GLSLUniform>& getUniforms() const {
      return uniforms;
    }

    void addConstant(GLSLConstant constant) {
      constants.insert(constant);
    }
    const std::set<GLSLConstant>& getConstants() const {
      return constants;
    }

    void addAttribute(GLSLAttribute att) {
      attributes.insert(att);
    }
    const std::set<GLSLAttribute>& getAttributes() const {
      return attributes;
    }

    void enableExtension(std::string extensionName) {
      enabledExtensions.insert(extensionName);
    }
    const std::set<std::string>& getEnabledExtensions() const {
      return enabledExtensions;
    }

    void disableExtension(std::string extensionName) {
      disabledExtensions.insert(extensionName);
    }
    const std::set<std::string>& getDisabledExtensions() const {
      return disabledExtensions;
    }

    void addMainVar(GLSLVariable var, int pos = -1) {
      std::list<GLSLVariable>::iterator it = mainVars.begin();
      if(var.type != "") {
        for(; it!=mainVars.end(); ++it) {
          if(it->name == var.name) {
            it->value = var.value;
            return;
          }
        }
      }
      if(pos > -1) {
        it = mainVars.begin();
        for(int i=0; i<pos; ++i, ++it) ;
        mainVars.insert(it, var);
      }
      else {
        mainVars.push_back(var);
      }
    }
    const std::list<GLSLVariable>& getMainVars() const {
      return mainVars;
    }

    void addSuffix(GLSLSuffix suffix) {
      suffixes.insert(suffix);
    }
    const std::set<GLSLSuffix>& getSuffixes() const {
      return suffixes;
    }

    void addExport(GLSLExport e) {
      exports.push_back(e);
    }
    const std::vector<GLSLExport>& getExports() const {
      return exports;
    }

    std::string generateFunctionCode() {
      return code() + "\n" + shaderCode;
    }

    virtual std::string code() const {
      return "";
    }

    void merge(ShaderFunc *u);

    std::vector<std::string> generateFunctionCall();

  protected:
    std::vector< std::pair< std::string,std::vector<std::string> > > funcs;
    std::string shaderCode;
    std::string name;
    // user variables
    std::set<GLSLUniform> uniforms;
    //
    std::set<GLSLConstant> constants;
    // passes calculations from vertex to fragment shader
    std::set<GLSLVarying> varyings;
    // per vertex attributes
    std::set<GLSLAttribute> attributes;
    // needed functions (tuple of name and code)
    std::map<std::string,std::string> deps;
    std::list<GLSLVariable> mainVars;
    std::vector<GLSLExport> exports;
    std::set<GLSLSuffix> suffixes;
    std::set<std::string> enabledExtensions;
    std::set<std::string> disabledExtensions;
    // minimum gl version
    int minVersion;

  }; // end of class ShaderFunc

} // end of namespace osg_material_manager

#endif /* OSG_MATERIAL_MANAGER_SHADER_FUNC_H */

