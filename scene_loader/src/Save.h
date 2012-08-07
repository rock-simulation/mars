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

#ifndef SAVE_H
#define SAVE_H

#ifdef _PRINT_HEADER_
#warning "Save.h"
#endif

#include <string>
#include <vector>

#include <mars/interfaces/sim/ControlCenter.h>

#include "zipit.h"
#include "SaveLoadStructs.h"

class QTextStream;

namespace mars {
  namespace scene_loader {

    class Save {
    public:
      Save(std::string filename, interfaces::ControlCenter* c,
           std::string tmpPath);
      unsigned int prepare();
      unsigned int save();

    private:
      std::vector<material_map> materials;
      unsigned long next_material_id;

      unsigned int generate(std::vector<const interfaces::BaseSensor*> *v_BaseSensor,QTextStream * out);

      unsigned int generate(interfaces::MaterialData *v_materialStruct,
                            QTextStream *out,
                            std::vector<std::string> *v_filenames,
                            unsigned long id);
      unsigned int generate(interfaces::NodeData *nodeData, QTextStream *out,
                            std::vector<std::string> *v_filenames);
      unsigned int generate(interfaces::JointData *jointData, QTextStream *out,
                            std::vector<std::string> *v_filenames);
      unsigned int generate(interfaces::MotorData *motorData, QTextStream *out,
                            std::vector<std::string> *v_filenames);
      unsigned int generate(interfaces::LightData *lightData, QTextStream *out,
                            std::vector<std::string> *v_filenames);
      unsigned int generate(interfaces::GraphicData *graphicData,
                            QTextStream *out,
                            std::vector<std::string> *v_filenames);
      unsigned int generate(interfaces::ControllerData *controller,
                            QTextStream *out,
                            std::vector<std::string> *v_filenames);


      unsigned int generate(QTextStream * out);

      //i_ErrorMsg * p_msg;
      interfaces::ControlCenter* control;
      saveStruct_t param;
      std::string s_tmpDirectory;
      std::string s_pathname;
      std::string s_filename;
  
      void writeConfigMap(const utils::ConfigMap &cfg, QTextStream * out,
                          std::string tag, bool handleSensor = false,
                          int depth = 0);

    };

  } // end of namespace scene_loader
} // end of namespace mars

#endif
