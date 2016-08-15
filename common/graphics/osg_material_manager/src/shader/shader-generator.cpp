/*
 *  Copyright 2011, 2012, 2016, DFKI GmbH Robotics Innovation Center
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

#include "shader-generator.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace osg_material_manager {

  using namespace std;

  void ShaderGenerator::addShaderFunction(ShaderFunc *func, ShaderType shaderType) {
    map<ShaderType,ShaderFunc*>::iterator it = functions.find(shaderType);
    if(it == functions.end()) {
      functions[shaderType] = func;
    } else {
      it->second->merge( func );
    }
  }

  string ShaderGenerator::generateSource(ShaderType shaderType) {
    map<ShaderType,ShaderFunc*>::iterator it = functions.find(shaderType);
    if(it == functions.end()) {
      return "";
    }
    ShaderFunc *u = it->second;

    stringstream code;
    if(u->getMinVersion() != 0)
      code << "#version " << u->getMinVersion() << endl;
    for(set<string>::iterator it = u->getEnabledExtensions().begin();
        it != u->getEnabledExtensions().end(); ++it)
      code << "#extension " << *it << " : enable" << endl;
    for(set<string>::iterator it = u->getDisabledExtensions().begin();
        it != u->getDisabledExtensions().end(); ++it)
      code << "#extension " << *it << " : disable" << endl;
    code << endl;

    for(set<GLSLUniform>::iterator it = u->getUniforms().begin();
        it != u->getUniforms().end(); ++it)
      code << "uniform " << *it << ";" << endl;

    for(set<GLSLConstant>::iterator it = u->getConstants().begin();
        it != u->getConstants().end(); ++it)
      code << "const " << *it << ";" << endl;

    switch(shaderType)
      {
      case SHADER_TYPE_VERTEX:
        for(set<GLSLVarying>::iterator it = u->getVaryings().begin();
            it != u->getVaryings().end(); ++it)
          code << "varying " << *it << ";" << endl;
        for(set<GLSLAttribute>::iterator it = u->getAttributes().begin();
            it != u->getAttributes().end(); ++it)
          code << "attribute " << *it << ";" << endl;
        break;
      case SHADER_TYPE_FRAGMENT:
        for(set<GLSLVarying>::iterator it = u->getVaryings().begin();
            it != u->getVaryings().end(); ++it)
          code << "varying " << *it << ";" << endl;
        break;
      default: break;
      }
    code << endl;


    std::vector< std::pair<std::string,std::string> > uDeps = u->getDeps();
    for(vector< pair<string,string> >::iterator it = uDeps.begin();
        it != uDeps.end(); ++it)
      code << it->second;

    code << u->generateFunctionCode() << endl;
    code << "void main()" << endl;
    code << "{" << endl;

    for(list<GLSLVariable>::const_iterator it = u->getMainVars().begin();
        it != u->getMainVars().end(); ++it)
      code << "    " << *it << ";" << endl;

    vector<string> calls = u->generateFunctionCall();
    for(vector<string>::iterator it = calls.begin(); it != calls.end(); ++it)
      code << "    " << *it << endl;

    for(vector<GLSLExport>::const_iterator it = u->getExports().begin();
        it != u->getExports().end(); ++it)
      code << "    " << *it << ";" << endl;
    for(set<GLSLSuffix>::iterator it = u->getSuffixes().begin();
        it != u->getSuffixes().end(); ++it)
      code << "    " << *it << ";" << endl;

    code << "}" << endl;

    return code.str();
  }

  static void printSource(const string &source) {
    std::string line;
    std::istringstream stream(source);
    int i = 0;
    while (stream.good()) {
      std::getline(stream, line);
      std::cerr << i++ << line << std::endl;
    }
  }

  osg::Program* ShaderGenerator::generate() {
    osg::Program *prog = new osg::Program();

    if(functions.find(SHADER_TYPE_GEOMETRY) != functions.end()) {
      osg::Shader *shader = new osg::Shader(osg::Shader::GEOMETRY);
      prog->addShader(shader);
      shader->setShaderSource( generateSource(SHADER_TYPE_GEOMETRY) );
    }
    if(functions.find(SHADER_TYPE_VERTEX) != functions.end()) {
      osg::Shader *shader = new osg::Shader(osg::Shader::VERTEX);
      prog->addShader(shader);
      shader->setShaderSource( generateSource(SHADER_TYPE_VERTEX) );
      //printSource( shader->getShaderSource() );
    }
    if(functions.find(SHADER_TYPE_FRAGMENT) != functions.end()) {
      osg::Shader *shader = new osg::Shader(osg::Shader::FRAGMENT);
      prog->addShader(shader);
      shader->setShaderSource( generateSource(SHADER_TYPE_FRAGMENT) );
      //printSource( shader->getShaderSource() );
    }

    return prog;
  }

} // end of namespace osg_material_manager
