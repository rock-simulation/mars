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

  const set<GLSLUniform>& YamlSP::getUniforms() const {
    if (function.get() == nullptr) {
      return set<GLSLUniform>();
    }
    return function.get()->getUniforms();
  }

  const set<GLSLAttribute>& YamlSP::getVaryings() const {
    if (function.get() == nullptr) {
      return set<GLSLAttribute>();
    }
    return function.get()->getVaryings();
  }

  const std::set<GLSLConstant>& YamlSP::getConstants() const {
    if (function.get() == nullptr) {
      return set<GLSLConstant>();
    }
    return function.get()->getConstants();
  }

  const std::set<GLSLAttribute>& YamlSP::getAttributes() const {
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
    return function.get()->code();
  }

  const set<string>& YamlSP::getEnabledExtensions() const {
    if (function.get() == nullptr) {
      cout << "Sadly nullptr.." << endl;
      return set<string>();
    }
    cout << "Returning enabled extensions!" << endl;
    return function.get()->getEnabledExtensions();
  }

  const set<string>& YamlSP::getDisabledExtensions() const {
    if (function.get() == nullptr) {
      return set<string>();
    }
    return function.get()->getDisabledExtensions();
  }

  const vector<pair<string, string> >& YamlSP::getDependencies() const {
    return function.get()->getDeps();
  }

  void YamlSP::addShaderFunction(ShaderFunc *func) {
    if (function.get() == nullptr) {
      function = unique_ptr<ShaderFunc>(func);
    } else {
      function.get()->merge(func);
    }
  }
}