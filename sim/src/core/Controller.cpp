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

/**
 * \file Controller.h
 * \author Malte Langosz
 * \brief "Controller" provides an architecture to log all relevant data from
 *        the simulation in file, GUI or any other receiver
 */

#define PACKAGE_SIZE 2048


#include "Controller.h"

#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/sim/SensorManagerInterface.h>
#include <mars/utils/misc.h>
#include <mars/utils/mathUtils.h>
#include <mars/interfaces/Logging.hpp>
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>

#include <cmath>
#include <cstring>

namespace mars {
  namespace sim {
    
    using namespace utils;
    using namespace interfaces;

#ifdef WIN32
    bool Controller::sock_init = false;
#endif

    
    Controller::Controller(sReal rate,
                           const std::vector<SimMotor*> &motors,
                           const std::vector<BaseSensor*> &sensors,
                           const std::vector<NodeData*> &sNodes,
                           ControlCenter* control, int nport) {
      std::vector<SimMotor*>::const_iterator iter;
      std::vector<BaseSensor*>::const_iterator jter;
      std::vector<NodeData*>::const_iterator lter;

      this->sController.rate = rate;
      this->motors  = motors;
      this->sensors = sensors;
      this->sNodes  = sNodes;
      this->control = control;
      // localhost = "192.168.101.57";
      hostname = "localhost";
      this->nport = nport;
      auto_connect = true;
      sock_state = 0;
      running = true;

      for(iter = motors.begin(); iter != motors.end(); iter++)
        sController.motors.push_back((*iter)->getIndex());
      for(jter = sensors.begin(); jter != sensors.end(); jter++)
        sController.sensors.push_back((*jter)->getID());
      for(lter = sNodes.begin(); lter != sNodes.end(); lter++)
        sController.sNodes.push_back((*lter)->index);
      sController.dylib_path = "";
      dy = 0;
      dylibController = 0;
      count_ms = 0;
#ifdef WIN32
      if(!Controller::sock_init) {
        /* Initialisiere TCP f�r Windows ("winsock") */
        WORD wVersionRequested;
        WSADATA wsaData;
        wVersionRequested = MAKEWORD (1, 1);
        if (WSAStartup (wVersionRequested, &wsaData) != 0) {
          LOG_ERROR("Controller: Fehler beim Initialisieren von Winsock");
          return;
        }
        else {
          LOG_ERROR("Controller: Winsock initialisiert");
        }
        Controller::sock_init = true;
      }
#endif
      connected = 0;
      conn = 0;
      //initServer(1500);
      //getClient();
      LOG_ERROR("Controller: try to connect to port: %d", nport);
      openClient(hostname.data(), nport);
      start();
    }

    Controller::~Controller(void){
      running = false;
      if(dy) {
#ifdef WIN32
        if(dylibController) {
          destroy_controller *tmp_des = (destroy_controller*)GetProcAddress(dy, "destroy_c");
          tmp_des(dylibController);
        }
        FreeLibrary(dy);
#else
        if(dylibController) {
          destroy_controller *tmp_des = (destroy_controller*)dlsym(dy, "destroy_c");
          tmp_des(dylibController);
        }
        dlclose(dy);
#endif
      }
      if(connected) close(conn);
      connected = false;
      while(!isFinished()) 
        msleep(10);
    }
    
    void Controller::setID(unsigned long id) {
      this->sController.id = id;
    }

    unsigned long Controller::getID(void) const {
      return sController.id;
    }

    const ControllerData Controller::getSController(void) const {
      return sController;
    }

    void Controller::getCoreExchange(core_objects_exchange *obj) const {
      obj->index = sController.id;
      obj->name  = "";
    }

    void Controller::update(sReal time_ms) {
      std::vector<BaseSensor*>::iterator iter;
      std::vector<SimMotor*>::iterator jter;
      char data[PACKAGE_SIZE];
      char *p = data;
      double value;
      double t_sensors[255];
      double *pt_sensors = t_sensors;
      double t_motors[100];
      double *pt_motors = t_motors;
      int flags = 0, count_val, i, command;
      sReal *sens_val;
      char *other_stuff = 0;
      char *pt_stuff;
      unsigned long command_id = 0;
      char cmd;
#ifdef WIN32
      int received;
#endif
      if ((count_ms += time_ms) >= sController.rate) {
        count_ms -= sController.rate;
        if (dylibController) {
          for (i=0; i<100; i++) t_sensors[i] = t_motors[i] = 0;
          for (iter = sensors.begin(); iter != sensors.end(); iter++) {
            count_val = (*iter)->getSensorData(&sens_val);
            for(i=0; i<count_val; i++) *(pt_sensors++) = (double)sens_val[i];
            free(sens_val);
          }
          /*
          if (sParams.size()) {
            other_stuff = (char*)malloc(sParams.size()*sizeof(sReal)+sizeof(int));
            pt_stuff = other_stuff;
            *(int*)pt_stuff = sParams.size();
            pt_stuff += sizeof(int);
            for(pter = sParams.begin(); pter != sParams.end(); pter++) {
              *(sReal*)pt_stuff = (*pter)->value;
              pt_stuff += sizeof(sReal);
            }
          }
          */

          dylibController->update(time_ms, t_sensors, t_motors,
                                  &flags, &other_stuff);
          if (other_stuff) {
            for (i=0, pt_stuff = other_stuff+sizeof(int);
                 i<*(int*)other_stuff; i++) {
              Vector force, pos;
              Quaternion q_rot;
              sRotation rot;
              //char p_name[20];
              unsigned long node_id;
              command = *(int*)pt_stuff;
              pt_stuff += sizeof(int);
              command_id = *(unsigned long*)pt_stuff;
              pt_stuff += sizeof(unsigned long);
              switch(command) {
              case COMMAND_NODE_POSITION:
                pos.x() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.y() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.z() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                control->nodes->setPosition(command_id, pos);
                break;
              case COMMAND_NODE_ROTATION:
                rot.alpha = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                rot.beta = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                rot.gamma = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                q_rot = eulerToQuaternion(rot);
                control->nodes->setRotation(command_id, q_rot);
                break;
              case COMMAND_NODE_APPLY_FORCE:
                force.x() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                force.y() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                force.z() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                control->nodes->applyForce(command_id, force);
                break;
              case COMMAND_NODE_APPLY_FORCE_AT:
                force.x() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                force.y() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                force.z() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.x() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.y() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.z() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                control->nodes->applyForce(command_id, force, pos);
                break;
              case COMMAND_PARAM_ADD:
                /* ToDo: implement via cfg
                ps = new paramStruct;
                strcpy(p_name, pt_stuff);
                pt_stuff += strlen(p_name)+1;
                ps->grouped = false;
                ps->parentName = "controller";
                ps->parameterName = p_name;
                ps->pr = (ParamReceiver*)this;
                ps->type = *(int*)pt_stuff;
                pt_stuff += sizeof(int);
                ps->row = *(int*)pt_stuff;
                pt_stuff += sizeof(int);
                ps->min = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                ps->max = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                ps->value = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                sParams.push_back(ps);
                control->sim->addParamStruct(ps);*/
                break;
              case COMMAND_NODE_CONTACT_MOTION1:
                control->nodes->setContactParamMotion1(command_id,
                                                       *(sReal*)pt_stuff);
                pt_stuff += sizeof(sReal);
                break;
              case COMMAND_NODE_RELOAD_EXTENT:
                pos.x() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.y() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.z() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                control->nodes->setReloadExtent(command_id, pos);
                break;
              case COMMAND_NODE_RELOAD_POSITION:
                pos.x() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.y() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.z() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                control->nodes->setReloadPosition(command_id, pos);
                break;
              case COMMAND_NODE_RELOAD_ANGLE:
                rot.alpha = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                rot.beta = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                rot.gamma = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                control->nodes->setReloadAngle(command_id, rot);
                break;
              case COMMAND_JOINT_RELOAD_OFFSET:
                rot.alpha = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                rot.beta = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                rot.gamma = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                control->joints->setReloadJointOffset(command_id, rot.alpha);
                break;
              case COMMAND_JOINT_RELOAD_AXIS:
                pos.x() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.y() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.z() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                control->joints->setReloadJointAxis(command_id, pos);
                break;
              case COMMAND_SIM_QUIT:
                pt_stuff += sizeof(sReal);
                pt_stuff += sizeof(sReal);
                pt_stuff += sizeof(sReal);
                LOG_INFO("Controller: got quit command");
                control->sim->exitMars();
                break;
              case COMMAND_NODE_VELOCITY:
                pos.x() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.y() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.z() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                control->nodes->setVelocity(command_id, pos);
                break;
              case COMMAND_NODES_CONNECT:
                node_id = *(unsigned long*)pt_stuff;
                pt_stuff += sizeof(unsigned long);
                control->sim->connectNodes(command_id, node_id);
                break;
              case COMMAND_NODES_DISCONNECT:
                node_id = *(unsigned long*)pt_stuff;
                pt_stuff += sizeof(unsigned long);
                control->sim->disconnectNodes(command_id, node_id);
                break;
              case COMMAND_NODE_ANGULAR_VELOCITY:
                pos.x() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.y() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.z() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                control->nodes->setAngularVelocity(command_id, pos);
                break;
              case COMMAND_NODE_RELOAD_FRICTION:
                pos.x() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.y() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pt_stuff += sizeof(sReal);
                control->nodes->setReloadFriction(command_id, pos.x(), pos.y());
                break;
              case COMMAND_NODE_RELOAD_QUATERNION:
                q_rot.x() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                q_rot.y() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                q_rot.z() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                q_rot.w() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                control->nodes->setReloadQuaternion(command_id, q_rot);
                break;
              case COMMAND_JOINT_RELOAD_ANCHOR:
                pos.x() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.y() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.z() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                control->joints->setReloadAnchor(command_id, pos);
                break;
              case COMMAND_PHYSICS_GRAVITY:
                pos.x() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.y() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                pos.z() = *(sReal*)pt_stuff;
                pt_stuff += sizeof(sReal);
                control->sim->setGravity(pos);
                break;
              default:
                break;
              }
            }
            free(other_stuff);
          }
          if (flags) {
            control->sim->resetSim();
            //if(!control->sim->isSimRunning()) control->sim->startStopTrigger();
          }
          else {
            for (jter = motors.begin(); jter != motors.end(); jter++, pt_motors++)
              (*jter)->setControlValue((sReal)*pt_motors);
          }
        }
        else if(connected) {
          // here we can communicate
#ifdef WIN32
          memset(data, 0, PACKAGE_SIZE);
#else
          bzero(data, PACKAGE_SIZE);
#endif

#ifndef MATTHIAS_AUS

          int count = 0;
          for(iter = sensors.begin();
              iter != sensors.end(); iter++) {
            count += (*iter)->getAsciiData(p+count);
          }

          if (send(conn, data, PACKAGE_SIZE, 0) == -1) {
            connected = false;
            sock_state = 0;
            LOG_ERROR("Controller: connection lost");
            return;
          }
#else
#warning "Disabled monster stuff here"
#endif

#ifdef WIN32
          memset(data, 0, PACKAGE_SIZE);
          received = 0;
          int tmp;
          while (received < PACKAGE_SIZE) {
            tmp = recv(conn, data+received, PACKAGE_SIZE-received, 0);
            if (tmp == -1) {
              received = PACKAGE_SIZE;
              connected = false;
              sock_state = 0;
              LOG_ERROR("Controller: connection lost");
              return;
            }
            else received += tmp;
          }
#else
          bzero(data, PACKAGE_SIZE);
          if (recv(conn, data, PACKAGE_SIZE, MSG_WAITALL) == -1) {
            connected = false;
            sock_state = 0;
            LOG_ERROR("Controller: connection lost");
            return;
          }
#endif
          if (data[0] == 's') {
            p = data+1;
            for (jter = motors.begin();
                 jter != motors.end(); jter++) {
              p += getSReal(p, &value);
              value *= 0.01745329251994;
              (*jter)->setControlValue((sReal)value);
            }
          }
          //the protocol is very simple
          //data= "m cmd1 id1 value1 [value2 value3 ...] [id2 value1 [value2 value3 ...] ...] e [ cmd2 id value cmd id value] f"
          //example data="m COMMAND_MOTOR_POSITION 1 20 3 40 e COMMAND_NODE_POSITION 1 0.1 0.2 0.3 f"
          // set the motor with id 1 and id 3 to the position 20 and 40 respectively
          //set the node with id 1 to the position [0.1 0.2 0.3]
          else if (data[0] == 'm') {
            Vector pos, vel;
            Quaternion q_rot;
            sRotation rot;
            p= data+1;
            getChar(p,&cmd);
            while ( cmd != 'f' ) {
              //get the command
              p += getSReal(p, &value);
#ifdef WIN32
              command_id=(UINT)value;
#else
              command_id=(uint)value;
#endif
              //p+=1;
              switch (command_id) {

              case COMMAND_MOTOR_POSITION:
                while ( cmd != 'e' && cmd != 'f' ) {
                  //get the motor id
                  //p-=1;
                  p += getSReal(p, &value);
#ifdef WIN32
                  UINT id = (UINT) value;
#else
                  uint id = (uint) value;
#endif
                  //assert(id=< motors.size());
                  p += getSReal(p, &value);
                  //value *= 0.01745329251994;
                  motors[id]->setControlValue((sReal)value);

                  // p += getChar(p, &cmd);
                  getChar(p, &cmd);
                  //command_id=(uint)value;
                }
                break;

              case COMMAND_MOTOR_MAX_VELOCITY:
                while ( cmd != 'e' && cmd != 'f' ) {
                  //get the motor id
                  //p-=1;
                  p += getSReal(p, &value);
#ifdef WIN32
                  UINT id = (UINT) value;
#else
                  uint id = (uint) value;
#endif
                  //assert(id=< motors.size());
                  p += getSReal(p, &value);
                  //value *= 0.01745329251994;
                  motors[id]->setMaxSpeed((sReal)value);

                  // p += getChar(p, &cmd);
                  getChar(p, &cmd);
                  //command_id=(uint)value;
                }
                break;

              case COMMAND_NODE_POSITION:
                while ( cmd != 'e' && cmd != 'f' ) {
                  p += getSReal(p, &value);
#ifdef WIN32
                  UINT id = (UINT) value;
#else
                  uint id = (uint) value;
#endif
                  p += getSReal(p, &value );
                  pos.x() = value;
                  p += getSReal(p, &value );
                  pos.y() = value;
                  p += getSReal(p, &value );
                  pos.z() = value;
                  control->nodes->setPosition(id, pos);
                  getChar(p, &cmd);
                }
                break;

              case COMMAND_NODE_VELOCITY:
                while ( cmd != 'e' && cmd != 'f' ) {
                  p += getSReal(p, &value);
#ifdef WIN32
                  UINT id = (UINT) value;
#else
                  uint id = (uint) value;
#endif
                  p += getSReal(p, &value );
                  vel.x() = value;
                  p += getSReal(p, &value );
                  vel.y() = value;
                  p += getSReal(p, &value );
                  vel.z() = value;
                  control->nodes->setVelocity(id, vel);
                  getChar(p, &cmd);
                }
                break;

              case COMMAND_NODE_ROTATION:
                while ( cmd != 'e' && cmd != 'f' ) {
                  p += getSReal(p, &value);
#ifdef WIN32
                  UINT id = (UINT) value;
#else
                  uint id = (uint) value;
#endif
                  p += getSReal(p, &value );
                  rot.alpha = value;
                  p += getSReal(p, &value );
                  rot.beta = value;
                  p += getSReal(p, &value );
                  rot.gamma = value;
                  // make quaternion and set rotation
                  q_rot = eulerToQuaternion(rot);
                  control->nodes->setRotation(id, q_rot);
                  getChar(p, &cmd);
                }
                break;

              case COMMAND_PHYSICS_PARAMETER:
                while ( cmd != 'e' && cmd != 'f' ) {
                  p += getSReal(p, &value);
#ifdef WIN32
                  UINT id = (UINT) value;
#else
                  uint id = (uint) value;
#endif
                  p += getSReal(p, &value );
                  /* To do handle this via cfg
                  paramStruct  param;
                  param.index = id;
                  param.value = value;
                  control->sim->receiveValue(&param);
                  */
                  if(id==1) {
                    // old id for calcms
                    if(control->cfg) {
                      control->cfg->setPropertyValue("Simulator", "calc_ms",
                                                     "value", value);
                    }
                  }
                  else if(id==5) {
                    // old id for realtime calc
                    if(control->cfg) {
                      control->cfg->setPropertyValue("Simulator",
                                                     "realtime calc",
                                                     "value", value);
                    }
                  }
                  getChar(p, &cmd);
                }
                break;
              case COMMAND_NODES_CONNECT:
                while ( cmd != 'e' && cmd != 'f' ) {
                  p += getSReal(p, &value);
                  unsigned long id1 = (unsigned long) value;
                  p += getSReal(p, &value );
                  unsigned long id2 = (unsigned long) value;
                  control->sim->connectNodes(id1, id2);
                  getChar(p, &cmd);
                }
                break;
              case COMMAND_NODES_DISCONNECT:
                while ( cmd != 'e' && cmd != 'f' ) {
                  p += getSReal(p, &value);
                  unsigned long id1 = (unsigned long) value;
                  p += getSReal(p, &value );
                  unsigned long id2 = (unsigned long) value;
                  control->sim->disconnectNodes(id1, id2);
                  getChar(p, &cmd);
                }
                break;

              case COMMAND_CONTACT_NODE_PARAMETER:
                while ( cmd != 'e' && cmd != 'f' ) {

                  p += getSReal(p, &value);
#ifdef WIN32
                  UINT id = (UINT) value;
#else
                  uint id = (uint) value;
#endif
                  contact_params cp_tmp;

                  p += getSReal(p, &value );
                  cp_tmp.erp=value;
                  p += getSReal(p, &value );
                  cp_tmp.cfm = value;
                  p += getSReal(p, &value );
                  cp_tmp.friction1 = value;
                  p += getSReal(p, &value );
                  cp_tmp.friction2 = value;
                  p += getSReal(p, &value );
                  if (value) {
                    Vector * friction_direction =(Vector*) malloc(sizeof(Vector)) ;
                    p += getSReal(p, &value );
                    friction_direction->x() = value;
                    p += getSReal(p, &value );
                    friction_direction->y() = value;
                    p += getSReal(p, &value );
                    friction_direction->z() = value;
                    cp_tmp.friction_direction1 = friction_direction;
                  }
                  else {
                    cp_tmp.friction_direction1 = 0;
                  }
                  p += getSReal(p, &value);
                  cp_tmp.motion1 = value;
                  p += getSReal(p, &value);
                  cp_tmp.motion2 = value;
                  p += getSReal(p, &value);
                  cp_tmp.fds1 = value;
                  p += getSReal(p, &value);
                  cp_tmp.fds2 = value;
                  p += getSReal(p, &value);
                  cp_tmp.bounce = value;
                  p += getSReal(p, &value);
                  cp_tmp.bounce_vel = value;
                  p += getSReal(p, &value);
                  cp_tmp.approx_pyramid = (bool) value;
                  control->nodes->setContactParams(id, cp_tmp);
                  //p += getChar(p, &cmd);
                  getChar(p, &cmd);
                  //command_id=(uint)value;
                }
                break;

              case COMMAND_MOTOR_PID:
                sReal mP, mI, mD;

                while ( cmd != 'e' && cmd != 'f' ) {
                  p += getSReal(p, &value);
#ifdef WIN32
                  UINT id = (UINT) value;
#else
                  uint id = (uint) value;
#endif
                  p += getSReal(p, &mP);

                  //value *= 0.01745329251994;
                  p += getSReal(p, &mI);
                  p += getSReal(p, &mD);
                  motors[id]->setPID(mP, mI, mD);

                  getChar(p, &cmd);
                  //command_id=(uint)value;
                }
                break;
              case COMMAND_NODE_ANGULAR_DAMPING:
                while ( cmd != 'e' && cmd != 'f' ) {

                  p += getSReal(p, &value);
#ifdef WIN32
                  UINT id = (UINT) value;
#else
                  uint id = (uint) value;
#endif
                  p += getSReal(p, &value);
                  control->nodes->setAngularDamping(id, value);
                  //p += getChar(p, &cmd);
                  getChar(p, &cmd);
                  //command_id=(uint)value;
                }
                break;

              case COMMAND_PHYSICS_GRAVITY:
                while ( cmd != 'e' && cmd != 'f' ) {
                  p += getSReal(p, &value);
                  p += getSReal(p, &value);
                  pos.x() = value;
                  p += getSReal(p, &value);
                  pos.y() = value;
                  p += getSReal(p, &value);
                  pos.z() = value;
                  control->sim->setGravity(pos);
                  getChar(p, &cmd);
                }
                break;

              default:
                break;
              } //\switch
              p += getChar(p, &cmd);
              if(cmd != 'f') getChar(p, &cmd);
              //p += getSReal(p, &value );
              //command_id=(uint)value;
            }//while not end of message
          }//if(data[0] == 'm')
          else if(data[0] == 'r') {
            // reset the world
            control->sim->resetSim();
          }
        }
        else {
          //if(conn) close(conn);
          //openClient(hostname.data(), nport);
          //connectClient();      
        }
      }
    }

    int Controller::getSReal(const char *data, sReal *value) const {
      size_t d=0, i=0;
      const size_t BUFFER_SIZE = 50;
      char v[BUFFER_SIZE];

#ifdef WIN32
      memset(v, 0, BUFFER_SIZE);
#else
      bzero(v, BUFFER_SIZE);
#endif
      while(data[d] == ' ' && data[d] != '\0' && d<BUFFER_SIZE) d++;
      while(data[d] != ' ' && data[d] != '\0' && data[d] != '\n'
            && data[d] != '\r' && d<BUFFER_SIZE) v[i++] = data[d++];
      sscanf(v, " %lf", value);
      if(std::isnan(*value)) {
        *value = 0.0;
      }
      return d;
    }

    int Controller::getChar(const char *data, char * c) const {
      size_t d=0;
      const size_t BUFFER_SIZE = 50;
      while (data[d] == ' ' && data[d] != '\0' && d<BUFFER_SIZE) d++;
      *c = data[d++];
      return d;
    }

    void Controller::resetData(void) {
      std::vector<unsigned long>::iterator iter;
      motors.clear();
      for(iter = sController.motors.begin();
          iter != sController.motors.end(); iter++) {
        motors.push_back(control->motors->getSimMotor((*iter)));
      }
      sensors.clear();
      BaseSensor *sensor;
      for(iter = sController.sensors.begin();
          iter != sController.sensors.end(); iter++) {
        sensor = control->sensors->getSimSensor(*iter);
        if(sensor) {
          sensors.push_back(sensor);
        }
      }
    }

    void Controller::handleError(void) {
      LOG_ERROR("Controller: handleError()");
      if(dylibController) {
        dylibController->handleError();
      }
    }

    void Controller::setDylibPath(const std::string &dylib_path) {
      sController.dylib_path = dylib_path;
      if (sController.dylib_path != "") {
#ifdef WIN32
        size_t needed = ::mbstowcs(NULL,sController.dylib_path.c_str(),sController.dylib_path.length());

        // allocate
        std::wstring output;
        output.resize(needed);

        // real call
        ::mbstowcs(&output[0],sController.dylib_path.c_str(),sController.dylib_path.length());

        // You asked for a pointer
        const wchar_t *pout = output.c_str();
        dy = LoadLibrary(sController.dylib_path.c_str());
        std::cout << sController.dylib_path<<"\n";
        if(!dy) {
          LOG_WARN("Controller: dynamic controller not loaded.");
        }
        else {
          //dlerror();
          create_controller *tmp_con = (create_controller*)GetProcAddress(dy, "create_c");
          if (!tmp_con) {
            LOG_ERROR("Controller: could not load controller symbol");
          }
          else {
            dylibController = tmp_con();
          }
        }
#else
        dy = dlopen(sController.dylib_path.c_str(), RTLD_LAZY);
        if (!dy) {
          LOG_WARN("Controller: dynamic controller not loaded: %s", dlerror());
        }
        else {
          dlerror();
          create_controller *tmp_con = (create_controller*)dlsym(dy, "create_c");
          if (dlerror()) {
            LOG_ERROR("Controller: could not load controller symbol");
          }
          else {
            dylibController = tmp_con();
            /*
              if (dylibController->iceInterface && control->ice) {
              control->ice->addInterface(dylibController->iceInterface,
              dylibController->interfaceName);
              }
            */
          }
        }
#endif
      }
    }


    int Controller::initServer(int port) {
      int s = 0;
      struct sockaddr_in sa;
      struct hostent *hp;

#ifdef WIN32
      memset(&sa, 0, sizeof(struct sockaddr_in));
#else
      bzero(&sa,sizeof(struct sockaddr_in));
#endif
      hp= (struct hostent *)gethostbyname( "localhost" );
      if (hp == NULL)
        return(-1);
      sa.sin_family= hp->h_addrtype;
      sa.sin_port= htons(port);
      if (( s = socket(AF_INET,SOCK_STREAM,0)) < 0)
        return(-2);
      if (bind( s ,(struct sockaddr *)&sa,sizeof sa) < 0) {
        close( s );
        return(-3);
      }
      listen(s, 3);
      server = s;
      return 0;
    }

    void Controller::getClient(void) {
#ifdef WIN32
      SOCKET t;

      t = accept(server, 0, 0);
#else
      struct sockaddr_in isa;
      socklen_t i;
      socklen_t t;

      i = sizeof(isa);
      getsockname(server,(struct sockaddr*)&isa,&i);

      do {
        t = accept(server,(struct sockaddr*)&isa,&i);
      } while(t<=0);

      fcntl( t , F_SETFL, O_NONBLOCK );
#endif
      conn = t;
    }

    int Controller::openClient(const char* host, int port) {
      struct sockaddr_in localAddr, servAddr;
      struct hostent *h;
      int rc;//, ret, flag;


      myServAddr = 0;
      h = gethostbyname(host);
      if (h==0) {
        if(sock_state == 0) {
          LOG_ERROR("Controller: no host on: %s", host);
        }
        sock_state = 1;
        return 1;
      }
      servAddr.sin_family = h->h_addrtype;
      memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
      servAddr.sin_port = htons(port);

      conn = socket(AF_INET, SOCK_STREAM, 0);
      if (conn<0) {
        if (sock_state==0) {
          LOG_ERROR("Controller: cannot open socket");
        }
        conn = 0;
        sock_state = 1;
        return 1;
      }

      localAddr.sin_family = AF_INET;
      localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
      localAddr.sin_port = htons(0);

      rc = bind(conn, (struct sockaddr *) &localAddr, sizeof(localAddr));
      if (rc<0) {
        if (sock_state == 0) {
          LOG_ERROR("Controller: %s: cannot bind port TCP %u", h->h_name, port);
        }
        sock_state = 1;
        return 1;
      }
      myServAddr = &servAddr;
      return connectClient();
    }

    int Controller::connectClient(void) {
      if (!myServAddr) return 1;
      /* connect to server */

      int rc = ::connect(conn, (struct sockaddr *) myServAddr, sizeof(*myServAddr));
      if (rc<0) {
        if (sock_state == 0) {
          LOG_ERROR("Controller: cannot connect");
        }
        sock_state = 1;
        return 1;
      }
      LOG_INFO("Controller: connected");
      connected = 1;
      sock_state = 1;
      return 0;
    }

    void Controller::run(void) {

      while (running) {
        if (!connected && auto_connect) {
          if (conn) {
#ifdef WIN32
            closesocket(conn);
#else
            close(conn);
#endif
            conn = 0;
          }
          openClient(hostname.data(), nport);      
        }
        msleep(400);
      }
    }

    void Controller::setAutoMode(bool mode) {
      auto_connect = mode;
    }

    void Controller::setIP(const std::string &ip) {
      hostname = ip;
    }

    void Controller::setPort(int port) {
      nport = port;
    }

    bool Controller::getAutoMode(void) const {
      return auto_connect;
    }

    const std::string Controller::getIP(void) const {
      return hostname;
    }

    int Controller::getPort(void) const {
      return nport;
    }

    void Controller::connect(void) {
      if(connected || conn) close(conn);
      openClient(hostname.data(), nport);
    }

    void Controller::disconnect(void) {
      if(connected) close(conn);
    }

    std::list<sReal> Controller::getSensorValues(void) {
      std::vector<BaseSensor*>::iterator iter;
      sReal *sens_val;
      std::list<sReal> sensorValues;

      for (iter=sensors.begin(); iter!=sensors.end(); ++iter) {
        int count_val = (*iter)->getSensorData(&sens_val);
        for(int i=0; i<count_val; i++) {
          sensorValues.push_back(sens_val[i]);
        }
        free(sens_val);
      }
      return sensorValues;
    }


  } // end of namespace sim
} // end of namespace mars
