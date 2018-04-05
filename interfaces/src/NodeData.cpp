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

#include "NodeData.h"
#include "terrainStruct.h"
#define FORWARD_DECL_ONLY
#include "sim/ControlCenter.h"
#include "sim/LoadCenter.h"
#include <mars/utils/mathUtils.h>
#include <mars/utils/misc.h>
#include "Logging.hpp"
#include <iostream>
#include <cstdio>


#define GET_VALUE(str, val, type)                    \
  if((it = config->find(str)) != config->end())      \
    val = it->second;

#define GET_OBJECT(str, val, type)              \
  if((it = config->find(str)) != config->end()) \
    type##FromConfigItem(it->second, &val);

#define SET_VALUE(str, val, b)                             \
  if(b || val != defaultNode.val)                             \
    (*config)[str] = val

#define SET_OBJECT(str, val, type, b)                                     \
  if(b || val.squaredNorm() - defaultNode.val.squaredNorm() < 0.0000001) { \
    type##ToConfigItem((*config)[str], &val);                          \
  }

namespace mars {

  namespace interfaces {

    using namespace mars::utils;

    //access these strings only via toString(const NodeType&) and typeFromString(const std::string&).
    //synchronize this list with the enum NodeType in MARSDefs.h
    static const char* sTypeNames[NUMBER_OF_NODE_TYPES] = {
        "undefined",
        "mesh",
        "box",
        "sphere",
        "capsule",
        "cylinder",
        "plane",
        "terrain",
        "reference",
        "empty"
      };

    const char* NodeData::toString(const NodeType &type) {
      if (type > 0 && type < NUMBER_OF_NODE_TYPES) {
        return sTypeNames[type];
      }
      else {
        std::cerr << "initPrimitive: NodeType " << type << " not supported." << std::endl;
        throw("Invalid Primitive type");
      }
    }

    NodeType NodeData::typeFromString(const std::string& str) {
      //search in array with strings
      int type;
      for (type = 0; type < NUMBER_OF_NODE_TYPES; ++type) {
        if (str == sTypeNames[type])
          return (NodeType)type;
      }

      if(sscanf(str.c_str(), "%d", &type)) {
        if(type > 0 && type < NUMBER_OF_NODE_TYPES) return (NodeType)type;
      }


      std::cerr << "NodeData::typeFromString(): unknown node type \"" << str << "\"." << std::endl;
      throw("unknown Primitive type string");
    }

    bool NodeData::fromConfigMap(ConfigMap *config,
                                 std::string filenamePrefix,
                                 LoadCenter *loadCenter) {
      ConfigMap::iterator it;
      bool check = false, massDensity = false;
      bool needMass = true;

      name = trim(config->get("name", name));

      frameID = trim(config->get("frameID", frameID)); 

      { // handle node mass
        if((it = config->find("mass")) != config->end()) {
          mass = it->second;
          // use some epsilon here
          if(fabs(mass) > 0.000000001) check = true;
        }

        if((it = config->find("density")) != config->end()) {
          density = it->second;
          // use some epsilon here
          if(fabs(density) > 0.000000001) {
            if(check) massDensity = true;
            check = true;
          }
        }

        if((it = config->find("noPhysical")) != config->end()) {
          noPhysical = it->second;
          if(noPhysical) needMass = false;
        }

        if((it = config->find("movable")) != config->end()) {
          movable = it->second;
          if(!movable) needMass = false;
        }

        /* there are valid cases were pyhsical objects have no mass
        if(needMass) {
          if(!check) {
            LOG_WARN("no mass nor density given for node %s.", name.c_str());
          }
          else if(massDensity) {

            LOG_WARN("mass and density given for node %s. Using only the mass.",
                     name.c_str());
            density = 0.0;
          }
        }
        */
      } // handle node mass

      { // handle node type
        if((it = config->find("physicmode")) != config->end()) {
          std::string typeName = (std::string)it->second;
          try {
            physicMode = typeFromString(trim(typeName));
          } catch(...) {
            //throw SceneParseException("invalid physicmode for node", -1);
            LOG_ERROR("could not get type for node: %s", name.c_str());
          }
        }

        origName = trim(config->get("origname", origName));
        filename = trim(config->get("filename", filename));

        if(filename == "PRIMITIVE") {
          if(origName.empty()) {
            origName = toString(physicMode);
          }
          /* this is a valid case
          else if(origName != toString(physicMode)) {
            std::string tmp = toString(physicMode);
            LOG_WARN("origname set to \"%s\" for primitive in node \"%s\" with physicMode \"%s\"",
                     origName.c_str(), name.c_str(),
                     tmp.c_str() );
          }
          */
        }
      } // handle node type

      GET_VALUE("groupid", groupID, Int);
      GET_VALUE("index", index, ULong);
      GET_OBJECT("position", pos, vector);
      GET_OBJECT("pivot", pivot, vector);
      GET_OBJECT("rotation", rot, quaternion);
      GET_OBJECT("extend", ext, vector);

      { // handle relative positioning
        GET_VALUE("relativeid", relative_id, ULong);
        if(relative_id) {
          unsigned int mapIndex;
          GET_VALUE("mapIndex", mapIndex, UInt);
          if(mapIndex && loadCenter) {
            relative_id = loadCenter->getMappedID(relative_id, MAP_TYPE_NODE,
                                                  mapIndex);
          }
        }
      }

      // handle terrain info
      if((it = config->find("t_srcname")) != config->end()) {
        terrain = new(terrainStruct);
        terrain->srcname = trim((std::string)it->second);

        GET_VALUE("t_width", terrain->targetWidth, Double);
        GET_VALUE("t_height", terrain->targetHeight, Double);
        GET_VALUE("t_scale", terrain->scale, Double);
        double texScale = 1.0;
        GET_VALUE("t_tex_scale", texScale, Double);
        terrain->texScaleX = texScale;
        terrain->texScaleY = (texScale*(terrain->targetWidth /
                                        terrain->targetHeight));
        GET_VALUE("t_tex_scale_x", terrain->texScaleX, Double);
        GET_VALUE("t_tex_scale_y", terrain->texScaleY, Double);
      }

      GET_OBJECT("visualposition", visual_offset_pos, vector);
      GET_OBJECT("visualrotation", visual_offset_rot, quaternion);

      if((it = config->find("visualsize")) != config->end()) {
        vectorFromConfigItem(it->second, &visual_size);
      }

      if((it = config->find("visualscale")) != config->end()) {
        vectorFromConfigItem(it->second, &visual_scale);
      }
      else {
        visual_scale = Vector(1.0, 1.0, 1.0);
      }

      { // handle contact info
        GET_VALUE("cmax_num_contacts", c_params.max_num_contacts, Int);
        GET_VALUE("cerp", c_params.erp, Double);
        GET_VALUE("ccfm", c_params.cfm, Double);
        GET_VALUE("cfriction1", c_params.friction1, Double);
        GET_VALUE("cfriction2", c_params.friction2, Double);
        GET_VALUE("cmotion1", c_params.motion1, Double);
        GET_VALUE("cmotion2", c_params.motion2, Double);
        GET_VALUE("cfds1", c_params.fds1, Double);
        GET_VALUE("cfds2", c_params.fds2, Double);
        GET_VALUE("cbounce", c_params.bounce, Double);
        GET_VALUE("cbounce_vel", c_params.bounce_vel, Double);
        GET_VALUE("capprox", c_params.approx_pyramid, Bool);
        GET_VALUE("coll_bitmask", c_params.coll_bitmask, Int);

        if((it = config->find("cfdir1")) != config->end()) {
          if(!c_params.friction_direction1) {
            c_params.friction_direction1 = new Vector();
          }
          vectorFromConfigItem(it->second, c_params.friction_direction1);
        }
      }

      GET_VALUE("inertia", inertia_set, Bool);
      GET_VALUE("i00", inertia[0][0], Double);
      GET_VALUE("i01", inertia[0][1], Double);
      GET_VALUE("i02", inertia[0][2], Double);
      GET_VALUE("i10", inertia[1][0], Double);
      GET_VALUE("i11", inertia[1][1], Double);
      GET_VALUE("i12", inertia[1][2], Double);
      GET_VALUE("i20", inertia[2][0], Double);
      GET_VALUE("i21", inertia[2][1], Double);
      GET_VALUE("i22", inertia[2][2], Double);

      GET_VALUE("linear_damping", linear_damping, Double);
      GET_VALUE("angular_damping", angular_damping, Double);
      GET_VALUE("angular_low", angular_low, Double);

      GET_VALUE("shadow_id", shadow_id, Int);
      GET_VALUE("shadowcaster", isShadowCaster, Bool);
      GET_VALUE("shadowreceiver", isShadowReceiver, Bool);

      if(!filenamePrefix.empty()) {
        if(filename != "PRIMITIVE")
          handleFilenamePrefix(&filename, filenamePrefix);
        if(terrain) {
          handleFilenamePrefix(&terrain->srcname, filenamePrefix);
        }
      }

      map = *config;
      if(!filenamePrefix.empty()) {
        map["filePrefix"] = filenamePrefix;
      }
      return true;
    }

    void NodeData::toConfigMap(ConfigMap *config, bool skipFilenamePrefix,
                               bool writeDefaults) {
      NodeData defaultNode;
      std::string filename_ = filename;
      std::string srcname_;

      if(terrain) srcname_ = terrain->srcname;

      if(skipFilenamePrefix) {
        if(filename != "PRIMITIVE")
          removeFilenamePrefix(&filename_);
        if(terrain)
          removeFilenamePrefix(&srcname_);
      }

      SET_VALUE("name", name, writeDefaults);
      SET_VALUE("frameID", frameID, writeDefaults);
      SET_VALUE("mass", mass, writeDefaults);
      SET_VALUE("density", density, writeDefaults);
      SET_VALUE("noPhysical", noPhysical, writeDefaults);
      SET_VALUE("movable", movable, writeDefaults);

      if(writeDefaults || physicMode != defaultNode.physicMode) {
        std::string tmp = toString(physicMode);
        (*config)["physicmode"] = tmp;
      }

      SET_VALUE("origname", origName, false);
      (*config)["filename"] = filename_;
      SET_VALUE("groupid", groupID, writeDefaults);
      SET_VALUE("index", index, false);
      // todo: if a relative id is set, export the relative pos and rot!
      SET_OBJECT("position", pos, vector, true);
      SET_OBJECT("pivot", pivot, vector, true);
      SET_OBJECT("rotation", rot, quaternion, true);
      SET_OBJECT("extend", ext, vector, true);
      SET_VALUE("relativeid", relative_id, writeDefaults);

      if(terrain) {
        (*config)["t_srcname"] = srcname_;
        (*config)["t_width"] = terrain->targetWidth;
        (*config)["t_height"] = terrain->targetHeight;
        (*config)["t_scale"] = terrain->scale;
        (*config)["t_tex_scale_x"] = terrain->texScaleX;
        (*config)["t_tex_scale_y"] = terrain->texScaleY;
      }

      SET_OBJECT("visualposition", visual_offset_pos, vector, true);
      SET_OBJECT("visualrotation", visual_offset_rot, quaternion, true);
      SET_OBJECT("visualsize", visual_size, vector, true);
      SET_OBJECT("visualscale", visual_scale, vector, true);

      SET_VALUE("cmax_num_contacts", c_params.max_num_contacts, writeDefaults);
      SET_VALUE("cerp", c_params.erp, writeDefaults);
      SET_VALUE("ccfm", c_params.cfm, writeDefaults);
      SET_VALUE("cfriction1", c_params.friction1, writeDefaults);
      SET_VALUE("cfriction2", c_params.friction2, writeDefaults);
      SET_VALUE("cmotion1", c_params.motion1, writeDefaults);
      SET_VALUE("cmotion2", c_params.motion2, writeDefaults);
      SET_VALUE("cfds1", c_params.fds1, writeDefaults);
      SET_VALUE("cfds2", c_params.fds2, writeDefaults);
      SET_VALUE("cbounce", c_params.bounce, writeDefaults);
      SET_VALUE("cbounce_vel", c_params.bounce_vel, writeDefaults);
      SET_VALUE("capprox", c_params.approx_pyramid, writeDefaults);
      SET_VALUE("coll_bitmask", c_params.coll_bitmask, writeDefaults);
      if(c_params.friction_direction1) {
        vectorToConfigItem((*config)["cfdir1"],
                           c_params.friction_direction1);
      }

      SET_VALUE("inertia", inertia_set, false);
      SET_VALUE("i00", inertia[0][0], false);
      SET_VALUE("i01", inertia[0][1], false);
      SET_VALUE("i02", inertia[0][2], false);
      SET_VALUE("i10", inertia[1][0], false);
      SET_VALUE("i11", inertia[1][1], false);
      SET_VALUE("i12", inertia[1][2], false);
      SET_VALUE("i20", inertia[2][0], false);
      SET_VALUE("i21", inertia[2][1], false);
      SET_VALUE("i22", inertia[2][2], false);

      SET_VALUE("linear_damping", linear_damping, writeDefaults);
      SET_VALUE("angular_damping", angular_damping, writeDefaults);
      SET_VALUE("angular_low", angular_low, writeDefaults);

      SET_VALUE("shadow_id", shadow_id, writeDefaults);
      SET_VALUE("shadowcaster", isShadowCaster, writeDefaults);
      SET_VALUE("shadowreceiver", isShadowReceiver, writeDefaults);
      SET_VALUE("material", material.name, writeDefaults);

    }

    void NodeData::getFilesToSave(std::vector<std::string> *fileList) {
      if(!filename.empty() && filename != "PRIMITIVE")
        fileList->push_back(filename);
      if(terrain)
        fileList->push_back(terrain->srcname);
    }


  } // end of namespace interfaces

} // end of namespace mars
