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

#include <osg/ComputeBoundsVisitor>
#include <osg/CullFace>

#include <iostream>
#include <cstdio>

namespace mars {
  namespace graphics {

    using namespace std;

    LoadDrawObject::LoadDrawObject(LoadDrawObjectInfo &inf, 
                                   const mars::utils::Vector &ext)
      : DrawObject(), info_(inf) {
    }

    std::list< osg::ref_ptr< osg::Geode > > LoadDrawObject::createGeometry() {
      osg::ref_ptr<osg::Node> readNode;
      osg::ref_ptr<osg::Geode> readGeode;
      std::list< osg::ref_ptr< osg::Geode > > geodes;
      bool found = false;

      if(info_.fileName.substr(info_.fileName.size()-5, 5) == ".bobj") {
        //readBobjFormat(info_.fileName);
        osg::ref_ptr<osg::Node> loadedNode = GuiHelper::readBobjFromFile(info_.fileName);
        if(!loadedNode.valid()) {
          std::cerr << "LoadDrawObject: no node loaded" << std::endl;
          return geodes; // TODO: error message
        }
        geodes.push_back(loadedNode->asGeode());
      }
      // import an .STL file
      else if((info_.fileName.substr(info_.fileName.size()-4, 4) == ".STL") ||
             (info_.fileName.substr(info_.fileName.size()-4, 4) == ".stl")) {
        osg::ref_ptr<osg::Node> loadedNode = GuiHelper::readNodeFromFile(info_.fileName);
        if(!loadedNode.valid()) {
          std::cerr << "LoadDrawObject: no node loaded" << std::endl;
          return geodes; // TODO: error message
        }
        // if the file is a .STL file, OSG read the node as a geode not as a
        // group
        readGeode = loadedNode->asGeode();
        if(!readGeode.valid()) {
          std::cerr << "LoadDrawObject: no geode found " << info_.fileName << std::endl;
          osg::ref_ptr<osg::Group> readGroup = loadedNode->asGroup();
          if(!readGroup.valid()) {
            std::cerr << "LoadDrawObject: no group found" << std::endl;
            return geodes; // TODO: error message
          }

          for (unsigned int i = 0; i < readGroup->getNumChildren(); ++i) {
            readNode = readGroup->getChild(i);
            if (readNode->getName() == info_.objectName || info_.objectName == "") {
              geodes.push_back(readNode->asGeode());
              found = true;
            }
          }

          std::list< osg::ref_ptr< osg::Geode > >::iterator it=geodes.begin();
          for(;it!=geodes.end(); ++it) {
            for(unsigned int i=0; i<(*it)->getNumDrawables(); ++i) {
              (*it)->getDrawable(i)->setUseDisplayList(false);
              (*it)->getDrawable(i)->setUseVertexBufferObjects(true);
            }
          }

          return geodes; // TODO: error message
        }
        geodes.push_back(readGeode);
      }
      // import an .OBJ file
      else {
        osg::ref_ptr<osg::Node> loadedNode = GuiHelper::readNodeFromFile(info_.fileName);
        if(!loadedNode.valid()) {
          std::cerr << "LoadDrawObject: no node loaded" << std::endl;
          return geodes; // TODO: error message
        }
        osg::ref_ptr<osg::Group> readGroup = loadedNode->asGroup();
        if(!readGroup.valid()) {
          std::cerr << "LoadDrawObject: no group found" << std::endl;
          return geodes; // TODO: error message
        }

        for (unsigned int i = 0; i < readGroup->getNumChildren(); ++i) {
          readNode = readGroup->getChild(i);
          if (readNode->getName() == info_.objectName || info_.objectName == "") {
            geodes.push_back(readNode->asGeode());
            found = true;
          }
        }

        if(!found) {
          std::cerr << "Failed to load object '" << info_.objectName
                    << "' from file '" << info_.fileName << "'" << endl;
        }
      }

      std::list< osg::ref_ptr< osg::Geode > >::iterator it=geodes.begin();
      for(;it!=geodes.end(); ++it) {
        for(unsigned int i=0; i<(*it)->getNumDrawables(); ++i) {
          (*it)->getDrawable(i)->setUseDisplayList(false);
          (*it)->getDrawable(i)->setUseVertexBufferObjects(true);
        }
      }

      return geodes;
    }

  } // end of graphics
} // end of mars
