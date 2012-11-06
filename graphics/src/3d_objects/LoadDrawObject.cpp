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
#include "MarsVBOGeom.h"

#include <osg/ComputeBoundsVisitor>
#include <osg/CullFace>
#include <osgUtil/Optimizer>

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
      std::list< osg::ref_ptr< osg::Geode > > geodes;
      bool found = false;

      if(info_.fileName.substr(info_.fileName.size()-5, 5) == ".bobj") {
        //readBobjFormat(info_.fileName);
        geodes.push_back(readBobjFormat(info_.fileName));
      }
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
          if (readNode->getName() == info_.objectName) {
            geodes.push_back(readNode->asGeode());
            found = true;
          }
        }

        if(!found) {
          std::cerr << "Failed to load object '" << info_.objectName
                    << "' from file '" << info_.fileName << "'" << endl;
        }
      }
      return geodes;
    }

    osg::Geode* LoadDrawObject::readBobjFormat(const std::string &filename) {
      FILE* input = fopen(filename.c_str(), "rb");
      if(!input) return 0;
      char buffer[312];

      int da, i, r, o, foo=0;
      int iData[3];
      float fData[4];

      osg::Geode *geode = new osg::Geode();
      std::vector<osg::Vec3> vertices;
      std::vector<osg::Vec3> normals;
      std::vector<osg::Vec2> texcoords;

      std::vector<osg::Vec3> vertices2;
      std::vector<osg::Vec3> normals2;
      std::vector<osg::Vec2> texcoords2;

      osg::ref_ptr<osg::Vec3Array> osgVertices = new osg::Vec3Array();
      osg::ref_ptr<osg::Vec2Array> osgTexcoords = new osg::Vec2Array();
      osg::ref_ptr<osg::Vec3Array> osgNormals = new osg::Vec3Array();

      while((r = fread(buffer+foo, 1, 256, input)) > 0 ) {
        o = 0;
        while(o < r+foo-50 || (r<256 && o < r+foo)) {
          da = *(int*)(buffer+o);
          o += 4;
          if(da == 1) {
            for(i=0; i<3; i++) {
              fData[i] = *(float*)(buffer+o);
              o+=4;
            }
            vertices.push_back(osg::Vec3(fData[0], fData[1], fData[2]));
          }
          else if(da == 2) {
            for(i=0; i<2; i++) {
              fData[i] = *(float*)(buffer+o);
              o+=4;
            }
            texcoords.push_back(osg::Vec2(fData[0], fData[1]));
          }
          else if(da == 3) {
            for(i=0; i<3; i++) {
              fData[i] = *(float*)(buffer+o);
              o+=4;
            }
            normals.push_back(osg::Vec3(fData[0], fData[1], fData[2]));
          }
          else if(da == 4) {
            for(i=0; i<3; i++) {
              iData[i] = *(int*)(buffer+o);
              o+=4;
            }
            // add osg vertices etc.
            osgVertices->push_back(vertices[iData[0]-1]);
            vertices2.push_back(vertices[iData[0]-1]);
            if(iData[1] > 0) {
              osgTexcoords->push_back(texcoords[iData[1]-1]);
              texcoords2.push_back(texcoords[iData[1]-1]);
            }
            osgNormals->push_back(normals[iData[2]-1]);
            normals2.push_back(normals[iData[2]-1]);

            for(i=0; i<3; i++) {
              iData[i] = *(int*)(buffer+o);
              o+=4;
            }
            // add osg vertices etc.
            osgVertices->push_back(vertices[iData[0]-1]);
            vertices2.push_back(vertices[iData[0]-1]);
            if(iData[1] > 0) {
              osgTexcoords->push_back(texcoords[iData[1]-1]);
              texcoords2.push_back(texcoords[iData[1]-1]);
            }
            osgNormals->push_back(normals[iData[2]-1]);
            normals2.push_back(normals[iData[2]-1]);

            for(i=0; i<3; i++) {
              iData[i] = *(int*)(buffer+o);
              o+=4;
            }
            // add osg vertices etc.
            osgVertices->push_back(vertices[iData[0]-1]);
            vertices2.push_back(vertices[iData[0]-1]);
            if(iData[1] > 0) {
              osgTexcoords->push_back(texcoords[iData[1]-1]);
              texcoords2.push_back(texcoords[iData[1]-1]);
            }
            osgNormals->push_back(normals[iData[2]-1]);
            normals2.push_back(normals[iData[2]-1]);
          }
        }
        foo = r+foo-o;
        if(r==256) memcpy(buffer, buffer+o, foo);
      }

#ifdef USE_MARS_VBO
      MarsVBOGeom *geometry = new MarsVBOGeom();
      geometry->setVertexArray(vertices2);
      geometry->setNormalArray(normals2);
      if(osgTexcoords->size() > 0)
        geometry->setTexCoordArray(texcoords2);
#else
      osg::Geometry* geometry = new osg::Geometry;
      geometry->setVertexArray(osgVertices.get());
      geometry->setNormalArray(osgNormals.get());
      geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
      if(osgTexcoords->size() > 0)
        geometry->setTexCoordArray(0, osgTexcoords.get());

      osg::DrawArrays* drawArrays = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,0,osgVertices->size());
      geometry->addPrimitiveSet(drawArrays);
#endif
      geode->addDrawable(geometry);
      geode->setName("bobj");

      fclose(input);
      osgUtil::Optimizer optimizer;
      optimizer.optimize( geode );
      return geode;
    }

  } // end of graphics
} // end of mars
