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
#include <mars/utils/misc.h>
#ifdef __unix__
#include <dlfcn.h>
#endif
namespace mars {

  using namespace osg_material_manager;

  namespace plugins {
    namespace PythonMars {

      using namespace configmaps;
      using namespace mars::utils;
      using namespace mars::interfaces;

      PythonMars::PythonMars(lib_manager::LibManager *theManager)
        : MarsPluginTemplateGUI(theManager, "PythonMars")      {
#ifdef __unix__
        // needed to be able to import numpy
        dlopen("libpython2.7.so.1", RTLD_LAZY | RTLD_GLOBAL);
#endif
      }

      PythonMars::~PythonMars() {
        if(materialManager) libManager->releaseLibrary("osg_material_manager");
      }

      void PythonMars::init() {
        std::string confPath = control->cfg->getOrCreateProperty("Config",
                                                                "config_path",
                                                                ".").sValue;
        ConfigMap map = ConfigMap::fromYamlFile(confPath + "/pypath.yml");
        if(map.hasKey("pypath")) {
          ConfigVector::iterator it = map["pypath"].begin();
          for(; it!=map["pypath"].end(); ++it) {
            PythonInterpreter::instance().addToPythonpath((std::string)*it);
          }
        }
        updateGraphics = false;
        nextStep = false;
        pf = new osg_points::PointsFactory();
        lf = new osg_lines::LinesFactory();
        materialManager = libManager->getLibraryAs<OsgMaterialManager>("osg_material_manager", true);
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
          interpreteGuiMaps();
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
            ConfigMap::iterator it = map.find("startSim");
            map.erase(it);
          }
          if(map.hasKey("stopSim") && (bool)map["stopSim"]) {
            control->sim->StopSimulation();
            ConfigMap::iterator it = map.find("stopSim");
            map.erase(it);
          }
          if(map.hasKey("updateTime")) {
            updateTime = map["updateTime"];
            ConfigMap::iterator it = map.find("updateTime");
            map.erase(it);
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
            ConfigMap::iterator it = map.find("log");
            map.erase(it);
          }

          if(map.hasKey("commands") && control->sim->isSimRunning()) {
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
            ConfigMap::iterator iit = map.find("commands");
            map.erase(iit);
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
            ConfigMap::iterator iit = map.find("config");
            map.erase(iit);
          }

          if(map.hasKey("request") && map["request"].isVector()) {
            requestMap = map["request"];
            ConfigMap::iterator it = map.find("request");
            map.erase(it);
          }

          guiMapMutex.lock();
          guiMaps.push_back(map);
          guiMapMutex.unlock();

        }
        nextStep = true;
      }

      void PythonMars::interpreteGuiMaps() {
        guiMapMutex.lock();
        std::vector<ConfigMap>::iterator it = guiMaps.begin();
        for(; it!=guiMaps.end(); ++it) {
          ConfigMap &map = *it;
          if(map.hasKey("PointCloud") && map["PointCloud"].isMap()) {
            mutex.lock();
            ConfigMap::iterator it = map["PointCloud"].beginMap();
            for(; it!=map["PointCloud"].endMap(); ++it) {
              std::string name = it->first;
              if(points.find(name) != points.end()) {
                PointStruct &point = points[name];
                control->graphics->removeOSGNode(point.p->getOSGNode());
                delete point.data;
                delete point.pydata;
                points.erase(name);
              }
              // create point cloud
              PointStruct point;
              point.p = pf->createPoints();
              point.p->setLineWidth(3.0);
              point.size = it->second;
              point.data = new double[point.size*3];
              point.pydata = new double[point.size*3];
              std::vector<osg_points::Vector> pV;
              pV.reserve(point.size);
              for(int i=0; i<point.size; ++i) {
                point.data[i*3] = ((double)i)/point.size*2;
                point.data[i*3+1] = (i%4)*0.1;
                point.data[i*3+2] = 1;
                pV.push_back(osg_points::Vector(point.data[i*3],
                                                point.data[i*3+1],
                                                point.data[i*3+2]));
              }
              points[name] = point;
              point.p->setData(pV);
              plugin->function("addPointCloudData").pass(STRING).pass(ONEDCARRAY).call(&name, point.pydata, point.size*3);
              control->graphics->addOSGNode(point.p->getOSGNode());
            }
            mutex.unlock();
          }
          if(map.hasKey("CameraSensor") && map["CameraSensor"].isMap()) {
            mutexCamera.lock();
            ConfigMap::iterator it = map["CameraSensor"].beginMap();
            for(; it!=map["CameraSensor"].endMap(); ++it) {
              std::string name = it->first;
              if(cameras.find(name) == cameras.end()) {
                unsigned long id = control->sensors->getSensorID(name);
                sReal *data;
                int num = control->sensors->getSensorData(id, &data);
                if(num) {
                  CameraStruct cam = {id, data, NULL, num};
                  cam.pydata = (sReal*)malloc(num*sizeof(sReal));
                  cameras[name] = cam;
                  plugin->function("addCameraData").pass(STRING).pass(ONEDCARRAY).call(&name, cam.pydata, num);
                }
              }
              else {
                CameraStruct &cam = cameras[name];
                sReal *data;
                int num = control->sensors->getSensorData(cam.id, &data);
                if(num == cam.size) {
                  memcpy(cam.data, data, num*sizeof(sReal));
                }
                if(num) free(data);
              }
            }
            mutexCamera.unlock();
          }

          if(map.hasKey("ConfigPointCloud")) {
            ConfigMap::iterator it = map["ConfigPointCloud"].beginMap();
            for(; it!=map["ConfigPointCloud"].endMap(); ++it) {
              if(points.find(it->first) == points.end()) continue;
              points[it->first].p->setLineWidth(it->second[0]);
              osg_points::Color c((double)it->second[1], (double)it->second[2],
                                  (double)it->second[3], 1.0);
              points[it->first].p->setColor(c);
            }
          }

          if(map.hasKey("Lines")) {
            ConfigMap::iterator it = map["Lines"].beginMap();
            for(; it!=map["Lines"].endMap(); ++it) {
              const std::string &name = it->first;
              ConfigVector::iterator it2 = it->second.begin();
              for(; it2!=it->second.end(); ++it2) {
                if(it2->isAtom()) {
                  if(lines.find(name) == lines.end()) continue;
                  std::string cmd = *it2;
                  if(cmd == "clear") {
                    lines[name].l->setData(std::list<osg_lines::Vector>());
                  }
                  else if(cmd == "remove") {
                    control->graphics->removeOSGNode(lines[name].l->getOSGNode());
                    lines.erase(name);
                  }
                }
                else {
                  ConfigMap &cmd = *it2;
                  if(lines.find(name) == lines.end()) {
                    LineStruct line;
                    line.l = lf->createLines();
                    line.l->setLineWidth(3.0);
                    line.l->drawStrip(false);
                    lines[name] = line;
                    control->graphics->addOSGNode(line.l->getOSGNode());
                  }
                  if(cmd.hasKey("append")) {
                    LineStruct &ls = lines[name];
                    osg_lines::Vector v((double)cmd["append"][0],
                                        (double)cmd["append"][1],
                                          (double)cmd["append"][2]);
                    ls.toAppend.push_back(v);
                  }
                  if(cmd.hasKey("config")) {
                    lines[name].l->setLineWidth((double)cmd["config"][0]);
                    osg_lines::Color c((double)cmd["config"][1],
                                       (double)cmd["config"][2],
                                       (double)cmd["config"][3], 1.0);
                    lines[name].l->setColor(c);
                  }
                }
              }
            }
          }
        }
        nextStep = true;
        guiMaps.clear();
        guiMapMutex.unlock();
      }

      void PythonMars::reset() {
        motorMap.clear();
        //plugin->reload();
      }

      void PythonMars::update(sReal time_ms) {
        static double nextUpdate = 0.0;
        if(time_ms > 0) {
          gpMutex.lock();
          if(updateTime > 0) {
            nextUpdate += time_ms;
            if(nextUpdate > updateTime) {
              nextUpdate = fmod(nextUpdate, updateTime);
            }
            else {
              gpMutex.unlock();
              return;
            }
          }
          if(pythonException) {
            gpMutex.unlock();
            return;
          }
          while(!nextStep) utils::msleep(2);
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
            iMap = ConfigItem();
            mutexCamera.lock();
            { // udpate point clouds
              std::map<std::string, CameraStruct>::iterator it = cameras.begin();
              for(; it!=cameras.end(); ++it) {
                memcpy(it->second.pydata, it->second.data,
                       it->second.size*sizeof(sReal));
              }
            }
            mutexCamera.unlock();
            mutex.lock();
            toConfigMap(plugin->function("update").pass(MAP).call(&sendMap).returnObject(), iMap);
            nextStep = true;
            mutex.unlock();
            mutexPoints.lock();
            { // udpate point clouds
              std::map<std::string, PointStruct>::iterator it = points.begin();
              for(; it!=points.end(); ++it) {
                memcpy(it->second.data, it->second.pydata,
                       it->second.size*3*sizeof(double));
              }
            }
            mutexPoints.unlock();
            interpreteMap(iMap);
          }
          catch(const std::exception &e) {
            LOG_FATAL("Error: %s", e.what());
            pythonException = true;
            mutex.unlock();
          }
          updateGraphics = true;
          gpMutex.unlock();
        }
        else {
          if(updateGraphics) {
            interpreteGuiMaps();
            mutexPoints.lock();
            { // udpate point clouds
              std::map<std::string, PointStruct>::iterator it = points.begin();
              for(; it!=points.end(); ++it) {
                std::vector<osg_points::Vector> pV;
                pV.reserve(it->second.size);
                for(int i=0; i<it->second.size; ++i) {
                  pV.push_back(osg_points::Vector(it->second.data[i*3],
                                                  it->second.data[i*3+1],
                                                  it->second.data[i*3+2]));
                }
                it->second.p->setData(pV);
              }
            }
            mutexPoints.unlock();

            { // update lines
              std::map<std::string, LineStruct>::iterator it = lines.begin();
              for(; it!=lines.end(); ++it) {
                std::vector<osg_lines::Vector>::iterator vt;
                for(vt=it->second.toAppend.begin();
                    vt!=it->second.toAppend.end(); ++vt) {
                  it->second.l->appendData(*vt);
                }
                it->second.toAppend.clear();
              }
            }
            updateGraphics = false;
          }
        }
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
          gpMutex.lock();
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
            gpMutex.unlock();
            return;
          }
          try {
            ConfigItem map;
            toConfigMap(plugin->function("init").call().returnObject(), map);
            interpreteMap(map);
            interpreteGuiMaps();
          }
          catch(const std::exception &e) {
            LOG_FATAL("Error: %s", e.what());
            pythonException = true;
          }
          gpMutex.unlock();
        }
        //plugin_win->show ();
      }

    } // end of namespace PythonMars
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::PythonMars::PythonMars);
CREATE_LIB(mars::plugins::PythonMars::PythonMars);
