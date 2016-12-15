/*
 *  Copyright 2011, 2016, DFKI GmbH Robotics Innovation Center
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

#ifndef MARS_GRAPHICS_YAML_SHADER_H
#define MARS_GRAPHICS_YAML_SHADER_H

#include "shader-function.h"
#include "configmaps/ConfigData.h"

#include <vector>
#include <string>

namespace osg_material_manager {

    class YamlShader : public ShaderFunc {
    public:
        YamlShader(std::string name, std::vector<std::string> &args, configmaps::ConfigMap &map, std::string resPath);
        std::string code() const;
    private:
        std::string source;
    };
} // end of namespace osg_material_manager

#endif //MARS_GRAPHICS_YAML_SHADER_H
