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

      sMotor.index = sMotor_.index;
      sMotor.type = sMotor_.type;
      sMotor.value = sMotor_.value;
      sMotor.name = sMotor_.name.c_str();
      axis = 0;
      position1 = 0;
      position2 = 0;
      position = &position1;
      desired_value = sMotor.value;
      speed=0;
      joint_velocity = desired_speed = 0;
      time = 10;
      current = 0;
      effort = 0;
      myJoint = 0;

      myPlayJoint = 0;
      active = true;

      // controller
      p=0;
      i=0;
      d=0;
      last_error = 0;
      integ_error = 0;
      updateController();

      initTemperatureEstimation();
      initCurrentEstimation();

      dbPackage.add("id", (long)sMotor.index);
      dbPackage.add("value", getValue());
      dbPackage.add("position", getPosition());
      dbPackage.add("current", getCurrent());
      dbPackage.add("effort", getEffort());

      dbIdIndex = dbPackage.getIndexByName("id");
      dbValueIndex = dbPackage.getIndexByName("value");
      dbPositionIndex = dbPackage.getIndexByName("position");
      dbCurrentIndex = dbPackage.getIndexByName("current");
      dbEffortIndex = dbPackage.getIndexByName("effort");

      std::string groupName, dataName;
      getDataBrokerNames(&groupName, &dataName);
      if(control->dataBroker) {
        dbPushId = control->dataBroker->pushData(groupName, dataName,
                                                 dbPackage, NULL,
                                                 data_broker::DATA_PACKAGE_READ_FLAG);
        control->dataBroker->registerTimedProducer(this, groupName, dataName,
                                                   "mars_sim/simTimer", 0);
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

    void SimMotor::updateController() {
      axis = (unsigned char) sMotor.axis;
      switch (sMotor.type) {
        case MOTOR_TYPE_POSITION:
          controlParameter = &position1;
          controlLimit = &(sMotor.maxSpeed);
          setJointControlParameter = &SimJoint::setSpeed;
          break;
        case MOTOR_TYPE_SPEED:
          controlParameter = &speed;
          controlLimit = &(sMotor.maxAcceleration); // this is a stand-in for acceleration
          setJointControlParameter = &SimJoint::setSpeed;
          break;
        case MOTOR_TYPE_EFFORT:
          controlParameter = &effort;
          controlLimit = &(sMotor.maxEffort);
          setJointControlParameter = &SimJoint::setEffort;
          break;
        case MOTOR_TYPE_UNDEFINED:
          // TODO: output error
          controlParameter = &position1; // default to position
          controlLimit = &(sMotor.maxSpeed);
          setJointControlParameter = &SimJoint::setSpeed;
          break;
        // the following types are deprecated
        case MOTOR_TYPE_PID:
          controlParameter = &position1;
          controlLimit = &(sMotor.maxSpeed);
          setJointControlParameter = &SimJoint::setSpeed;
          break;
        case MOTOR_TYPE_DC:
          controlParameter = &speed;
          controlLimit = &(sMotor.maxAcceleration);
          setJointControlParameter = &SimJoint::setSpeed;
          break;
        case MOTOR_TYPE_PID_FORCE:
          // TODO: deprecated error
          break;
      }
    }

    void SimMotor::runController(sReal time) {
      // the following implements a simple PID controller using the value
      // pointed to by controlParameter
      if(desired_value>sMotor.maxValue)
        desired_value = sMotor.maxValue;
      if(desired_value<sMotor.minValue)
        desired_value = sMotor.minValue;

      er = desired_value - *controlParameter;
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
      if(iPart > sMotor.maxSpeed)
      {
        iPart = sMotor.maxSpeed;
        integ_error = sMotor.maxSpeed / sMotor.i;
      }

      if(iPart < -sMotor.maxSpeed)
      {
        iPart = -sMotor.maxSpeed;
        integ_error = -sMotor.maxSpeed / sMotor.i;
      }

      // set desired velocity. @todo add inertia
      vel = desired_speed;
      // P part of the motor
      vel += er * sMotor.p;
      // I part of the motor
      vel += iPart;
      // D part of the motor
      vel += ((er - last_error)/time) * sMotor.d;
      last_error = er;
    }

    void SimMotor::update(sReal time_ms) {
      sReal er = 0;
      sReal vel = 0;
      time = time_ms;// / 1000;
      sReal play_position = 0.0;

      // if the attached joint does not exist (any more)
      if (!myJoint) deactivate();

      if(active) {
        // set play offset to 0
        if(myPlayJoint) play_position = myPlayJoint->getActualAngle1();

        refreshPosition();
        position1 += play_position;

        runController(time_ms);

        // this passes speed, position or force to the attached
        // joint's setSpeed1/2 or setTorque1/2 methods
        (myJoint->*setJointControlParameter)(*controlParameter, axis);

        estimateCurrent();
        estimateTemperature(time_ms);

        if(vel > getMomentaryMaxSpeed())
          vel = getMomentaryMaxSpeed();
        else
        if(vel < -getMomentaryMaxSpeed)
          vel = -getMomentaryMaxSpeed;

        if(sMotor.axis == 1) {
          myJoint->setVelocity(vel);
        }
        else if(sMotor.axis == 2) {
          myJoint->setVelocity2(vel);
        }
      }
    }

    void SimMotor::estimateCurrent() {
      // calculate current
      effort = myJoint->getMotorTorque();
      joint_velocity = myJoint->getSpeed();
      current = (kXY*fabs(effort*joint_velocity) +
                 kX*fabs(effort) +
                 kY*fabs(joint_velocity) + k);
      if(current < 0.0)
        current = 0.0;
    }




    void SimMotor::estimateTemperature(sReal time_ms) {
      temperature = temperature - calcHeatDissipation(time_ms) + calcHeatProduction(time_ms);
    }

    /*
     * Calculate the heat energy dissipating in the update interval.
     * Normally, heat transfer would be calculated as
     * P = k*A*(Ti - Ta)/d
     * where k is the thermal conductivity, A the surface area,
     * d the distance (material thickness) and Ti the temperature inside
     * and Ta the ambient temperature. Since k, A and d are all constant
     * in our case, heatTransferCoefficient = k*A/d and thus:
     */
    sReal SimMotor::calcHeatDissipation(sReal time_ms) const {
      return (heatTransferCoefficient * (temperature - ambientTemperature))*time_ms/1000.0;
    }

    /*
     * Calculates the heat energy lost from motor activity in the update interval.
     * heatlossCoefficient would be the fraction of the energy the motor gets from the
     * power supply which gets transformed into heat energy.
     */
    sReal SimMotor::calcHeatProduction(sReal time_ms) const {
      return (current * voltage * heatlossCoefficient)*time_ms/1000.0;
    }

    void SimMotor::initTemperatureEstimation() {
      temperature = 0;
      voltage = 0;
      heatlossCoefficient = 0;
      heatTransferCoefficient = 0;
      heatCapacity = 0;
    }

    void SimMotor::initCurrentEstimation() {
      kXY = 0;
      kX = 0;
      kY = 0;
      k = 0;
      current = 0;
    }

    void SimMotor::refreshPosition(){
      if(sMotor.axis == 1)
        position1 = myJoint->getActualAngle1();
      else
        position2 = myJoint->getActualAngle2();
    }

    void SimMotor::refreshPositions() {
      position1 = myJoint->getActualAngle1();
      position2 = myJoint->getActualAngle2();
    }

    void SimMotor::refreshAngle(){ // deprecated
      refreshPosition();
    }

    /*
     * This function can be overloaded in a child class in order to
     * implement a variable effort.
     */
    sReal SimMotor::getMomentaryMaxEffort() const {
      return sMotor.maxEffort;
    }

    /*
     * This function can be overloaded in a child class in order to
     * implement a variable speed.
     */
    sReal SimMotor::getMomentaryMaxSpeed() const {
      return sMotor.maxSpeed;
    }


// from here on only getters and setters

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

    int SimMotor::getAxis() const {
      return sMotor.axis;
    }

    const std::string SimMotor::getName() const {
      return sMotor.name;
    }

    void SimMotor::setName(const std::string &newname) {
      sMotor.name = newname;
    }

    void SimMotor::setDesiredMotorAngle(sReal angle) { // deprecated
      setDesiredPosition(angle);
    }

    void SimMotor::setDesiredPosition(sReal position) {
      desired_value = position;
      sMotor.value = position;
    }

    void SimMotor::setDesiredMotorVelocity(sReal vel) { // deprecated
      setDesiredSpeed(vel);
    }

    void SimMotor::setDesiredSpeed(sReal vel) {
      desired_speed = vel;
    }

    sReal SimMotor::getDesiredPosition() const {
      return desired_value;
    }

    sReal SimMotor::getDesiredMotorAngle() const { // deprecated
      return getDesiredPosition();
    }

    void SimMotor::setMaxEffort(sReal force) {
      sMotor.maxEffort = force;
      if(sMotor.axis == 1) {
        myJoint->setForceLimit(sMotor.maxEffort);
      }
      else if(sMotor.axis == 2) {
        myJoint->setForceLimit2(sMotor.maxEffort);
      }
    }

    void SimMotor::setMotorMaxForce(sReal force) { // deprecated
      setMaxEffort(force);
    }

    sReal SimMotor::getMaxEffort() const {
      return sMotor.maxEffort;
    }

    sReal SimMotor::getMotorMaxForce() const { // deprecated
      return getMaxEffort();
    }

    sReal SimMotor::getPosition() const {
      return position1;
    }

    sReal SimMotor::getActualAngle() const { // deprecated
      return getPosition();
    }

    void SimMotor::setPosition(sReal angle) {
      position1 = angle;
    }

    void SimMotor::setActualAngle(sReal angle) { // deprecated
      setPosition(angle);
    }

    void SimMotor::setMaxSpeed(sReal value) {
      sMotor.maxSpeed = fabs(value);
    }

    void SimMotor::setMaximumVelocity(sReal value) { // deprecated
      setMaxSpeed(value);
    }

    sReal SimMotor::getMaxSpeed() const {
      return sMotor.maxSpeed;
    }

    sReal SimMotor::getMaximumVelocity() const { // deprecated
      return getMaxSpeed();
    }


    bool SimMotor::isServo() const {
      // TODO: the 2 should be replaced by the correct enum
      return (controlParameter == &position1 || controlParameter == &position2);
    }

    // TODO: where is this->type ever used? shouldn't it be sMotor.type?
    //       should use the enum MotorType defined in MARSDefs.h
    void SimMotor::setType(interfaces::MotorType mtype){
      sMotor.type = mtype;
    }

    void SimMotor::setVelocity(sReal v) {
      speed = v;
    }

    sReal SimMotor::getSpeed() const {
      return speed;
    }

    sReal SimMotor::getVelocity() const { // deprecated
      return getSpeed();
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

    void SimMotor::setSMotor(const MotorData &sMotor) {
      this->sMotor = sMotor;
      if(sMotor.type == MOTOR_TYPE_PID ||
         sMotor.type == MOTOR_TYPE_PID_FORCE) {
        desired_value = sMotor.value;
      }
      else if(sMotor.type == MOTOR_TYPE_DC) {
        speed = sMotor.value;
      }
      // we can initialize the motor here
      // but maybe we should implement a function for that later
      if(myJoint && (sMotor.type != MOTOR_TYPE_PID_FORCE)) {
        // in this first implementation we only set the first axis
        if(sMotor.axis == 1) {
          myJoint->setJointAsMotor(1);
          myJoint->setForceLimit(sMotor.maxEffort);
        }
        else if(sMotor.axis == 2) {
          // the same things for the second axis
          myJoint->setJointAsMotor(2);
          myJoint->setForceLimit2(sMotor.maxEffort);
        }
        else {
          fprintf(stderr, "ERROR: Motor %s does not have %d axes.\n", sMotor.name.c_str(), sMotor.axis);
          }
      }
    }


    const MotorData SimMotor::getSMotor(void) const {
      return sMotor;
    }

    void SimMotor::setValue(sReal value) {
      switch (sMotor.type) {
      case MOTOR_TYPE_PID:
        sMotor.value = desired_value = value;
        if(!control->sim->isSimRunning()) {
          myJoint->setOfflineValue(desired_value);
        }
        break;
      case MOTOR_TYPE_DC:
        speed = value;
        break;
      case MOTOR_TYPE_PID_FORCE:
        desired_value = value;
        break;
      case MOTOR_TYPE_UNDEFINED:
        break;
      }
    }

    sReal SimMotor::getValue(void) const {
      switch (sMotor.type) {
      case MOTOR_TYPE_PID:
        return desired_value;
        break;
      case MOTOR_TYPE_DC:
        return speed;
        break;
      case MOTOR_TYPE_PID_FORCE:
        return desired_value;
        break;
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
        obj->value = desired_value;
        break;
      case MOTOR_TYPE_DC:
        obj->value = speed;
        break;
      case MOTOR_TYPE_PID_FORCE:
        obj->value = position1;
        break;
      case MOTOR_TYPE_UNDEFINED:
        break;
      }
      //obj->pos = &pos;
      //obj->rot = &rot;
    }

    sReal SimMotor::getCurrent(void) const {
      return current;
    }

    sReal SimMotor::getEffort() const {
      return effort;
    }

    sReal SimMotor::getTorque(void) const { // deprecated
      return getEffort();
    }

    void SimMotor::deactivate(void) {
      active = false;
    }

    void SimMotor::activate(void) {
      active = true;
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

      void SimMotor::produceData(const data_broker::DataInfo &info,
                                 data_broker::DataPackage *dbPackage,
                                 int callbackParam) {
        dbPackage->set(dbIdIndex, (long)sMotor.index);
        dbPackage->set(dbValueIndex, getValue());
        dbPackage->set(dbPositionIndex, getPosition());
        dbPackage->set(dbCurrentIndex, getCurrent());
        dbPackage->set(dbEffortIndex, getTorque());
      }

      void SimMotor::receiveData(const data_broker::DataInfo& info,
                                 const data_broker::DataPackage& package,
                                 int id) {
        sReal value;
        package.get(0, &value);
        setValue(value);
      }

  } // end of namespace sim
} // end of namespace mars
