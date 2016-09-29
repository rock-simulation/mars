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
#include <osg/Geode>
#include <osg/LOD>

#include <mars/osg_material_manager/OsgMaterial.h>
#include <mars/osg_material_manager/MaterialNode.h>


namespace mars {
  namespace graphics {

    class GraphicsManager;

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
      virtual void setMaterial(const std::string &name);
      virtual void collideSphere(mars::utils::Vector pos,
                                 mars::interfaces::sReal radius);

      virtual void setPosition(const mars::utils::Vector &_pos);
      virtual void setQuaternion(const mars::utils::Quaternion &_q);
      virtual const mars::utils::Vector& getPosition()
      { return position_; }
      virtual const mars::utils::Quaternion& getQuaternion()
      { return quaternion_; }

      virtual void setScaledSize(const mars::utils::Vector &scaledSize);
      void setScale(const mars::utils::Vector &scale);

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

      osg::Group* getObject(void) const {
        return group_.get();
      }
      osg::PositionAttitudeTransform* getPosTransform(void) const {
        return posTransform_.get();
      }
      osg::MatrixTransform* getScaleTransform(void) const {
        return scaleTransform_.get();
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
      osg_material_manager::MaterialNode* getStateGroup() {return materialNode.get();}
      void addLODGeodes(std::list< osg::ref_ptr< osg::Geode > > geodes,
                        float start, float end);

      void seperateMaterial();

    protected:
      unsigned long id_;
      unsigned int nodeMask_;

      std::string stateFilename_;
      bool selected_, selectable_, showSelected;
      osg::ref_ptr<osg_material_manager::MaterialNode> materialNode;
      osg::ref_ptr<osg::Group> group_;
      std::list< osg::ref_ptr<osg::Geometry> > geometry_;
      osg::ref_ptr<osg::Geode> normal_geode;
      osg::ref_ptr<osg::LOD> lod;
      osg::ref_ptr<osg::PositionAttitudeTransform> posTransform_;
      osg::ref_ptr<osg::MatrixTransform> scaleTransform_;

      mars::utils::Vector position_, pivot_, geometrySize_, scaledSize_;
      mars::utils::Quaternion quaternion_;

      int maxNumLights;
      bool sharedStateGroup;
      bool isHidden;
      GraphicsManager *g;
      virtual std::list< osg::ref_ptr< osg::Geode > > createGeometry() = 0;
    }; // end of class DrawObject

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_DRAW_OBJECT_H */
