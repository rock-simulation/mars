/*
 *  Copyright 2011, 2012, 2014, DFKI GmbH Robotics Innovation Center
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

#include <cstdlib>

#include "ScanningSonar.h"

#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/sim/SensorManagerInterface.h>
#include <mars/utils/mathUtils.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/interfaces/sim/LoadCenter.h>

#include <mars/data_broker/DataBrokerInterface.h>

#include "SimMotor.h"
#include "SimNode.h"
#include "RaySensor.h"

#include <algorithm>
#include <sstream>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* ScanningSonar::instanciate(ControlCenter *control,
                                           BaseConfig *config ){
      ScanningSonarConfig *cfg = dynamic_cast<ScanningSonarConfig*>(config);
      assert(cfg);
      return new ScanningSonar(control,*cfg);
    }

    ScanningSonar::ScanningSonar(ControlCenter *control,
                                 ScanningSonarConfig _config):
      BaseCameraSensor<double>(_config.id,_config.name,64,512,1,true),
      BaseNodeSensor(_config.id,_config.name),
      SensorInterface(control),
      config(_config)
    {
      position = _config.position;
      orientation = _config.orientation;
      attached_node = config.attached_node;
      head_position.setZero();
      head_orientation.setIdentity();

      std::vector<unsigned long>::iterator iter;
      dbPosIndices[0] = -1;

      nodeID[0] = 0;
      nodeID[1] = 0;
      jointID[0] = 0;
      jointID[1] = 0;
      rayID = 0;
      raySensor = 0;

      config.extension -= Vector(0, 0, config.extension[2]/2.0); //Split the Sonar in two parts, to separate fixed and moving part

      //Adding Nodes, Joints and Motor for Scanning Sonar
      std::stringstream s;
      s.str(""); s << config.name << "_fixed_joint";
      NodeData ns_fixed( s.str(),position,orientation);
      ns_fixed.initPrimitive(NODE_TYPE_CYLINDER,config.extension,0);
      ns_fixed.movable = true;
      ns_fixed.noPhysical = false;
      ns_fixed.c_params.coll_bitmask = 0;
      nodeID[0] = control->nodes->addNode(&ns_fixed);

      s.str(""); s << config.name << "_play_joint";
      NodeData ns_play(s.str(), config.position + (orientation * Vector(0, 0, config.extension[2])), orientation);
      ns_play.initPrimitive(NODE_TYPE_CYLINDER,config.extension,0);
      ns_play.movable = true;
      ns_play.noPhysical = false;
      ns_play.c_params.coll_bitmask = 0;
      nodeID[1] = control->nodes->addNode(&ns_play);

      s.str(""); s << config.name << "_body_to_fixed";
      JointData js1(s.str(),JOINT_TYPE_FIXED,attached_node,nodeID[0]);
      js1.axis1 = orientation * Vector(0, 0, 1);
      js1.anchorPos= 3;
      jointID[0] = control->joints->addJoint(&js1);
      s.str(""); s << config.name << "_fixed_to_play";
      JointData js2(s.str(),JOINT_TYPE_HINGE,nodeID[0],nodeID[1]);
      js2.axis1 = orientation * Vector(0, 0, 1);
      js2.anchorPos = 3;
      jointID[1] = control->joints->addJoint(&js2);


      s.str(""); s << config.name << "_motor";
      MotorData ms(s.str(),MOTOR_TYPE_DC);
      ms.maxSpeed = 10.0;
      ms.maxEffort= 6.0;
      ms.p = 1;
      /*
      ms.Km = 0.007812;
      ms.Ra = 0.007812;
      */
      ms.jointIndex = jointID[1];
      ms.axis = 1;
      ms.value = 1;
      motorID = control->motors->addMotor(&ms);

      switch_motor_direction = false;
  
      assert(nodeID[0]);
      assert(nodeID[1]);
      assert(jointID[0]);
      assert(jointID[1]);
      assert(motorID);

      std::string groupName, dataName;
      //bool erg = control->nodes->getDataBrokerNames(attached_node, &groupName, &dataName);
      bool erg = control->nodes->getDataBrokerNames(nodeID[1], &groupName, &dataName);
      (void)erg;
      assert(erg);
      control->dataBroker->registerTimedReceiver(this, groupName, dataName,"mars_sim/simTimer",config.updateRate);


      if(config.only_ray){
        RayConfig cfg;
        s.str(""); s << config.name << "_ray_sensor";
        cfg.name = s.str();
        cfg.attached_node = nodeID[1];
        cfg.updateRate = config.updateRate;
        raySensor = dynamic_cast<RaySensor*>(control->sensors->createAndAddSensor("RaySensor",&cfg));
        assert(raySensor);
        rayID = raySensor->getID();
      }

      if(control->graphics) {
        hudElementStruct hudCam;
        hudCam.type            = HUD_ELEMENT_TEXTURE;
        hudCam.width           = 420;
        hudCam.height          = 280;
        hudCam.texture_width   = 420;
        hudCam.texture_height  = 280;
        hudCam.posx            = 40 + (hudCam.width * config.hud_pos); // aligned in a row
        hudCam.posy            = 30;
        hudCam.border_color[0] = 0.0;
        hudCam.border_color[1] = 0.58824;
        hudCam.border_color[2] = 0.0;
        hudCam.border_color[3] = 1.0;
        hudCam.border_width    = 5.0;

        unsigned int cam_id=0;
        if(config.show_cam)
          cam_id = control->graphics->addHUDElement(&hudCam);

        s.str(""); s << config.name << "_camera";
        cam_window_id = control->graphics->new3DWindow(0, true,0,0,s.str().c_str());

        gw = control->graphics->get3DWindow(cam_window_id);
        gw->setGrabFrames(false);
        if(gw) {
          gc = gw->getCameraInterface();
          gc->setViewport(0,0,cols,rows);
          gc->setFrustumFromRad(3.0/180.0*M_PI,30.0/180.0*M_PI,0.5,100);
          //gc->setFrustumFromRad(150.0/180.0*M_PI,90.0/180.0*M_PI,0.5,100); //Debug
          control->graphics->addGraphicsUpdateInterface((GraphicsUpdateInterface*)this);
        }

        if(config.show_cam)
          control->graphics->setHUDElementTextureRTT(cam_id, cam_window_id,false);
      }
      control->nodes->addNodeSensor(this);
    }

    ScanningSonar::~ScanningSonar(void){
      control->dataBroker->unregisterTimedReceiver(this, "*", "*","mars_sim/simTimer");
    }


    int ScanningSonar::getSensorData(double ** data) const {
      if(!gw) return 0;

      SimMotor *motor = control->motors->getSimMotor(motorID);
      //Quaternion q = motor->getJoint()->getAttachedNode2()->getRotation().inverse() * motor->getJoint()->getAttachedNode1()->getRotation();
      Quaternion q = motor->getJoint()->getAttachedNode()->getRotation().inverse() * motor->getJoint()->getAttachedNode(2)->getRotation();


      double bearing = mars::utils::getYaw(q);

      if(raySensor){
        std::vector<double> res = raySensor->getSensorData();
        (*data) = new double[res.size()+1];
        memcpy((*data)+1,&res[0],sizeof(double)*res.size());
        (*data)[0] = bearing;
        return res.size()+1;
      }

      (*data) = new double[(int)(config.maxDist/config.resolution)+1];
      double *res = (*data);
      int width, height;
      float *img_data;
      gw->getRTTDepthData(&img_data, width, height);


      res[0] = bearing;
      for(int i=0;i<config.maxDist/config.resolution;i++){
        res[i+1] = 0;
      }

      for(int x = 0; x < width; x++){
        for(int y = 0;y < height; y++){
          double dist = img_data[y+(x*height)];
          if((int)(dist/config.resolution)+1 < (int)(config.maxDist/config.resolution)+1)
            res[(int)(dist/config.resolution)+1]++;
        }
      }

      static int wth = width*height;
      for(int i=0;i<config.maxDist/config.resolution;i++){
        res[i+1]= std::min((res[i+1]/wth)*255.0*config.gain,255.0);
      }
      free(img_data);
      return config.maxDist/config.resolution;
    }

    void ScanningSonar::preGraphicsUpdate(void) {
    }

    void ScanningSonar::receiveData(const data_broker::DataInfo &info,
                                    const data_broker::DataPackage &package,
                                    int callbackParam) {
      CPP_UNUSED(info);
      if(dbPosIndices[0] == -1) {
        dbPosIndices[0] = package.getIndexByName("position/x");
        dbPosIndices[1] = package.getIndexByName("position/y");
        dbPosIndices[2] = package.getIndexByName("position/z");
        dbRotIndices[0] = package.getIndexByName("rotation/x");
        dbRotIndices[1] = package.getIndexByName("rotation/y");
        dbRotIndices[2] = package.getIndexByName("rotation/z");
        dbRotIndices[3] = package.getIndexByName("rotation/w");
      }
      package.get(dbPosIndices[0], &head_position.x());
      package.get(dbPosIndices[1], &head_position.y());
      package.get(dbPosIndices[2], &head_position.z());
      package.get(dbRotIndices[0], &head_orientation.x());
      package.get(dbRotIndices[1], &head_orientation.y());
      package.get(dbRotIndices[2], &head_orientation.z());
      package.get(dbRotIndices[3], &head_orientation.w());
      head_position +=config.pos_offset;
      head_orientation= head_orientation * config.ori_offset ;

      if(gc) {
        gc->updateViewportQuat(head_position.x(), head_position.y(), head_position.z(),head_orientation.x(), head_orientation.y(), head_orientation.z(), head_orientation.w());
      }
  
      SimMotor *motor = control->motors->getSimMotor(motorID);
      if(motor && config.ping_pong_mode)
        {
          Quaternion q = motor->getJoint()->getAttachedNode()->getRotation().inverse() * motor->getJoint()->getAttachedNode(2)->getRotation();
          double bearing = mars::utils::getYaw(q);

          bool range_switch = false;
          if(config.right_limit - config.left_limit >= 0)
            range_switch = true;
          if((range_switch && (bearing <= config.left_limit || bearing >= config.right_limit)) ||
             (!range_switch && (bearing <= config.left_limit && bearing >= config.right_limit)))
            {
              switch_motor_direction = true;
            }
          else if(switch_motor_direction)
            {
              switch_motor_direction = false;
              motor->setVelocity(motor->getVelocity() * -1.0);
            }        
        }
    }

    BaseConfig* ScanningSonar::parseConfig(ControlCenter *control,
                                           ConfigMap *config) {

      ScanningSonarConfig *cfg = new ScanningSonarConfig();

      unsigned int mapIndex = (*config)["mapIndex"];
      unsigned long attachedNodeID = (*config)["attached_node"];
      if(mapIndex) {
        attachedNodeID = control->loadCenter->getMappedID(attachedNodeID,
                                                          interfaces::MAP_TYPE_NODE,
                                                          mapIndex);
      }
      cfg->attached_node = attachedNodeID;

      ConfigMap::iterator it;
      ConfigMap::iterator it2;

      cfg->show_cam = config->get("show_cam", false);
      cfg->updateRate = config->get("rate", 0UL);

      if(cfg->show_cam) {
        if((it2 = it->second.find("hud_idx")) !=
           it->second.endMap())
          cfg->hud_pos = it2->second;
      }

      if((it = config->find("only_ray")) != config->end())
        cfg->only_ray = it->second;

      if((it = config->find("resolution")) != config->end())
        cfg->resolution = it->second;

      if((it = config->find("gain")) != config->end())
        cfg->gain = it->second;

      if((it = config->find("maxDistance")) != config->end())
        cfg->maxDist = it->second;

      if((it = config->find("internal_width")) != config->end())
        cfg->width = it->second;

      if((it = config->find("internal_height")) != config->end())
        cfg->height = it->second;

      if((it = config->find("position")) != config->end()) {
        cfg->position[0] = it->second["x"];
        cfg->position[1] = it->second["y"];
        cfg->position[2] = it->second["z"];
      }

      if((it = config->find("position_offset")) != config->end()) {
        cfg->pos_offset[0] = it->second["x"];
        cfg->pos_offset[1] = it->second["y"];
        cfg->pos_offset[2] = it->second["z"];
      }

      if((it = config->find("extension")) != config->end()) {
        cfg->extension[0] = it->second["x"];
        cfg->extension[1] = it->second["y"];
        cfg->extension[2] = it->second["z"];
      }

      if((it = config->find("orientation_offset")) != config->end()) {
        if(it->second.hasKey("yaw")) {
          Vector euler;
          euler.x() = it->second["roll"];
          euler.y() = it->second["pitch"];
          euler.z() = it->second["yaw"];
          cfg->ori_offset *= eulerToQuaternion(euler);
        }
        else {
          Quaternion q;
          q.x() = it->second["x"];
          q.y() = it->second["y"];
          q.z() = it->second["z"];
          q.w() = it->second["w"];
          cfg->ori_offset *= q;
        }
      }

      if((it = config->find("orientation")) != config->end()) {
        if(it->second.hasKey("yaw")) {
          Vector euler;
          euler.x() = it->second["roll"];
          euler.y() = it->second["pitch"];
          euler.z() = it->second["yaw"];
          cfg->orientation = eulerToQuaternion(euler);
        }
        else {
          cfg->orientation.x() = it->second["x"];
          cfg->orientation.y() = it->second["y"];
          cfg->orientation.z() = it->second["z"];
          cfg->orientation.w() = it->second["w"];
        }
      }

      return cfg;
    }

    ConfigMap ScanningSonar::createConfig(){
      return ConfigMap();
    }

    void ScanningSonar::setPingPongConfig(bool ping_pong_mode, float left_limit, float right_limit)
    {
      config.ping_pong_mode = ping_pong_mode;
      config.left_limit = left_limit;
      config.right_limit = right_limit;
    }

  } // end of namespace sim
} // end of namespace mars
