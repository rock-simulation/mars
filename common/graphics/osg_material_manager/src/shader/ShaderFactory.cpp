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

#include "ShaderFactory.h"

namespace osg_material_manager {
  void ShaderFactory::setShaderProvider(IShaderProvider *provider, ShaderType shader_type) {
    // TODO: How to handle overwriting of providers?
    this->providers[shader_type] = std::unique_ptr<IShaderProvider>(provider);
  }

  string ShaderFactory::generateShaderSource(ShaderType shader_type) {
    if (providers.count(shader_type) == 1) { // Provider for given shader type exists
      // TODO: Generate the source code with list of uniforms, varyings, glsl version, extensions, attributes and generated main function
    }
    return "No source specified";
    // TODO: How to handle missing providers? Exception, empty string?
  }

  osg::Program* ShaderFactory::generateProgram() {
    // TODO: Generate correct program
    osg::Program *program = new osg::Program();
    return program;
  }
}