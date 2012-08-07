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
 *  OceanDrawObject.h
 *  General CubeDrawObject to inherit from.
 *
 *  Created by Roemmermann on 21.10.09.
 */

#ifdef __OCEAN__

#ifndef MARS_GRAPHICS_OCEAN_DRAW_OBJECT_H
#define MARS_GRAPHICS_OCEAN_DRAW_OBJECT_H

#include "DrawObject.h"
#include "SkyDome.h"

#include <mars/base/Vector.h>

#include <string>
#include <osg/TextureCubeMap>
#include <osgDB/ReadFile>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osg/Program>
#include <osgText/Text>
#include <osg/CullFace>
#include <osg/Fog>
#include <osgText/Font>
#include <osg/Switch>
#include <osg/Texture3D>
#include <osgViewer/View>
#include <osgOcean/Version>
#include <osgOcean/OceanScene>
#include <osgOcean/FFTOceanSurface>
#include <osgOcean/SiltEffect>
#include <osgOcean/ShaderManager>

namespace mars {
  namespace graphics {

    class OceanDrawObject : public DrawObject {

    public:
      enum SCENE_TYPE{ CLEAR, DUSK, CLOUDY };

      OceanDrawObject();
      virtual ~OceanDrawObject(void);

      virtual void createObject(unsigned long id_, void* data,
                                const mars::base::Vector *_pivot = 0);
      virtual void exportObject(void);
      void setViewer(osgViewer::View* view);
      void addScene(osg::Group* scene);
      osg::Group* getScene(void);

    private:
      SCENE_TYPE _sceneType;
      osg::Vec2f windDirection;
      float windSpeed;
      float depth;
      float reflectionDamping;
      float scale;
      bool  isChoppy;
      float choppyFactor;
      float crestFoamHeight;
      float waveScale;

      std::vector<std::string> _cubemapDirs;
      std::vector<osg::Vec4f>  _lightColors;
      std::vector<osg::Vec4f>  _fogColors;
      std::vector<osg::Vec3f>  _underwaterAttenuations;
      std::vector<osg::Vec4f>  _underwaterDiffuse;

      std::vector<osg::Vec3f>  _sunPositions;
      std::vector<osg::Vec4f>  _sunDiffuse;
      std::vector<osg::Vec4f>  _waterFogColors;

      osg::ref_ptr<osg::TextureCubeMap> _cubemap;
      osg::ref_ptr<osgOcean::FFTOceanSurface> _oceanSurface;
      osg::ref_ptr<osgOcean::OceanScene> _oceanScene;
      osg::ref_ptr<SkyDome> _skyDome;

      osg::ref_ptr<osg::TextureCubeMap> loadCubeMapTextures(const std::string& dir);
      osg::Vec4f intColor(unsigned int r, unsigned int g,
                          unsigned int b, unsigned int a = 255);
    }; // end of class OceanDrawObject

  } // end of namespace graphics 
} // end of namespace mars

#endif // MARS_GRAPHICS_OCEAN_DRAW_OBJECT_H

#endif // __OCEAN__
