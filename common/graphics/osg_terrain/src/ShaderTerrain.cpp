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

/**
 * \file ShaderTerrain.cpp
 * \author Malte Langosz
 * \brief 
 **/

#include "ShaderTerrain.hpp"
#include "Terrain.h"

#include <mars/utils/misc.h>

#include <osg/ComputeBoundsVisitor>
#include <osg/PolygonMode>
#include <osg/LineWidth>
#include <osg/Geode>


namespace osg_terrain {

  bool PosTransform::computeLocalToWorldMatrix(osg::Matrix& matrix,
                                               osg::NodeVisitor* nv) const  {
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
    if (cv) {
      osg::Vec3 eyePointLocal = cv->getEyeLocal();
      if(overridePos) {
        eyePointLocal[0] = x;
        eyePointLocal[1] = y;
        eyePointLocal[2] = z;
      }
      else {
        eyePointLocal[0] = eyePointLocal[0]-fmod(eyePointLocal[0], 6.0);
        eyePointLocal[1] = eyePointLocal[1]-fmod(eyePointLocal[1], 6.0);
        eyePointLocal[2] = zOffset;
      }
      matrix.preMultTranslate(eyePointLocal);
    }
    return true;
  }

  bool PosTransform::computeWorldToLocalMatrix(osg::Matrix& matrix,
                                               osg::NodeVisitor* nv) const {
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
    if (cv) {
      osg::Vec3 eyePointLocal = cv->getEyeLocal();
      if(overridePos) {
        eyePointLocal[0] = x;
        eyePointLocal[1] = y;
        eyePointLocal[2] = z;
      }
      else {
        eyePointLocal[0] = eyePointLocal[0]-fmod(eyePointLocal[0], 6.0);
        eyePointLocal[1] = eyePointLocal[1]-fmod(eyePointLocal[1], 6.0);
        eyePointLocal[2] = zOffset;
      }
      matrix.postMultTranslate(-eyePointLocal);
    }
    return true;
  }

  ShaderTerrain::ShaderTerrain(configmaps::ConfigMap map) {
    fprintf(stderr, "create shader terrain object\n");
    posTransform = new PosTransform();
    posTransform->setCullingActive(false);
    osg::ref_ptr<osg::Node> node;
    std::string file = map["file"];
    if(map.hasKey("pos/z")) {
      posTransform->zOffset = map["pos/z"];
    }
    else {
      posTransform->zOffset = 0.;
    }
    posTransform->overridePos = false;
    if(mars::utils::getFilenameSuffix(file) == ".bobj") {
      node = readBobjFromFile(file);
    }
    else {
      node = osgDB::readNodeFile(file);
    }
    node->setNodeMask(0xff | 0x1000);

    // handle bounding box
    boundCallback = new ShaderTerrain::BoundCallback();
    osg::ComputeBoundsVisitor cbbv;
    node->accept(cbbv);
    osg::BoundingBox bb = cbbv.getBoundingBox();
    boundCallback->size[0] = fabs(bb.xMax() - bb.xMin());
    boundCallback->size[1] = fabs(bb.yMax() - bb.yMin());
    boundCallback->size[2] = map["size/z"];;
    fprintf(stderr, "size: %g %g %g %g\n", boundCallback->size[0],
            boundCallback->size[1], boundCallback->size[2], posTransform->zOffset);
    ShaderTerrain::BoundVisitor visitor;
    visitor.s = boundCallback.get();
    node->accept(visitor);

    osg::PolygonMode *polyModeObj = new osg::PolygonMode;
    polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
    bool wireframe = map["wireframe"];
    if(wireframe) {
      osg::ref_ptr<osg::LineWidth> linew = new osg::LineWidth(3.0);
      node->getOrCreateStateSet()->setAttributeAndModes(linew.get(),
                                                        osg::StateAttribute::ON);        
      node->getOrCreateStateSet()->setAttribute( polyModeObj );
    }
    posTransform->addChild(node.get());
    this->addChild(posTransform.get());
  }

  ShaderTerrain::~ShaderTerrain() {

  }

  osg::ref_ptr<osg::Node> ShaderTerrain::getNode() {
    return posTransform;
  }

  osg::BoundingBox ShaderTerrain::BoundCallback::computeBound(const osg::Drawable &) const {
    return osg::BoundingBox(-0.5*size[0], -0.5*size[1], 0, 0.5*size[0], 0.5*size[1], size[2]);
  }

  void ShaderTerrain::setPos(double x, double y, double z) {
    posTransform->x = x;
    posTransform->y = y;
    posTransform->z = z;
    posTransform->overridePos = true;;    
  }

  void ShaderTerrain::BoundVisitor::apply(osg::Node &searchNode){
    // search for geometries and generate tangents for them
    osg::Geode* geode=dynamic_cast<osg::Geode*>(&searchNode);
    if(geode) {
      for(unsigned int i=0; i<geode->getNumDrawables(); ++i) {
        osg::Geometry* geom=dynamic_cast<osg::Geometry*>(geode->getDrawable(i));
        if(geom) {
          geom->setComputeBoundingBoxCallback(s);
          geom->setUseDisplayList(false);
          geom->setUseVertexBufferObjects(true);
          geom->dirtyBound();
        }
      }
    }

    traverse(searchNode);
  }

} // end of namespace: osg_terrain
