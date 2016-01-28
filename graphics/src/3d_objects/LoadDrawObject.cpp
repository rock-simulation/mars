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

/*
 *  LoadDrawObject.cpp
 *  General LoadDrawObject to inherit from.
 *
 *  Created by Roemmermann on 21.10.09.
 */

#include "LoadDrawObject.h"
#include "gui_helper_functions.h"
#include "MarsMaterial.h"

#include <osg/ComputeBoundsVisitor>
#include <osg/CullFace>

#include <iostream>
#include <cstdio>

namespace mars {
  namespace graphics {

    using namespace std;

    LoadDrawObject::LoadDrawObject(GraphicsManager *g,
                                   configmaps::ConfigMap &map,
                                   const mars::utils::Vector &ext)
      : DrawObject(g), info_(map) {
    }

    std::list< osg::ref_ptr< osg::Geode > > LoadDrawObject::createGeometry() {
      osg::ref_ptr<osg::Node> readNode;
      osg::ref_ptr<osg::Geode> readGeode;
      std::list< osg::ref_ptr< osg::Geode > > geodes;
      bool found = false;
      std::string filename;
      std::string p = ".";

      if(info_.find("filePrefix") != info_.end()) {
        p = (std::string)info_["filePrefix"];
      };

      if(info_.find("lod") != info_.end()) {
        double start, end;
        configmaps::ConfigVector::iterator it;
        for(it=info_["lod"].begin(); it!=info_["lod"].end(); ++it) {
          start = (*it)["start"];
          end = (*it)["end"];
          filename = (std::string)(*it)["filename"];
          if(filename[0] != '/') {
            filename = p+"/"+filename;
          }
          geodes = loadGeodes(filename, "");
          addLODGeodes(geodes, start, end);
        }
      }
      filename = (std::string)info_["filename"];
      if(filename[0] != '/') {
        filename = p+"/"+filename;
      }
      return loadGeodes(filename, (std::string)info_["origname"]);
    }

    std::list< osg::ref_ptr< osg::Geode > > LoadDrawObject::loadGeodes(std::string filename, std::string objname) {
      std::list< osg::ref_ptr< osg::Geode > > geodes;
      bool found = false;

      if(filename.substr(filename.size()-5, 5) == ".bobj") {
        osg::ref_ptr<osg::Node> loadedNode = GuiHelper::readBobjFromFile(filename);
        if(!loadedNode.valid()) {
          std::cerr << "LoadDrawObject: no node loaded" << std::endl;
          return geodes; // TODO: error message
        }
        found = true;
        geodes.push_back(loadedNode->asGeode());
      }
      // import an .STL file
      else if((filename.substr(filename.size()-4, 4) == ".STL") ||
              (filename.substr(filename.size()-4, 4) == ".stl") ||
              (filename.substr(filename.size()-4, 4) == ".obj")) {
        osg::ref_ptr<osg::Node> loadedNode = GuiHelper::readNodeFromFile(filename);
        if(!loadedNode.valid()) {
          std::cerr << "LoadDrawObject: no node loaded" << std::endl;
          return geodes; // TODO: error message
        }
        osg::ref_ptr<osg::Geode> readGeode = loadedNode->asGeode();
        if(!readGeode.valid()) {
          osg::ref_ptr<osg::Group> readGroup = loadedNode->asGroup();
          if(!readGroup.valid()) {
            std::cerr << "LoadDrawObject: no geode or group found " << filename << std::endl;
            return geodes; // TODO: error message
          }

          for (unsigned int i = 0; i < readGroup->getNumChildren(); ++i) {
            osg::ref_ptr<osg::Node> readNode = readGroup->getChild(i);
            if (objname == "" || readNode->getName() == objname) {
              geodes.push_back(readNode->asGeode());
              found = true;
            }
          }
        } else {
          geodes.push_back(readGeode);
        }
      }

      if(!found) {
        std::cerr << "Failed to load object '" << objname
                  << "' from file '" << filename << "'" << endl;
      }
      else {
        std::list< osg::ref_ptr< osg::Geode > >::iterator it=geodes.begin();
        for(;it!=geodes.end(); ++it) {
          for(unsigned int i=0; i<(*it)->getNumDrawables(); ++i) {
            (*it)->getDrawable(i)->setUseDisplayList(false);
            (*it)->getDrawable(i)->setUseVertexBufferObjects(true);
          }
        }
      }

      return geodes;
    }

  } // end of graphics
} // end of mars
