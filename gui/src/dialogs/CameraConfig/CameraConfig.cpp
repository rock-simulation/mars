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
 * \file CameraConfig.cpp
 * \author Malte R�mmermann
 * \brief "CameraConfig"
 **/

#include "CameraConfig.h"

#include <mars/utils/mathUtils.h>
#include <cstdio>

#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>

using namespace std;

namespace mars {
  namespace gui {

    CameraConfig::CameraConfig(interfaces::ControlCenter *c) {
      offset_rot.setIdentity();
      node_rot.setIdentity(); 
      control = c;
      gc = 0;
      lock_camera = false;
      lock_rotation = false;
      node_id = 0;
      offset_euler = (utils::sRotation) {0.0, 0.0, 0.0};
      update_ticks = 0;

      dbMapping.add("position/x", &node_pos[0]);
      dbMapping.add("position/y", &node_pos[1]);
      dbMapping.add("position/z", &node_pos[2]);
      dbMapping.add("rotation/x", &node_rot.x());
      dbMapping.add("rotation/y", &node_rot.y());
      dbMapping.add("rotation/z", &node_rot.z());
      dbMapping.add("rotation/w", &node_rot.w());
    }

    CameraConfig::~CameraConfig() {}
  
    void CameraConfig::updateCamera(void) {
      if(lock_camera) {
        utils::Quaternion q;
        q.setIdentity();
        utils::Vector p;
        dataMutex.lock();
        if(lock_rotation) {
          q = offset_rot;
          p = offset_pos;
        }
        else {
          q = node_rot*offset_rot;
          //p = QVRotate(node_rot, offset_pos);
          p = (node_rot * offset_pos);
        }
        p += node_pos;
        dataMutex.unlock();
        gc->updateViewportQuat(p.x(), p.y(), p.z(),
                               q.x(), q.y(), q.z(), q.w());
        if(++update_ticks > 10) setLockID(node_id);
      }
    }

    void CameraConfig::setWindowID(unsigned long id) {
      interfaces::GraphicsWindowInterface *gw;
      gw = control->graphics->get3DWindow(id);
  
      if(gw) {
        gc = gw->getCameraInterface();
      }
      win_id = id;
      char text[32];
      sprintf(text, "camera_config%lu.txt", win_id);
      loadConfig(std::string(text));
    }

    void CameraConfig::setLockID(unsigned long node_id) {
      std::string groupName, dataName;
      if(lock_camera) {
        control->nodes->getDataBrokerNames(this->node_id, &groupName, &dataName);
        control->dataBroker->unregisterSyncReceiver(this, groupName, dataName);
        if(node_id) {
          control->nodes->getDataBrokerNames(node_id, &groupName, &dataName);
          control->dataBroker->registerSyncReceiver(this, groupName, dataName);
        }
      }
      this->node_id = node_id;
    }

    bool CameraConfig::setLockCamera(bool option) {
      if(lock_camera != option && node_id) {
        std::string groupName, dataName;
        control->nodes->getDataBrokerNames(node_id, &groupName, &dataName);
        if(option) {
          control->dataBroker->registerSyncReceiver(this, groupName, dataName);
        }
        else {
          control->dataBroker->unregisterSyncReceiver(this, groupName, dataName);
        }
        lock_camera = option;
      }
      return lock_camera;
    }

    void CameraConfig::setLockRotation(bool option) {
      lock_rotation = option;
    }

    void CameraConfig::receiveData(const data_broker::DataInfo &info,
                                   const data_broker::DataPackage &package,
                                   int callbackParam) {
      std::string groupName, dataName;
      control->nodes->getDataBrokerNames(node_id, &groupName, &dataName);
      dataMutex.lock();
      if(info.groupName == groupName && info.dataName == dataName) {
        dbMapping.readPackage(package);
        update_ticks = 0;
      }
      dataMutex.unlock();
      updateCamera();
    }


    void CameraConfig::saveConfig(std::string filename) {
      FILE* file = fopen(filename.data(), "w");

      std::vector<double> frt = getFrustumSettings();

      if(file) {
        fprintf(file, "window_id: %lu\n", win_id);
        fprintf(file, "node_id: %lu\n", node_id);
        fprintf(file, "lock_camera: %d\n", lock_camera);
        fprintf(file, "lock_rotation: %d\n", lock_rotation);
        fprintf(file, "offset_pos: %g %g %g\n", offset_pos.x(),
                offset_pos.y(), offset_pos.z());
        fprintf(file, "offset_euler: %g %g %g\n", offset_euler.alpha,
                offset_euler.beta, offset_euler.gamma);
        fprintf(file, "offset_rot: %g %g %g %g\n", offset_rot.x(),
                offset_rot.y(), offset_rot.z(), offset_rot.w());
        fprintf(file, "frustum: %g %g %g %g %g %g\n", frt.at(0), frt.at(1),
                frt.at(2), frt.at(3), frt.at(4), frt.at(5));
        fclose(file);
      }
    }

    void CameraConfig::loadConfig(std::string filename) {
      FILE* file = fopen(filename.data(), "r");

      std::vector<double> frt;

      if(file) {
        int b;
        unsigned long tmp_id;
        fscanf(file, "window_id: %lu\n", &tmp_id);
        fscanf(file, "node_id: %lu\n", &node_id);
        fscanf(file, "lock_camera: %d\n", &b);
        lock_camera = b;
        fscanf(file, "lock_rotation: %d\n", &b);
        lock_rotation = b;
        fscanf(file, "offset_pos: %lf %lf %lf\n", &(offset_pos.x()),
               &(offset_pos.y()), &(offset_pos.z()));
        fscanf(file, "offset_euler: %lf %lf %lf\n", &(offset_euler.alpha),
               &(offset_euler.beta), &(offset_euler.gamma));
        fscanf(file, "offset_rot: %lf %lf %lf %lf\n", &(offset_rot.x()),
               &(offset_rot.y()), &(offset_rot.z()), &(offset_rot.w()));
        fscanf(file, "frustum: %lf %lf %lf %lf %lf %lf\n", &(frust.left), &(frust.right),
               &(frust.bottom), &(frust.top), &(frust.z_near), &(frust.z_far));

        frt.push_back(frust.left);
        frt.push_back(frust.right);
        frt.push_back(frust.bottom);
        frt.push_back(frust.top);
        frt.push_back(frust.z_near);
        frt.push_back(frust.z_far);

        setFrustum(frt);

        fclose(file);
      }
    }

    void CameraConfig::setOffsetPos(utils::Vector pos) {
      offset_pos = pos;
    }

    void CameraConfig::setOffsetRot(utils::sRotation rot) {
      offset_euler = rot;
      //offset_rot = Quaternion(offset_euler);
      offset_rot = utils::eulerToQuaternion(offset_euler);
    }


    void CameraConfig::setFrustum(std::vector<double> frustum) {

      if(gc)
        {
          gc->setFrustum(frustum.at(0),
                         frustum.at(1),
                         frustum.at(2),
                         frustum.at(3),
                         frustum.at(4),
                         frustum.at(5));
        }

      /*
       * OUTDATED CODE, MAY BE ERASED
       *
       frust.left = frustum[0];
       frust.right = frustum[1];
       frust.bottom = frustum[2];
       frust.top = frustum[3];
       frust.z_near = frustum[4];
       frust.z_far = frustum[5];
      */
    }


    std::vector<double> CameraConfig::getFrustumSettings(void) {

      std::vector<double> frustum;

      if(gc)
        {
          gc->getFrustum(frustum);
        }

      /*
       * OUTDATED CODE, MAY BE ERASED
       *
       frustum.push_back(frust.left);
       frustum.push_back(frust.right);
       frustum.push_back(frust.bottom);
       frustum.push_back(frust.top);
       frustum.push_back(frust.z_near);
       frustum.push_back(frust.z_far);
      */

      return frustum;
    }

  } // end of namespace gui
} // end of namespace mars
