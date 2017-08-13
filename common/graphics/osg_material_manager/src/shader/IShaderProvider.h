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

#include <vector>
#include "shader-types.h"

namespace osg_material_manager {

  using namespace std;

  class IShaderProvider {
  public:

    IShaderProvider(string res_path) {
      resPath = res_path;
    }

    virtual int getMinVersion() = 0;
    virtual vector<GLSLUniform> getUniforms() = 0;
    virtual vector<GLSLAttribute> getVaryings() = 0;
    virtual vector<string> getEnabledExtensions() = 0;
    virtual vector<string> getDisabledExtensions() = 0;
    virtual vector<string> getDependencies() = 0;
    virtual string generateMainSource() = 0;

  protected:
    string resPath;
  };
}

#endif //MARS_DEV_ISHADERPROVIDER_H
