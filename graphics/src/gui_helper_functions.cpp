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

#include "gui_helper_functions.h"
#include <iostream>
#include <osg/TriangleFunctor>
#include <osgDB/ReadFile>

//#include <osgDB/WriteFile>
#include <osg/MatrixTransform>
#include <osg/ComputeBoundsVisitor>
#include <osgUtil/Optimizer>

#ifdef HAVE_OSG_VERSION_H
#include <osg/Version>
#else
#include <osg/Export>
#endif

#include <opencv2/opencv.hpp>

#include <mars_utils/mathUtils.h>

namespace mars {
  namespace graphics {

    using namespace std;
    using mars::utils::Color;
    using mars::utils::Vector;
    using mars::utils::Quaternion;
    using mars::interfaces::snmesh;

    vector<nodeFileStruct> GuiHelper::nodeFiles;
    vector<textureFileStruct> GuiHelper::textureFiles;
    vector<imageFileStruct> GuiHelper::imageFiles;

    /////////////

    osg::Vec4 toOSGVec4(const Color &col)
    {
      return osg::Vec4(col.r, col.g, col.b, col.a);
    }
    osg::Vec4 toOSGVec4(const Vector &v, float w)
    {
      return osg::Vec4(v.x(), v.y(), v.z(), w);
    }

    //////////////

    GeodeVisitor::GeodeVisitor(const std::string name):osg::NodeVisitor(TRAVERSE_ALL_CHILDREN){
      resultNode=NULL;
      this->name=name;
    }

    void GeodeVisitor::apply(osg::Node &searchNode){
      //go through node and return geode
      osg::Geode* dynamicTry=searchNode.asGeode();
      if(dynamicTry) {
        resultNode=dynamicTry;
        return;
      }
      else resultNode = 0;
      traverse(searchNode);
    }

    osg::Geode* GeodeVisitor::getNode(){
      return resultNode;
    }

    //////////////

    /** \brief This visitor class is used to get triangles from a mesh */
    class GetVerticesFunctor {
    public:
      // This will store all triangle vertices.
      std::vector<osg::Vec3> vertices;

      // When triangle vertices are added to 'vertices', they'll be transformed
      // by this matrix. This is useful because 'osg::TriangleFunctor' operates
      // on the model coordinate system, and we want do draw our normals in the
      // world coordinate system.
      osg::Matrix transformMatrix;

      // This will be called once for each triangle in the geometry. As
      // parameters, it takes the three triangle vertices and a boolean
      // parameter indicating if the vertices are coming from a "real" vertex
      // array or from a temporary vertex array created from some other geometry
      // representation.
      // The implementation is quite simple: we just store the vertices
      // (transformed by 'transformMatrix') in a 'std::vector'.
#if (OPENSCENEGRAPH_MAJOR_VERSION < 3 || ( OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION < 5) || ( OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION == 5 && OPENSCENEGRAPH_PATCH_VERSION < 9))
      void operator() (const osg::Vec3& v1, const osg::Vec3& v2,
                       const osg::Vec3& v3, bool treatVertexDataAsTemporary) {
        (void)treatVertexDataAsTemporary;
#elif (OPENSCENEGRAPH_MAJOR_VERSION > 3 || (OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION > 5) || (OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION == 5 && OPENSCENEGRAPH_PATCH_VERSION >= 9))
        void operator() (const osg::Vec3& v1, const osg::Vec3& v2,
                         const osg::Vec3& v3) {
#else
#error Unknown OSG Version
#endif
          vertices.push_back (v1 * transformMatrix);
          vertices.push_back (v2 * transformMatrix);
          vertices.push_back (v3 * transformMatrix);
        }

        vector<osg::Vec3>& getVertices() {
          return vertices;
        }
      };

      ///////////////////

      GuiHelper::GuiHelper(interfaces::GraphicsManagerInterface *gi) {
        this->gi = gi;
      }

      /*
        void GuiHelper::setGraphicsWidget(GraphicsWidget *widget){
        this->gw = widget;
        }

        bool GuiHelper::validateGraphicsWidget(void){
        return ( this->gw != NULL && this->gw != 0 );
        }
      */

      void GuiHelper::clearStates(osg::ref_ptr<osg::Node> node) {
        osg::ref_ptr<osg::Group> group;
        if(node != 0) {
          node->getOrCreateStateSet()->clear();
          group = node->asGroup();
          if (group != 0)
            for (unsigned int i=0; i<group->getNumChildren(); i++)
              clearStates(group->getChild(i));
        }
      }

      /**
       * converts the mesh of an osgNode to the snmesh struct
       */
      snmesh GuiHelper::convertOsgNodeToSnMesh(osg::Node* node, double scaleX,
                                               double scaleY, double scaleZ,
                                               double pivotX, double pivotY,
                                               double pivotZ) {
        vector<int> indices;
        vector<osg::Vec3> OSGvertices;
        snmesh mesh;
        //visitor for getting drawables inside node
        GeodeVisitor visitor("PLACEHOLDER");
        osg::Geode* geode;
        osg::Node* tmpNode;
        osg::ref_ptr<osg::Group> osgGroupFromRead = NULL;
        if ((osgGroupFromRead = node->asGroup()) == 0) {
          fprintf(stderr, "error\n");
        }
        // todo: parse the tree recursive and search for drawables
        //get geometries of node
        int indexcounter = 0;
        for (size_t m = 0; m < osgGroupFromRead->getNumChildren(); m++) {
          tmpNode = osgGroupFromRead->getChild(m);

          tmpNode->accept(visitor);
          if(!(geode = visitor.getNode())) {
            fprintf(stderr, "ERROR: getting node for mesh\n");
            continue;
          }
          for (size_t j = 0; j<geode->getNumDrawables(); j++) {
            //initialize functors for getting the drawables
            osg::TriangleFunctor<GetVerticesFunctor> triangleFunctor_;
            geode->getDrawable(j)->accept(triangleFunctor_);

            //Here we get the triangles
            vector<osg::Vec3>& OSGverticestemp = triangleFunctor_.getVertices();
            for (unsigned int i = 0; i < OSGverticestemp.size(); i++) {
              OSGvertices.push_back(OSGverticestemp[i]);
              indices.push_back(indexcounter);
              indexcounter++;
            }
          }
        }

        //store vertices in a mydVector3 structure
        mars::interfaces::mydVector3 *vertices = 0;
        if(OSGvertices.size() > 0){
          vertices = new mars::interfaces::mydVector3[OSGvertices.size()];
        }
        //  dVector3 *normals = new dVector3[normals_x.size()];
        int *indexarray = 0;
        if(indices.size() > 0){
          indexarray = new int[indices.size()];
        }

        //convert osg vertice vector to standard array
        for (size_t i = 0; i < OSGvertices.size(); i++) {
          vertices[i][0] = (OSGvertices[i][0] - pivotX) * scaleX;
          vertices[i][1] = (OSGvertices[i][1] - pivotY) * scaleY;
          vertices[i][2] = (OSGvertices[i][2] - pivotZ) * scaleZ;
        }

        //construct an appropriate index array
        for (int i = 0; i < indexcounter; i++) {
          indexarray[i] = indices[i];
        }

        mesh.vertices = vertices;
        mesh.vertexcount = OSGvertices.size();
        //  mesh.normals = normals;
        mesh.indices = indexarray;
        mesh.indexcount = indexcounter;
        return mesh;
      }

      Vector GuiHelper::getExtend(osg::Node *oGroup){
        osg::ComputeBoundsVisitor cbbv;
        oGroup->accept(cbbv);
        osg::BoundingBox bb = cbbv.getBoundingBox();
        Vector ex;
        // compute bounding box has to be done in this way
        (fabs(bb.xMax()) > fabs(bb.xMin())) ? ex.x() = fabs(bb.xMax() - bb.xMin())
          : ex.x() = fabs(bb.xMin() - bb.xMax());
        (fabs(bb.yMax()) > fabs(bb.yMin())) ? ex.y() = fabs(bb.yMax() - bb.yMin())
          : ex.y() = fabs(bb.yMin() - bb.yMax());
        (fabs(bb.zMax()) > fabs(bb.zMin())) ? ex.z() = fabs(bb.zMax() - bb.zMin())
          : ex.z() = fabs(bb.zMin() - bb.zMax());

        return ex;
      }

      std::vector<double> GuiHelper::getMeshSize(const std::string &filename) {
        std::vector<double> r;
        Vector size(0, 0, 0);
        if(filename.substr(filename.size()-5, 5) == ".bobj") {
          size = getExtend(GuiHelper::readBobjFromFile(filename));
        }
        else {
          size = getExtend(GuiHelper::readNodeFromFile(filename));
        }
        r.push_back(size.x());
        r.push_back(size.y());
        r.push_back(size.z());
        return r;
      }

      void GuiHelper::getPhysicsFromMesh(mars::interfaces::NodeData* node) {
        if(node->filename.substr(node->filename.size()-5, 5) == ".bobj") {
          getPhysicsFromNode(node, GuiHelper::readBobjFromFile(node->filename));
        }
        else {
          getPhysicsFromNode(node, GuiHelper::readNodeFromFile(node->filename));
        }
      }

      void GuiHelper::getPhysicsFromNode(mars::interfaces::NodeData* node,
                                         osg::ref_ptr<osg::Node> completeNode) {
        osg::ref_ptr<osg::Group> myCreatedGroup;
        osg::ref_ptr<osg::Group> myGroupFromRead;
        osg::ref_ptr<osg::Geode> myGeodeFromRead;
        nodemanager tempnode;
        bool found = false;
        // check whether it is a osg::Group (.obj file)
        if(!completeNode.valid()){
          throw std::runtime_error("cannot read node from file");
        }

        if((myGeodeFromRead = completeNode->asGeode()) != 0) {
          //if the node was read from a .stl-file it read as geode not as group
          myCreatedGroup = new osg::Group();
          osg::ref_ptr<osg::StateSet> stateset = myCreatedGroup->getOrCreateStateSet();
          completeNode->setStateSet(stateset.get());
          myCreatedGroup->addChild(completeNode.get());
        }
        else if((myGroupFromRead = completeNode->asGroup()) != 0){
          //go through the read node group and combine the parts of the actually
          //handled node
          myCreatedGroup = new osg::Group();
          osg::ref_ptr<osg::StateSet> stateset = myCreatedGroup->getOrCreateStateSet();
          for (unsigned int i = 0; i < myGroupFromRead->getNumChildren(); i ++) {
            osg::ref_ptr<osg::Node> myTestingNode = myGroupFromRead->getChild(i);
            if (myTestingNode == 0) {
              return;
            }
            if (myTestingNode->getName() == node->origName or node->origName.size() == 0) {
              myTestingNode->setStateSet(stateset.get());
              myCreatedGroup->addChild(myTestingNode.get());
              found = true;
            } else {
              if (found) {
                break;
                found = false;
              }
            }
          }
        }

        osg::ComputeBoundsVisitor cbbv;
        myCreatedGroup.get()->accept(cbbv);
        osg::BoundingBox bb = cbbv.getBoundingBox();
        Vector ex;
        // compute bounding box has to be done in this way
        (fabs(bb.xMax()) > fabs(bb.xMin())) ? ex.x() = fabs(bb.xMax() - bb.xMin())
          : ex.x() = fabs(bb.xMin() - bb.xMax());
        (fabs(bb.yMax()) > fabs(bb.yMin())) ? ex.y() = fabs(bb.yMax() - bb.yMin())
          : ex.y() = fabs(bb.yMin() - bb.yMax());
        (fabs(bb.zMax()) > fabs(bb.zMin())) ? ex.z() = fabs(bb.zMax() - bb.zMin())
          : ex.z() = fabs(bb.zMin() - bb.zMax());

        if (node->map.find("loadSizeFromMesh") != node->map.end()) {
          if (node->map["loadSizeFromMesh"]) {
            Vector physicalScale;
            utils::vectorFromConfigItem(&(node->map["physicalScale"][0]), &physicalScale);
            node->ext=Vector(ex.x()*physicalScale.x(), ex.y()*physicalScale.y(), ex.z()*physicalScale.z());
          }
        }

        //compute scale factor
        double scaleX = 1, scaleY = 1, scaleZ = 1;
        if (ex.x() != 0) scaleX = node->ext.x() / ex.x();
        if (ex.y() != 0) scaleY = node->ext.y() / ex.y();
        if (ex.z() != 0) scaleZ = node->ext.z() / ex.z();

        // create transform and group Node for the actual node
        osg::ref_ptr<osg::PositionAttitudeTransform> transform;
        osg::ref_ptr<osg::MatrixTransform> tx;

        transform = new osg::PositionAttitudeTransform();
        tx = new osg::MatrixTransform;
        tx->setMatrix(osg::Matrix::scale(scaleX, scaleY, scaleZ));
        tx->setDataVariance(osg::Node::STATIC);
        tx->addChild(myCreatedGroup.get());

        //add the node to a transformation to make him movable
        transform->addChild(tx.get());
        transform->setPivotPoint(osg::Vec3(node->pivot.x()*scaleX,
                                           node->pivot.y()*scaleY,
                                           node->pivot.z()*scaleZ));
        transform->setPosition(osg::Vec3(node->pos.x() + node->visual_offset_pos.x(),
                                         node->pos.y() + node->visual_offset_pos.y(),
                                         node->pos.z() + node->visual_offset_pos.z()));
        //set rotation
        osg::Quat oquat;
        Quaternion qrot = node->rot * node->visual_offset_rot;
        oquat.set(qrot.x(), qrot.y(), qrot.z(), qrot.w());
        transform->setAttitude(oquat);

        tempnode.transform = transform.get();
        tempnode.node = myCreatedGroup.get();
        tempnode.matrix = tx.get();
        tempnode.offset = node->visual_offset_pos;
        tempnode.r_off = node->visual_offset_rot;

        node->mesh = GuiHelper::convertOsgNodeToSnMesh(tempnode.node.get(),
                                                       scaleX, scaleY, scaleZ,
                                                       node->pivot.x(),
                                                       node->pivot.y(),
                                                       node->pivot.z());
      }

      osg::ref_ptr<osg::Node> GuiHelper::readNodeFromFile(string fileName) {
        std::vector<nodeFileStruct>::iterator iter;

        for(iter = GuiHelper::nodeFiles.begin();
            iter != GuiHelper::nodeFiles.end(); iter++) {
          if((*iter).fileName == fileName) return (*iter).node;
        }
        nodeFileStruct newNodeFile;
        newNodeFile.fileName = fileName;
        newNodeFile.node = osgDB::readNodeFile(fileName);
        GuiHelper::nodeFiles.push_back(newNodeFile);
        return newNodeFile.node;
      }


      osg::ref_ptr<osg::Node> GuiHelper::readBobjFromFile(const std::string &filename) {

        std::vector<nodeFileStruct>::iterator iter;

        for(iter = GuiHelper::nodeFiles.begin();
            iter != GuiHelper::nodeFiles.end(); iter++) {
          if((*iter).fileName == filename) return (*iter).node;
        }
        nodeFileStruct newNodeFile;
        newNodeFile.fileName = filename;

        FILE* input = fopen(filename.c_str(), "rb");
        if(!input) {
          fprintf(stderr, "ERROR: reading file: %s\n", filename.c_str());
          return 0;
        }
        char buffer[312];

        int da, i, r, o, foo=0;
        int iData[3];
        float fData[4];

        osg::Geode *geode = new osg::Geode();
        std::vector<osg::Vec3> vertices;
        std::vector<osg::Vec3> normals;
        std::vector<osg::Vec2> texcoords;
        std::vector<osg::Vec4> colors;

        std::vector<osg::Vec3> vertices2;
        std::vector<osg::Vec3> normals2;
        std::vector<osg::Vec2> texcoords2;
        std::vector<osg::Vec4> colors2;

        osg::ref_ptr<osg::Vec3Array> osgVertices = new osg::Vec3Array();
        osg::ref_ptr<osg::Vec2Array> osgTexcoords = new osg::Vec2Array();
        osg::ref_ptr<osg::Vec3Array> osgNormals = new osg::Vec3Array();
        osg::ref_ptr<osg::Vec4Array> osgColors;
        osg::ref_ptr<osg::DrawElementsUInt> osgIndices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
        bool useIndices = true;
        while((r = fread(buffer+foo, 1, 256, input)) > 0 ) {
          o = 0;
          while(o < r+foo-50 || (r<256 && o < r+foo)) {
            memcpy(&da, buffer+o, sizeof(int));
            //da = *(int*)(buffer+o);
            o += 4;
            if(da == 1) {
              for(i=0; i<3; i++) {
                memcpy(fData+i, buffer+o, sizeof(float));
                //fData[i] = *(float*)(buffer+o);
                o+=4;
              }
              vertices.push_back(osg::Vec3(fData[0], fData[1], fData[2]));
            }
            else if(da == 2) {
              for(i=0; i<2; i++) {
                memcpy(fData+i, buffer+o, sizeof(float));
                //fData[i] = *(float*)(buffer+o);
                o+=4;
              }
              texcoords.push_back(osg::Vec2(fData[0], fData[1]));
            }
            else if(da == 3) {
              for(i=0; i<3; i++) {
                memcpy(fData+i, buffer+o, sizeof(float));
                //fData[i] = *(float*)(buffer+o);
                o+=4;
              }
              normals.push_back(osg::Vec3(fData[0], fData[1], fData[2]));
            }
            else if(da == 4) {
              // 1. vertice
              for(i=0; i<3; i++) {
                memcpy(iData+i, buffer+o, sizeof(int));
                //iData[i] = *(int*)(buffer+o);
                o+=4;
              }
              if(iData[0] != iData[2]) {
                useIndices = false;
              }
              // add osg vertices etc.
              osgIndices->push_back(iData[0]-1);
              vertices2.push_back(vertices[iData[0]-1]);
              if(colors.size() == vertices.size()) {
                colors2.push_back(colors[iData[0]-1]);
              }
              if(iData[1] > 0) {
                texcoords2.push_back(texcoords[iData[1]-1]);
              }
              normals2.push_back(normals[iData[2]-1]);

              // 2. vertice
              for(i=0; i<3; i++) {
                memcpy(iData+i, buffer+o, sizeof(int));
                //iData[i] = *(int*)(buffer+o);
                o+=4;
              }
              if(iData[0] != iData[2]) {
                useIndices = false;
              }
              osgIndices->push_back(iData[0]-1);
              // add osg vertices etc.
              vertices2.push_back(vertices[iData[0]-1]);
              if(colors.size() == vertices.size()) {
                colors2.push_back(colors[iData[0]-1]);
              }
              if(iData[1] > 0) {
                texcoords2.push_back(texcoords[iData[1]-1]);
              }
              normals2.push_back(normals[iData[2]-1]);

              // 3. vertice
              for(i=0; i<3; i++) {
                memcpy(iData+i, buffer+o, sizeof(int));
                //iData[i] = *(int*)(buffer+o);
                o+=4;
              }
              if(iData[0] != iData[2]) {
                useIndices = false;
              }
              osgIndices->push_back(iData[0]-1);
              // add osg vertices etc.
              vertices2.push_back(vertices[iData[0]-1]);
              if(colors.size() == vertices.size()) {
                colors2.push_back(colors[iData[0]-1]);
              }
              if(iData[1] > 0) {
                texcoords2.push_back(texcoords[iData[1]-1]);
              }
              normals2.push_back(normals[iData[2]-1]);
            }
            else if(da == 5) { // vertex colors
              for(i=0; i<4; i++) {
                memcpy(fData+i, buffer+o, sizeof(float));
                //fData[i] = *(float*)(buffer+o);
                o+=4;
              }
              // debug indices:
              // float index = (fData[0]* 0.996108955f + fData[1]*0.003891045f)*65535.0f;
              // fprintf(stderr, " %ld->%ld", colors.size(), lroundf(index));
              colors.push_back(osg::Vec4(fData[0], fData[1], fData[2], fData[3]));
            }
          }
          foo = r+foo-o;
          if(r==256) memcpy(buffer, buffer+o, foo);
        }
        fprintf(stderr, "\n");

        if(useIndices) {
          if(colors.size() > 0) {
            osgColors = new osg::Vec4Array();
          }
          for(size_t i=0; i<vertices.size(); ++i) {
            osgVertices->push_back(vertices[i]);
            osgNormals->push_back(normals[i]);
            if(colors.size() > 0) {
              osgColors->push_back(colors[i]);
            }
            if(texcoords.size() > i) {
              osgTexcoords->push_back(texcoords[i]);
            }
          }
        }
        else {
          for(size_t i=0; i<vertices2.size(); ++i) {
            osgVertices->push_back(vertices2[i]);
          }
          for(size_t i=0; i<normals2.size(); ++i) {
            osgNormals->push_back(normals2[i]);
          }
          for(size_t i=0; i<texcoords2.size(); ++i) {
            osgTexcoords->push_back(texcoords2[i]);
          }
          if(colors2.size() > 0) {
            osgColors = new osg::Vec4Array();
          }
          for(size_t i=0; i<colors2.size(); ++i) {
            osgColors->push_back(colors2[i]);
          }
        }

        osg::Geometry* geometry = new osg::Geometry;
        geometry->setVertexArray(osgVertices.get());
        geometry->setNormalArray(osgNormals.get());
        geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        if(osgColors.valid()) {
          fprintf(stderr, "Bobj: add colors per vertex. %ld - %ld\n", osgVertices->size(), osgColors->size());
          geometry->setColorArray(osgColors.get());
          geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
        }
        if(osgTexcoords->size() > 0) {
          geometry->setTexCoordArray(0, osgTexcoords.get());
        }
        if(useIndices) {
          geometry->addPrimitiveSet(osgIndices);
        }
        else {
          osg::DrawArrays* drawArrays = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,0,osgVertices->size());
          geometry->addPrimitiveSet(drawArrays);
        }
        geode->addDrawable(geometry);
        geode->setName("bobj");

        fclose(input);
        osgUtil::Optimizer optimizer;
        //optimizer.optimize( geode );

        newNodeFile.node = geode;
        GuiHelper::nodeFiles.push_back(newNodeFile);
        return newNodeFile.node;
      }

      // TODO: should not be in graphics!
      // physics without graphics would need this too.
      // maybe move to NodeFactory ??
      void GuiHelper::readPixelData(mars::interfaces::terrainStruct *terrain) {

        cv::Mat img;

        img=cv::imread(terrain->srcname, cv::IMREAD_ANYDEPTH);
        if(img.data) {
          terrain->width = img.cols;
          terrain->height = img.rows;
          fprintf(stderr, "w h = %d %d\n", img.cols, img.rows);
          terrain->pixelData = (double*)calloc((terrain->width*
                                                terrain->height),
                                               sizeof(double));

          if(img.channels() != 1) {
            fprintf(stderr, "Error: loading heightmap data. Only single channel grayscale images can be loaded!\n");
            return;
          }
          cv::Scalar s;
          int count = 0;
          int depth = img.depth();
          double imageMaxValue = pow(2., 8);
          if(depth==CV_16U) {
            imageMaxValue = pow(2., 16);
          }
          for(int y=terrain->height-1; y>=0; y--) {
            for(int x=0; x<terrain->width; x++) {
              if(depth == CV_16U) {
                s = img.at<ushort>(y,x);
              } else {
                s = img.at<uchar>(y,x);
              }
              terrain->pixelData[count++] = ((double)s[0])/(imageMaxValue-1);
            }
          }
          img.release();
        }
      }

      osg::ref_ptr<osg::Texture2D> GuiHelper::loadTexture(string filename) {
        std::vector<textureFileStruct>::iterator iter;

        for (iter = textureFiles.begin();
             iter != textureFiles.end(); iter++) {
          if ((*iter).fileName == filename) {
            return (*iter).texture;
          }
        }
        textureFileStruct newTextureFile;
        newTextureFile.fileName = filename;
        newTextureFile.texture = new osg::Texture2D;
        newTextureFile.texture->setDataVariance(osg::Object::DYNAMIC);
        newTextureFile.texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
        newTextureFile.texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
        newTextureFile.texture->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);

        osg::Image* textureImage = loadImage(filename);
        newTextureFile.texture->setImage(textureImage);
        textureFiles.push_back(newTextureFile);

        return newTextureFile.texture;
      }

      osg::ref_ptr<osg::Image> GuiHelper::loadImage(string filename) {
        std::vector<imageFileStruct>::iterator iter;

        for (iter = imageFiles.begin();
             iter != imageFiles.end(); iter++) {
          if ((*iter).fileName == filename) {
            return (*iter).image;
          }
        }
        imageFileStruct newImageFile;
        newImageFile.fileName = filename;
        osg::Image* image = osgDB::readImageFile(filename);
        newImageFile.image = image;
        imageFiles.push_back(newImageFile);

        return newImageFile.image;
      }

    } // end of namespace graphics
  } // end of namespace mars
