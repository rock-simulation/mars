/*
 *  Copyright 2012, 2014, DFKI GmbH Robotics Innovation Center
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
 * \file SMURFLoader.cpp
 * \author Malte Langosz, Kai von Szadkowski
 */

#include "SMURFLoader.h"
#include "zipit.h"

#include <QtXml>
#include <QDomNodeList>

#include <algorithm>
#include <lib_manager/LibManager.hpp>
#include <lib_manager/LibInterface.hpp>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/EntityManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/interfaces/GraphicData.h>
#include <mars/sim/SimEntity.h>
#include <mars/utils/misc.h>
#include <mars/utils/mathUtils.h>

#include <QtXml>
#include <QDomNodeList>

namespace mars {
  namespace smurf {

    using namespace mars::utils;
    using namespace mars::interfaces;


    SMURFLoader::SMURFLoader(lib_manager::LibManager *theManager) :
      interfaces::LoadSceneInterface(theManager), control(NULL){

      mars::interfaces::SimulatorInterface *marsSim;
      marsSim = libManager->getLibraryAs<mars::interfaces::SimulatorInterface>("mars_sim");
      if(marsSim) {
	control = marsSim->getControlCenter();
	control->loadCenter->loadScene[".zsmurf"] = this; // zipped smurf model
	control->loadCenter->loadScene[".zsmurfs"] = this; // zipped smurf scene
	control->loadCenter->loadScene[".zsmurfa"] = this; // zipped smurf assembly
	control->loadCenter->loadScene[".smurf"] = this; // smurf model
	control->loadCenter->loadScene[".smurfs"] = this; // smurf scene
	control->loadCenter->loadScene[".smurfa"] = this; // smurf assembly
	control->loadCenter->loadScene[".svg"] = this; // smurfed vector graphic
	control->loadCenter->loadScene[".urdf"] = this; // urdf model
	LOG_INFO("smurf_loader: added SMURF loader to loadCenter");
      }

      factoryManager =
	    theManager->acquireLibraryAs<mars::entity_generation::EntityFactoryManager>(
		"mars_entity_factory");
    }

    SMURFLoader::~SMURFLoader() {
      if(control) {
	control->loadCenter->loadScene.erase(".zsmurf");
	control->loadCenter->loadScene.erase(".zsmurfs");
	control->loadCenter->loadScene.erase(".zsmurfa");
	control->loadCenter->loadScene.erase(".smurf");
	control->loadCenter->loadScene.erase(".smurfs");
	control->loadCenter->loadScene.erase(".smurfa");
	control->loadCenter->loadScene.erase(".svg");
	control->loadCenter->loadScene.erase(".urdf");
	libManager->releaseLibrary("mars_sim");
      }
      libManager->releaseLibrary("mars_entity_factory");
    }

    void SMURFLoader::checkEncodings() {
	    bool existing = true;
	    QString str("<xml><easter_egg>3.1418</easter_egg></xml>");
	    QString tmpFilename = QString(tmpPath.c_str()) + QString("mars-encoding-check");
	    QFile tmpFile(tmpFilename);
	    if (!QFile::exists(tmpFilename)) {
	      tmpFile.open(QIODevice::WriteOnly);
	      QTextStream out(&tmpFile);
	      out << str;
	      tmpFile.close();
	      existing = false;
	    }
	    if (!tmpFile.open(QIODevice::ReadOnly)) {
	      LOG_FATAL("Cannot open language checking file\n");
	      exit(-1);
	    }
	    QDomDocument doc;
	    if (!doc.setContent(&tmpFile, false)) {
	      LOG_FATAL("Cannot parse language checking file\n");
	      exit(-2);
	    }
	    QDomElement root = doc.documentElement();
	    QDomElement tmpElement;
	    if (root.elementsByTagName(QString("easter_egg")).at(0).toElement().text().toDouble()
		!= 3.1418) {
	      LOG_ERROR(
		  "Encoding of the system is invalid, therefore Scene loading will fail quitting here to prevent errors later");
	      exit(-3);
	    }
	    tmpFile.close();
	    if (!existing) {
	      tmpFile.remove();
	    }
	  }

	  unsigned int SMURFLoader::parseSVG(std::vector<configmaps::ConfigMap> *configList,
	      std::string sceneFilename) {
	    checkEncodings();
	    //  HandleFileNames h_filenames;
	    std::vector<std::string> v_filesToLoad;
	    QString xmlErrorMsg = "";
	    int xmlErrorLine, xmlErrorCol = 0;

	    //creating a handle for the xmlfile
	    QFile file(sceneFilename.c_str());
	    std::string path = getPathOfFile(sceneFilename);

	    QLocale::setDefault(QLocale::C);

	    LOG_INFO("Load: loading scene: %s", sceneFilename.c_str());

	    //test to open the xmlfile
	    if (!file.open(QIODevice::ReadOnly)) {
	      std::cout << "Error while opening scene file content " << sceneFilename
		  << " in Load.cpp->parseScene" << std::endl;
	      std::cout << "Make sure your scenefile name corresponds to"
		  << " the name given to the enclosed .scene file" << std::endl;
	      return 0;
	    }

	    //test to pass the content from the xmlfile to the DOM-Object
	    QDomDocument doc;
	    if (!doc.setContent(&file, false, &xmlErrorMsg, &xmlErrorLine, &xmlErrorCol)) {
	      file.close();
	      std::cout << "error passing the file content in->Load.cpp->parseScene" << std::endl;
	      std::cout << "Message: " << xmlErrorMsg.toStdString() << "\n" << "Line: " << xmlErrorLine
		  << "\n" << "Collumn: " << xmlErrorCol << "\n" << std::endl;
	      return 0;
	    }

	    //the XML-file has been opend and is ready for parsing it's content
	    //the first element is the "root" element
	    QDomElement root = doc.documentElement();
	    QDomElement tmpElement;
	    double version = 0.0;

	    QDomNodeList xmlnodelist = root.elementsByTagName(QString("version"));
	    if (!xmlnodelist.isEmpty()) {
	      tmpElement = xmlnodelist.at(0).toElement();
	      version = tmpElement.text().toDouble();
	    }
	    if (version < 0.199999) {
	      LOG_WARN("Load: old SceneFile version. Take care of the following changes:");
	      LOG_WARN("\t- the sphere object get radius as size instead of diameter");
	      LOG_WARN(
		  "\t- the origin numbers for primitives are replaced by strings (box, shpere, etc.)");
	      LOG_WARN(
		  "\t- the sensor type is a string instead of a number, like NodePosition, JointPosition, or NodeCOM");
	      LOG_WARN("\t- the JOint6DOF sensor gets nodeID and jointID instead of id list");
	      LOG_WARN("\t- the actual_pos and acutal_rot is removed from nodes");
	    }

	    global_width = root.attribute("width").toDouble();
	    global_length = root.attribute("height").toDouble();

    //  std::vector<configmaps::ConfigMap> poiList;
	    //first checking wether there is a node with name "node"
	    //by passing it in a xmlnodelist and checking if it's not empty.
	    //if so, there is at least an element with name "node"

	    int start = 0;
	    int end = 0;
	    double angle = 0.0;
	    std::string transform = "";
	    std::string originstring;
	    std::vector<std::string> transform_elements;
	    std::vector<std::string> originelements;
	    std::vector<std::string> descelements;
	    std::string description;
	    double origin_z = 0;
	    QDomElement rect;
	    QDomElement originpath;
	    Vector origvec;
	    xmlnodelist = root.elementsByTagName(QString("g"));
	    Quaternion orientation;
	    Vector translation;
	    for (int i = 0; i < xmlnodelist.size(); i++) {
	      //getGenericConfig(&map, xmlnodelist.at(i).toElement());
	      ConfigMap map;
	      //// TODO: do this if on Windows
	      //std::string current_locale_text = qs.toLocal8Bit().constData();
	      map["file"] = path + xmlnodelist.at(i).toElement().elementsByTagName("title").at(0).toElement().text().toUtf8().constData();
	      description = xmlnodelist.at(i).toElement().elementsByTagName("desc").at(0).toElement().text().toUtf8().constData();
	      descelements = explodeString(' ', description);
	      if (descelements.at(0) == "origin_z:") {
		origin_z = ::atof(descelements.at(1).c_str());
	      }
	      //TODO make this generic
	      transform = xmlnodelist.at(i).toElement().attribute("transform", "").toUtf8().constData();
	      if (transform == "") {
		translation = Vector(0, 0, 0);
		angle = 0;
		fprintf(stderr, "No transform for entity found.");
	      } else {
		start = transform.find("(");
		end = transform.find(")");
		transform_elements = explodeString(',', transform.substr(start + 1, end - start - 1));
		fprintf(stderr, "Found transform with %i elements.", (int)transform_elements.size());
		if (transform_elements.size() > 4) {
		  translation = Vector(::atof(transform_elements.at(4).c_str()),
		       ::atof(transform_elements.at(5).c_str()), 0.0);
		  double sinus = ::atof(transform_elements.at(1).c_str());
		  double cosinus = ::atof(transform_elements.at(0).c_str());
		  angle = atan2(sinus, cosinus);
		  fprintf(stderr, "angle: %g", angle);
		} else {
		  if (transform_elements.size() == 2) {

		    translation = Vector(::atof(transform_elements.at(0).c_str()),
			::atof(transform_elements.at(1).c_str()), 0.0);

		  } else {
		    translation = Vector(0, 0, 0);
		  }
		  angle = 0.0;
		}
	      }
	      map["orientation_z"] = radToDeg(-angle);
	      orientation = utils::eulerToQuaternion(Vector(0, 0, radToDeg(angle)));
	      //orientation = Quaternion(orientation.w(), orientation.x(), -orientation.y(), orientation.z());

	      rect = xmlnodelist.at(i).toElement().elementsByTagName("rect").at(0).toElement();
	      originpath = xmlnodelist.at(i).toElement().elementsByTagName("path").at(0).toElement();
	      originelements = explodeString(',',
		  explodeString(' ', originpath.attribute("d").toUtf8().constData()).at(1));
	      fprintf(stderr, "Originvector x: %s, y: %s, angle: %g\n", ((std::string) originelements.at(0)).c_str(),
		  ((std::string) originelements.at(1)).c_str(), radToDeg(angle));
	      origvec = Vector(::atof(originelements.at(0).c_str()),
		  ::atof(originelements.at(1).c_str()), 0.0);
	      origvec = translation + (orientation * origvec);
	      //origvec = orientation * (translation +  origvec);
	      map["origin_x"] = origvec(0);
    //          if (angle > 0.001)
    //            origvec(1) = global_width - origvec(1);
    //          if (angle < -0.001)
    //            map["origin_y"] = origvec(1) - global_width;
    //          else
	      map["origin_y"] = global_width - origvec(1);
	      //map["origin_y"] = origvec(1);
	      map["origin_z"] = origin_z;
	      fprintf(stderr, "%g, %g\n", origvec(0), origvec(1));
	      //map["field_width"] = rect.attribute("height").toDouble();
	      //map["field_length"] = rect.attribute("width").toDouble();
	      configList->push_back(map);
	    }
    //
    //        for (std::vector<configmaps::ConfigMap>::iterator it = entityList.begin();
    //            it != entityList.end(); ++it) {
    //          map = ConfigMap::fromYamlFile((std::string) (*it)["id"] + ".yml", true);
    //          if (it->find("transform") != it->end()) {
    //            //fprintf(stderr, ((std::string) (*it)["id"] + ".yml").c_str());
    //
    //            transform = (std::string) (*it)["transform"];
    //            start = transform.find("(");
    //            end = angle.find(")");
    //
    //            fprintf(stderr, "\n transform: %g\n", (double) map["orientation_z"]);
    //          }
    //
    //          //map["orientation_z"] = ::atof(angle.c_str());
    //          //fprintf(stderr, ((std::string)(*it)["width"]+"\n").c_str());//, ((std::string)(*it)["width"]).c_str(), "\n");
    //        }

    //  xmlnodelist = root.elementsByTagName(QString("square"));
    //  for (int i = 0; i < xmlnodelist.size(); i++) {
    //    getGenericConfig(&poiList, xmlnodelist.at(i).toElement());
    //  }

	    file.close();
	    return 1;
	}

    /**
       * Loads a URDF, SMURF, SMURFS (SMURF scene) or SMURFA (SMURF assembly) file.
       * @param filename The name of the file to be loaded as a full path.
       * @param tmpPath The path to the temporary directory to which files should be unzipped
       *        if the file format requires it.
       * @param robotname This parameter is obsolete with URDF and SMURF and was only
       *        needed for MARS scenes. It may be removed in the future.
       * @return 1 if the file was successfully loaded, 0 otherwise
       */

    void SMURFLoader::loadEntity(configmaps::ConfigMap* it, std::string path, bool do_not_create) {
      std::string uri = (std::string)(*it)["file"];
      if (uri == "") {
	uri = (std::string)(*it)["URI"]; // backwards compatibility
      }
      std::string uri_extension = utils::getFilenameSuffix(uri);
      // get type from file extension if there is no type
      if ((std::string)(*it)["type"] == "")
	(*it)["type"] = uri_extension.substr(1);
      // handle absolute paths
      std::string combined_path;
      if (uri.substr(0,1) == "/") {
	combined_path = utils::getPathOfFile(uri);
      } else {
	combined_path = path+utils::getPathOfFile(uri);
      }
      if(combined_path[combined_path.size()-1] != '/') combined_path.append("/");
      (*it)["path"] = combined_path;
      (*it)["load_path"] = path;
      utils::removeFilenamePrefix(&uri);
      (*it)["file"] = uri;
      std::string fulluri = combined_path+uri;
      // the following allows adding an old MARS scene file in a smurf scene
      if (((std::string)(*it)["type"] == "scn") || ((std::string)(*it)["type"] == "scene") || ((std::string)(*it)["type"] == "yml")) {
	std::string name = "";
	if(it->hasKey("name")) {
	  name << (*it)["name"];
	}
	control->loadCenter->loadScene[uri_extension]->loadFile(fulluri,
								path, name);
	if(name != "") {
	  mars::sim::SimEntity* entity = control->entities->getEntity(name);
	  entity->appendConfig(*it);
	  entity->setInitialPose();
	}
      }
      else if(((std::string)(*it)["type"] == "smurfs") || ((std::string)(*it)["type"] == "smurfa")) {
	// backup internal state:
	std::string tmpPath_b = tmpPath;
	tmpPath = "";
	double global_width_b = global_width;
	double global_length_b = global_length;
	std::vector<configmaps::ConfigMap> entitylist_b;
	//entitylist_b.swap(entitylist);
	// pass the rotation and position offsets to the assembly
	ConfigStruct cfg_struct;
	if (it->hasKey("parent")) cfg_struct.parent = (std::string)(*it)["parent"];
	if (it->hasKey("anchor")) cfg_struct.anchor = (std::string)(*it)["anchor"];
	getPoseFromConfigMap(*it, &(cfg_struct.pos), &(cfg_struct.rot));
	/* We use directly the load procedure as we know which one to use
	 * instead going the detour via the loadCenter. This enables us to use
	 * an overload of this method without changing all the interfaces.
	 * In the future it might be a good idea to implement passing of the
	 * cfgstruct to other loaders , too, to apply the transformation there
	 * as well.
	*/
	this->loadFile(fulluri, path, (std::string)(*it)["name"], &cfg_struct, do_not_create);
	/* formerly:
	control->loadCenter->loadScene[uri_extension]->loadFile(fulluri,
	    path, (std::string)(*it)["name"]);
	*/
	// restore internal state:
	tmpPath = tmpPath_b;
	global_width = global_width_b;
	global_length = global_length_b;
	//entitylist.swap(entitylist_b);
      }
      else {
	entitylist.push_back(*it);
      }
    }

    //TODO: remove parameter "robotname"
    bool SMURFLoader::loadFile(std::string filename, std::string tmpPath,
			      std::string robotname) {
      return loadFile(filename, tmpPath, robotname, nullptr, false);
    }

    bool SMURFLoader::loadFile(std::string filename, std::string tmpPath,
			      std::string robotname,
			      void* args, bool do_not_create/*=false*/) {
      LOG_INFO("urdf_loader: prepare loading");
      if (!do_not_create) entitylist.clear();

      // split up filename in path + _filename and retrieve file extension
      std::string file_extension = utils::getFilenameSuffix(filename);
      fprintf(stderr, "SMURFLoader::loadFile: file_extension=%s\n", file_extension.c_str());
      std::string path = utils::getPathOfFile(filename);
      std::string _filename = filename;
      utils::removeFilenamePrefix(&_filename);

      // need to unzip into a temporary directory
      if (file_extension == ".zsmurf" || file_extension == ".zsmurfs" || file_extension == ".zsmurfa") {
	if (unzip(tmpPath, filename) == 0) {
	  path = tmpPath;
	  return 0;
	}
      }

      // read in the provided file - .smurfs / .smurf / .urdf
      configmaps::ConfigMap map;
      std::string uri;
      std::string uri_extension;
      fprintf(stderr, "Reading in %s...\n", (path+_filename).c_str());
      if(file_extension == ".smurfs" || file_extension == ".smurfa") {
        configmaps::ConfigVector::iterator it;
        map = configmaps::ConfigMap::fromYamlFile(path+_filename, true);
        //map.toYamlFile("smurfs_debugmap.yml");
        // first check for plugins to load:
        std::string libName = "";
        lib_manager::LibInfo libInfo;
        if (map.hasKey("plugins")) {
          for(auto nt: map["plugins"]) {
            libName << nt;
            libInfo = libManager->getLibraryInfo(libName);
            if(libInfo.name != libName) {
              libManager->loadLibrary(libName, NULL, false);
            }
          }
        }
	for (it = map["smurfs"].begin(); it != map["smurfs"].end(); ++it) { // backwards compatibility
	  configmaps::ConfigMap &m = *it;
	  applyConfigStruct((ConfigStruct*)args, m);
	  loadEntity(&m, path, true);
	}
	for (it = map["entities"].begin(); it != map["entities"].end(); ++it) { // new tag
	  configmaps::ConfigMap &m = *it;
	  applyConfigStruct((ConfigStruct*)args, m);
	  loadEntity(&m, path, true);
	}
	for (it = map["smurfa"].begin(); it != map["smurfa"].end(); ++it) { // backwards compatibility
	  configmaps::ConfigMap &m = *it;
	  applyConfigStruct((ConfigStruct*)args, m, true);
	  // add assembly entry to the entity's map so we can find it later
	  m["assembly"] = robotname;
	  loadEntity(&m, path, true);
	}
	// parse physics
	if (file_extension != ".smurfa") {
	  if (map.hasKey("physics")) {
	    configmaps::ConfigMap physicsmap = map["physics"];
	    if (physicsmap.hasKey("gravity")) {
	      utils::Vector gravvec;
	      //configmaps::ConfigMap gravmap = physicsmap["gravity"];
	      //gravvec.x() = gravmap["x"];
	      gravvec.x() = physicsmap["gravity"]["x"];
	      gravvec.y() = physicsmap["gravity"]["y"];
	      gravvec.z() = physicsmap["gravity"]["z"];
	      control->sim->setGravity(gravvec);
	      }
	    if (physicsmap.hasKey("ode")) {
	      if (physicsmap["ode"].hasKey("cfm")) {
		control->cfg->setPropertyValue("Simulator", "world cfm", "value", (sReal)(physicsmap["ode"]["cfm"]));
	      }
	      if (physicsmap["ode"].hasKey("erp")) {
		control->cfg->setPropertyValue("Simulator", "world erp", "value", (sReal)(physicsmap["ode"]["erp"]));
	      }
	      if (physicsmap["ode"].hasKey("stepsize")) {
		control->cfg->setPropertyValue("Simulator", "calc_ms", "value", (sReal)(physicsmap["ode"]["stepsize"]));
	      }
	    }
	  }
	  if (map.hasKey("environment")) {
	    configmaps::ConfigMap envmap = map["environment"];
	    if (envmap.hasKey("skybox")) {
	      control->cfg->createParam("Scene","skydome_path",
					cfg_manager::stringParam);
	      control->cfg->createParam("Scene","skydome_enabled",
					cfg_manager::boolParam);
	      // check if path is relative to smurfs scene
	      std::string skyboxPath = envmap["skybox"]["path"];
	      skyboxPath = pathJoin(path, skyboxPath);
	      if(!pathExists(skyboxPath)) {
		skyboxPath << envmap["skybox"]["path"];
	      }
	      control->cfg->setPropertyValue("Scene", "skydome_path", "value",
					     skyboxPath);
	      control->cfg->setPropertyValue("Scene", "skydome_enabled", "value",
					     true);
	    }
      if(envmap.hasKey("animations")) {
        if(envmap["animations"].hasKey("config_path")) {
          control->cfg->createParam("Scene","animations_path", cfg_manager::stringParam);
          control->cfg->setPropertyValue("Scene", "animations_path", "value", std::string(envmap["animations"]["config_path"]));
        }
      }
	    if (envmap.hasKey("terrain")) {
	      control->cfg->createParam("Scene","terrain_path", cfg_manager::stringParam);
	      control->cfg->setPropertyValue("Scene", "terrain_path", "value", std::string(envmap["terrain"]["path"]));
	      if(envmap["terrain"].hasKey("mesh")) {
          control->cfg->createParam("Scene","mesh", cfg_manager::stringParam);
          control->cfg->setPropertyValue("Scene", "mesh", "value", std::string(envmap["terrain"]["mesh"]));
	      }
	      if(envmap["terrain"].hasKey("mesh_material")) {
          control->cfg->createParam("Scene","mesh_material", cfg_manager::stringParam);
          control->cfg->setPropertyValue("Scene", "mesh_material", "value", std::string(envmap["terrain"]["mesh_material"]));
	      }
	      if(envmap["terrain"].hasKey("mesh_scale")) {
          control->cfg->createParam("Scene","mesh_scale", cfg_manager::doubleParam);
          control->cfg->setPropertyValue("Scene", "mesh_scale", "value", (double)(envmap["terrain"]["mesh_scale"]));
	      }
	    }
	    if(control->graphics) {
	      interfaces::GraphicData goptions = control->graphics->getGraphicOptions();
	      if (envmap.hasKey("background")) {
		Color bgcol;
		bgcol.fromConfigItem(envmap["background"]);
		goptions.clearColor = bgcol;
	      }
	      if (envmap.hasKey("fog")) {
		goptions.fogEnabled = true;
		goptions.fogDensity = envmap["fog"]["density"];
		goptions.fogStart = envmap["fog"]["start"];
		goptions.fogEnd = envmap["fog"]["end"];
		Color fogcol;
		fogcol.fromConfigItem(envmap["fog"]["color"]);
		goptions.fogColor = fogcol;
	      } else {
		goptions.fogEnabled = false;
	      }
	      control->graphics->setGraphicOptions(goptions);
	    }
	  }
	  if (map.hasKey("lights")) {
	    for (it = map["lights"].begin(); it!= map["lights"].end(); ++it) {
	      LightData light;
	      int valid = light.fromConfigMap(*it, "", control->loadCenter);
	      if(!valid) {
		fprintf(stderr, "Load: error while loading light\n");
		return 0;
	      }
	      control->sim->addLight(light);
	    }
	  }
	}
      } else if(file_extension == ".smurf") {
	// if we have only one smurf, only one with rudimentary data is added to the smurf list
	  //map["URI"] = _filename;
	  // map = configmaps::ConfigMap::fromYamlFile(path+_filename, true);
	  map["path"] = path;
	  map["file"] = _filename;
	  map["type"] = "smurf";
	  map["name"] = robotname;
	  entitylist.push_back(map);
      } else if(file_extension == ".urdf") {
	  map["file"] = _filename;
	  map["path"] = path;
	  map["name"] = "";
	  map["type"] = "urdf";
	  entitylist.push_back(map);
      } else if(file_extension == ".svg") {
	/* A smurf svg can contain multiple entities, thus we have to loop over them.
	 * Each svg object must contain a "file" property, specifying the yaml file in
	 * which the missing parameters are defined, so we have to append that.
	 */
	std::vector<configmaps::ConfigMap> svg_entities;
	parseSVG(&svg_entities, path+_filename);
	for(std::vector<configmaps::ConfigMap>::iterator it = svg_entities.begin();
	    it!=svg_entities.end(); ++it) {
	  map = *it;
	  map.append(ConfigMap::fromYamlFile((std::string)map["file"], true)); //FIXME: path?
	  fprintf(stderr, "Loading config for svg entity: %s\n", ((std::string)map["file"]).c_str());
	  entitylist.push_back(map);
	}
      }

      if (!do_not_create) {
	std::vector<std::string> loaded;
	std::vector<configmaps::ConfigMap> loadLater;
	unsigned int entitiesToLoad = entitylist.size();
	unsigned int iterations = 0;
	//load assembled smurfs
	fprintf(stderr, "Creating %d simulation entities...\n", entitiesToLoad);
	while (entitylist.size() != 0) {
	  for (std::vector<configmaps::ConfigMap>::iterator sit = entitylist.begin();
	    sit != entitylist.end(); ++sit) {
	    configmaps::ConfigMap tmpmap;
	    tmpmap = *sit;
	    std::string dep1 = ""; std::string dep2 = ""; int pos;
	    if (tmpmap.hasKey("parent")) {
	      dep1 = ((std::string)tmpmap["parent"]);
	      pos = dep1.find("::");
	      if (pos != (int) std::string::npos) {
		dep1 = dep1.substr(0, pos);
	      } else {
		dep1 = "";
	      }
	    }
	    if (tmpmap.hasKey("anchor")) {
	      dep2 = ((std::string)tmpmap["anchor"]);
	      pos = dep2.find("::");
	      if (dep2 == "parent") {
		dep2 = dep1;
	      } else if (pos != (int) std::string::npos) {
		dep2 = dep2.substr(0, pos);
	      } else {
		dep2 = "";
	      }
	    }
	    if((dep1.empty() && dep2.empty()) ||
		(
		  dep1 == dep2 &&
		  std::find(loaded.begin(), loaded.end(), dep2) != loaded.end()
		) ||
		(
		  std::find(loaded.begin(), loaded.end(), dep1) != loaded.end() &&
		  std::find(loaded.begin(), loaded.end(), dep2) != loaded.end()
		) ||
		iterations > entitiesToLoad //load everything if something is wrong
	      )
	    {
	      factoryManager->createEntity(tmpmap);
	      loaded.push_back((std::string)tmpmap["name"]);
	    } else {
	      loadLater.push_back(tmpmap);
	    }
	  }
	  entitylist = loadLater;
	  loadLater.clear();
	  iterations++;
	}
	if (iterations-1 > entitiesToLoad)
	  fprintf(stderr, "WARNING: Loading all remaining entities as max iter is reached!\n");
      }

      return 1; //TODO: check number of successfully loaded entities before returning 1
    }

    bool SMURFLoader::loadFile(std::string filename, std::string tmpPath,
			       std::string robotname, utils::Vector pos, utils::Vector rot) {
      LOG_INFO("The scene loader assotiated to the file %s does not take into account the position and rotation received as parameter");
      return loadFile(filename, tmpPath, robotname);
    }

    int SMURFLoader::saveFile(std::string filename, std::string tmpPath) {
      return 0;
    }


    unsigned int SMURFLoader::unzip(const std::string& destinationDir,
				     const std::string& zipFilename) {
      if (!utils::createDirectory(destinationDir))
	return 0;

      Zipit myZipFile(zipFilename);
      LOG_INFO("Load: unsmurfing zipped SMURF: %s", zipFilename.c_str());

      if (!myZipFile.unpackWholeZipTo(destinationDir))
	return 0;

      return 1;
    }

    void SMURFLoader::applyConfigStruct(ConfigStruct *cfg_struct,
      configmaps::ConfigMap& m, bool is_smurfa/*=false*/)
    {
      if (cfg_struct != nullptr) {
	if (!m.hasKey("parent") ||
	     (m.hasKey("parent") && (std::string)m["parent"] == "world")) {
	  if (!is_smurfa || (m.hasKey("root") && (bool)m["root"] && is_smurfa)) {
	    m["anchor"] = cfg_struct->anchor;
	    m["parent"] = cfg_struct->parent;
	  }
	  transformConfigMapPose(cfg_struct->pos, cfg_struct->rot, &m);
	}
      }
    }


    void SMURFLoader::transformConfigMapPose(Vector pos_offset,
      Quaternion rot_offset, configmaps::ConfigMap* map)
    {
      Vector pos(0,0,0);
      Quaternion rot(1,0,0,0);
      if ((map->hasKey("position"))) {
	pos.x() = (*map)["position"][0];
	pos.y() = (*map)["position"][1];
	pos.z() = (*map)["position"][2];
      }
      if ((map->hasKey("rotation"))) {
	Vector tmpV;
	switch ((*map)["rotation"].size()) {
	case 1:
	  tmpV[0] = 0;
	  tmpV[1] = 0;
	  tmpV[2] = (*map)["rotation"][0];
	  rot = utils::eulerToQuaternion(tmpV);
	  break;
	case 3:
	  tmpV[0] = (*map)["rotation"][0];
	  tmpV[1] = (*map)["rotation"][1];
	  tmpV[2] = (*map)["rotation"][2];
	  rot = utils::eulerToQuaternion(tmpV);
	  break;
	case 4:
	  rot.x() = (sReal)(*map)["rotation"][1];
	  rot.y() = (sReal)(*map)["rotation"][2];
	  rot.z() = (sReal)(*map)["rotation"][3];
	  rot.w() = (sReal)(*map)["rotation"][0];
	  break;
	}
      }
      pos = rot_offset * pos + pos_offset;
      rot = rot_offset * rot;
      rot.normalize();
      if (map->hasKey("position")) {
	(*map)["position"][0] = pos.x();
	(*map)["position"][1] = pos.y();
	(*map)["position"][2] = pos.z();
      } else {
	(*map)["position"].push_back(pos.x());
	(*map)["position"].push_back(pos.y());
	(*map)["position"].push_back(pos.z());
      }
      if (map->hasKey("rotation")) {
	switch ((*map)["rotation"].size()) {
	case 1:
	  (*map)["rotation"][0] = rot.w();
	  (*map)["rotation"].push_back(rot.x());
	  (*map)["rotation"].push_back(rot.y());
	  (*map)["rotation"].push_back(rot.z());
	  break;
	case 3:
	  (*map)["rotation"][0] = rot.w();
	  (*map)["rotation"][1] = rot.x();
	  (*map)["rotation"][2] = rot.y();
	  (*map)["rotation"].push_back(rot.z());
	  break;
	case 4:
	  (*map)["rotation"][0] = rot.w();
	  (*map)["rotation"][1] = rot.x();
	  (*map)["rotation"][2] = rot.y();
	  (*map)["rotation"][3] = rot.z();
	  break;
	}
      } else {
	(*map)["rotation"].push_back(rot.w());
	(*map)["rotation"].push_back(rot.x());
	(*map)["rotation"].push_back(rot.y());
	(*map)["rotation"].push_back(rot.z());
      }
    }

    void SMURFLoader::getPoseFromConfigMap(configmaps::ConfigMap &map,
      utils::Vector* pos, utils::Quaternion* rot)
    {
      if ((map.hasKey("position"))) {
        if(map["position"].isVector()) {
          pos->x() = map["position"][0];
          pos->y() = map["position"][1];
          pos->z() = map["position"][2];
        } else {
          pos->x() = map["position"]["x"];
          pos->y() = map["position"]["y"];
          pos->z() = map["position"]["z"];
        }
      }
      if ((map.hasKey("rotation"))) {
	Vector tmpV;
	switch (map["rotation"].size()) {
	case 1:
	  tmpV[0] = 0;
	  tmpV[1] = 0;
	  tmpV[2] = map["rotation"][0];
	  *rot = utils::eulerToQuaternion(tmpV);
	  break;
	case 3:
	  tmpV[0] = map["rotation"][0];
	  tmpV[1] = map["rotation"][1];
	  tmpV[2] = map["rotation"][2];
	  *rot = utils::eulerToQuaternion(tmpV);
	  break;
	case 4:
    if(map["rotation"].isVector()) {
      rot->x() = (sReal)map["rotation"][1];
      rot->y() = (sReal)map["rotation"][2];
      rot->z() = (sReal)map["rotation"][3];
      rot->w() = (sReal)map["rotation"][0];
    } else {
      rot->x() = (sReal)map["rotation"]["x"];
      rot->y() = (sReal)map["rotation"]["y"];
      rot->z() = (sReal)map["rotation"]["z"];
      rot->w() = (sReal)map["rotation"]["w"];
    }
	  break;
	}
      }
    }

  } // end of namespace smurf
} // end of namespace mars

DESTROY_LIB(mars::smurf::SMURFLoader);
CREATE_LIB(mars::smurf::SMURFLoader);
