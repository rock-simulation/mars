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

#include "SimMotor.h"
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/sim/SimulatorInterface.h>

#include <mars/data_broker/DataBrokerInterface.h>

#include <cstdio>
#include <cmath>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    SimMotor::SimMotor(ControlCenter *c, const MotorData &sMotor_)
      : control(c) {

      //  setSMotor(sMotor_);
      sMotor.index = sMotor_.index;
      sMotor.type = sMotor_.type;
      sMotor.value = sMotor_.value;
      sMotor.name = sMotor_.name.c_str();
      myJoint = 0;
      last_error = 0;
      integ_error = 0;
      desired_position = sMotor.value;
      actual_position = 0;
      actual_velocity=0;
      joint_velocity = desired_velocity = 0;
      time = 10;
      current = 0;
      torque = 0;
      myJoint = 0;
      myPlayJoint = 0;
      activated = true;

      kXY = 100.0*((2.01198e-005) / ((9.81*0.07)*(2*M_PI/60)));
      kX  = 0.00865849 / (9.81*0.07);
      kY  = 100.0*(0.000155975 / (2*M_PI/60));
      k   = -0.0728643;

      kXY = 100.0*((0.00002) / ((9.81*0.07)*(2*M_PI/60)));
      kX  = 0.00422 / (9.81*0.07);
      kY  = 100.0*(0.00014 / (2*M_PI/60));
      k   = 0.02;

      kXY = 100.0*((0.00002) / ((9.81*0.07)*(2*M_PI/60)));
      kX  = 0.00512 / (9.81*0.07);
      kY  = 100.0*(0.00006 / (2*M_PI/60));
      k   = 0.025;

      dbPackage.add("id", (long)sMotor.index);
      dbPackage.add("value", getValue());
      dbPackage.add("position", getActualPosition());
      dbPackage.add("current", getCurrent());
      dbPackage.add("torque", getTorque());

      dbIdIndex = dbPackage.getIndexByName("id");
      dbValueIndex = dbPackage.getIndexByName("value");
      dbPositionIndex = dbPackage.getIndexByName("position");
      dbCurrentIndex = dbPackage.getIndexByName("current");
      dbTorqueIndex = dbPackage.getIndexByName("torque");

      std::string groupName, dataName;
      getDataBrokerNames(&groupName, &dataName);
      if(control->dataBroker) {
        dbPushId = control->dataBroker->pushData(groupName, dataName,
                                                 dbPackage, NULL,
                                                 data_broker::DATA_PACKAGE_READ_FLAG);
        control->dataBroker->registerTimedProducer(this, groupName, dataName,
                                                   "mars_sim/simTimer", 0);

        /*
        mars::data_broker::DataPackage dbPackage;
        dbPackage.add("value", getValue());
        std::string name = dataName + "/value";
        control->dataBroker->pushData(groupName, name, dbPackage,
                                      this, mars::data_broker::DATA_PACKAGE_READ_WRITE_FLAG);
        
        control->dataBroker->registerSyncReceiver(this, groupName, name, 0);
        */
      }
    }

    SimMotor::~SimMotor(void) {
      std::string groupName, dataName;
      getDataBrokerNames(&groupName, &dataName);
      if(control->dataBroker) {
        control->dataBroker->unregisterTimedProducer(this, groupName, dataName,
                                                     "mars_sim/simTimer");
        control->dataBroker->unregisterSyncReceiver(this, "*", "*");
      }
      // if we have to delete something we can do it here
      if(myJoint) myJoint->unsetJointAsMotor(sMotor.axis);
    }

    void SimMotor::produceData(const data_broker::DataInfo &info,
                               data_broker::DataPackage *dbPackage,
                               int callbackParam) {
      dbPackage->set(dbIdIndex, (long)sMotor.index);
      dbPackage->set(dbValueIndex, getValue());
      dbPackage->set(dbPositionIndex, getActualPosition());
      dbPackage->set(dbCurrentIndex, getCurrent());
      dbPackage->set(dbTorqueIndex, getTorque());
    }

    void SimMotor::receiveData(const data_broker::DataInfo& info,
                               const data_broker::DataPackage& package,
                               int id) {
      sReal value;
      package.get(0, &value);
      setValue(value);
    }

    void SimMotor::attachJoint(SimJoint *joint){
      myJoint = joint;
    }

    void SimMotor::attachPlayJoint(SimJoint *joint){
      myPlayJoint = joint;
    }

    SimJoint* SimMotor::getJoint() const {
      return myJoint;
    }

    SimJoint* SimMotor::getPlayJoint() const {
      return myPlayJoint;
    }

    void SimMotor::setAxis(int tempAxis){
      axis = tempAxis;
    }

    int SimMotor::getAxis() const {
      return axis;
    }

    const std::string SimMotor::getName() const {
      return sMotor.name;
    }

    void SimMotor::setName(const std::string &newname) {
      sMotor.name = newname;
    }


    void SimMotor::setDesiredMotorAngle(sReal angle) {
      desired_position = angle;
      sMotor.value = angle;
    }

    void SimMotor::setDesiredMotorVelocity(sReal vel) {
      desired_velocity = vel;
    }

    sReal SimMotor::getDesiredMotorAngle() const {
      return desired_position;
    }

    void SimMotor::setMotorMaxForce(sReal force) {
      sMotor.motorMaxForce = force;
      if(sMotor.axis == 1) {
        myJoint->setForceLimit(sMotor.motorMaxForce);
      }
      else if(sMotor.axis == 2) {
        myJoint->setForceLimit2(sMotor.motorMaxForce);
      }
    }

    sReal SimMotor::getMotorMaxForce() const {
      return motorMaxForce;
    }

    sReal SimMotor::getActualAngle() const {
      return actualAngle1;
    }

    void SimMotor::setActualAngle(sReal angle) {
      actualAngle1 = angle;
    }

    void SimMotor::setMaximumVelocity(sReal value) {
      sMotor.maximumVelocity = fabs(value);
    }

    sReal SimMotor::getMaximumVelocity() const {
      return sMotor.maximumVelocity;
    }


    bool SimMotor::isServo() const {
      // TODO: the 2 should be replaced by the correct enum
      return (type == 2);
    }

    // TODO: where is this->type ever used? shouldn't it be sMotor.type?
    //       should use the enum MotorType defined in MARSDefs.h
    void SimMotor::setType(int Type){
      type = Type;
    }

    void SimMotor::setVelocity(sReal v) {
      actual_velocity = v;
    }

    sReal SimMotor::getVelocity() const {
      return actual_velocity;
    }

    void SimMotor::setP(sReal p) {
      sMotor.p = p;
    }

    void SimMotor::setI(sReal i) {
      sMotor.i = i;
    }

    void SimMotor::setD(sReal d) {
      sMotor.d = d;
    }

    sReal SimMotor::getP() const {
      return p;
    }

    sReal SimMotor::getI() const {
      return i;
    }

    sReal SimMotor::getD() const {
      return d;
    }

    void SimMotor::refreshAngle(){
      actualAngle1 = myJoint->getActualAngle1();
      actualAngle2 = myJoint->getActualAngle2();
    }


    void SimMotor::setSMotor(const MotorData &sMotor) {
      this->sMotor = sMotor;
      if(sMotor.type == MOTOR_TYPE_PID ||
         sMotor.type == MOTOR_TYPE_PID_FORCE) {
        desired_position = sMotor.value;
      }
      else if(sMotor.type == MOTOR_TYPE_DC) {
        actual_velocity = sMotor.value;
      }
      // we can initialize the motor here
      // but maybe we should implement a function for that later
      if(myJoint && (sMotor.type != MOTOR_TYPE_PID_FORCE)) {
        // in this first implementation we only set the first axis
        if(sMotor.axis == 1) {
          myJoint->setJointAsMotor(1);
          myJoint->setForceLimit(sMotor.motorMaxForce);
        }
        else if(sMotor.axis == 2) {
          // the same things for the second axis
          myJoint->setJointAsMotor(2);
          myJoint->setForceLimit2(sMotor.motorMaxForce);
        }
        else
          {
            fprintf(stderr,
                    "ERROR: Unknown axis number in <motor> element: %d\n",
                    sMotor.axis);
          }
      }
    }


    const MotorData SimMotor::getSMotor(void) const {
      return sMotor;
    }

    void SimMotor::update(sReal time_ms) {
      // updates the motor
      // a timing should be added
      sReal er = 0;
      sReal vel = 0;
      time = time_ms;// / 1000;
      sReal play_position = 0.0;

      if(activated) {
        if(myPlayJoint) play_position = myPlayJoint->getActualAngle1();
        if(myJoint) {
          if(sMotor.axis == 1)
            actual_position = myJoint->getActualAngle1();
          else
            actual_position = myJoint->getActualAngle2();
          actual_position += play_position;

          switch (sMotor.type) {
          case MOTOR_TYPE_PID:
          {
            if(desired_position>sMotor.max_val) 
                desired_position = sMotor.max_val;
            if(desired_position<sMotor.min_val) 
                desired_position = sMotor.min_val;
            /*
              if(desired_position>2*M_PI) desired_position=0;
              else if(desired_position>M_PI) desired_position=-2*M_PI+desired_position;
              else if(desired_position<-2*M_PI) desired_position=0;
              else if(desired_position<-M_PI) desired_position=2*M_PI+desired_position;
            */
            er = desired_position - actual_position;
            if(er > M_PI) 
                er = -2*M_PI+er;
            else 
                if(er < -M_PI) 
                    er = 2*M_PI+er;
                
            integ_error += er*time;
            
            //anti wind up, this code limits the integral error
            //part of the pid to the maximum velocity. This makes
            //the pid react way faster. This also eleminates the 
            //overshooting errors seen before in the simulation
            double iPart = integ_error * sMotor.i;
            if(iPart > sMotor.maximumVelocity)
            {
                iPart = sMotor.maximumVelocity;
                integ_error = sMotor.maximumVelocity / sMotor.i;
            }

            if(iPart < -sMotor.maximumVelocity)
            {
                iPart = -sMotor.maximumVelocity;
                integ_error = -sMotor.maximumVelocity / sMotor.i;
            }

            // set desired velocity. @todo add inertia
            vel = desired_velocity;
            // P part of the motor
            vel += er * sMotor.p;
            // I part of the motor
            vel += iPart;
            // D part of the motor
            vel += ((er - last_error)/time) * sMotor.d;
            last_error = er;
          }
            break;
          case MOTOR_TYPE_DC:
            vel = actual_velocity;
            break;
          case MOTOR_TYPE_DX117:
            vel = actual_velocity;
            break;
          case MOTOR_TYPE_PID_FORCE:
            if(desired_position>2*M_PI) desired_position=0;
            else if(desired_position>M_PI) desired_position=-2*M_PI+desired_position;
            else if(desired_position<-2*M_PI) desired_position=0;
            else if(desired_position<-M_PI) desired_position=2*M_PI+desired_position;

            er = desired_position - actual_position;
            if(er > M_PI) er = -2*M_PI+er;
            else if(er < -M_PI) er = 2*M_PI+er;
            integ_error += er*time;
            // P part of the motor
            torque = er * sMotor.p;
            // I part of the motor
            torque += integ_error * sMotor.i;
            // D part of the motor
            torque += ((er - last_error)/time) * sMotor.d;
            last_error = er;
            if(torque > sMotor.motorMaxForce) torque = sMotor.motorMaxForce;
            else if(torque < -sMotor.motorMaxForce) torque = -sMotor.motorMaxForce;
            break;
          case MOTOR_TYPE_UNDEFINED:
            break;
          }

          if(sMotor.type == MOTOR_TYPE_PID_FORCE) {
            if(sMotor.axis == 1) {
              myJoint->setTorque(torque);
            }
            else if(sMotor.axis == 2) {
              myJoint->setTorque2(torque);
            }
          }
          else {
            // calculate current
            torque = myJoint->getMotorTorque();
            joint_velocity = myJoint->getVelocity();
            current = (kXY*fabs(torque*joint_velocity) +
                       kX*fabs(torque) +
                       kY*fabs(joint_velocity) + k);
            if(current < 0.0) 
                current = 0.0;
            if(vel > sMotor.maximumVelocity) 
                vel = sMotor.maximumVelocity;
            else 
                if(vel < -sMotor.maximumVelocity) 
                    vel = -sMotor.maximumVelocity;

            if(sMotor.axis == 1) {
              myJoint->setVelocity(vel);
            }
            else if(sMotor.axis == 2) {
              myJoint->setVelocity2(vel);
            }
          }
        }
      }
    }

    void SimMotor::setValue(sReal value) {
      switch (sMotor.type) {
      case MOTOR_TYPE_PID:
        sMotor.value = desired_position = value;
        if(!control->sim->isSimRunning()) {
          myJoint->setOfflineValue(desired_position);
        }
        break;
      case MOTOR_TYPE_DC:
        actual_velocity = value;
        break;
      case MOTOR_TYPE_PID_FORCE:
        desired_position = value;
        break;
      case MOTOR_TYPE_DX117:
      case MOTOR_TYPE_UNDEFINED:
        break;
      }
    }

    sReal SimMotor::getValue(void) const {
      switch (sMotor.type) {
      case MOTOR_TYPE_PID:
        return desired_position;
        break;
      case MOTOR_TYPE_DC:
        return actual_velocity;
        break;
      case MOTOR_TYPE_PID_FORCE:
        return desired_position;
        break;
      case MOTOR_TYPE_DX117:
      case MOTOR_TYPE_UNDEFINED:
        break;
      }
      return 0.0;
    }

    void SimMotor::setPID(sReal mP, sReal mI, sReal mD) {
      switch (sMotor.type) {
      case MOTOR_TYPE_PID_FORCE:
        sMotor.p = mP;
        sMotor.i = mI;
        sMotor.d = mD;
        break;
      case MOTOR_TYPE_PID:
        sMotor.p = mP;
        sMotor.i = mI;
        sMotor.d = mD;
        break;
      case MOTOR_TYPE_DC:
        //the information are not relevant for this type
        break;
      case MOTOR_TYPE_DX117:
      case MOTOR_TYPE_UNDEFINED:
        break;
      }
    }


    unsigned long SimMotor::getIndex(void) const {
      return sMotor.index;
    }

    unsigned long SimMotor::getJointIndex(void) const {
      return sMotor.jointIndex;
    }

    void SimMotor::getCoreExchange(core_objects_exchange* obj) const {
      obj->index = sMotor.index;
      obj->name = sMotor.name;
      obj->groupID = sMotor.type;
      switch (sMotor.type) {
      case MOTOR_TYPE_PID:
        obj->value = desired_position;
        break;
      case MOTOR_TYPE_DC:
        obj->value = actual_velocity;
        break;
      case MOTOR_TYPE_PID_FORCE:
        obj->value = actual_position;
        break;
      case MOTOR_TYPE_DX117:
      case MOTOR_TYPE_UNDEFINED:
        break;
      }
      //obj->pos = &pos;
      //obj->rot = &rot;
    }

    sReal SimMotor::getActualPosition(void) const {
      return actual_position;
    }

    sReal SimMotor::getCurrent(void) const {
      return current;
    }

    sReal SimMotor::getTorque(void) const {
      return torque;
    }

    void SimMotor::deactivate(void) {
      activated = false;
    }

    void SimMotor::getDataBrokerNames(std::string *groupName,
                                      std::string *dataName) const {
      char format[] = "Motors/%05lu_%s";
      int size = snprintf(0, 0, format, sMotor.index, sMotor.name.c_str());
      char buffer[size];
      sprintf(buffer, format, sMotor.index, sMotor.name.c_str());
      *groupName = "mars_sim";
      *dataName = buffer;
    }

  } // end of namespace sim
} // end of namespace mars
