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

#ifndef MARS_DEV_ISHADERPROVIDER_H
#define MARS_DEV_ISHADERPROVIDER_H

#include <set>
#include <vector>
#include "shader-types.h"

namespace osg_material_manager {

  using namespace std;

  /**
   * Abstract class for shader providers. An implementation of this class should suffice to generate valid shader code
   * when used by the ShaderFactory.
   */
  class IShaderProvider {
  public:

    IShaderProvider(string res_path) {
      resPath = res_path;
    }

    /**
     * Returns the minimal glsl version needed for the generated shader
     */
    virtual int getMinVersion() = 0;

    /**
     * Returns the set of uniforms needed by this shader.
     */
    virtual const set<GLSLUniform>& getUniforms() const = 0;

    /**
     * Returns the set of varyings needed/created by this shader
     */
    virtual const set<GLSLAttribute>& getVaryings() const = 0;

    /**
     * Returns the set of extensions enabled by this shader
     */
    virtual const set<string>& getEnabledExtensions() const = 0;

    /**
     * Returns the set of extensions disabled by this shader
     */
    virtual const set<string>& getDisabledExtensions() const = 0;

    /**
     * Returns this shaders dependencies
     */
    virtual const vector<pair<string, string> >& getDependencies() const = 0;

    /**
     * Returns the set of constants created by this shader
     */
    virtual const std::set<GLSLConstant>& getConstants() const = 0;

    /**
     * Returns the set of attributes needed by this shader
     */
    virtual const std::set<GLSLAttribute>& getAttributes() const = 0;

    /**
     * Generates the main function of this shader
     */
    virtual string generateMainSource() = 0;

    /**
     * Generates function definitions for this shader.
     * Should also contain everything else outside of the main function that is not defining
     * uniforms, varyings, extensions, constants or attributes.
     */
    virtual string generateDefinitions() = 0;

  protected:
    /**
     * The resource path for accessing definition files
     */
    string resPath;
  };
}

#endif //MARS_DEV_ISHADERPROVIDER_H
