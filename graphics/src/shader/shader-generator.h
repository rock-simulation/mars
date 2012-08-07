/*
 *  Copyright 2011, 2012, DFKI GmbH Robotics Innovation Center
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

#ifndef MARS_GRAPHICS_SHADER_GENERATOR_H
#define MARS_GRAPHICS_SHADER_GENERATOR_H

#include "shader-function.h"

#ifdef __APPLE__
#include <AGL/agl.h>
#else
#include <GL/gl.h>
#endif
#include <osg/Shader>
#include <osg/Program>

#include <string>
#include <map>

#include <mars/interfaces/MARSDefs.h>


namespace mars {
  namespace graphics {

    class ShaderGenerator {
    public:
      ShaderGenerator() {};

      void addShaderFunction(ShaderFunc *func,
                             mars::interfaces::ShaderType shaderType);

      std::string generateSource(mars::interfaces::ShaderType shaderType);

      osg::Program* generate();

    private:
      std::map< mars::interfaces::ShaderType, ShaderFunc* > functions;
    }; // end of class ShaderGenerator

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_SHADER_GENERATOR_H */

