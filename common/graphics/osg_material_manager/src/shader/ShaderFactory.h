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

#ifndef MARS_DEV_SHADERFACTORY_H
#define MARS_DEV_SHADERFACTORY_H

#include <map>
#include <memory>
#include <osg/Program>
#include "IShaderProvider.h"

namespace osg_material_manager {

  using namespace std;

  enum ShaderType {
    SHADER_TYPE_FRAGMENT,
    SHADER_TYPE_GEOMETRY,
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_FFP
  };

  /**
   * This class is responsible for generating the osg::Program for shading operations.
   * The IShaderProvider provide the necessary information to generate a shader for each ShaderType
   */
  class ShaderFactory {
  public:
    /**
     * Sets a shader provider for given shader type
     * The function takes ownership of the provider pointer by storing it inside of an std::unique_ptr!
     * @param provider A pointer to the shader provider
     * @param shader_type The shader type this provider serves
     */
    void setShaderProvider(IShaderProvider *provider, ShaderType shader_type);

    /**
     * Generates the complete source code for a shader program of given type
     * TODO: Only generate if not generated before/no changes
     * @param shader_type The type to generate the shader for
     * @return String containing the shader source
     */
    string generateShaderSource(ShaderType shader_type);

    /**
     * Generates the entire osg::Program containing all shader types served by a provider.
     * @return The ready to use osg::Program
     */
    osg::Program* generateProgram();

  private:
    /**
     * This map contains the current provider for each shader type.
     * Since the provider pointers are stored inside of an unique_ptr the memory management is taken care of.
     */
    map<ShaderType, std::unique_ptr<IShaderProvider> > providers;
  };
}


#endif //MARS_DEV_SHADERFACTORY_H
