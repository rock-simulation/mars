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

#ifndef OSG_MATERIAL_MANAGER_GL_NORMAL_MAPPING_H
#define OSG_MATERIAL_MANAGER_GL_NORMAL_MAPPING_H

#include "shader-function.h"

#include <vector>
#include <string>

namespace osg_material_manager {

  class BumpMapFrag : public ShaderFunc {
  public:
    BumpMapFrag(std::vector<std::string> &args, std::string resPath);
    std::string code() const;
  private:
    std::string source;
  }; // end of class BumpMapFrag

  class BumpMapVert : public ShaderFunc {
  public:
    BumpMapVert(std::vector<std::string> &args, std::string resPath);
    std::string code() const;
  private:
    std::string source;
  }; // end of class BumpMapVert

} //end of namespace osg_material_manager

#endif /* OSG_MATERIAL_GL_NORMAL_MAPPING_H */

