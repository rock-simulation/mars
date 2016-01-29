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
#include <osg/LOD>
#include <osg/Depth>



namespace mars {
  namespace graphics {

    class GraphicsManager;
    class MarsMaterial;

    class DrawObject {
    public:
      static osg::ref_ptr<osg::Material> selectionMaterial;

      DrawObject(GraphicsManager *g);
      virtual ~DrawObject();

      void createObject(unsigned long id,
                        const mars::utils::Vector &_pivot,
                        unsigned long sharedID);

      void setStateFilename(const std::string &filename, int create);
      void exportState(void);
      void exportModel(const std::string &filename);

      // the material struct can also contain a static texture (texture file)
      virtual void setMaterial(const mars::interfaces::MaterialData &mStruct,
                               bool useFog = false, bool useNoise = false,
                               bool drawLineLaser = false,
                               bool marsShadow = false);
      void setMarsMaterial(MarsMaterial *m) {marsMaterial = m;}
      // can be used for dynamic textures
      virtual void setBlending(bool mode);
      virtual void collideSphere(mars::utils::Vector pos,
                                 mars::interfaces::sReal radius);

      void updateLights(std::vector<mars::interfaces::LightData*> &lightList);

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
      void setShowSelected(bool val);
      unsigned long getID(void) const;
      void setID(unsigned long _id);
      void showNormals(bool val);

      osg::ref_ptr<osg::PositionAttitudeTransform> getTransform() {
        return posTransform_;
      }
      osg::ref_ptr<osg::MatrixTransform> getScaleMatrix() {
        return scaleTransform_;
      }
      osg::ref_ptr<osg::Material> getMaterial();

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
      void setUseFog(bool val);
      void setUseNoise(bool val);
      void setDrawLineLaser(bool val);
      void setUseShadow(bool val);
      void setUseMARSShader(bool val) {
        fprintf(stderr, "DrawObject: setUserMARSShader no longer valid!\n");
      }

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

      void setMaxNumLights(int n) {maxNumLights = n;}
      void show();
      void hide();
      osg::Group* getStateGroup() {return stateGroup_.get();}
      void addLODGeodes(std::list< osg::ref_ptr< osg::Geode > > geodes,
                        float start, float end);

      void generateTangents(osg::ref_ptr<osg::Geometry> g);
      void seperateMaterial();
      void setTransparency(float t);

    protected:
      unsigned long id_;
      unsigned int nodeMask_;

      std::string stateFilename_;
      bool selected_, selectable_, showSelected;
      bool useFog, useNoise;
      bool getLight;
      float brightness_;
      osg::ref_ptr<osg::Uniform> lightPosUniform, lightAmbientUniform, lightDiffuseUniform;
      osg::ref_ptr<osg::Uniform> lightSpecularUniform, lightIsSpotUniform;
      osg::ref_ptr<osg::Uniform> lightSpotDirUniform, lightIsDirectionalUniform;
      osg::ref_ptr<osg::Uniform> lightConstantAttUniform, lightLinearAttUniform, lightQuadraticAttUniform;
      osg::ref_ptr<osg::Uniform> lightIsSetUniform, lightCosCutoffUniform, lightSpotExponentUniform;

      MarsMaterial* marsMaterial;

      osg::ref_ptr<osg::Group> group_, mGroup_, stateGroup_;
      std::list< osg::ref_ptr<osg::Geometry> > geometry_;
      osg::ref_ptr<osg::Geode> normal_geode;
      osg::ref_ptr<osg::LOD> lod;
      MarsMaterial *material_;
      osg::ref_ptr<osg::PositionAttitudeTransform> posTransform_;
      osg::ref_ptr<osg::MatrixTransform> scaleTransform_;
      osg::ref_ptr<osg::BlendEquation> blendEquation_;
      osg::ref_ptr<osg::Uniform> brightnessUniform;
      osg::ref_ptr<osg::Uniform> transparencyUniform;
      osg::ref_ptr<osg::Uniform> lineLaserPosUniform;
      osg::ref_ptr<osg::Uniform> lineLaserNormalUniform;
      osg::ref_ptr<osg::Uniform> lineLaserColor;
      osg::ref_ptr<osg::Uniform> lineLaserDirection;
      osg::ref_ptr<osg::Uniform> lineLaserOpeningAngle;
      osg::ref_ptr<osg::Uniform> useFogUniform, useNoiseUniform;
      osg::ref_ptr<osg::Uniform> useShadowUniform;
      osg::ref_ptr<osg::Uniform> numLightsUniform;
      osg::ref_ptr<osg::Uniform> drawLineLaserUniform;
      osg::ref_ptr<osg::StateSet> materialState;
      osg::ref_ptr<osg::Depth> depth;

      mars::utils::Vector position_, pivot_, geometrySize_, scaledSize_;
      mars::utils::Quaternion quaternion_;

      bool drawLineLaser;
      bool marsShadow;
      int maxNumLights;
      bool sharedStateGroup;
      bool tangentsGenerated;
      bool isHidden;
      utils::Vector lineLasePos, lineLaserNormal;
      GraphicsManager *g;
      virtual std::list< osg::ref_ptr< osg::Geode > > createGeometry() = 0;
    }; // end of class DrawObject

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_DRAW_OBJECT_H */
