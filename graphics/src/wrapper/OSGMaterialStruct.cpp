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

/*
 * OSGMaterialStruct.cpp
 *
 *  Created on: 19.04.2011
 *      Author: daniel
 */

#include "OSGMaterialStruct.h"

#include "gui_helper_functions.h"

namespace mars {
  namespace graphics {

    OSGMaterialStruct::OSGMaterialStruct(const mars::interfaces::MaterialData &mat)
    {
      setColorMode(osg::Material::OFF);
      setAmbient(osg::Material::FRONT, toOSGVec4(mat.ambientFront));
      setAmbient(osg::Material::BACK, toOSGVec4(mat.ambientBack));
      setSpecular(osg::Material::FRONT, toOSGVec4(mat.specularFront));
      setSpecular(osg::Material::BACK, toOSGVec4(mat.specularBack));
      setDiffuse(osg::Material::FRONT, toOSGVec4(mat.diffuseFront));
      setDiffuse(osg::Material::BACK, toOSGVec4(mat.diffuseBack));
      setEmission(osg::Material::FRONT, toOSGVec4(mat.emissionFront));
      setEmission(osg::Material::BACK, toOSGVec4(mat.emissionBack));
      setShininess(osg::Material::FRONT_AND_BACK, mat.shininess);
      setTransparency(osg::Material::FRONT_AND_BACK, mat.transparency);
    }

  } // end of namespace graphics
} // end of namespace mars
