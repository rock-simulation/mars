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
 * \file CameraConfig.h
 * \author Malte R�mmermann
 * \brief "CameraConfig"
 **/

#ifndef CAMERA_CONFIG_H
#define CAMERA_CONFIG_H

#ifdef _PRINT_HEADER_
#warning "CameraConfig.h"
#endif

#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/data_broker/DataPackageMapping.h>

#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>
#include <QMutex>

namespace mars {

  namespace interfaces {
    class GraphicsCameraInterface;
  }

  namespace gui {


    struct Frustum {
      double left;
      double right;
      double bottom;
      double top;
      double z_near;
      double z_far;
    };

    class CameraConfig : public data_broker::ReceiverInterface {

    public:
      CameraConfig(interfaces::ControlCenter *c);
      ~CameraConfig();
  
      void updateCamera(void);
      void setWindowID(unsigned long id);
      inline unsigned long getWindowID(void) {return win_id;}
      void setLockID(unsigned long node_id);
      inline unsigned long getLockID(void) {return node_id;}
      bool setLockCamera(bool option);
      inline bool getLockCamera(void) {return lock_camera;}
      void setLockRotation(bool option);
      inline bool getLockRotation(void) {return lock_rotation;}
      void saveConfig(std::string filename);
      void loadConfig(std::string filename);
      void setOffsetPos(utils::Vector pos);
      inline const utils::Vector* getOffsetPos(void) {return &offset_pos;}
      void setOffsetRot(utils::sRotation rot);
      inline const utils::sRotation* getOffsetRot(void) {return &offset_euler;}
  
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);

      std::vector<double> getFrustumSettings(void);
      void setFrustum(std::vector<double> frustum);

    private:
      interfaces::ControlCenter* control;
      interfaces::GraphicsCameraInterface* gc;
      utils::Vector offset_pos;
      utils::Quaternion offset_rot;
      utils::sRotation offset_euler;
      utils::Vector node_pos;
      utils::Quaternion node_rot;
      QMutex dataMutex;
      Frustum frust;


      bool lock_camera, lock_rotation;
      unsigned int update_ticks;
      unsigned long node_id;
      unsigned long win_id;
      data_broker::DataPackageMapping dbMapping;
    };

  } // end of namespace gui
} // end of namespace mars

#endif // CAMERA_CONFIG_H
