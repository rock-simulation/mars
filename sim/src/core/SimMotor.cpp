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
#include <algorithm>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    SimMotor::SimMotor(ControlCenter *c, const MotorData &sMotor_)
      : control(c) {

      sMotor.index = sMotor_.index;
      sMotor.type = sMotor_.type;
      sMotor.value = sMotor_.value;
      sMotor.value = 0;
      sMotor.name = sMotor_.name.c_str();
      sMotor.maxSpeed = sMotor_.maxSpeed;
      sMotor.maxEffort = sMotor_.maxEffort;
      sMotor.maxAcceleration = sMotor_.maxAcceleration;
      sMotor.maxValue = sMotor_.maxValue;
      sMotor.minValue = sMotor_.minValue;
      axis = 0;
      position1 = 0;
      position2 = 0;
      position = &position1;
      velocity=0;
      joint_velocity = 0;
      time = 10;
      current = 0;
      effort = 0;
      tmpmaxeffort = 0;
      tmpmaxspeed = 0;
      myJoint = 0;
      mimic = false;
      mimic_multiplier=1.0;
      mimic_offset=0;
      maxEffortApproximation = &utils::pipe;
      maxSpeedApproximation = &utils::pipe;
      maxeffort_coefficients = NULL;
      maxspeed_coefficients = NULL;
      maxspeed_x = &sMotor.maxSpeed;
      maxeffort_x = &sMotor.maxEffort;

      myPlayJoint = 0;
      active = true;

      // controller
      p=0;
      i=0;
      d=0;
      last_error = 0;
      integ_error = 0;
      controlValue = 0;
      updateController();

      initTemperatureEstimation();
      initCurrentEstimation();

      dbPackage.add("id", (long)sMotor.index);
      dbPackage.add("value", controlValue);
      dbPackage.add("position", getPosition());
      dbPackage.add("current", getCurrent());
      dbPackage.add("torque", getEffort());

      dbIdIndex = dbPackage.getIndexByName("id");
      dbControlParameterIndex = dbPackage.getIndexByName("value");
      dbPositionIndex = dbPackage.getIndexByName("position");
      dbCurrentIndex = dbPackage.getIndexByName("current");
      dbEffortIndex = dbPackage.getIndexByName("torque");

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
      if(myJoint) myJoint->detachMotor(sMotor.axis);

      // delete any coefficient vectors we might have created
      delete maxspeed_coefficients;
      delete maxeffort_coefficients;
      mimics.clear();
    }

    void SimMotor::addMimic(SimMotor* mimic) {
      mimics[mimic->getName()] = mimic;
    }

    void SimMotor::removeMimic(std::string mimicname) {
      mimics.erase(mimicname);
    }

    void SimMotor::clearMimics() {
      mimics.clear();
    }

    void SimMotor::setMimic(sReal multiplier, sReal offset) {
      mimic = true;
      mimic_multiplier = multiplier;
      mimic_offset = offset;
    }

    void SimMotor::setMaxEffortApproximation(utils::ApproximationFunction type,
      std::vector<double>* coefficients) {
      switch (type) {
        case FUNCTION_PIPE:
          maxEffortApproximation =&utils::pipe;
          maxeffort_x = &sMotor.maxEffort;
          break;
        case FUNCTION_POLYNOM3:
          maxEffortApproximation =&utils::polynom3;
          maxeffort_x = position;
          break;
        case FUNCTION_POLYNOM5:
          maxEffortApproximation =&utils::polynom5;
          maxeffort_x = position;
          break;
        case FUNCTION_GAUSSIAN:
        case FUNCTION_UNKNOWN:
          LOG_WARN("SimMotor: Approximation function not implemented or unknown.");
          break;
      }
      maxeffort_coefficients = coefficients;
    }

    void SimMotor::setMaxSpeedApproximation(utils::ApproximationFunction type,
      std::vector<double>* coefficients) {
      switch (type) {
        case FUNCTION_PIPE:
          maxSpeedApproximation = &utils::pipe;
          maxspeed_x = &sMotor.maxSpeed;
          break;
        case FUNCTION_POLYNOM3:
          maxSpeedApproximation = &utils::polynom3;
          maxspeed_x = position;
          break;
        case FUNCTION_POLYNOM5:
          maxSpeedApproximation = &utils::polynom5;
          maxspeed_x = position;
          break;
        case FUNCTION_GAUSSIAN:
        case FUNCTION_UNKNOWN:
          LOG_WARN("SimMotor: Approximation function not implemented or unknown.");
          break;
      }
      maxspeed_coefficients = coefficients;
    }

    void SimMotor::updateController() {
      axis = (unsigned char) sMotor.axis;
      switch (sMotor.type) {
        case MOTOR_TYPE_POSITION:
        case MOTOR_TYPE_PID: // deprecated
          controlParameter = &velocity;
          controlValue = sMotor.value;
          controlLimit = &(sMotor.maxSpeed);
          setJointControlParameter = &SimJoint::setVelocity;
          runController = &SimMotor::runPositionController;
          break;
        case MOTOR_TYPE_VELOCITY:
        case MOTOR_TYPE_DC: //deprecated
          controlParameter = &velocity;
          controlValue = sMotor.value;
          controlLimit = &(sMotor.maxAcceleration); // this is a stand-in for acceleration
          setJointControlParameter = &SimJoint::setVelocity;
          runController = &SimMotor::runVeloctiyController;
          break;
        case MOTOR_TYPE_PID_FORCE: // deprecated
        case MOTOR_TYPE_EFFORT:
          controlParameter = &effort;
          controlValue = sMotor.value;
          controlLimit = &(sMotor.maxEffort);
          setJointControlParameter = &SimJoint::setEffort;
          runController = &SimMotor::runEffortController;
          break;
        case MOTOR_TYPE_UNDEFINED:
          // TODO: output error
          controlParameter = &velocity; // default to position
          controlValue = sMotor.value;
          controlLimit = &(sMotor.maxSpeed);
          setJointControlParameter = &SimJoint::setVelocity;
          runController = &SimMotor::runPositionController;
          break;
      }
      //TODO: update the remaining parameters
    }

    void SimMotor::runEffortController(sReal time) {
      // limit to range of motion
      controlValue = std::max(sMotor.minValue,
        std::min(controlValue, sMotor.maxValue));

      if(controlValue > 2*M_PI)
        controlValue = 0;
      else if(controlValue > M_PI)
        controlValue = -2*M_PI + controlValue;
      else if(controlValue < -2*M_PI)
        controlValue = 0;
      else if(controlValue < -M_PI)
        controlValue = 2*M_PI + controlValue;

      error = controlValue - *position;
      if(error > M_PI) error = -2*M_PI + error;
      else if(error < -M_PI) error = 2*M_PI + error;
      integ_error += error * time;
      // P part of the motor
      effort = error * sMotor.p;
      // I part of the motor
      effort += integ_error * sMotor.i;
      // D part of the motor
      effort += ((error - last_error)/time) * sMotor.d;
      last_error = error;
      effort = std::max(-sMotor.maxEffort, std::min(effort, sMotor.maxEffort));
    }

    void SimMotor::runVeloctiyController(sReal time) {
      *controlParameter = controlValue;
    }

    void SimMotor::runPositionController(sReal time) {
      // the following implements a simple PID controller using the value
      // pointed to by controlParameter

      controlValue = mimic_multiplier * controlValue + mimic_offset;

      // limit to range of motion
      controlValue = std::max(sMotor.minValue,
        std::min(controlValue, sMotor.maxValue));

      // calculate control values
      error = controlValue - *position;
      if (std::abs(error) < 0.000001)
        error = 0.0;

      // FIXME: not sure if this makes sense, because it forbids turning
      //        motors in the same direction for multiple revolutions
      // and can possibly lead to the motor turning in the wrong direction
      // beyond its limit, as limit checking was done BEFORE??
      //if(er > M_PI)
      //  er = -2*M_PI+er;
      //else
      //if(er < -M_PI)
      //  er = 2*M_PI+er;

      integ_error += error*time;

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
      velocity = 0; // by setting a different value we could specify a minimum
      // P part of the motor
      velocity += error * sMotor.p;
      // I part of the motor
      velocity += iPart;
      // D part of the motor
      velocity += ((error - last_error)/time) * sMotor.d;
      last_error = error;
    }

    void SimMotor::update(sReal time_ms) {
      time = time_ms;// / 1000;
      sReal play_position = 0.0;

      // if the attached joint does not exist (any more)
      if (!myJoint) deactivate();

      if(active) {
        // set play offset to 0
        if(myPlayJoint) play_position = myPlayJoint->getPosition();

        refreshPosition();
        *position += play_position;

        // call control function for current motor type
        (this->*runController)(time_ms);

        // cap speed
        tmpmaxspeed = getMomentaryMaxSpeed();
        velocity = std::max(-tmpmaxspeed, std::min(velocity, tmpmaxspeed));
        // cap effort
        tmpmaxeffort = getMomentaryMaxEffort();
        effort = std::max(-tmpmaxeffort, std::min(effort, tmpmaxeffort));
        myJoint->setEffortLimit(tmpmaxeffort, axis);

        for(std::map<std::string, SimMotor*>::iterator it = mimics.begin();
          it != mimics.end(); ++it) {
            it->second->setControlValue(controlValue);
            //it->second->setControlValue(*position);
          }


        // estimate motor parameters based on achieved status
        estimateCurrent();
        estimateTemperature(time_ms);

        // pass speed (position/speed control) or torque to the attached
        // joint's setSpeed1/2 or setTorque1/2 methods
        (myJoint->*setJointControlParameter)(*controlParameter, axis);
        //for mimic in myJoint->mimics:
        //  mimic->*setJointControlParameter)(mimic_multiplier*controlParameter, axis);
      }
    }

    void SimMotor::estimateCurrent() {
      // calculate current
      effort = myJoint->getMotorTorque();
      joint_velocity = myJoint->getVelocity();
      current = (kXY*fabs(effort*joint_velocity) +
                 kX*fabs(effort) +
                 kY*fabs(joint_velocity) + k);
      current = fabs(current);
    }

    void SimMotor::estimateTemperature(sReal time_ms) {
      temperature = temperature - calcHeatDissipation(time_ms) + calcHeatProduction(time_ms);
    }

    /*
     * Calculates the heat energy dissipating in the update interval.
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
      kXY = 100.0*((0.00002) / ((9.81*0.07)*(2*M_PI/60)));
      kX  = 0.00512 / (9.81*0.07);
      kY  = 100.0*(0.00006 / (2*M_PI/60));
      k   = 0.025;
      current = 0;
    }

    void SimMotor::refreshPosition(){
      if(sMotor.axis == 1)
        position1 = myJoint->getPosition();
      else
        position2 = myJoint->getPosition(2);
    }

    void SimMotor::refreshPositions() {
      position1 = myJoint->getPosition();
      position2 = myJoint->getPosition(2);
    }

    void SimMotor::refreshAngle(){ // deprecated
      refreshPosition();
    }

    /*
     * This function can be overloaded in a child class in order to
     * implement a specifically variable effort.
     */
    sReal SimMotor::getMomentaryMaxEffort() {
      return (*maxEffortApproximation)(maxeffort_x, maxeffort_coefficients);
    }

    /*
     * This function can be overloaded in a child class in order to
     * implement a specifically speed.
     */
    sReal SimMotor::getMomentaryMaxSpeed() {
      return (*maxSpeedApproximation)(maxspeed_x, maxspeed_coefficients);
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
      switch(sMotor.type) {
        case MOTOR_TYPE_PID:
        case MOTOR_TYPE_POSITION:
        case MOTOR_TYPE_PID_FORCE:
        case MOTOR_TYPE_EFFORT:
          controlValue = angle;
          break;
        case MOTOR_TYPE_VELOCITY:
        case MOTOR_TYPE_DC:
        case MOTOR_TYPE_UNDEFINED:
          break;
      }
    }

    void SimMotor::setDesiredMotorVelocity(sReal veloctiy) { // deprecated
      switch(sMotor.type) {
        case MOTOR_TYPE_DC:
        case MOTOR_TYPE_VELOCITY:
          controlValue = velocity;
          break;
        case MOTOR_TYPE_PID:
        case MOTOR_TYPE_POSITION:
        case MOTOR_TYPE_PID_FORCE:
        case MOTOR_TYPE_EFFORT:
        case MOTOR_TYPE_UNDEFINED:
          break;
      }
    }

    sReal SimMotor::getDesiredMotorAngle() const { // deprecated
      switch(sMotor.type) {
        case MOTOR_TYPE_PID:
        case MOTOR_TYPE_POSITION:
        case MOTOR_TYPE_PID_FORCE:
        case MOTOR_TYPE_EFFORT:
          return controlValue;
          break;
        case MOTOR_TYPE_DC:
        case MOTOR_TYPE_VELOCITY:
        case MOTOR_TYPE_UNDEFINED:
          break;
        }
      return 0.0; // return 0 if it doesn't apply
    }

    void SimMotor::setMaxEffort(sReal force) {
      sMotor.maxEffort = force;
      myJoint->setEffortLimit(sMotor.maxEffort, axis);
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

    sReal SimMotor::getActualPosition() const { // deprecated
      return getPosition();
    }

    void SimMotor::setPosition(sReal angle) {
      position1 = angle;
    }

    void SimMotor::setActualAngle(sReal angle) { // deprecated
      setPosition(angle);
    }

    void SimMotor::setMaxSpeed(sReal speed) {
      sMotor.maxSpeed = fabs(speed);
    }

    void SimMotor::setMaximumVelocity(sReal v) { // deprecated
      setMaxSpeed(v);
    }

    sReal SimMotor::getMaxSpeed() const {
      return sMotor.maxSpeed;
    }

    sReal SimMotor::getMaximumVelocity() const { // deprecated
      return getMaxSpeed();
    }

    bool SimMotor::isServo() const {
      return !(sMotor.type == MOTOR_TYPE_DC ||
        sMotor.type == MOTOR_TYPE_VELOCITY ||
        sMotor.type == MOTOR_TYPE_UNDEFINED);
    }

    void SimMotor::setType(interfaces::MotorType mtype){
      sMotor.type = mtype;
      updateController();
    }

    void SimMotor::setVelocity(sReal v) {
      switch(sMotor.type) {
        case MOTOR_TYPE_DC:
        case MOTOR_TYPE_VELOCITY:
          setControlValue(v);
          break;
        case MOTOR_TYPE_PID:
        case MOTOR_TYPE_POSITION:
        case MOTOR_TYPE_PID_FORCE:
        case MOTOR_TYPE_EFFORT:
        case MOTOR_TYPE_UNDEFINED:
          break;
      }
    }

    sReal SimMotor::getControlParameter(void) const {
      return *controlParameter;
    }

    sReal SimMotor::getVelocity() const { // deprecated
      return velocity;
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
      if(myJoint && (sMotor.type != MOTOR_TYPE_PID_FORCE)) {
          myJoint->attachMotor(axis);
          myJoint->setEffortLimit(sMotor.maxEffort, axis);
      }
    }

    const MotorData SimMotor::getSMotor(void) const {
      return sMotor;
    }

    void SimMotor::setValue(sReal value) {
      setControlValue(value);
    }

    void SimMotor::setControlValue(interfaces::sReal value) {
      controlValue = value;
      if(!control->sim->isSimRunning()) {
        myJoint->setOfflinePosition(value);
        refreshPositions();
      }
    }

    /*switch (sMotor.type) {
      case MOTOR_TYPE_POSITION:
      case MOTOR_TYPE_PID:
        desired_position = value;
        break;
      case MOTOR_TYPE_SPEED:
      case MOTOR_TYPE_DC:
        desired_speed = value;
        break;
      case MOTOR_TYPE_PID_FORCE:
        desired_position = value;
        break;
      case MOTOR_TYPE_UNDEFINED:
        break;
      }*/

    sReal SimMotor::getValue(void) const {
      return getControlValue();
    }

    sReal SimMotor::getControlValue(void) const {
      return controlValue;
    }

    void SimMotor::setPID(sReal mP, sReal mI, sReal mD) {
      switch (sMotor.type) {
      case MOTOR_TYPE_PID: // deprecated
      case MOTOR_TYPE_POSITION:
      case MOTOR_TYPE_PID_FORCE: // deprecated
      case MOTOR_TYPE_EFFORT:
        sMotor.p = mP;
        sMotor.i = mI;
        sMotor.d = mD;
        break;
      case MOTOR_TYPE_DC: // deprecated
      case MOTOR_TYPE_VELOCITY:
      case MOTOR_TYPE_UNDEFINED:
        // information not relevant for these types
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
      obj->value = controlValue;
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
        dbPackage->set(dbControlParameterIndex, controlValue);
        dbPackage->set(dbPositionIndex, getPosition());
        dbPackage->set(dbCurrentIndex, getCurrent());
        dbPackage->set(dbEffortIndex, getEffort());
      }

      void SimMotor::receiveData(const data_broker::DataInfo& info,
                                 const data_broker::DataPackage& package,
                                 int id) {
        sReal value;
        package.get(0, &value);
        setControlValue(value);
      }

      void SimMotor::setValueDesiredVelocity(interfaces::sReal value) {
        switch (sMotor.type) {
        case MOTOR_TYPE_VELOCITY:
        case MOTOR_TYPE_DC: //deprecated
          setControlValue(value);
          break;
        case MOTOR_TYPE_PID:
        case MOTOR_TYPE_POSITION:
        case MOTOR_TYPE_PID_FORCE:
        case MOTOR_TYPE_EFFORT:
        case MOTOR_TYPE_UNDEFINED:
          break;
        }
      };

  } // end of namespace sim
} // end of namespace mars
