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
 * \file Viz.cpp
 * \author Malte Langosz
 *
 */

#include "Viz.h"

#include <lib_manager/LibManager.hpp>
#include <lib_manager/LibInterface.hpp>
#include <mars/utils/misc.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>
#include <mars/scene_loader/Load.h>
#include <mars/interfaces/utils.h>
#include <mars/interfaces/JointData.h>
#include <mars/interfaces/MotorData.h>
#include <mars/interfaces/ControllerData.h>
#include <mars/interfaces/terrainStruct.h>
#include <mars/utils/mathUtils.h>
#include <QWidget>
#include <mars/main_gui/MainGUI.h>

#ifdef WIN32
  #include <Windows.h>
#endif

#include <getopt.h>
#include <signal.h>

namespace mars {

  using namespace interfaces;

  namespace viz {

    void exit_main(int signal) {
#ifndef WIN32
      if(signal == SIGPIPE) {
        return;
      }
#endif
      if (signal) {
        fprintf(stderr, "\nI think we exit with an error! Signal: %d\n", signal);
        exit(-1);
      } else {
        fprintf(stderr, "\n################################\n");
        fprintf(stderr, "## everything closed fine ^-^ ##\n");
        fprintf(stderr, "################################\n\n");
      }
    }

    Viz::Viz() : lib_manager::LibInterface(new lib_manager::LibManager()),
                 configDir(".") {
#ifdef WIN32
      // request a scheduler of 1ms
      timeBeginPeriod(1);
#endif //WIN32
    }

    Viz::Viz(lib_manager::LibManager *theManager) : lib_manager::LibInterface(theManager),
                                                    configDir("."){
#ifdef WIN32
      // request a scheduler of 1ms
      timeBeginPeriod(1);
#endif //WIN32
    }

    Viz::~Viz() {
      //! close simulation
      exit_main(0);

      libManager->releaseLibrary("mars_graphics");
      libManager->releaseLibrary("cfg_manager");

      libManager->clearLibraries();

      delete libManager;


#ifdef WIN32
      // end scheduler of 1ms
      timeEndPeriod(1);
#endif //WIN32

    }

    void Viz::init(bool createWindow) {

      std::string coreConfigFile = configDir+"/core_libs.txt";

      // then check locals
      setlocale(LC_ALL,"C");

      // Test if current locale supports ENGLISH number interpretation
      struct lconv *locale = localeconv();
      fprintf(stderr, "Active locale (LC_ALL): ");
      if( *(locale->decimal_point) != '.') {
        fprintf(stderr, " [FAIL] Current locale conflicts with mars\n");
        exit(1);
      } else {
        fprintf(stderr, " [OK]\n");
      }

      // load the simulation core_libs:
      libManager->addLibrary(this);
      libManager->loadConfigFile(coreConfigFile);

      mars::cfg_manager::CFGManagerInterface *cfg;
      cfg = libManager->getLibraryAs<mars::cfg_manager::CFGManagerInterface>("cfg_manager", true);
      if(!cfg) {
        fprintf(stderr, "can not load needed library \"cfg_manager\".\n");
        exit(-1);
      }
      if(cfg) {
        cfg_manager::cfgPropertyStruct prop;
        prop = cfg->getOrCreateProperty("Config", "config_path",
                                              configDir);
        prop.sValue = configDir;
        cfg->setProperty(prop);
        cfg->getOrCreateProperty("Graphics", "backfaceCulling", false);
        cfg->getOrCreateProperty("Graphics", "num multisamples", 0);
        std::string configFile = configDir+"/mars_Preferences.yaml";
        cfg->loadConfig(configFile.c_str());
      }

      graphics = libManager->getLibraryAs<GraphicsManagerInterface>("mars_graphics", true);
      if(!graphics) {
        libManager->loadLibrary("cfg_manager");
        graphics = libManager->getLibraryAs<GraphicsManagerInterface>("mars_graphics");
        if(!graphics) {
          fprintf(stderr, "can not load needed library \"mars_graphics\".\n");
          exit(-1);
        }
      }

      graphics->initializeOSG(NULL, createWindow);
      graphics->hideCoords();

      coreConfigFile = configDir+"/other_libs.txt";
      control = new ControlCenter();
      control->loadCenter = new LoadCenter();

      control->dataBroker = libManager->getLibraryAs<data_broker::DataBrokerInterface>("data_broker");
      if(control->dataBroker) {
        ControlCenter::theDataBroker = control->dataBroker;
      }

      libManager->loadConfigFile(coreConfigFile);

      mars::main_gui::MainGUI *mainGui = NULL;
      mainGui = libManager->getLibraryAs<mars::main_gui::MainGUI>("main_gui");

      void *widget = graphics->getQTWidget(1);
      if(widget) {
        if(mainGui) {
          mainGui->mainWindow_p()->setCentralWidget((QWidget*)widget);
        }
        ((QWidget*)widget)->show();
      }
      if(mainGui) mainGui->show();

    }

    void Viz::loadScene(std::string filename, std::string robotname) {

      std::string tmpPath = "./tmp/";
      scene_loader::Load load(filename, control, tmpPath, "");
      load.prepareLoad();
      load.parseScene();

      std::map<std::string, ForwardTransform> jointsI;
      std::map<unsigned long, NodeData> nodeMapI;
      std::map<unsigned long, NodeData> nodeMapReady;
      std::map<unsigned long, NodeData>::iterator it1;
      std::map<unsigned long, NodeData>::iterator it2;
      std::map<unsigned long, NodeData>::iterator it3;
      std::list<JointData> jointList;
      std::list<JointData>::iterator jointIt;

      for(unsigned int i=0; i<load.jointList.size(); ++i) {
        JointData joint;
        joint.fromConfigMap(&load.jointList[i], tmpPath, NULL);
        jointList.push_back(joint);
      }

      for(unsigned int i=0; i<load.materialList.size(); ++i) {
        MaterialData material;
        unsigned long id;
        material.fromConfigMap(&load.materialList[i], tmpPath);
        if((id = load.materialList[i]["id"])) {
          load.materials[id] = material;
        }
      }

      for(unsigned int i=0; i<load.nodeList.size(); ++i) {
        NodeData node;
        node.fromConfigMap(&load.nodeList[i], tmpPath, NULL);

        if(node.terrain) {
          graphics->getLoadHeightmapInterface()->readPixelData(node.terrain);
        }

        // handle material
        configmaps::ConfigMap::iterator it;
        if((it = load.nodeList[i].find("material_id")) != load.nodeList[i].end()) {
          unsigned long id = it->second;
          if(id) {
            std::map<unsigned long, MaterialData>::iterator it = load.materials.find(id);
            if(it != load.materials.end())
              node.material = it->second;
          }
        }
        nodeMapI[node.index] = node;
        nodeMapI[node.index].index = graphics->addDrawObject(node);
        if(node.terrain) {
          nodeMapI[node.index].pivot = utils::Vector(node.terrain->targetWidth*0.5,
                                                     node.terrain->targetHeight*0.5,
                                                     node.terrain->scale*0.0);
        }
      }

      bool done = false;
      while(!done) {
        done = true;
        for(it1=nodeMapI.begin(); it1!=nodeMapI.end(); ++it1) {
          if(it1->second.relative_id) {
            it2 = nodeMapReady.find(it1->second.relative_id);
            if(it2 != nodeMapReady.end()) {
              getAbsFromRel(it2->second, &it1->second);
              nodeMapReady[it1->first] = it1->second;
              nodeMapI.erase(it1);
              done = false;
              break;
            }
          }
          else {
            nodeMapReady[it1->first] = it1->second;
            nodeMapI.erase(it1);
            done = false;
            break;
          }
        }
      }

      assert(nodeMapI.empty());

      nodeMapReady.swap(nodeMapI);
      for(it1=nodeMapI.begin(); it1!=nodeMapI.end(); ++it1) {
        utils::Vector offset = it1->second.rot * it1->second.visual_offset_pos;
        it1->second.pos += offset;
        it1->second.rot = it1->second.rot * it1->second.visual_offset_rot;

        graphics->setDrawObjectPos(it1->second.index, it1->second.pos);
        graphics->setDrawObjectRot(it1->second.index, it1->second.rot);
      }

      // handle relations
      nodeMapReady.clear();
      it1 = nodeMapI.find(1);
      if(it1!=nodeMapI.end()) {
        nodeMapReady[it1->first] = it1->second;
        nodeMapI.erase(it1);

        it1 = nodeMapReady.begin();
        done = false;
        while(!done && !nodeMapI.empty()) {
          done = true;
          for(it1=nodeMapReady.begin(); it1!=nodeMapReady.end(); ++it1) {
            it2 = nodeMapI.begin();
            while(it2 != nodeMapI.end()) {
              if(it2->second.groupID == it1->second.groupID) {
                NodeData node = it2->second;
                utils::Vector v = it1->second.pos - it1->second.rot*it1->second.pivot;
                node.pos = it1->second.rot.inverse() * (node.pos - v);
                node.rot = it1->second.rot.inverse() * node.rot;

                graphics->makeChild(it1->second.index, it2->second.index);
                graphics->setDrawObjectPos(node.index, node.pos);
                graphics->setDrawObjectRot(node.index, node.rot);

                nodeMapReady[it2->first] = it2->second;
                nodeMapI.erase(it2++);
                done = false;
              }
              else {
                bool connected = false;
                bool invert = false;
                for(jointIt=jointList.begin(); jointIt!=jointList.end();
                    ++jointIt) {
                  if((jointIt->nodeIndex1 == it1->first &&
                      jointIt->nodeIndex2 == it2->first)) {
                    connected = true;
                    done = false;
                    invert = true;
                    break;
                  }
                  else if((jointIt->nodeIndex2 == it1->first &&
                        jointIt->nodeIndex1 == it2->first)) {
                    connected = true;
                    done = false;
                    break;
                  }
                }
                if(connected) {
                  NodeData node = it2->second;
                  utils::Vector v = it1->second.pos - it1->second.rot*it1->second.pivot;
                  node.pos = it1->second.rot.inverse() * (node.pos - v);
                  node.rot = it1->second.rot.inverse() * node.rot;

                  // ToDo: - handle second axis for hing2 and universal
                  //       - handle if we have to invert the axis depending on
                  //         on the node order
                  ForwardTransform ft;
                  ft.name = jointIt->name;
                  if(jointIt->anchorPos == ANCHOR_NODE1) {
                    ft.anchor = it1->second.pos;
                  }
                  else if(jointIt->anchorPos == ANCHOR_NODE2) {
                    ft.anchor = it2->second.pos;
                  }
                  else if(jointIt->anchorPos == ANCHOR_CENTER) {
                    ft.anchor = (it1->second.pos + it2->second.pos) * 0.5;
                  }
                  else {
                    ft.anchor = jointIt->anchor;
                  }

                  ft.anchor = it1->second.rot.inverse() * (ft.anchor - v);
                  ft.relPos = node.pos - ft.anchor;
                  ft.axis = it1->second.rot.inverse() * jointIt->axis1;
                  ft.axis.normalize();
                  ft.q = node.rot;
                  if(invert) {
                    ft.axis *= -1;
                    ft.value = -jointIt->angle1_offset;
                    ft.offset = -jointIt->angle1_offset;
                  }
                  else {
                    ft.value = jointIt->angle1_offset;
                    ft.offset = jointIt->angle1_offset;
                  }
                  ft.id = node.index;
                  ft.jointId = jointIt->index;
                  if(jointIt->type == JOINT_TYPE_SLIDER) {
                    ft.linear = true;
                  }
                  else {
                    ft.linear = false;
                  }

                  jointMapByName[jointIt->name] = ft;
                  jointMapById[ft.jointId] = &jointMapByName[jointIt->name];
                  jointMapByNodeId[ft.id] = &jointMapByName[jointIt->name];
                  graphics->makeChild(it1->second.index, it2->second.index);
                  graphics->setDrawObjectPos(node.index, node.pos);
                  graphics->setDrawObjectRot(node.index, node.rot);
                  if(jointIt->type != JOINT_TYPE_FIXED) {
                    std::string packageName;
                    if(robotname.empty()) {
                      packageName = jointIt->name;
                    }
                    else {
                      packageName = robotname+"/"+jointIt->name;
                    }
                    data_broker::DataPackage dbPackage;
                    dbPackage.add("value", 0.0);
                    control->dataBroker->pushData("viz", packageName,
                                                  dbPackage, NULL,
                                                  data_broker::DATA_PACKAGE_READ_WRITE_FLAG);
                    control->dataBroker->registerSyncReceiver(this, "viz",
                                                              packageName,
                                                              ft.id);
                  }

                  nodeMapReady[it2->first] = it2->second;
                  nodeMapI.erase(it2++);
                }
                else ++it2;
              }
            }
            if(!done) break;
          }
        }
        std::map<unsigned long, MotorData> motorMapById;
        for(unsigned int i=0; i<load.motorList.size(); ++i) {
          MotorData motor;
          motor.fromConfigMap(&load.motorList[i], tmpPath, NULL);
          motorMapById[motor.index] = motor;
        }
        if(!load.controllerList.empty()) {
          ControllerData controller;
          controller.fromConfigMap(&load.controllerList[0], tmpPath, NULL);
          for(unsigned int i=0; i<controller.motors.size(); ++i) {
            jointByControllerIdx.push_back(jointMapById[motorMapById[controller.motors[i]].jointIndex]);
          }
        }
      }

      for(it1=nodeMapI.begin(); it1!=nodeMapI.end(); ++it1) {
        nodeMapById[it1->second.index] = it1->second;
        nodeMapByName[it1->second.name] = it1->second;
      }
    }


    void Viz::setJointValue(std::string jointName, double value) {
      std::map<std::string, ForwardTransform>::iterator it;
      it = jointMapByName.find(jointName);
      if(it!=jointMapByName.end()) {
        setJointValue(&jointMapByName[jointName], value);
      }
    }

    void Viz::setJointValue(unsigned int controllerIdx, double value) {
      assert(controllerIdx < jointByControllerIdx.size());
      setJointValue(jointByControllerIdx[controllerIdx], value);
    }

    void Viz::setJointValue(ForwardTransform *joint, double value) {
      joint->value = value;
      if(joint->linear) {
        utils::Vector v = joint->axis*joint->value + joint->relPos;
        graphics->setDrawObjectPos(joint->id, joint->anchor+v);
      }
      else {
        utils::Quaternion q = utils::angleAxisToQuaternion(joint->value+joint->offset,
                                                           joint->axis);
        utils::Vector v = q * joint->relPos;
        graphics->setDrawObjectPos(joint->id, joint->anchor+v);
        graphics->setDrawObjectRot(joint->id, q * joint->q);
      }
    }

    void Viz::setNodePosition(const std::string &nodeName, const utils::Vector &pos) {
      std::map<std::string, interfaces::NodeData>::iterator it;
      it=nodeMapByName.find(nodeName);
      if(it!=nodeMapByName.end()) {
        setNodePosition(nodeMapByName[nodeName].index, pos);
      }
    }

    void Viz::setNodePosition(const unsigned long &id, const utils::Vector &pos) {
      graphics->setDrawObjectPos(id, pos);
    }

    void Viz::setNodeOrientation(const std::string &nodeName, const utils::Quaternion &q) {
      std::map<std::string, interfaces::NodeData>::iterator it;
      it=nodeMapByName.find(nodeName);
      if(it!=nodeMapByName.end()) {
        setNodeOrientation(nodeMapByName[nodeName].index, q);
      }
    }

    void Viz::setNodeOrientation(const unsigned long &id, const utils::Quaternion &q) {
      graphics->setDrawObjectRot(id, q);
    }

    void Viz::receiveData(const data_broker::DataInfo& info,
                          const data_broker::DataPackage& package,
                          int id) {
      double value;
      package.get(0, &value);
      setJointValue(jointMapByNodeId[id], value);
      // package.get("force1/x", force);
    }

  } // end of namespace viz
} // end of namespace mars
