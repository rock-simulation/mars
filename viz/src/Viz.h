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

#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/NodeData.h>

namespace mars {

  namespace lib_manager {
    class LibManager;
  }

  namespace interfaces {
    class GraphicsManagerInterface;
  }

  namespace graphics {
    class GraphicsManager;
  }

  namespace cfg_manager {
    class CFGManager;
  }


  namespace viz {

    struct ForwardTransform {
      utils::Vector anchor;
      utils::Vector relPos;
      utils::Vector axis;
      utils::Quaternion q;
      double angle, offset;
      unsigned long id;
    };

    class GraphicsTimer;

    void exit_main(int signal);

    class Viz {
    public:
      Viz();
      Viz(lib_manager::LibManager *theManager);
      ~Viz();

      static interfaces::ControlCenter *control;

      void start(int argc, char **argv);

      void loadScene(std::string filename);
      void setJointAngle(std::string jointName, double angle);
      void setNodePosition(const std::string &nodeName,
                           const utils::Vector &pos);
      void setNodePosition(const unsigned long &id, const utils::Vector &pos);
      void setNodeOrientation(const std::string &nodeName,
                              const utils::Quaternion &q);
      void setNodeOrientation(const unsigned long &id,
                              const utils::Quaternion &q);

      interfaces::GraphicsManagerInterface *graphics;


    private:
      std::string configDir;
      lib_manager::LibManager *libManager;

      std::map<unsigned long, interfaces::NodeData> nodeMapById;
      std::map<std::string, interfaces::NodeData> nodeMapByName;
      std::map<std::string, ForwardTransform> joints;
      graphics::GraphicsManager *gM;
      cfg_manager::CFGManager *c;
    };

  } // end of namespace viz
} // end of namespace mars

#endif // end of namespace MARS_VIZ_H
