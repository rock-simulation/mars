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
 *  \file ODEJoint.h
 *  \autor Larbi Abdenebaoui
 *  \brief "ODEJoint" declares the physics for the joint using ode 
 *
 *  core "SimJoint" and the physics "ODEJoint"
 *  SimJoint ------> JointInterface(i_Joint) ------------>	ODEJoint
 *  
 */

#include <mars/utils/MutexLocker.h>

#include "ODEJoint.h"
#include "objects/ODEObject.h"

#include <cstdio>

namespace mars {
namespace sim {

    using namespace utils;
    using namespace interfaces;

    /**
     * \brief the constructor of the Joint physics 
     *   initialize the attributes of the object 
     */
    ODEJoint::ODEJoint(std::shared_ptr<PhysicsInterface> world){
      theWorld = std::static_pointer_cast<WorldPhysics>(world);
      jointId = ball_motor = 0;
      jointCFM = 0.0;
      cfm = cfm1 = cfm2 = erp1 = erp2 = 0;
      lo1 = lo2 = hi1 = hi2 = 0;
      damping = 0;
      spring = 0;
      body1 = 0;
      body2 = 0;
      joint_created = false;
    }

    /**
     * \brief Destroys the joint in the physics world.
     *
     * pre:
     *     - theWorld is the correct world object
     *
     * post:
     *     - all physical representation of the joint should be cleared
     */
    ODEJoint::~ODEJoint(void) {
      MutexLocker locker(&(theWorld->iMutex));
      if (jointId) {
        dJointDestroy(jointId);
      }
    }


    void ODEJoint::calculateCfmErp(const JointData *jointS) {
        // how to set erp and cfm
        // ERP = h kp / (h kp + kd)
        // CFM = 1 / (h kp + kd)
        damping = (dReal)jointS->damping_const_constraint_axis1;
        spring = (dReal)jointS->spring_const_constraint_axis1;
        dReal h = theWorld->getWorldStep();
        cfm = damping;
        erp1 = h*(dReal)jointS->spring_const_constraint_axis1
          +(dReal)jointS->damping_const_constraint_axis1;
        cfm1 = h*(dReal)jointS->spring_const_constraint_axis1
          +(dReal)jointS->damping_const_constraint_axis1;
        erp2 = h*(dReal)jointS->spring_const_constraint_axis2
          +(dReal)jointS->damping_const_constraint_axis2;
        cfm2 = h*(dReal)jointS->spring_const_constraint_axis2
          +(dReal)jointS->damping_const_constraint_axis2;

        lo1 = jointS->lowStopAxis1;
        hi1 = jointS->highStopAxis1;
        lo2 = jointS->lowStopAxis2;
        hi2 = jointS->highStopAxis2;
        // we don't want to run in the trap where kp and kd are set to zero
        cfm = (cfm>0)?1/cfm:0.000000000001;
        erp1 = (erp1>0)?h*(dReal)jointS->spring_const_constraint_axis1/erp1:0;
        cfm1 = (cfm1>0)?1/cfm1:0.00000000001;
        erp2 = (erp2>0)?h*(dReal)jointS->spring_const_constraint_axis2/erp2:0;
        cfm2 = (cfm2>0)?1/cfm2:0.000000000001;
    }

    bool ODEJoint::isJointCreated(){
      return joint_created;
    }

    /**
     * \brief create the joint with the informations giving from jointS 
     * 
     */
    bool ODEJoint::createJoint(interfaces::JointData *jointS, const std::shared_ptr<NodeInterface> node1,
                                   const std::shared_ptr<NodeInterface> node2) {
#ifdef _VERIFY_WORLD_
      fprintf(stderr, "joint %d  ;  %d  ;  %d  ;  %.4f, %.4f, %.4f  ;  %.4f, %.4f, %.4f\n",
              jointS->index, jointS->nodeIndex1, jointS->nodeIndex2,
              jointS->anchor.x(), jointS->anchor.y(), jointS->anchor.z(),
              jointS->axis1.x(), jointS->axis1.y(), jointS->axis1.z());
#endif
      MutexLocker locker(&(theWorld->iMutex));
      if ( theWorld && theWorld->existsWorld() ) {
        //get the bodies from the interfaces nodes
        //here we have to make some verifications
        const std::shared_ptr<ODEObject> n1 = std::dynamic_pointer_cast<ODEObject>(node1);
        const std::shared_ptr<ODEObject> n2 = std::dynamic_pointer_cast<ODEObject>(node2);
        dBodyID b1 = 0, b2 = 0;

        calculateCfmErp(jointS);
        if(jointS->config.hasKey("jointCFM")) {
          jointCFM = jointS->config["jointCFM"];
        }

        joint_type = jointS->type;
        if(n1) b1 = n1->getBody();
        if(n2) b2 = n2->getBody();
        body1 = b1;
        body2 = b2;

        bool ret;
        ret = createODEJoint(jointS, body1, body2);
        if(ret == 0) {
          // Error createing the joint         
          return 0;
        }

        // we have created a joint. To get the information
        // of the forces the joint attached to the bodies
        // we need to set a feedback pointer for the joint (ode stuff)
        dJointSetFeedback(jointId, &feedback);
        joint_created = true;
        return 1;
      }
      return 0;
    }

    bool ODEJoint::createODEJoint(interfaces::JointData *jointS, dBodyID body1, dBodyID body2){
      std::cout << "ODEJoint: using default createODEJoint func. Did you forget to override it?." << std::endl;
      LOG_WARN("ODEObject: using default createODEJoint func. Did you forget to override it?.");
      return 0;
    }

    ///get the anchor of the joint
    void ODEJoint::getAnchor(Vector* anchor) const {
      dReal pos[4] = {0,0,0,0};
      MutexLocker locker(&(theWorld->iMutex));

      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        dJointGetHingeAnchor(jointId, pos);
        break;
      case JOINT_TYPE_HINGE2:
        dJointGetHinge2Anchor(jointId, pos);    
	break;
      case JOINT_TYPE_SLIDER:
        // the slider joint has no ancher point
	break;
      case JOINT_TYPE_BALL:
        dJointGetBallAnchor(jointId, pos);
	break;
      case JOINT_TYPE_UNIVERSAL:
        dJointGetUniversalAnchor(jointId, pos);
        break;
      default:
        // no correct type is spezified, so no physically node will be created
        break;
      }
      anchor->x() = pos[0];
      anchor->y() = pos[1];
      anchor->z() = pos[2];
    }

    // the next force and velocity methods are only in a beta state
    void ODEJoint::setForceLimit(sReal max_force) {
      MutexLocker locker(&(theWorld->iMutex));

      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        dJointSetHingeParam(jointId, dParamFMax, (dReal)max_force);
        break;
      case JOINT_TYPE_HINGE2:
        dJointSetHinge2Param(jointId, dParamFMax, (dReal)max_force);
	break;
      case JOINT_TYPE_SLIDER:
        dJointSetSliderParam(jointId, dParamFMax, (dReal)max_force);  
	break;
      case JOINT_TYPE_UNIVERSAL:
        dJointSetUniversalParam(jointId, dParamFMax, (dReal)max_force);      
	break;
      }
    }

    void ODEJoint::setForceLimit2(sReal max_force) {
      MutexLocker locker(&(theWorld->iMutex));

      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        break;
      case JOINT_TYPE_HINGE2:
        dJointSetHinge2Param(jointId, dParamFMax2, (dReal)max_force);
	break;
      case JOINT_TYPE_SLIDER:
	break;
      case JOINT_TYPE_UNIVERSAL:
        dJointSetUniversalParam(jointId, dParamFMax2, (dReal)max_force);      
	break;
      }
    }

    void ODEJoint::setVelocity(sReal velocity) {
      MutexLocker locker(&(theWorld->iMutex));

      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        dJointSetHingeParam(jointId, dParamVel, (dReal)velocity);
        break;
      case JOINT_TYPE_HINGE2:
        dJointSetHinge2Param(jointId, dParamVel, (dReal)velocity);
	break;
      case JOINT_TYPE_SLIDER:
        dJointSetSliderParam(jointId, dParamVel, (dReal)velocity);  
	break;
      case JOINT_TYPE_UNIVERSAL:
        dJointSetUniversalParam(jointId, dParamVel, (dReal)velocity);      
	break;
      }
    }

    void ODEJoint::setVelocity2(sReal velocity) {
      MutexLocker locker(&(theWorld->iMutex));

      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        break;
      case JOINT_TYPE_HINGE2:
        dJointSetHinge2Param(jointId, dParamVel2, (dReal)velocity);
	break;
      case JOINT_TYPE_SLIDER:
	break;
      case JOINT_TYPE_UNIVERSAL:
        dJointSetUniversalParam(jointId, dParamVel2, (dReal)velocity);      
	break;
      }
    }

    sReal ODEJoint::getPosition(void) const {
      MutexLocker locker(&(theWorld->iMutex));

      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        return (sReal)dJointGetHingeAngle(jointId);
        break;
      case JOINT_TYPE_HINGE2:
        return (sReal)dJointGetHinge2Angle1(jointId);
	break;
      case JOINT_TYPE_SLIDER:
        return (sReal)dJointGetSliderPosition(jointId);
	break;
      case JOINT_TYPE_UNIVERSAL:
        return (sReal)dJointGetUniversalAngle1(jointId);
	break;
      }
      return 0;
    }

    sReal ODEJoint::getPosition2(void) const {
      MutexLocker locker(&(theWorld->iMutex));

      switch(joint_type) {
      case JOINT_TYPE_UNIVERSAL:
        return (sReal)dJointGetUniversalAngle2(jointId);
	    break;
      }
      return 0;
    }

    /// set the anchor i.e. the position where the joint is created of the joint 
    void ODEJoint::setAnchor(const Vector &anchor){
      MutexLocker locker(&(theWorld->iMutex));

      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        dJointSetHingeAnchor(jointId, anchor.x(), anchor.y(), anchor.z());
        //std::cout << " " << anchor.z();
        break;
      case JOINT_TYPE_HINGE2:
        dJointSetHinge2Anchor(jointId, anchor.x(), anchor.y(), anchor.z());    
        break;
      case JOINT_TYPE_SLIDER:
        // the slider joint has no ancher point
        break;
      case JOINT_TYPE_BALL:
        dJointSetBallAnchor(jointId, anchor.x(), anchor.y(), anchor.z());
        break;
      case JOINT_TYPE_UNIVERSAL:
        dJointSetUniversalAnchor(jointId, anchor.x(), anchor.y(), anchor.z());
        break;
      default:
        // no correct type is spezified, so no physically node will be created
        break;
      }
    }

    /**
     * \brief Set the Axis of the Joint to a new position
     *
     * pre:
     *
     * post:
     */
    void ODEJoint::setAxis(const Vector &axis){
      MutexLocker locker(&(theWorld->iMutex));

      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        dJointSetHingeAxis(jointId, axis.x(), axis.y(), axis.z());
        break;
      case JOINT_TYPE_HINGE2:
        dJointSetHinge2Axis1(jointId, axis.x(), axis.y(), axis.z());
        break;
      case JOINT_TYPE_SLIDER:
        dJointSetSliderAxis(jointId, axis.x(), axis.y(), axis.z());
        break;
      case JOINT_TYPE_BALL:
        // the ball joint has no axis
        break;
      case JOINT_TYPE_UNIVERSAL:
        dJointSetUniversalAxis1(jointId, axis.x(), axis.y(), axis.z());
        break;
      default:
        // no correct type is spezified, so no physically node will be created
        break;
      }
    }

    /**
     * \brief Set the Axis2 of the Joint
     *
     * pre:
     *
     * post:
     */
    void ODEJoint::setAxis2(const Vector &axis){
      MutexLocker locker(&(theWorld->iMutex));
      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        // the hinge joint has only one axis
        break;
      case JOINT_TYPE_HINGE2:
        dJointSetHinge2Axis2(jointId, axis.x(), axis.y(), axis.z());
        break;
      case JOINT_TYPE_SLIDER:
        // the slider joint has only one axis
        break;
      case JOINT_TYPE_BALL:
        // the ball joint has no axis
        break;
      case JOINT_TYPE_UNIVERSAL:
        dJointSetUniversalAxis2(jointId, axis.x(), axis.y(), axis.z());
        break;
      default:
        // no correct type is spezified, so no physically node will be created
        break;
      }
    }

    /**
     * \brief Gets the actual axis of a joint
     *
     * pre:
     *     - the joint should be created and should have a axis vector
     * 
     * post:
     *     - the given axis struct should be filled with correct values
     */
    void ODEJoint::getAxis(Vector* axis) const {
      dReal pos[4] = {0,0,0,0};
      MutexLocker locker(&(theWorld->iMutex));

      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        dJointGetHingeAxis(jointId, pos);
        break;
      case JOINT_TYPE_HINGE2:
        dJointGetHinge2Axis1(jointId, pos);    
        break;
      case JOINT_TYPE_SLIDER:
        dJointGetSliderAxis(jointId, pos);    
        break;
      case JOINT_TYPE_BALL:
        // the ball joint has no axis
        break;
      case JOINT_TYPE_UNIVERSAL:
        dJointGetUniversalAxis1(jointId, pos);
        break;
      default:
        // no correct type is spezified, so no physically node will be created
        break;
      }
      axis->x() = (sReal)pos[0];
      axis->y() = (sReal)pos[1];
      axis->z() = (sReal)pos[2];
    }

    /**
     * \brief Gets the actual second axis of a joint
     *
     * pre:
     *     - the joint should be created and should have second axis vector
     * 
     * post:
     *     - the given axis struct should be filled with correct values
     */
    void ODEJoint::getAxis2(Vector* axis) const {
      dReal pos[4] = {0,0,0,0};
      MutexLocker locker(&(theWorld->iMutex));

      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        // the hinge joint has only one axis
        break;
      case JOINT_TYPE_HINGE2:
        dJointGetHinge2Axis2(jointId, pos);    
        break;
      case JOINT_TYPE_SLIDER:
        // the slider joint has only one axis
        break;
      case JOINT_TYPE_BALL:
        // the ball joint has no axis
        break;
      case JOINT_TYPE_UNIVERSAL:
        dJointGetUniversalAxis2(jointId, pos);
        break;
      default:
        // no correct type is spezified, so no physically node will be created
        break;
      }
      axis->x() = (sReal)pos[0];
      axis->y() = (sReal)pos[1];
      axis->z() = (sReal)pos[2];
    }

    ///set the world informations
    void ODEJoint::setWorldObject(std::shared_ptr<PhysicsInterface>  world){
      theWorld = std::static_pointer_cast<WorldPhysics>(world);
    }

    void ODEJoint::setJointAsMotor(int axis) {
      MutexLocker locker(&(theWorld->iMutex));
      switch(joint_type) {
        // todo: need to handle the distinction whether to set or not to
        //       set the hi and low stop differently
      case  JOINT_TYPE_HINGE:
        //if(!lo1 && !hi)
        {
          dJointSetHingeParam(jointId, dParamLoStop, -dInfinity);
          dJointSetHingeParam(jointId, dParamHiStop, dInfinity);
        }
        break;
      case JOINT_TYPE_HINGE2:
        if(!lo1 && !hi1 && axis == 1) {
          dJointSetHinge2Param(jointId, dParamLoStop, -dInfinity);
          dJointSetHinge2Param(jointId, dParamHiStop, dInfinity);
        }
        if(!lo2 && !hi2 && axis == 2) {
          dJointSetHinge2Param(jointId, dParamLoStop2, -dInfinity);
          dJointSetHinge2Param(jointId, dParamHiStop2, dInfinity);
        }
        break;
      case JOINT_TYPE_SLIDER:
        if(!lo1 && !hi1) {
          dJointSetSliderParam(jointId, dParamLoStop, -dInfinity);
          dJointSetSliderParam(jointId, dParamHiStop, dInfinity);
        }
        break;
      case JOINT_TYPE_UNIVERSAL:
        /*
          if(!lo1 && !hi1 && axis == 1) {
          dJointSetUniversalParam(jointId, dParamLoStop, -dInfinity);
          dJointSetUniversalParam(jointId, dParamHiStop, dInfinity);
          }
          if(!lo2 && !hi2 && axis == 2) {
          dJointSetUniversalParam(jointId, dParamLoStop2, -dInfinity);
          dJointSetUniversalParam(jointId, dParamHiStop2, dInfinity);
          }
        */
        break;
      }
    }

    void ODEJoint::unsetJointAsMotor(int axis) {
      MutexLocker locker(&(theWorld->iMutex));
      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        dJointSetHingeParam(jointId, dParamLoStop, lo1);
        dJointSetHingeParam(jointId, dParamHiStop, hi1);
        break;
      case JOINT_TYPE_HINGE2:
        if(axis == 1) {
          dJointSetHinge2Param(jointId, dParamLoStop, lo1);
          dJointSetHinge2Param(jointId, dParamHiStop, hi1);
        }
        else if(axis == 2) {
          dJointSetHinge2Param(jointId, dParamLoStop2, lo2);
          dJointSetHinge2Param(jointId, dParamHiStop2, hi2);
        }
	break;
      case JOINT_TYPE_SLIDER:
        dJointSetSliderParam(jointId, dParamLoStop, lo1);
        dJointSetSliderParam(jointId, dParamHiStop, hi1);
	break;
      case JOINT_TYPE_UNIVERSAL:
        if(axis == 1) {
          dJointSetUniversalParam(jointId, dParamLoStop, lo1);
          dJointSetUniversalParam(jointId, dParamHiStop, hi1);
        }
        else if(axis == 2) {
          dJointSetUniversalParam(jointId, dParamLoStop2, lo2);
          dJointSetUniversalParam(jointId, dParamHiStop2, hi2);
        }
	break;
      }
    }

    /**
     * \brief Gets the force the joint applies to the first body
     *
     * pre:
     *     - the joint should be created
     * 
     * post:
     *
     */
    void ODEJoint::getForce1(Vector *f) const {
      f->x() = (sReal)feedback.f1[0];
      f->y() = (sReal)feedback.f1[1];
      f->z() = (sReal)feedback.f1[2];
    }

    /**
     * \brief Gets the force the joint applies to the second body
     *
     * pre:
     *     - the joint should be created
     * 
     * post:
     *
     */
    void ODEJoint::getForce2(Vector *f) const {
      f->x() = (sReal)feedback.f2[0];
      f->y() = (sReal)feedback.f2[1];
      f->z() = (sReal)feedback.f2[2];
    }

    /**
     * \brief Gets the torque the joint applies to the first body
     *
     * pre:
     *     - the joint should be created
     * 
     * post:
     *
     */
    void ODEJoint::getTorque1(Vector *t) const {
      t->x() = (sReal)feedback.t1[0];
      t->y() = (sReal)feedback.t1[1];
      t->z() = (sReal)feedback.t1[2];
    }

    /**
     * \brief Gets the torque the joint applies to the second body
     *
     * pre:
     *     - the joint should be created
     * 
     * post:
     *
     */
    void ODEJoint::getTorque2(Vector *t) const {
      t->x() = (sReal)feedback.t2[0];
      t->y() = (sReal)feedback.t2[1];
      t->z() = (sReal)feedback.t2[2];
    }

    /**
     * \brief reset the achor to the actual position.  If a Node is moved or
     * rotated by the editor, this function resets the constrains the joint
     * applies to the Nodes that are connected.
     *
     * pre:
     *     - the joint should be created
     * 
     * post:
     *
     */
    void ODEJoint::reattacheJoint(void) {
      dReal pos[4] = {0,0,0,0};
      MutexLocker locker(&(theWorld->iMutex));

      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        dJointGetHingeAnchor(jointId, pos);
        dJointSetHingeAnchor(jointId, pos[0], pos[1], pos[2]);
        break;
      case JOINT_TYPE_HINGE2:
        dJointGetHinge2Anchor(jointId, pos);    
        dJointSetHinge2Anchor(jointId, pos[0], pos[1], pos[2]);
        break;
      case JOINT_TYPE_SLIDER:
        // the slider joint has no ancher point
        break;
      case JOINT_TYPE_BALL:
        dJointGetBallAnchor(jointId, pos);
        dJointSetBallAnchor(jointId, pos[0], pos[1], pos[2]);
        break;
      case JOINT_TYPE_UNIVERSAL:
        dJointGetUniversalAnchor(jointId, pos);
        dJointSetUniversalAnchor(jointId, pos[0], pos[1], pos[2]);
        break;
      case JOINT_TYPE_FIXED:
        dJointDestroy(jointId);
        jointId = dJointCreateFixed(theWorld->getWorld(), 0);
        dJointAttach(jointId, body1, body2);
        dJointSetFixed(jointId);
        dJointSetFeedback(jointId, &feedback);
        // used for the integration study of the SpaceClimber
        //dJointSetFixedParam(jointId, dParamCFM, cfm1);//0.0002);
        break;
      default:
        // no correct type is spezified, so no physically node will be created
        break;
      }
    }

    /**
     * \brief Return the torque vector for the first axis of the joint
     *
     *     v1[0] = b1_pos[0] - anchor[0];
     *     v1[1] = b1_pos[1] - anchor[1];
     *     v1[2] = b1_pos[2] - anchor[2];
     *     radius = dSqrt(v1[0]*v1[0]+
     *                    v1[1]*v1[1]+
     *                    v1[2]*v1[2]);
     *     normal[0] =  axis[1]*v1[2] - axis[2]*v1[1];
     *     normal[1] = -axis[0]*v1[2] + axis[2]*v1[0];
     *     normal[2] =  axis[0]*v1[1] - axis[1]*v1[0];
     *     dot = (normal[0]*feedback.f1[0]+
     *            normal[1]*feedback.f1[1]+
     *            normal[2]*feedback.f1[2]);
     *     normal[0] *= dot*radius;
     *     normal[1] *= dot*radius;
     *     normal[2] *= dot*radius;
     */
    void ODEJoint::getAxisTorque(Vector *t) const {
      t->x() = axis1_torque.x();
      t->y() = axis1_torque.y();
      t->z() = axis1_torque.z();
    }

    void ODEJoint::getAxis2Torque(Vector *t) const {
      t->x() = axis2_torque.x();
      t->y() = axis2_torque.y();
      t->z() = axis2_torque.z();
    }

    /**
     * \brief we need to calculate the axis torques and joint load before
     * we can return it.
     *
     */
    void ODEJoint::update(void) {
      const dReal *b1_pos, *b2_pos;
      dReal anchor[4], axis[4], axis2[4];
      int calc1 = 0, calc2 = 0;
      dReal radius, dot, torque;
      dReal v1[3], normal[3], load[3], tmp1[3], axis_force[3];
      MutexLocker locker(&(theWorld->iMutex));

      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        dJointGetHingeAnchor(jointId, anchor);
        dJointGetHingeAxis(jointId, axis);
        calc1 = 1;
        break;
      case JOINT_TYPE_HINGE2:
        dJointGetHinge2Anchor(jointId, anchor);    
        dJointGetHinge2Axis1(jointId, axis);
        dJointGetHinge2Axis2(jointId, axis2);
        calc1 = 1;
        calc2 = 0;
	break;
      case JOINT_TYPE_SLIDER:
        dJointGetSliderAxis(jointId, axis);
        calc1 = 2;
	break;
      case JOINT_TYPE_BALL:
        // no axis
	break;
      case JOINT_TYPE_UNIVERSAL:
        dJointGetUniversalAnchor(jointId, anchor);
        dJointGetUniversalAxis1(jointId, axis);
        dJointGetUniversalAxis2(jointId, axis2);
        calc1 = 1;
        calc2 = 0;
        break;
      default:
        // no correct type is spezified, so no physically node will be created
        break;
      }
      motor_torque = feedback.lambda;
      axis1_torque.x() = axis1_torque.y() = axis1_torque.z() = 0;
      axis2_torque.x() = axis2_torque.y() = axis2_torque.z() = 0;
      joint_load.x() = joint_load.y() = joint_load.z() = 0;
      if(calc1 == 1) {
        if(body1) {
          b1_pos = dBodyGetPosition(body1);
          dOP(v1, -, b1_pos, anchor);
          //radius = dLENGTH(v1);
          dCROSS(normal, =, axis, v1);
          dot = dDOT(normal, feedback.f1);
          dOPEC(normal, *=, dot);
          dOP(load, -, feedback.f1, normal);
          dCROSS(tmp1, =, v1, normal);
          axis1_torque.x() = (sReal)tmp1[0];
          axis1_torque.y() = (sReal)tmp1[1];
          axis1_torque.z() = (sReal)tmp1[2];
          dCROSS(tmp1, =, v1, load);
          joint_load.x() = (sReal)tmp1[0];
          joint_load.y() = (sReal)tmp1[1];
          joint_load.z() = (sReal)tmp1[2];
          // now nearly the same for the torque
          dot = dDOT(axis, feedback.t1);
          dOPC(tmp1, *, axis, dot);
          dOP(load, -, feedback.t1, tmp1);
          axis1_torque.x() += (sReal)tmp1[0];
          axis1_torque.y() += (sReal)tmp1[1];
          axis1_torque.z() += (sReal)tmp1[2];
          joint_load.x() += (sReal)load[0];
          joint_load.y() += (sReal)load[1];
          joint_load.z() += (sReal)load[2];
        }
        else if(body2) {
          // now we do it correct
          // first get the position vector v1
          b2_pos = dBodyGetPosition(body2);
          dOP(v1, -, b2_pos, anchor);
      
          // then differentiate the torque from the force
          dot = dDOT(feedback.f2, v1) / dDOT(v1, v1);
          dOPC(axis_force, *, v1, dot);
      
          // the difference is the torque
          dOP(tmp1, -, feedback.f2, axis_force);

          // the torque value is given by:
          torque = dLENGTH(tmp1) / dLENGTH(v1);
          // then get the normal to v1 and the torque vector
          dCROSS(normal, =, v1, tmp1);
          // and scale the normal to represent the correct torque length
          dOPEC(normal, *=, torque / dLENGTH(normal));
          // now the normal represents the torque vector at the anchor
          // and we make a projection to the axis
          dot = dDOT(normal, axis);
          dOPC(tmp1, *, axis, dot);
          dOP(load, -, normal, tmp1);
          //dCROSS(tmp1, =, v1, normal);
          axis1_torque.x() = (sReal)tmp1[0];
          axis1_torque.y() = (sReal)tmp1[1];
          axis1_torque.z() = (sReal)tmp1[2];
          //dCROSS(tmp1, =, v1, load);
          joint_load.x() = (sReal)load[0];
          joint_load.y() = (sReal)load[1];
          joint_load.z() = (sReal)load[2];
          // now nearly the same for the torque
          dot = dDOT(feedback.t2, axis);
          dOPC(tmp1, *, axis, dot);
          dOP(load, -, feedback.t2, tmp1);
          //axis1_torque.x() += (sReal)tmp1[0];
          //axis1_torque.y() += (sReal)tmp1[1];
          //axis1_torque.z() += (sReal)tmp1[2];
          joint_load.x() += (sReal)load[0];
          joint_load.y() += (sReal)load[1];
          joint_load.z() += (sReal)load[2];
        }
      }
      else if(calc1 == 2) {
        // this is for the slider
      }
      if(calc2 == 1) {
        if(body1) {
          b1_pos = dBodyGetPosition(body1);
          dOP(v1, -, b1_pos, anchor);
          radius = dLENGTH(v1);
          dCROSS(normal, =, axis2, v1);
          dot = dDOT(normal, feedback.f1);
          dOPEC(normal, *=, dot*radius);
          axis2_torque.x() = (sReal)normal[0];
          axis2_torque.y() = (sReal)normal[1];
          axis2_torque.z() = (sReal)normal[2];
          // now nearly the same for the torque
          dot = dDOT(axis2, feedback.t1);
          axis2_torque.x() += (sReal)(axis2[0]*dot);
          axis2_torque.y() += (sReal)(axis2[1]*dot);
          axis2_torque.z() += (sReal)(axis2[2]*dot);
        }
        if(body2) {
          b2_pos = dBodyGetPosition(body2);
          dOP(v1, -, b2_pos, anchor);
          radius = dLENGTH(v1);
          dCROSS(normal, =, axis2, v1);
          dot = dDOT(normal, feedback.f2);
          dOPEC(normal, *=, dot*radius);
          axis2_torque.x() += (sReal)normal[0];
          axis2_torque.y() += (sReal)normal[1];
          axis2_torque.z() += (sReal)normal[2];
          // now nearly the same for the torque
          dot = dDOT(axis2, feedback.t2);
          axis2_torque.x() += (sReal)(axis2[0]*dot);
          axis2_torque.y() += (sReal)(axis2[1]*dot);
          axis2_torque.z() += (sReal)(axis2[2]*dot);
        }
      }
    }

    void ODEJoint::getJointLoad(Vector *t) const {
      t->x() = joint_load.x();
      t->y() = joint_load.y();
      t->z() = joint_load.z();
    }

    sReal ODEJoint::getVelocity(void) const {
      MutexLocker locker(&(theWorld->iMutex));
      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        return (sReal)dJointGetHingeAngleRate(jointId);
        break;
      case JOINT_TYPE_HINGE2:
        return (sReal)dJointGetHinge2Angle1Rate(jointId);
	break;
      case JOINT_TYPE_SLIDER:
        return (sReal)dJointGetSliderPositionRate(jointId);
	break;
      case JOINT_TYPE_BALL:
        // no axis
	break;
      case JOINT_TYPE_UNIVERSAL:
        return (sReal)dJointGetUniversalAngle1Rate(jointId);
        break;
      default:
        // no correct type is spezified, so no physically node will be created
        break;
      }
      return 0;
    }

    sReal ODEJoint::getVelocity2(void) const {
      MutexLocker locker(&(theWorld->iMutex));
      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        break;
      case JOINT_TYPE_HINGE2:
        return (sReal)dJointGetHinge2Angle2Rate(jointId);
	break;
      case JOINT_TYPE_SLIDER:
	break;
      case JOINT_TYPE_BALL:
        // no axis
	break;
      case JOINT_TYPE_UNIVERSAL:
        return (sReal)dJointGetUniversalAngle2Rate(jointId);
        break;
      default:
        // no correct type is spezified, so no physically node will be created
        break;
      }
      return 0;
    }

    void ODEJoint::setTorque(sReal torque) {
      MutexLocker locker(&(theWorld->iMutex));
      switch(joint_type) {
      case JOINT_TYPE_HINGE:
        dJointAddHingeTorque(jointId, torque);
        break;
      case JOINT_TYPE_HINGE2:
	break;
      case JOINT_TYPE_SLIDER:
        dJointAddSliderForce(jointId, torque);
	break;
      case JOINT_TYPE_BALL:
        // no axis
	break;
      case JOINT_TYPE_UNIVERSAL:

        break;
      default:
        // no correct type is spezified, so no physically node will be created
        break;
      }
    }

    void ODEJoint::setTorque2(sReal torque) {
      CPP_UNUSED(torque);
      switch(joint_type) {
      case  JOINT_TYPE_HINGE:
        break;
      case JOINT_TYPE_HINGE2:
        break;
      case JOINT_TYPE_SLIDER:
        break;
      case JOINT_TYPE_BALL:
        // no axis
        break;
      case JOINT_TYPE_UNIVERSAL:
        break;
      default:
        // no correct type is spezified, so no physically node will be created
        break;
      }
    }

    void ODEJoint::changeStepSize(const JointData &jointS) {
      MutexLocker locker(&(theWorld->iMutex));
      if(theWorld && theWorld->existsWorld()) {
        calculateCfmErp(&jointS);

        switch(jointS.type) {
        case  JOINT_TYPE_HINGE:
          if(damping>0.00000001) {
            dJointSetHingeParam(jointId, dParamFMax, damping);
            dJointSetHingeParam(jointId, dParamVel, 0);
          }
          if(spring > 0.00000001) {
            dJointSetHingeParam(jointId, dParamLoStop, lo1);
            dJointSetHingeParam(jointId, dParamHiStop, hi1);
            dJointSetHingeParam(jointId, dParamStopCFM, cfm1);
            dJointSetHingeParam(jointId, dParamStopERP, erp1);
          }
          else if(lo1 != 0) {
            dJointSetHingeParam(jointId, dParamLoStop, lo1);
            dJointSetHingeParam(jointId, dParamHiStop, hi1);    
          }
          break;
        case JOINT_TYPE_HINGE2:
          if(damping>0.00000001) {
            dJointSetHinge2Param(jointId, dParamFMax, damping);
            dJointSetHinge2Param(jointId, dParamVel, 0);
            dJointSetHinge2Param(jointId, dParamFMax2, damping);
            dJointSetHinge2Param(jointId, dParamVel2, 0);
          }
          if(spring > 0.00000001) {
            dJointSetHinge2Param(jointId, dParamLoStop, lo1);
            dJointSetHinge2Param(jointId, dParamHiStop, hi1);
            dJointSetHinge2Param(jointId, dParamLoStop2, lo2);
            dJointSetHinge2Param(jointId, dParamHiStop2, hi2);
            dJointSetHinge2Param(jointId, dParamStopCFM, cfm1);
            dJointSetHinge2Param(jointId, dParamStopERP, erp1);
            dJointSetHinge2Param(jointId, dParamStopCFM2, cfm2);
            dJointSetHinge2Param(jointId, dParamStopERP2, erp2);
            dJointSetHinge2Param(jointId, dParamCFM, cfm);
          }
          break;
        case JOINT_TYPE_SLIDER:
          if(damping > 0.00000001) {
            dJointSetSliderParam(jointId, dParamFMax, damping);
            dJointSetSliderParam(jointId, dParamVel, 0);
          }
          if(spring > 0.00000001) {
            dJointSetSliderParam(jointId, dParamLoStop, lo1);
            dJointSetSliderParam(jointId, dParamHiStop, hi1);
            dJointSetSliderParam(jointId, dParamStopCFM, cfm1);
            dJointSetSliderParam(jointId, dParamStopERP, erp1);
          }
          else if(lo1 != 0) {
            dJointSetSliderParam(jointId, dParamLoStop, lo1);
            dJointSetSliderParam(jointId, dParamHiStop, hi1);
          }
          break;
        case JOINT_TYPE_BALL:
          /*
            if(damping > 0.00000001) {
            dJointSetAMotorParam(ball_motor, dParamVel, 0);
            dJointSetAMotorParam(ball_motor, dParamVel2, 0);
            dJointSetAMotorParam(ball_motor, dParamVel3, 0);
            dJointSetAMotorParam(ball_motor, dParamFMax, damping);
            dJointSetAMotorParam(ball_motor, dParamFMax2, damping);
            dJointSetAMotorParam(ball_motor, dParamFMax3, damping);
            }
            else if(spring > 0.00000001) {
            dJointSetAMotorParam(ball_motor, dParamLoStop, lo2);
            dJointSetAMotorParam(ball_motor, dParamLoStop2, lo2);
            dJointSetAMotorParam(ball_motor, dParamLoStop3, lo2);
            dJointSetAMotorParam(ball_motor, dParamHiStop, hi1);
            dJointSetAMotorParam(ball_motor, dParamHiStop2, hi2);
            dJointSetAMotorParam(ball_motor, dParamHiStop3, hi2);
            dJointSetAMotorParam(ball_motor, dParamCFM, cfm1);
            dJointSetAMotorParam(ball_motor, dParamCFM2, cfm2);
            dJointSetAMotorParam(ball_motor, dParamCFM3, cfm2);
            dJointSetAMotorParam(ball_motor, dParamERP, erp1);
            dJointSetAMotorParam(ball_motor, dParamERP2, erp2);
            dJointSetAMotorParam(ball_motor, dParamERP3, erp2);
            }
          */
          break;
        case JOINT_TYPE_UNIVERSAL:
          if(damping>0.00000001) {
            dJointSetUniversalParam(jointId, dParamFMax, damping);
            dJointSetUniversalParam(jointId, dParamVel, 0);
            dJointSetUniversalParam(jointId, dParamFMax2, damping);
            dJointSetUniversalParam(jointId, dParamVel2, 0);
          }
          if(spring > 0.00000001) {
            dJointSetUniversalParam(jointId, dParamLoStop, lo1);
            dJointSetUniversalParam(jointId, dParamHiStop, hi1);
            dJointSetUniversalParam(jointId, dParamLoStop2, lo2);
            dJointSetUniversalParam(jointId, dParamHiStop2, hi2);
            dJointSetUniversalParam(jointId, dParamStopCFM, cfm1);
            dJointSetUniversalParam(jointId, dParamStopERP, erp1);
            dJointSetUniversalParam(jointId, dParamStopCFM2, cfm2);
            dJointSetUniversalParam(jointId, dParamStopERP2, erp2);
            dJointSetUniversalParam(jointId, dParamCFM, cfm);
          }
          break;
        default:
          // no correct type is spezified, so no physically node will be created
          break;
        }
      }
    }

    sReal ODEJoint::getMotorTorque(void) const {
      return motor_torque;
    }

    interfaces::sReal ODEJoint::getLowStop() const {
      switch(joint_type) {
      case JOINT_TYPE_HINGE:
        return dJointGetHingeParam(jointId, dParamLoStop);
      case JOINT_TYPE_HINGE2:
        return dJointGetHinge2Param(jointId, dParamLoStop);
      case JOINT_TYPE_SLIDER:
        return dJointGetSliderParam(jointId, dParamLoStop);
      default:
        // not implemented yes
        fprintf(stderr, "mars::sim::ODEJoint: lowstop for type %d not implemented yet\n", joint_type);
        return 0.;
      }
    }

    interfaces::sReal ODEJoint::getHighStop() const {
      switch(joint_type) {
      case JOINT_TYPE_HINGE:
        return dJointGetHingeParam(jointId, dParamHiStop);
      case JOINT_TYPE_HINGE2:
        return dJointGetHinge2Param(jointId, dParamHiStop);
      case JOINT_TYPE_SLIDER:
        return dJointGetSliderParam(jointId, dParamHiStop);
      default:
        // not implemented yes
        fprintf(stderr, "mars::sim::ODEJoint: highstop for type %d not implemented yet\n", joint_type);
        return 0.;
      }
    }

    interfaces::sReal ODEJoint::getLowStop2() const {
      switch(joint_type) {
      case JOINT_TYPE_HINGE2:
        return dJointGetHinge2Param(jointId, dParamLoStop2);
      default:
        // not implemented yes
        fprintf(stderr, "mars::sim::ODEJoint: lowstop for type %d not implemented yet\n", joint_type);
        return 0.;
      }
    }

    interfaces::sReal ODEJoint::getHighStop2() const {
      switch(joint_type) {
      case JOINT_TYPE_HINGE2:
        return dJointGetHinge2Param(jointId, dParamHiStop2);
      default:
        // not implemented yes
        fprintf(stderr, "mars::sim::ODEJoint: highstop for type %d not implemented yet\n", joint_type);
        return 0.;
      }
    }

    interfaces::sReal ODEJoint::getCFM() const {
      switch(joint_type) {
      case JOINT_TYPE_HINGE:
        return dJointGetHingeParam(jointId, dParamCFM);
      case JOINT_TYPE_HINGE2:
        return dJointGetHinge2Param(jointId, dParamCFM);
      case JOINT_TYPE_SLIDER:
        return dJointGetSliderParam(jointId, dParamCFM);
      default:
        // not implemented yes
        fprintf(stderr, "mars::sim::ODEJoint: cfm for type %d not implemented yet\n", joint_type);
        return 0.;
      }
    }

    void ODEJoint::setLowStop(interfaces::sReal lowStop) {
      switch(joint_type) {
      case JOINT_TYPE_HINGE:
        dJointSetHingeParam(jointId, dParamLoStop, lowStop);
        break;
      case JOINT_TYPE_HINGE2:
        dJointSetHinge2Param(jointId, dParamLoStop, lowStop);
        break;
      case JOINT_TYPE_SLIDER:
        dJointSetSliderParam(jointId, dParamLoStop, lowStop);
        break;
      default:
        // not implemented yes
        fprintf(stderr, "mars::sim::ODEJoint: highstop for type %d not implemented yet\n", joint_type);
        break;
      }
    }

    void ODEJoint::setHighStop(interfaces::sReal highStop) {
      switch(joint_type) {
      case JOINT_TYPE_HINGE:
        dJointSetHingeParam(jointId, dParamHiStop, highStop);
        break;
      case JOINT_TYPE_HINGE2:
        dJointSetHinge2Param(jointId, dParamHiStop, highStop);
        break;
      case JOINT_TYPE_SLIDER:
        dJointSetSliderParam(jointId, dParamHiStop, highStop);
        break;
      default:
        // not implemented yes
        fprintf(stderr, "mars::sim::ODEJoint: highstop for type %d not implemented yet\n", joint_type);
        break;
      }
    }

    void ODEJoint::setLowStop2(interfaces::sReal lowStop) {
      switch(joint_type) {
      case JOINT_TYPE_HINGE2:
        dJointSetHinge2Param(jointId, dParamLoStop, lowStop);
        break;
      default:
        // not implemented yes
        fprintf(stderr, "mars::sim::ODEJoint: highstop for type %d not implemented yet\n", joint_type);
        break;
      }
    }

    void ODEJoint::setHighStop2(interfaces::sReal highStop) {
      switch(joint_type) {
      case JOINT_TYPE_HINGE2:
        dJointSetHinge2Param(jointId, dParamHiStop2, highStop);
        break;
      default:
        // not implemented yes
        fprintf(stderr, "mars::sim::ODEJoint: highstop for type %d not implemented yet\n", joint_type);
        break;
      }
    }

    void ODEJoint::setCFM(interfaces::sReal cfm) {
      switch(joint_type) {
      case JOINT_TYPE_HINGE:
        dJointSetHingeParam(jointId, dParamCFM, cfm);
        break;
      case JOINT_TYPE_HINGE2:
        dJointSetHinge2Param(jointId, dParamCFM, cfm);
        break;
      case JOINT_TYPE_SLIDER:
        dJointSetSliderParam(jointId, dParamCFM, cfm);
        break;
      default:
        // not implemented yes
        fprintf(stderr, "mars::sim::ODEJoint: cfm for type %d not implemented yet\n", joint_type);
        break;
      }
    }

} // end of namespace sim
} // end of namespace mars
