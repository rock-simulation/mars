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

#ifndef MARS_INTERFACES_LIGHT_DATA_H
#define MARS_INTERFACES_LIGHT_DATA_H

#include <mars/utils/Color.h>
#include <configmaps/ConfigData.h>
#include <mars/utils/Vector.h>

#include <string>

namespace mars {
  namespace interfaces {

    // forward declaration
    class LoadCenter;

    /**
     * LightData is a struct to exchange light information between the
     * Dialog_Add_Light, the MainWindow and the osg widget
     */
    class LightData {
    public:
      bool fromConfigMap(configmaps::ConfigMap *config, std::string filenamePrefix,
                         LoadCenter *loadCenter = 0);
      void toConfigMap(configmaps::ConfigMap *config,
                       bool skipFilenamePrefix = false);
      void getFilesToSave(std::vector<std::string> *fileList);

      unsigned int index;
      std::string name; //light name
      utils::Vector pos; //light position
      utils::Vector lookAt; //light direction
      utils::Color ambient; //color of the ambient part of the light
      utils::Color diffuse; //color of the diffuse part of the light
      utils::Color specular; //specular color of the light
      double constantAttenuation;
      double linearAttenuation;
      double quadraticAttenuation;
      int type;
      double angle; //angle of the spot cutoff
      double exponent; //spot exponent
      bool directional; //directional light
      std::string node; // where the light should be attached
      unsigned long drawID;
      configmaps::ConfigMap map;
    }; // end of struct LightData

  } // end of namespace interfaces
} // end of namespace mars

#endif // MARS_INTERFACES_LIGHT_DATA_H
