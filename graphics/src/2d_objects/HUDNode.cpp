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

#include "HUDNode.h"

namespace mars {
  namespace graphics {

    HUDDataType::HUDDataType(osg::Geometry *g, osg::Camera *c) {
      geometry = g;
      camera = c;
    }

    HUDDataType::~HUDDataType(void) {
    }

    void HUDDataType::updateRect(void) {
      osg::Matrixd viewMatrix = camera->getInverseViewMatrix();
      osg::Vec3d offset(0, 0, 0);
      osg::ref_ptr<osg::Vec3Array> corners = new osg::Vec3Array(4);

      double left, right, bottom, top, zNear, zFar, scale = 1;
      camera->getProjectionMatrixAsFrustum(left, right, bottom, top, zNear, zFar);
      //if(zNear < 0.005) {
      scale = 0.1/zNear;
      left *= scale;
      right *= scale;
      top *= scale;
      bottom *= scale;
      zNear *= scale;
      zFar *= scale;
      //}
      double s1 = 1.0;
      (*corners)[0].set(left*s1, top*s1, -zNear*s1);
      (*corners)[1].set(left*s1, bottom*s1, -zNear*s1);
      (*corners)[2].set(right*s1, bottom*s1, -zNear*s1);
      (*corners)[3].set(right*s1, top*s1, -zNear*s1);
      for(int i = 0; i < 4; i++) {
        (*corners)[i] = (*corners)[i] * viewMatrix;
      }
      geometry->setVertexArray(corners.get());        
    }

    void HUDNodeCallback::operator()(osg::Node *node, osg::NodeVisitor *nv) {
      osg::ref_ptr<HUDDataType> HUDData =
        dynamic_cast<HUDDataType*> (node->getUserData());
      if(HUDData) {
        HUDData->updateRect();
      }
      traverse(node, nv);
    }

  } // end of namespace graphics
} // end of namespace mars
