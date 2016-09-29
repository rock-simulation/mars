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


#include "shader-function.h"
#include <sstream>

namespace osg_material_manager {

  using namespace std;

  void ShaderFunc::merge(ShaderFunc *u) {
    minVersion = max(minVersion, u->minVersion);
    for(vector< pair< string,vector<string> > >::iterator it = u->funcs.begin();
        it != u->funcs.end(); ++it)
      funcs.push_back( *it );
    for(set<string>::iterator it = u->getEnabledExtensions().begin();
        it != u->getEnabledExtensions().end(); ++it)
      enabledExtensions.insert(*it);
    for(set<string>::iterator it = u->getDisabledExtensions().begin();
        it != u->getDisabledExtensions().end(); ++it)
      disabledExtensions.insert(*it);
    for(set<GLSLUniform>::iterator it = u->getUniforms().begin();
        it != u->getUniforms().end(); ++it)
      uniforms.insert(*it);
    for(set<GLSLConstant>::iterator it = u->getConstants().begin();
        it != u->getConstants().end(); ++it)
      constants.insert(*it);
    for(set<GLSLVarying>::iterator it = u->getVaryings().begin();
        it != u->getVaryings().end(); ++it)
      varyings.insert(*it);
    for(set<GLSLAttribute>::iterator it = u->getAttributes().begin();
        it != u->getAttributes().end(); ++it)
      attributes.insert(*it);
    std::vector< std::pair<std::string,std::string> > uDeps = u->getDeps();
    for(vector< pair<string,string> >::iterator it = uDeps.begin();
        it != uDeps.end(); ++it)
      deps[it->first] = it->second;
    for(list<GLSLVariable>::const_iterator it = u->getMainVars().begin();
        it != u->getMainVars().end(); ++it)
      mainVars.push_back(*it);
    for(vector<GLSLExport>::const_iterator it = u->getExports().begin();
        it != u->getExports().end(); ++it)
      exports.push_back(*it);
    for(set<GLSLSuffix>::iterator it = u->getSuffixes().begin();
        it != u->getSuffixes().end(); ++it)
      suffixes.insert(*it);
    shaderCode += "\n" + u->generateFunctionCode();
  }

  vector<string> ShaderFunc::generateFunctionCall() {
    vector<string> calls;
    for(vector< pair< string,vector<string> > >::iterator it = funcs.begin();
        it != funcs.end(); ++it) {
      string call = it->first + "( ";
      int numArgs = it->second.size();

      if(numArgs > 0) {
        call += it->second[0];
        for(int i=1; i<numArgs; ++i) {
          call += ", " + it->second[i];
        }
      }
      call += " );";
      calls.push_back(call);
    }
    return calls;
  }

} // end of namespace osg_material_manager
