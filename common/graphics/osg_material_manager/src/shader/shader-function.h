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
    ShaderFunc(std::string name, std::vector<std::string> args, unsigned int priority=0) {
      funcs.push_back(FunctionCall(name, args, priority));
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

    void addMainVar(GLSLVariable var, int priority=0) {
      mainVars.push_back(MainVar(var.name, var.type, var.value, priority));
      addMainVarDec((GLSLAttribute) {var.type, var.name});
    }
    const std::list<MainVar>& getMainVars() const {
      return mainVars;
    }

    void addMainVarDec(GLSLAttribute att) {
      std::set<GLSLAttribute>::const_iterator it = mainVarDecs.begin();
      mainVarDecs.insert(att);
    }

    const std::set<GLSLAttribute>& getMainVarDecs() const {
      return mainVarDecs;
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

    void addSnippet(PrioritizedLine l) {
      snippets.push_back(l);
    }

    const std::vector<PrioritizedLine>& getSnippets() const {
      return snippets;
    }

    const std::vector<FunctionCall>& getFunctionCalls() const {
      return funcs;
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
    std::vector<FunctionCall> funcs;
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
    std::list<MainVar> mainVars;
    std::set<GLSLAttribute> mainVarDecs;
    std::vector<GLSLExport> exports;
    std::set<GLSLSuffix> suffixes;
    std::set<std::string> enabledExtensions;
    std::set<std::string> disabledExtensions;
    std::vector<PrioritizedLine> snippets;
    // minimum gl version
    int minVersion;

  private:
    static bool mainVarDecs_unique_pred(GLSLAttribute &first, GLSLAttribute &second) {
      return first.name == second.name;
    }

  }; // end of class ShaderFunc

} // end of namespace osg_material_manager

#endif /* OSG_MATERIAL_MANAGER_SHADER_FUNC_H */

