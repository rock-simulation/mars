/*
 *  Copyright 2022, DFKI GmbH Robotics Innovation Center
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
 * \file CoreODEObjects.h
 * \author Malte Roemmermann, Muhammad Haider Khan Lodhi
 * \brief A collection of OBE object classes
 *
 */

#ifndef CORE_ODE_OBJECTS_H
#define CORE_ODE_OBJECTS_H

#ifdef _PRINT_HEADER_
  #warning "CoreODEObjects.h"
#endif

#include <mars/utils/MutexLocker.h>
#include "ODEObject.h"
#include <string>

//TODO remove?
#ifndef ODE11
  #define dTriIndex int
#endif

namespace mars {
namespace sim {

    class ODEBox : public ODEObject {
    public:
      ODEBox(std::shared_ptr<interfaces::PhysicsInterface> world, interfaces::NodeData * nodeData);
      virtual ~ODEBox(void);
      //TODO createGeometry vs createCollision? nBody vs nCollision
      //     review header comment on ODEBox
      virtual bool createODEGeometry(interfaces::NodeData *node) override; 
    };

    class ODECapsule : public ODEObject {
    public:
      ODECapsule(std::shared_ptr<interfaces::PhysicsInterface> world, interfaces::NodeData * nodeData);
      virtual ~ODECapsule(void);
      virtual bool createODEGeometry(interfaces::NodeData *node) override; 
    };

    class ODECylinder : public ODEObject {
    public:
      ODECylinder(std::shared_ptr<interfaces::PhysicsInterface> world, interfaces::NodeData * nodeData);
      virtual ~ODECylinder(void);
      virtual bool createODEGeometry(interfaces::NodeData *node) override; 
    };

    class ODEHeightField : public ODEObject {
    public:
      ODEHeightField(std::shared_ptr<interfaces::PhysicsInterface> world, interfaces::NodeData * nodeData);
      virtual ~ODEHeightField(void);
      virtual bool createODEGeometry(interfaces::NodeData *node) override; 
      virtual void destroyNode(void) override;
      dReal heightCallback(int x, int y);

    protected:
      interfaces::terrainStruct *terrain;
      dReal *height_data;  
    };

    class ODEMesh : public ODEObject {
    public:
      ODEMesh(std::shared_ptr<interfaces::PhysicsInterface> world, interfaces::NodeData * nodeData);
      virtual ~ODEMesh(void);
      virtual bool createODEGeometry(interfaces::NodeData *node) override;
      virtual void destroyNode(void) override;
    protected:
      dVector3 *myVertices;
      dTriIndex *myIndices;
      dTriMeshDataID myTriMeshData; 
    };

    class ODEPlane : public ODEObject {
    public:
      ODEPlane(std::shared_ptr<interfaces::PhysicsInterface> world, interfaces::NodeData * nodeData);
      virtual ~ODEPlane(void);
      virtual bool createODEGeometry(interfaces::NodeData *node) override; 
    };

    class ODESphere : public ODEObject {
    public:
      ODESphere(std::shared_ptr<interfaces::PhysicsInterface> world, interfaces::NodeData * nodeData);
      virtual ~ODESphere(void);
      virtual bool createODEGeometry(interfaces::NodeData *node) override;
    };

} // end of namespace sim
} // end of namespace mars

#endif  // CORE_ODE_OBJECTS_H
