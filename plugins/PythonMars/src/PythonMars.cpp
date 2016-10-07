/*
 *  Copyright 2016, DFKI GmbH Robotics Innovation Center
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
 * \file PythonMars.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief
 *
 * Version 0.1
 */


#include "PythonMars.h"
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/sim/SensorManagerInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/data_broker/DataPackage.h>

namespace mars {

  using namespace osg_material_manager;

  namespace plugins {
    namespace PythonMars {

      using namespace configmaps;
      using namespace mars::utils;
      using namespace mars::interfaces;

      PythonMars::PythonMars(lib_manager::LibManager *theManager)
        : MarsPluginTemplateGUI(theManager, "PythonMars")      {
      }

      void PythonMars::init() {
        pf = new osg_points::PointsFactory();
        materialManager = libManager->getLibraryAs<OsgMaterialManager>("osg_material_manager", true);
        PythonInterpreter::instance().addToPythonpath("python");
        std::string resPath = control->cfg->getOrCreateProperty("Preferences",
                                                                "resources_path",
                                                                "../../share").sValue;
        resPath += "/PythonMars/python";
        PythonInterpreter::instance().addToPythonpath(resPath.c_str());
        pythonException = false;
        gui->addGenericMenuAction("../PythonMars/Reload", 1, this);
        try {
          plugin = PythonInterpreter::instance().import("mars_plugin");
          ConfigItem map;
          toConfigMap(plugin->function("init").call().returnObject(), map);
          interpreteMap(map);
        }
        catch(const std::exception &e) {
          LOG_FATAL("Error: %s", e.what());
          pythonException = true;
        }
        control->sim->switchPluginUpdateMode(PLUGIN_SIM_MODE | PLUGIN_GUI_MODE,
                                             this);
      }

      void PythonMars::interpreteMap(ConfigItem &map) {
        if(map.isMap()) {
          if(map.hasKey("startSim") && (bool)map["startSim"]) {
            control->sim->StartSimulation();
          }
          if(map.hasKey("stopSim") && (bool)map["stopSim"]) {
            control->sim->StopSimulation();
          }
          if(map.hasKey("log")) {
            if(map["log"].hasKey("debug")) {
              ConfigVector::iterator it = map["log"]["debug"].begin();
              for(; it!=map["log"]["debug"].end(); ++it) {
                LOG_DEBUG((std::string)*it);
              }
            }
            if(map["log"].hasKey("error")) {
              ConfigVector::iterator it = map["log"]["error"].begin();
              for(; it!=map["log"]["error"].end(); ++it) {
                LOG_ERROR((std::string)*it);
              }
            }
          }

          if(map.hasKey("commands")) {
            ConfigMap::iterator it = map["commands"].beginMap();
            for(; it!=map["commands"].endMap(); ++it) {
              std::string name = it->first;
              if(it->second.hasKey("value")) {
                double value = it->second["value"];
                if(motorMap.find(name) == motorMap.end()) {
                  unsigned long id = control->motors->getID(name);
                  if(id) {
                    motorMap[name] = id;
                    control->motors->setMotorValue(id, value);
                  }
                }
                else {
                  control->motors->setMotorValue(motorMap[name], value);
                }
              }
            }
          }

          if(map.hasKey("config")) {
            ConfigMap::iterator it = map["config"].beginMap();
            for(; it!=map["config"].endMap(); ++it) {
              std::string group = it->first;
              if(!it->second.isMap()) continue;
              ConfigMap::iterator it2 = it->second.beginMap();
              if(!it2->second.isAtom()) continue;
              ConfigAtom &atom = it2->second;
              std::string name = it2->first;
              std::string value = atom.toString().c_str();
              cfg_manager::cfgParamInfo info;
              info = control->cfg->getParamInfo(group, name);
              switch(info.type) {
              case cfg_manager::boolParam:
                control->cfg->setProperty(group, name,
                                          (bool)atoi(value.c_str()));
                break;
              case cfg_manager::doubleParam:
                control->cfg->setProperty(group, name, atof(value.c_str()));
                break;
              case cfg_manager::intParam:
                control->cfg->setProperty(group, name, atoi(value.c_str()));
                break;
              case cfg_manager::stringParam:
                control->cfg->setProperty(group, name, value);
                break;
              case cfg_manager::noParam:
                switch(atom.getType()) {
                case ConfigAtom::BOOL_TYPE:
                  control->cfg->getOrCreateProperty(group, name, (bool)atom);
                  break;
                case ConfigAtom::INT_TYPE:
                  control->cfg->getOrCreateProperty(group, name, (int)atom);
                  break;
                case ConfigAtom::DOUBLE_TYPE:
                  control->cfg->getOrCreateProperty(group, name, (double)atom);
                  break;
                default:
                  control->cfg->getOrCreateProperty(group, name,
                                                    atom.toString());
                  break;
                }
                break;
              default:
                break;
              }
            }
          }

          if(map.hasKey("PointCloud")) {
            ConfigMap::iterator it = map["PointCloud"].beginMap();
            for(; it!=map["PointCloud"].endMap(); ++it) {
              std::string name = it->first;
              if(points.find(name) != points.end()) {
                control->graphics->removeOSGNode(points[name]->getOSGNode());
                delete pointsData[name];
                points.erase(name);
                pointsData.erase(name);
                pointsSize.erase(name);
              }
              int size = it->second;
              // create point cloud
              osg_points::Points *p = pf->createPoints();
              p->setLineWidth(3.0);
              points[name] = p;
              double *data = new double[size*3];
              std::vector<osg_points::Vector> pV;
              pV.reserve(size);
              for(int i=0; i<size; ++i) {
                data[i*3] = ((double)i)/size*2;
                data[i*3+1] = (i%4)*0.1;
                data[i*3+2] = 1;
                pV.push_back(osg_points::Vector(data[i*3], data[i*3+1],
                                                data[i*3+2]));
              }
              p->setData(pV);
              pointsData[name] = data;
              pointsSize[name] = size;
              plugin->function("addPointCloudData").pass(STRING).pass(ONEDCARRAY).call(&name, data, size*3);
              control->graphics->addOSGNode(p->getOSGNode());
            }
          }

          if(map.hasKey("ConfigPointCloud")) {
            ConfigMap::iterator it = map["ConfigPointCloud"].beginMap();
            for(; it!=map["ConfigPointCloud"].endMap(); ++it) {
              if(points.find(it->first) == points.end()) continue;
              points[it->first]->setLineWidth(it->second[0]);
              osg_points::Color c((double)it->second[1], (double)it->second[2],
                                  (double)it->second[3], 1.0);
              points[it->first]->setColor(c);
            }
          }

          if(map.hasKey("request") && map["request"].isVector()) {
            requestMap = map["request"];
          }
        }
      }

      void PythonMars::reset() {
        motorMap.clear();
        //plugin->reload();
      }

      PythonMars::~PythonMars() {
      }


      void PythonMars::update(sReal time_ms) {
        mutex.lock();
        if(time_ms > 0) {
          if(pythonException) {
            mutex.unlock();
            return;
          }
          ConfigItem map;
          ConfigMap sendMap;

          ConfigVector::iterator it = requestMap.begin();
          for(; it!=requestMap.end(); ++it) {
            if(!it->hasKey("type")) continue;
            if(!it->hasKey("name")) continue;
            std::string type = (*it)["type"];
            std::string name = (*it)["name"];

            if(type == "Node") {
              unsigned long id = control->nodes->getID(name);
              Vector pos = control->nodes->getPosition(id);
              Quaternion rot = control->nodes->getRotation(id);
              sendMap["Nodes"][name]["pos"]["x"] = pos.x();
              sendMap["Nodes"][name]["pos"]["y"] = pos.y();
              sendMap["Nodes"][name]["pos"]["z"] = pos.z();
              sendMap["Nodes"][name]["rot"]["x"] = rot.x();
              sendMap["Nodes"][name]["rot"]["y"] = rot.y();
              sendMap["Nodes"][name]["rot"]["z"] = rot.z();
              sendMap["Nodes"][name]["rot"]["w"] = rot.w();
            }

            if(type == "Sensor") {
              unsigned long id = control->sensors->getSensorID(name);
              sReal *data;
              int num = control->sensors->getSensorData(id, &data);
              for(int i=0; i<num; ++i) {
                sendMap["Sensors"][name][i] = data[i];
              }
              if(num) free(data);
            }

            if(type == "Config") {
              if(!it->hasKey("group")) continue;
              std::string group = (*it)["group"];
              cfg_manager::cfgParamInfo info;
              info = control->cfg->getParamInfo(group, name);
              switch(info.type) {
              case cfg_manager::boolParam:
                {
                  bool v;
                  control->cfg->getPropertyValue(group, name, "value", &v);
                  sendMap["Config"][group][name] = v;
                  break;
                }
              case cfg_manager::doubleParam:
                {
                  double v;
                  control->cfg->getPropertyValue(group, name, "value", &v);
                  sendMap["Config"][group][name] = v;
                  break;
                }
              case cfg_manager::intParam:
                {
                  int v;
                  control->cfg->getPropertyValue(group, name, "value", &v);
                  sendMap["Config"][group][name] = v;
                  break;
                }
              case cfg_manager::stringParam:
                {
                  std::string v;
                  control->cfg->getPropertyValue(group, name, "value", &v);
                  sendMap["Config"][group][name] = v;
                  break;
                }
              default:
                break;
              }
            }

          }
          try {
            toConfigMap(plugin->function("update").pass(MAP).call(&sendMap).returnObject(), map);
            interpreteMap(map);
          }
          catch(const std::exception &e) {
            LOG_FATAL("Error: %s", e.what());
            pythonException = true;
          }
        }
        else {
          // udpate point clouds
          std::map<std::string, osg_points::Points*>::iterator it = points.begin();
          for(; it!=points.end(); ++it) {
            double *d = pointsData[it->first];
            int size = pointsSize[it->first];
            std::vector<osg_points::Vector> pV;
            pV.reserve(size);
            for(int i=0; i<size; ++i) {
              pV.push_back(osg_points::Vector(d[i*3], d[i*3+1], d[i*3+2]));
            }
            it->second->setData(pV);
          }
        }
        mutex.unlock();
        // control->motors->setMotorValue(id, value);
      }

      void PythonMars::receiveData(const data_broker::DataInfo& info,
                                    const data_broker::DataPackage& package,
                                    int id) {
        // package.get("force1/x", force);
      }

      void PythonMars::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {

        if(_property.paramId == example.paramId) {
          example.dValue = _property.dValue;
        }
      }

      void PythonMars::menuAction (int action, bool checked)
      {
        if(action == 1) {
          mutex.lock();
          pythonException = false;
          try {
            if(plugin)
              plugin->reload();
            else
              plugin = PythonInterpreter::instance().import("mars_plugin");
          }
          catch(const std::exception &e) {
            LOG_FATAL("Error: %s", e.what());
            plugin.reset();
            pythonException = true;
            mutex.unlock();
            return;
          }
          try {
            ConfigItem map;
            toConfigMap(plugin->function("init").call().returnObject(), map);
            interpreteMap(map);
          }
          catch(const std::exception &e) {
            LOG_FATAL("Error: %s", e.what());
            pythonException = true;
          }
          mutex.unlock();
        }
        //plugin_win->show ();
      }

    } // end of namespace PythonMars
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::PythonMars::PythonMars);
CREATE_LIB(mars::plugins::PythonMars::PythonMars);
