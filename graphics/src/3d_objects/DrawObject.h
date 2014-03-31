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
 *  DrawObject.h
 *  General DrawObject to inherit from.
 *
 *  Created by Roemmermann on 20.10.09.
 */

#ifndef MARS_GRAPHICS_DRAW_OBJECT_H
#define MARS_GRAPHICS_DRAW_OBJECT_H

#ifdef _PRINT_HEADER_
  #warning "DrawObject.h"
#endif

#include <mars/interfaces/MARSDefs.h>
#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>
#include <mars/interfaces/LightData.h>
#include <mars/interfaces/MaterialData.h>

#include <string>
#include <vector>
#include <list>
#include <map>

#include <osg/Material>
#include <osg/Group>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osg/BlendEquation>
#include <osg/Uniform>
#include <osg/Geode>
#include <osg/Texture2D>

#define COLOR_MAP_UNIT 0
#define NORMAL_MAP_UNIT 1
#define BUMP_MAP_UNIT 3
#define TANGENT_UNIT 7
#define DEFAULT_UV_UNIT 0

namespace mars {
  namespace graphics {

    class DrawObject {
    public:
      static osg::ref_ptr<osg::Material> selectionMaterial;

      DrawObject();
      virtual ~DrawObject();

      void createObject(unsigned long id,
                        const mars::utils::Vector &_pivot);

      void setStateFilename(const std::string &filename, int create);
      void exportState(void);
      void exportModel(const std::string &filename);

      // the material struct can also contain a static texture (texture file)
      virtual void setMaterial(const mars::interfaces::MaterialData &mStruct,
                               bool useFog = false, bool useNoise = false,
                               bool drawLineLaser = false);
      // can be used for dynamic textures
      virtual void setTexture(osg::Texture2D *texture);
      virtual void setNormalMap(const std::string &normalMap);
      virtual void setBumpMap(const std::string &bumpMap);
      virtual void setBlending(bool mode);
      virtual void collideSphere(mars::utils::Vector pos, 
                                 mars::interfaces::sReal radius);

      typedef std::map<mars::interfaces::ShaderType, std::string> foo;

      void updateShader(std::vector<mars::interfaces::LightData*> &lightList,
                        bool reload=false,
                        const foo &shaderSource=foo());

      virtual void setPosition(const mars::utils::Vector &_pos);
      virtual void setQuaternion(const mars::utils::Quaternion &_q);
      virtual const mars::utils::Vector& getPosition()
      { return position_; }
      virtual const mars::utils::Quaternion& getQuaternion()
      { return quaternion_; }

      virtual void setScaledSize(const mars::utils::Vector &scaledSize);
      void setScale(const mars::utils::Vector &scale);
      virtual void generateTangents();

      void removeBits(unsigned int bits);
      void setBits(unsigned int bits);
      void setNodeMask(unsigned int mask) {
        nodeMask_ = mask;    
        group_->setNodeMask(mask);
      }

      void setRenderBinNumber(int number);
      bool containsNode(osg::Node* node);

      virtual void setSelected(bool val);
      bool isSelected(void) {
        return selected_;
      }
      void setSelectable(bool val) {
        selectable_ = val;
      }

      unsigned long getID(void) const;
      void setID(unsigned long _id);
      void showNormals(bool val);

      osg::ref_ptr<osg::PositionAttitudeTransform> getTransform() {
        return posTransform_;
      }
      osg::ref_ptr<osg::MatrixTransform> getScaleMatrix() {
        return scaleTransform_;
      }
      osg::ref_ptr<osg::Material> getMaterial() {
        return material_;
      }
      osg::Group* getObject(void) const {
        return group_.get();
      }
      osg::PositionAttitudeTransform* getPosTransform(void) const {
        return posTransform_.get();
      }
      osg::MatrixTransform* getScaleTransform(void) const {
        return scaleTransform_.get();
      }
  
      void setBrightness(float val);
      void setUseMARSShader(bool val) {useMARSShader = val;}
      void setUseFog(bool val);
      void setUseNoise(bool val);

        /**
       * Sets the line laser
       * @pos: position of the laser
       * @normal: normalvector of the laser-plane
       * @color: color of the laser in RGB
       * @laser: Angle of the laser, as an direction-vector
       * @openingAngle: Opening angle of the laser; for complete laserLine, choose PI
       */
      void setExperimentalLineLaser(utils::Vector lineLasePos,
                                    utils::Vector lineLaserNormal,
                                    utils::Vector lineLaserColor,
                                    utils::Vector LaserAngle,
                                    float openingAngle);

    protected:
      unsigned long id_;
      unsigned int nodeMask_;

      std::string stateFilename_;
      bool selected_, selectable_;
      bool useFog, useNoise;
      bool getLight;
      float brightness_;
      osg::Program *lastProgram;
      osg::Uniform *normalMapUniform, *bumpMapUniform, *baseImageUniform;

      osg::ref_ptr<osg::Group> group_;
      std::list< osg::ref_ptr<osg::Geometry> > geometry_;
      osg::ref_ptr<osg::Geode> normal_geode;
      osg::ref_ptr<osg::Material> material_;
      osg::ref_ptr<osg::Texture2D> colorMap_;
      osg::ref_ptr<osg::Texture2D> normalMap_;
      osg::ref_ptr<osg::Texture2D> bumpMap_;
      osg::ref_ptr<osg::PositionAttitudeTransform> posTransform_;
      osg::ref_ptr<osg::MatrixTransform> scaleTransform_;
      osg::ref_ptr<osg::BlendEquation> blendEquation_;
      osg::ref_ptr<osg::Uniform> brightnessUniform;
      osg::ref_ptr<osg::Uniform> transparencyUniform;
      osg::ref_ptr<osg::Uniform> texScaleUniform;
      osg::ref_ptr<osg::Uniform> lineLaserPosUniform;
      osg::ref_ptr<osg::Uniform> lineLaserNormalUniform;
      osg::ref_ptr<osg::Uniform> lineLaserColor;
      osg::ref_ptr<osg::Uniform> lineLaserDirection;
      osg::ref_ptr<osg::Uniform> lineLaserOpeningAngle;
  
      mars::utils::Vector position_, pivot_, geometrySize_, scaledSize_;
      mars::utils::Quaternion quaternion_;

      bool hasShaderSources;
      bool useMARSShader;
      bool drawLineLaser;
      utils::Vector lineLasePos, lineLaserNormal;

      std::vector<mars::interfaces::LightData*> lastLights;

      virtual std::list< osg::ref_ptr< osg::Geode > > createGeometry() = 0;
    }; // end of class DrawObject

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_DRAW_OBJECT_H */
