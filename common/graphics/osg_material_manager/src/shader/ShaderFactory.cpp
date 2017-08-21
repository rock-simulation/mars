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

#include <sstream>
#include <iostream>
#include "ShaderFactory.h"

namespace osg_material_manager {
  void ShaderFactory::setShaderProvider(IShaderProvider *provider, ShaderType shader_type) {
    // TODO: How to handle overwriting of providers?
    this->providers[shader_type] = std::unique_ptr<IShaderProvider>(provider);
  }

  string ShaderFactory::generateShaderSource(ShaderType shader_type) {
    if (providers.count(shader_type) == 1) { // Provider for given shader type exists
      IShaderProvider *u = providers[shader_type].get();
      stringstream code;
      if (u->getMinVersion() != 0)
        code << "#version " << u->getMinVersion() << endl;
      for (set<string>::iterator it = u->getEnabledExtensions().begin();
           it != u->getEnabledExtensions().end(); ++it)
        code << "#extension " << *it << " : enable" << endl;
      for (set<string>::iterator it = u->getDisabledExtensions().begin();
           it != u->getDisabledExtensions().end(); ++it)
        code << "#extension " << *it << " : disable" << endl;
      code << endl;

      for (set<GLSLUniform>::iterator it = u->getUniforms().begin();
           it != u->getUniforms().end(); ++it)
        code << "uniform " << *it << ";" << endl;

      for (set<GLSLConstant>::iterator it = u->getConstants().begin();
           it != u->getConstants().end(); ++it)
        code << "const " << *it << ";" << endl;
      switch (shader_type) {
        case SHADER_TYPE_VERTEX:
          for (set<GLSLVarying>::iterator it = u->getVaryings().begin();
               it != u->getVaryings().end(); ++it)
            code << "varying " << *it << ";" << endl;
          for (set<GLSLAttribute>::iterator it = u->getAttributes().begin();
               it != u->getAttributes().end(); ++it)
            code << "attribute " << *it << ";" << endl;
          break;
        case SHADER_TYPE_FRAGMENT:
          for (set<GLSLVarying>::iterator it = u->getVaryings().begin();
               it != u->getVaryings().end(); ++it)
            code << "varying " << *it << ";" << endl;
          break;
        default:
          break;
      }
      code << endl;

      std::vector<std::pair<std::string, std::string> > uDeps = u->getDependencies();
      for (vector<pair<string, string> >::iterator it = uDeps.begin();
           it != uDeps.end(); ++it)
        code << it->second;

      code << endl;

      code << u->generateDefinitions() << endl;

      code << providers[shader_type].get()->generateMainSource();
      return code.str();
    } else {
      return ""; // TODO: How to handle missing providers? Exception, empty string?
    }
  }

  osg::Program *ShaderFactory::generateProgram() {
    osg::Program *program = new osg::Program();

    if (providers.count(SHADER_TYPE_GEOMETRY) == 1) {
      osg::Shader *shader = new osg::Shader(osg::Shader::GEOMETRY);
      program->addShader(shader);
      shader->setShaderSource(generateShaderSource(SHADER_TYPE_GEOMETRY));
    }
    if (providers.count(SHADER_TYPE_VERTEX) == 1) {
      osg::Shader *shader = new osg::Shader(osg::Shader::VERTEX);
      program->addShader(shader);
      shader->setShaderSource(generateShaderSource(SHADER_TYPE_VERTEX));
    }
    if (providers.count(SHADER_TYPE_FRAGMENT) == 1) {
      osg::Shader *shader = new osg::Shader(osg::Shader::FRAGMENT);
      program->addShader(shader);
      shader->setShaderSource(generateShaderSource(SHADER_TYPE_FRAGMENT));
    }

    return program;
  }
}