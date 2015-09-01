/*
 *  Copyright 2013, DFKI GmbH Robotics Innovation Center
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
 * \file Viz.h
 * \author Malte Langosz
 *
 */

#ifndef MARS_VIZ_H
#define MARS_VIZ_H


#ifdef _PRINT_HEADER_
  #warning "Viz.h"
#endif

#include <lib_manager/LibInterface.hpp>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/NodeData.h>
#include <mars/data_broker/ReceiverInterface.h>

namespace mars {

  namespace viz {

    struct ForwardTransform {
      utils::Vector anchor;
      utils::Vector relPos;
      utils::Vector axis;
      utils::Quaternion q;
      double value, offset;
      unsigned long id;
      unsigned long jointId;
      bool linear;
      std::string name;
    };

    void exit_main(int signal);

    class Viz : public lib_manager::LibInterface,
                public data_broker::ReceiverInterface {
    public:
      Viz();
      Viz(lib_manager::LibManager *theManager);
      virtual ~Viz();

      // LibInterface methods
      virtual int getLibVersion() const
      { return 1; }
      virtual const std::string getLibName() const
      { return "mars_viz"; }
      CREATE_MODULE_INFO();

      void init(bool createWindow=true);

      void loadScene(std::string filename, std::string robotname="");
      void setJointValue(std::string jointName, double value);
      void setJointValue(unsigned int controllerIdx, double value);
      void setNodePosition(const std::string &nodeName,
                           const utils::Vector &pos);
      void setNodePosition(const unsigned long &id, const utils::Vector &pos);
      void setNodeOrientation(const std::string &nodeName,
                              const utils::Quaternion &q);
      void setNodeOrientation(const unsigned long &id,
                              const utils::Quaternion &q);

      interfaces::GraphicsManagerInterface *graphics;

      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);


    private:
      std::string configDir;

      std::map<unsigned long, interfaces::NodeData> nodeMapById;
      std::map<std::string, interfaces::NodeData> nodeMapByName;
      std::map<std::string, ForwardTransform> jointMapByName;
      std::map<unsigned long, ForwardTransform*> jointMapById;
      std::map<unsigned long, ForwardTransform*> jointMapByNodeId;
      std::vector<ForwardTransform*> jointByControllerIdx;
      interfaces::ControlCenter *control;

      void setJointValue(ForwardTransform *joint, double value);

    };

  } // end of namespace viz
} // end of namespace mars

#endif // end of namespace MARS_VIZ_H
