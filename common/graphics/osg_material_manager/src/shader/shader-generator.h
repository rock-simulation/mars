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

#ifndef OSG_MATERIAL_MANAGER_SHADER_GENERATOR_H
#define OSG_MATERIAL_MANAGER_SHADER_GENERATOR_H

#include "shader-function.h"

#ifdef __APPLE__
#include <AGL/agl.h>
#else
#include <GL/gl.h>
#endif
#include <osg/Program>

#include <map>

namespace osg_material_manager {

  enum ShaderType {
    SHADER_TYPE_FRAGMENT,
    SHADER_TYPE_GEOMETRY,
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_FFP
  };
  
  class ShaderGenerator {
  public:
    ShaderGenerator() {};

    void addShaderFunction(ShaderFunc *func, ShaderType shaderType);

    std::string generateSource(ShaderType shaderType);
    osg::Program* generate();

  private:
    std::map<ShaderType, ShaderFunc* > functions;
  }; // end of class ShaderGenerator

} // end of namespace osg_material_manager

#endif /* OSG_MATERIAL_MANAGER_SHADER_GENERATOR_H */

