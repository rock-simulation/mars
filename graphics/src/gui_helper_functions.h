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

#ifndef GUI_HELPER_FUCTIONS_H
#define GUI_HELPER_FUCTIONS_H

#ifdef _PRINT_HEADER_
  #warning "gui_helper_functions.h"
#endif

#include <osg/Group>
#include <osg/Node>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/PositionAttitudeTransform>

#include <vector>
#include <sstream>

#include <mars/interfaces/sim_common.h>
#include <mars/interfaces/terrainStruct.h>
#include <mars/interfaces/sim/LoadCenter.h>

#include <mars/interfaces/graphics/GraphicsManagerInterface.h>


namespace mars {
  namespace graphics {

    class GraphicsWidget;

    /**
     * internal struct for GraphicManager.h connecting the node indices
     * with the osg nodes
     */
    struct nodemanager {
      osg::ref_ptr<osg::Group> node;
      osg::ref_ptr<osg::PositionAttitudeTransform> transform;
      osg::ref_ptr<osg::MatrixTransform> matrix;
      unsigned long index;
      std::string name;
      mars::utils::Vector offset;
      mars::utils::Quaternion r_off;
      mars::interfaces::NodeData snode;
    }; // end of struct nodemanager

    struct nodeFileStruct {
      std::string fileName;
      osg::ref_ptr<osg::Node> node;
    }; // end of struct nodeFileStruct

    struct textureFileStruct {
      std::string fileName;
      osg::ref_ptr<osg::Texture2D> texture;
    }; // end of struct textureFileStruct

    struct imageFileStruct {
      std::string fileName;
      osg::ref_ptr<osg::Image> image;
    }; // end of struct imageFileStruct

    osg::Vec4 toOSGVec4(const mars::utils::Color &col);
    osg::Vec4 toOSGVec4(const mars::utils::Vector &v, float w);

    /** \brief used to get a geode from a nodegroup */
    class GeodeVisitor: public osg::NodeVisitor{
    public:
      GeodeVisitor(const std::string name);

      virtual void apply(osg::Node &searchNode);

      osg::Geode *getNode();

    private:
      osg::Geode *resultNode;
      std::string name;

    }; // end of class GeodeVisitor


    class GuiHelper : public interfaces::LoadMeshInterface,
                      public interfaces::LoadHeightmapInterface {
    public:
      GuiHelper(interfaces::GraphicsManagerInterface *gi);

      //void setGraphicsWidget(GraphicsWidget *widget);
      //bool validateGraphicsWidget(void);

      static void clearStates(osg::ref_ptr<osg::Node> node);
      /** \brief converts the mesh of an osgNode to the snmesh struct */
      static mars::interfaces::snmesh convertOsgNodeToSnMesh(osg::Node *node, 
                                                       double scaleX,
                                                       double scaleY,
                                                       double scaleZ,
                                                       double pivotX,
                                                       double pivotY,
                                                       double pivotZ);

      mars::utils::Vector getExtend(osg::Group* oGroup);
      void initGraphics();

      virtual void getPhysicsFromMesh(mars::interfaces::NodeData *node);
      virtual void readPixelData(mars::interfaces::terrainStruct *terrain);

      static osg::ref_ptr<osg::Node> readNodeFromFile(std::string fileName);
      static osg::ref_ptr<osg::Node> readBobjFromFile(const std::string &filename);
      static osg::ref_ptr<osg::Texture2D> loadTexture(std::string filename);
      static osg::ref_ptr<osg::Image> loadImage(std::string filename);

    private:
      osg::Geometry *my_geo;
      osg::ref_ptr<osg::Vec3Array> my_v3;
      interfaces::GraphicsManagerInterface *gi;
      //GraphicsWidget *gw;
      //for compatibility
      mars::interfaces::GraphicData gs;
      static std::vector<nodeFileStruct> nodeFiles;
      // vector to prevent double load of textures
      static std::vector<textureFileStruct> textureFiles;
      // vector to prevent double load of images
      static std::vector<imageFileStruct> imageFiles;
      void getPhysicsFromNode(mars::interfaces::NodeData* node,
                              osg::ref_ptr<osg::Node> completeNode);
    }; // end of class GuiHelper

  } // end of namespace graphics
} // end of namespace mars

#endif  // GUI_HELPER_FUCTIONS_H
