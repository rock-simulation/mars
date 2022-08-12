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

#include "ODEHeightField.h"
#include <mars/interfaces/terrainStruct.h>

namespace mars {
namespace sim {

  using namespace utils;
  using namespace interfaces;

  ODEHeightField::ODEHeightField(std::shared_ptr<PhysicsInterface> world, NodeData * nodeData) : ODEObject(world, nodeData) {
    // At this moment we have not much things to do here. ^_^
    std::cout << "DEBUGGG: in ODEHeightField Constructor " << __FILE__ << ":" << __LINE__ << std::endl;
    height_data = 0;
    createNode(nodeData); // pass a function pointer?
  }

  ODEHeightField::~ODEHeightField(void) {
    std::cout << "DEBUGGG: in ODEHeightField Destructor " << __FILE__ << ":" << __LINE__ << std::endl;
    if(height_data) free(height_data);
  }

  dReal heightfield_callback(void* pUserData, int x, int z ) {
    return ((ODEHeightField*)pUserData)->heightCallback(x, z);
  }

  void ODEHeightField::destroyNode(void) {
    ODEObject::destroyNode();
    height_data = 0;
  }

  dReal ODEHeightField::heightCallback(int x, int y) {

    return (dReal)height_data[(y*terrain->width)+x]*terrain->scale;
  }

  bool ODEHeightField::createODEGeometry(interfaces::NodeData *node){
    dMatrix3 R;
    unsigned long size;
    int x, y;
    terrain = node->terrain;
    size = terrain->width*terrain->height;
    if(!height_data) height_data = (dReal*)calloc(size, sizeof(dReal));
    for(x=0; x<terrain->height; x++) {
      for(y=0; y<terrain->width; y++) {
        height_data[(terrain->height-(x+1))*terrain->width+y] = (dReal)terrain->pixelData[x*terrain->width+y];
      }
    }
    // build the ode representation
    dHeightfieldDataID heightid = dGeomHeightfieldDataCreate();

    // Create an finite heightfield.
    dGeomHeightfieldDataBuildCallback(heightid, this, heightfield_callback,
                                      terrain->targetWidth,
                                      terrain->targetHeight,
                                      terrain->width, terrain->height,
                                      REAL(1.0), REAL( 0.0 ),
                                      REAL(1.0), 0);
    // Give some very bounds which, while conservative,
    // makes AABB computation more accurate than +/-INF.
    dGeomHeightfieldDataSetBounds(heightid, REAL(-terrain->scale*2.0),
                                  REAL(terrain->scale*2.0));
    //dGeomHeightfieldDataSetBounds(heightid, -terrain->scale, terrain->scale);
    nGeom = dCreateHeightfield(theWorld->getSpace(), heightid, 1);
    dRSetIdentity(R);
    dRFromAxisAndAngle(R, 1, 0, 0, M_PI/2);
    dGeomSetRotation(nGeom, R);
    std::cout << "Created ODEHeightField!" << std::endl;
    return true;
  }
} // end of namespace sim
} // end of namespace mars
