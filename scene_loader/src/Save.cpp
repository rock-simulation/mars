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
 *
 * \file Save.cpp
 * \author Malte Roemmermann
 * this class stores the internal datas of every object in a xml-file.
 * afterwords the xml-file gets pushed in to the archive and the original file
 * on the harddisk is deleted.
 * the xml-file includes all necessary information to reload the scene.
 *
 * this class also generates the zipfiles, containig the 3d-files
 *
 * all functions return 0 if succeed, otherwise 1
 *
 * the structure of the generated xml-file is like:
 * <SceneFile>
 *  <node Name=STRING>
 *  ... node entries
 *  </node>
 *  <any other entries>
 * ... any other entires
 *  </any other entries>
 * </SceneFile>
 */

#include <QFile>
#include <QTextStream>

#include "Save.h"

#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/sim/SensorManagerInterface.h>
#include <mars/interfaces/sim/ControllerManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsCameraInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/interfaces/sensor_bases.h>
#include <mars/interfaces/utils.h>
#include <mars/interfaces/Logging.hpp>

#include <QFile>

#include <mars/utils/misc.h>

namespace mars {
  namespace scene_loader {

    using namespace interfaces;

    Save::Save(std::string filename, ControlCenter* c, std::string tmpPath) :
      control(c), s_tmpDirectory(tmpPath) {

      if(!utils::pathExists(s_tmpDirectory)) {
        LOG_INFO("Save:: given tmp dir \"%s\" does not exist!", s_tmpDirectory.c_str());
        if (!utils::createDirectory(s_tmpDirectory)) {
          LOG_FATAL("Save:: tmp dir \"%s\" could also not be created!", s_tmpDirectory.c_str());
          throw std::runtime_error("Invalid path for Saveis given at __FILE__:__LINE__");
        }
        else {
          LOG_INFO("Save:: created tmp dir \"%s\".", s_tmpDirectory.c_str());
        }
      }
      s_filename = filename;
      utils::removeFilenamePrefix(&s_filename);
      utils::removeFilenameSuffix(&s_filename);
      s_pathname = utils::getPathOfFile(filename);
    }

    /**
     * Save::save()
     * calls all methods to save the scene.
     *
     */
    unsigned int Save::save() {
      std::vector<std::string> v_filenames;
      std::vector<std::string> v_fullfilenames;
      std::string s_scenename;
      std::string s_zipfile;
      std::vector<material_map>::iterator iter;
      next_material_id = 1;
      s_zipfile=s_pathname;
      s_zipfile.append(s_filename);
      s_zipfile.append(".scn");
      s_scenename = s_tmpDirectory + "/";
      s_scenename += s_filename;
      s_scenename.append(".scene");

      //if a file with the same name, allready exists, it is removed
      remove(s_scenename.c_str());
      LOG_DEBUG("Save: %s", s_scenename.c_str());
      //adding the scenefile to the zip
      v_fullfilenames.push_back(s_scenename);

      QFile xmlfile((QString) s_scenename.c_str());
      if (!xmlfile.open(QIODevice::WriteOnly |QIODevice::Append |
                        QIODevice::Text)) {
        std::cout<<"something went wrong, while generating the *scene file"
                 <<std::endl << s_scenename.c_str() << std::endl;
        return 0;
      }
      QTextStream out(&xmlfile);
      out.setRealNumberNotation(QTextStream::FixedNotation);
      out <<"<SceneFile>"<<"\n";
      out <<"  <version>0.1</version>"<<"\n";
      //generating the header
      if (!param.v_nodeList.empty()) {
        out <<"  <nodelist>"<<"\n";
        for(unsigned int i=0; i<param.v_nodeList.size(); ++i) {
          if (generate(&param.v_nodeList[i], &out, &v_fullfilenames)==0) {
            std::cout<<"error while generating the node";
            std::cout<<"entries in the xmlfile"<<std::endl;
            return 0;
          }
        }
        out <<"  </nodelist>"<<"\n";
      }
      if (!param.v_jointList.empty()) {
        out <<"  <jointlist>"<<"\n";
        for(unsigned int i=0; i<param.v_jointList.size(); ++i) {
          if (generate(&param.v_jointList[i], &out, &v_fullfilenames)==0) {
            LOG_ERROR("Save:: failed generating joint %s",
                      param.v_jointList[i].name.c_str());
            return 0;
          }
        }
        out <<"  </jointlist>"<<"\n";
      }

      if (!param.v_motorList.empty()) {
        out <<"  <motorlist>"<<"\n";
        for(unsigned int i=0; i<param.v_motorList.size(); ++i) {
          if (generate(&param.v_motorList[i], &out, &v_fullfilenames)==0) {
            LOG_ERROR("Save:: failed generating motor %s",
                      param.v_motorList[i].name.c_str());
            return 0;
          }
        }
        out <<"  </motorlist>"<<"\n";
      }

      if (!param.v_sensorList.empty()) {
        if (generate(&param.v_sensorList, &out)==0) {
          std::cout<<"error while generating the sensor";
          std::cout<<"entries in the xmlfile"<<std::endl;
          return 0;
        }
      }

      if (!param.v_controllerList.empty()) {
        out <<"  <controllerlist>"<<"\n";
        for(unsigned int i=0; i<param.v_controllerList.size(); ++i) {
          if (generate(&param.v_controllerList[i], &out, &v_fullfilenames)==0) {
            LOG_ERROR("Save:: failed generating controller %lu",
                      param.v_controllerList[i].id);
            return 0;
          }
        }
        out <<"  </controllerlist>"<<"\n";
      }

      if (!param.v_lightList.empty()) {
        out <<"  <lightlist>"<<"\n";
        for(unsigned int i=0; i<param.v_lightList.size(); ++i) {
          if (generate(&param.v_lightList[i], &out, &v_fullfilenames)==0) {
            LOG_ERROR("Save:: failed generating light %s",
                      param.v_lightList[i].name.c_str());
            return 0;
          }
        }
        out <<"  </lightlist>"<<"\n";
      }

      out <<"  <materiallist>\n";
      std::cout<<"num Matirials:"<<materials.size()<<std::endl;
      for(iter = materials.begin(); iter != materials.end(); iter++) {
        if(!generate(&((*iter).material), &out, &v_fullfilenames,
                     (*iter).material_id)) {
          std::cout<<"could not save material"<<std::endl;
        }
      }
      out <<"  </materiallist>\n";


      interfaces::GraphicData options = control->graphics->getGraphicOptions();
      if(!generate(&options, &out, &v_fullfilenames)) {
        LOG_ERROR("Save:: faild generating graphic options");
      }

      out <<"</SceneFile>"<<"\n";


      xmlfile.close();
      if (!v_fullfilenames.empty()) {
        Zipit zipfile(s_zipfile);
        LOG_WARN("Save: save to: %s", s_zipfile.c_str());
        //removing doubleentries,
        for (unsigned int i=0; i<v_fullfilenames.size(); i++) {
          for (unsigned int j=i+1; j<v_fullfilenames.size(); j++) {
            if (v_fullfilenames[i]==v_fullfilenames[j]) {
              v_fullfilenames.erase(v_fullfilenames.begin()+j);
              j--;//and decrementing the counter j, because there may be more
              //objects of interest
            }
          }
        }
        for (unsigned int i=0; i<v_fullfilenames.size(); i++) {
          s_filename = v_fullfilenames[i];
          utils::removeFilenamePrefix(&s_filename);
          v_filenames.push_back(s_filename);
        }
        if (zipfile.addToZip(v_filenames, v_fullfilenames)!=0) {
          return 0;
        }
      }
      return 1;
    }

   /**
     * prepare()
     * this method prepares the internal datastructs and fills them with values
     */
    unsigned int Save::prepare() {
      std::vector<interfaces::core_objects_exchange> objList;
      std::vector<interfaces::core_objects_exchange>::iterator it;

      //obtaining the number of elements and storing them in a vector
      control->nodes->getListNodes(&objList);
      for (it=objList.begin(); it!=objList.end(); ++it) {
        param.v_nodeList.push_back(control->nodes->getFullNode(it->index));
      }
      control->joints->getListJoints(&objList);
      for (it=objList.begin(); it!=objList.end(); ++it) {
        param.v_jointList.push_back(control->joints->getFullJoint(it->index));
      }
      control->motors->getListMotors(&objList);
      for (it=objList.begin(); it!=objList.end(); ++it) {
        param.v_motorList.push_back(control->motors->getFullMotor(it->index));
      }
      control->sensors->getListSensors(&objList);
      for (it=objList.begin(); it!=objList.end(); ++it) {
        param.v_sensorList.push_back(control->sensors->getFullSensor(it->index));
      }
      control->controllers->getListController(&objList);
      for (it=objList.begin(); it!=objList.end(); ++it) {
        param.v_controllerList.push_back(control->controllers->getFullController(it->index));
      }
      control->graphics->getLights(&param.v_lightList);
      if(save()==0){
        std::cout<<"error while saving return from save_Scene in simulator.cpp"
                 <<std::endl;
        return 0;
      }
      return 1;
    }

    void Save::writeConfigMap(const configmaps::ConfigMap &cfg, QTextStream * out,
                              std::string tag, bool handleSensor, int depth) {

      configmaps::ConfigMap::const_iterator it;
      configmaps::ConfigVector::const_iterator it2;
      bool close = false;
      char text[255];

      if(handleSensor && depth == 0) {
        it = cfg.find("name");
        std::string name = it->second.toString();
        it = cfg.find("type");
        std::string type = it->second.toString();
        // FIXME: potential Buffer Overflow!!!!
        sprintf(text, "    <sensor name=\"%s\" type=\"%s\">", name.c_str(),
                type.c_str());

        //LOG_DEBUG("%s", text);

        *out <<text<<"\n";

        close = true;
        depth+=3;
      }
      else if(depth==0) {
        //LOG_DEBUG("  <%s>", tag.c_str());
        *out << "    <" << QString(tag.c_str()) << ">\n";
        depth+=3;
        close = true;
      }

      for(int i=0; i<depth*2; i++) text[i] = ' ';

      for(it=cfg.begin(); it!=cfg.end(); ++it) {
        if(handleSensor && (depth < 3 && (it->first == "name" || it->first == "type"))) continue;
        for(it2=it->second.begin(); it2!=it->second.end(); ++it2) {
          if(it2->size() == 0) {
            sprintf(text+depth*2, "<%s>%s</%s>", it->first.c_str(),
                    it2->toString().c_str(), it->first.c_str());
            *out << text << "\n";
            //LOG_DEBUG("%s", text);
          }
          else {
            sprintf(text+depth*2, "<%s>", it->first.c_str());
            *out << text << "\n";
            //LOG_DEBUG("%s", text);
            writeConfigMap((configmaps::ConfigMap&)(*it2), out, tag, handleSensor, depth+1);
            sprintf(text+depth*2, "</%s>", it->first.c_str());
            *out << text << "\n";
            //LOG_DEBUG("%s", text);
          }
        }
      }

      if(close) {
        *out << "    </"<< QString(tag.c_str()) <<">\n";
        //LOG_DEBUG("  </%s>", tag.c_str());
      }
    }

    unsigned int Save::generate(std::vector<const interfaces::BaseSensor*> *v_BaseSensor,
                                QTextStream * out) {
      std::vector<const interfaces::BaseSensor*>::iterator it;

      if(v_BaseSensor->size() > 0) {
        *out << "<sensorlist>\n";
        for(it=v_BaseSensor->begin(); it!=v_BaseSensor->end(); ++it) {
          configmaps::ConfigMap cfg = (*it)->createConfig();
          writeConfigMap(cfg, out, std::string("sensor"), true);
        }
        *out << "</sensorlist>\n";
      }
      return 1;
    }

    unsigned int Save::generate(interfaces::MaterialData *materialData,
                                QTextStream *out,
                                std::vector<std::string> *v_filenames,
                                unsigned long id) {
      configmaps::ConfigMap config;
      config["id"] = id;
      materialData->toConfigMap(&config, true);
      materialData->getFilesToSave(v_filenames);
      writeConfigMap(config, out, std::string("material"));
      return 1;
    }

    unsigned int Save::generate(interfaces::NodeData *nodeData,
                                QTextStream *out,
                                std::vector<std::string> *v_filenames) {
      configmaps::ConfigMap config;

      if(nodeData->relative_id) { // handle relative positioning
        interfaces::NodeData parentNode;
        parentNode = control->nodes->getFullNode(nodeData->relative_id);
        interfaces::getRelFromAbs(parentNode, nodeData);
      }
      nodeData->toConfigMap(&config, true);
      nodeData->getFilesToSave(v_filenames);

      {  // handle material
        unsigned long mat_id;
        std::vector<material_map>::iterator iter;
        material_map tmp_mat_map;

        mat_id = 0;
        for(iter = materials.begin(); iter != materials.end(); iter++) {
          if((*iter).material == nodeData->material) {
            mat_id = (*iter).material_id; break;}
        }
        if(!mat_id) {
          tmp_mat_map.material = nodeData->material;
          mat_id = tmp_mat_map.material_id = next_material_id;
          materials.push_back(tmp_mat_map);
          next_material_id++;
        }
        config["material_id"] = mat_id;
      }

      writeConfigMap(config, out, std::string("node"));
      return 1;
    }

    unsigned int Save::generate(interfaces::JointData *jointData,
                                QTextStream *out,
                                std::vector<std::string> *v_filenames) {
      configmaps::ConfigMap config;

      jointData->toConfigMap(&config, true);
      jointData->getFilesToSave(v_filenames);

      writeConfigMap(config, out, std::string("joint"));
      return 1;
    }

    unsigned int Save::generate(interfaces::MotorData *motorData,
                                QTextStream *out,
                                std::vector<std::string> *v_filenames) {
      configmaps::ConfigMap config;

      motorData->toConfigMap(&config, true);
      motorData->getFilesToSave(v_filenames);

      writeConfigMap(config, out, std::string("motor"));
      return 1;
    }

    unsigned int Save::generate(interfaces::LightData *lightData,
                                QTextStream *out,
                                std::vector<std::string> *v_filenames) {
      configmaps::ConfigMap config;

      lightData->toConfigMap(&config, true);
      lightData->getFilesToSave(v_filenames);

      writeConfigMap(config, out, std::string("light"));
      return 1;
    }

    unsigned int Save::generate(interfaces::GraphicData *graphicData,
                                QTextStream *out,
                                std::vector<std::string> *v_filenames) {
      configmaps::ConfigMap config;

      graphicData->toConfigMap(&config, true);
      graphicData->getFilesToSave(v_filenames);

      writeConfigMap(config, out, std::string("graphicOptions"));
      return 1;
    }

    unsigned int Save::generate(interfaces::ControllerData *controller,
                                QTextStream *out,
                                std::vector<std::string> *v_filenames) {
      configmaps::ConfigMap config;

      controller->toConfigMap(&config, true);
      controller->getFilesToSave(v_filenames);

      writeConfigMap(config, out, std::string("controller"));
      return 1;
    }

  } // end of namespace scene_loader
} // end of namespace mars
