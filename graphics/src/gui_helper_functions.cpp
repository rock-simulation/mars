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

#ifdef WIN32
 #include <cv.h>
 #include <highgui.h>
#else
 #include <opencv/cv.h>
 #include <opencv/highgui.h>
#endif

namespace mars {
  namespace graphics {

    using namespace std;
    using mars::utils::Color;
    using mars::utils::Vector;
    using mars::utils::Quaternion;
    using mars::interfaces::snmesh;

    vector<nodeFileStruct> GuiHelper::nodeFiles;
    vector<textureFileStruct> GuiHelper::textureFiles;

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
      osg::Geode* dynamicTry=dynamic_cast<osg::Geode*>(&searchNode);
      if(dynamicTry) {
        resultNode=dynamicTry;
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
      void operator() (const osg::Vec3& v1, const osg::Vec3& v2,
                       const osg::Vec3& v3, bool treatVertexDataAsTemporary) {
        treatVertexDataAsTemporary = treatVertexDataAsTemporary;
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
      GeodeVisitor* visitor=new GeodeVisitor("PLACEHOLDER");
      osg::Geode* geode;
      osg::Node* tmpNode;
      osg::ref_ptr<osg::Group> osgGroupFromRead = NULL;
      if ((osgGroupFromRead = node->asGroup()) == 0)
        fprintf(stderr, "error\n");

      //get geometries of node
      int indexcounter = 0;
      for (size_t m = 0; m < osgGroupFromRead->getNumChildren(); m++) {
        tmpNode = osgGroupFromRead->getChild(m);
        tmpNode->accept(*visitor);
        geode = visitor->getNode();
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
      delete visitor;
      return mesh;
    }

    Vector GuiHelper::getExtend(osg::Group *oGroup){
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

    void GuiHelper::getPhysicsFromOBJ(mars::interfaces::NodeData* node) {
      osg::ref_ptr<osg::Node> completeNode;
      osg::ref_ptr<osg::Group> myCreatedGroup;
      osg::ref_ptr<osg::Group> myGroupFromRead;
      nodemanager tempnode;
      bool found = false;

      completeNode  = GuiHelper::readNodeFromFile(node->filename);
      myGroupFromRead = completeNode->asGroup();

      //go through the read node group and combine the parts of the actually
      //handled node
      myCreatedGroup = new osg::Group();
      osg::ref_ptr<osg::StateSet> stateset = myCreatedGroup->getOrCreateStateSet();
      for (unsigned int i = 0; i < myGroupFromRead->getNumChildren(); i ++) {
        osg::ref_ptr<osg::Node> myTestingNode = myGroupFromRead->getChild(i);
        if (myTestingNode == 0) {
          return;
        }
        if (myTestingNode->getName() == node->origName) {
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

      //compute scale factor
      double scaleX = 1, scaleY = 1, scaleZ = 1;
      if (ex.x() != 0) scaleX = node->visual_size.x() / ex.x();
      if (ex.y() != 0) scaleY = node->visual_size.y() / ex.y();
      if (ex.z() != 0) scaleZ = node->visual_size.z() / ex.z();

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

    // TODO: should not be in graphics!
    // physics without graphics would need this too.
    // maybe move to NodeFactory ??
    void GuiHelper::readPixelData(mars::interfaces::terrainStruct *terrain) {

#if !defined (WIN32) && !defined (__linux__)
      IplImage* img=0;

      img=cvLoadImage(terrain->srcname.data(), -1);
      if(img) {
        terrain->width = img->width;
        terrain->height = img->height;
        terrain->pixelData = (double*)calloc((terrain->width*
                                              terrain->height),
                                             sizeof(double));


        CvScalar s;
        int count = 0;
        double imageMaxValue = pow(2., img->depth);
        for(int x=0; x<terrain->width; x++) {
          for(int y=terrain->height-1; y>=0; y--) {

            s=cvGet2D(img,x,y);
            terrain->pixelData[count++] = ((double)s.val[0])/imageMaxValue;
            if(y==0 || y == terrain->height-1 ||
               x==0 || x == terrain->width-1)
              terrain->pixelData[count-1] -= 0.002;
            if(terrain->pixelData[count-1] <= 0.0)
              terrain->pixelData[count-1] = 0.001;

          }
        }
        cvReleaseImage(&img);
      }

#else
      //QImage image(QString::fromStdString(snode->filename));
#ifdef __linux__

      std::string absolutePathIndicator = terrain->srcname.substr(0,1);
      if( absolutePathIndicator != "/") {
        fprintf(stderr, "Terrain file needs to be provided by using an absolute path: currently given \"%s\"\n", terrain->srcname.c_str());
        // exit(0);
      }
#endif

      osg::Image* image = osgDB::readImageFile(terrain->srcname);

      if(image) {
        terrain->width = image->s();
        terrain->height = image->t();
        //if(terrain->pixelData) free(terrain->pixelData);

        terrain->pixelData = (double*)calloc((terrain->width*
                                              terrain->height),
                                             sizeof(double));
        //image->setPixelFormat(GL_RGB);
        //image->setDataType(GL_UNSIGNED_SHORT);
        //osgDB::writeImageFile(*image, std::string("test.jpg"));
        float r = 0;
	//float g = 0, b = 0;
        int count = 0;
        osg::Vec4 pixel;
        for (int y = 1; y <= terrain->height; y++){
          for (int x = 1; x <= terrain->width; x++){
            pixel = image->getColor(osg::Vec2(x, y));
            r = pixel[0];
            //g = pixel[1];
            //b = pixel[2];
            //QColor converter(image.pixel(x, y));
            //converter.getRgb(&r, &g, &b);
            //convert to greyscale by common used scale
            terrain->pixelData[count++] = r;//((r*0.3+g*0.59+b*0.11));
          }
        }
      }
#endif
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

      osg::Image* textureImage = osgDB::readImageFile(filename);
      newTextureFile.texture->setImage(textureImage);
      textureFiles.push_back(newTextureFile);

      return newTextureFile.texture;
    }

  } // end of namespace graphics
} // end of namespace mars
