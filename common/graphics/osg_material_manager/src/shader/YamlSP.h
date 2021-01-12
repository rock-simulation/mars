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

#ifndef MARS_DEV_YAMLSP_H
#define MARS_DEV_YAMLSP_H

#include <memory>

#include "IShaderProvider.h"
#include "configmaps/ConfigData.h"
#include "shader-function.h"
#include "ShaderFactory.h"

namespace osg_material_manager {

  using namespace std;
  using namespace configmaps;

  /**
   * Implementation (not much more than a wrapper) of the IShaderProvider for use with the old YamlShader files.
   * Intended only for demonstration and not recommended for productive use anymore.
   * (The shader functions still have to be created manually by code or YamlShader instances)
   */
  class YamlSP : public IShaderProvider {
  public:
    YamlSP(string res_path);
    virtual ~YamlSP() {}

    int getMinVersion();

    const set<GLSLUniform>& getUniforms() const;

    const set<GLSLAttribute>& getVaryings() const;

    const set<string>& getEnabledExtensions() const;

    const set<string>& getDisabledExtensions() const;

    const vector<pair<string, string> > getDependencies() const;

    const std::set<GLSLConstant>& getConstants() const;

    const std::set<GLSLAttribute>& getAttributes() const;

    string generateMainSource();

    string generateDefinitions();

    /**
     * Adds a shader function to this Provider or merges it with an existing one if present.
     * This function takes ownership of the given pointer and manages its memory.
     * @param func The function to add/merge to this provider
     */
    void addShaderFunction(ShaderFunc *func);

    /**
     * Sets up uniforms, exports, etc in code needed by the Yaml Shader Files
     * @param shader_type The Shader Type this YamlSP is the provider for
     * @param material The entire material ConfigMap
     * @param has_texture Bool determining if a texture is used in the shader or not
     * @param use_world_tex_coords Determines if to use global texture coordinates or not
     */
    void setupShaderEnv(ShaderType shader_type, ConfigMap material, bool has_texture, bool use_world_tex_coords);

  private:
    /**
     * Pointer to the shader function containing all necessary information to generate the shader code
     */
    unique_ptr<ShaderFunc> function;
  };
}


#endif //MARS_DEV_YAMLSP_H
