/*
 *  Copyright 2016, DFKI GmbH Robotics Innovation Center
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
 * \file Terrain.cpp
 * \author Malte Langosz (malte.langosz@dfki.de)
 * \brief
 *
 * Version 0.1
 */

#include "Terrain.h"

#include <configmaps/ConfigData.h>
#include <mars/graphics/gui_helper_functions.h>
#include <mars/utils/misc.h>
#include <mars/osg_material_manager/OsgMaterial.h>

#include <osg/LOD>
#include <osg/PositionAttitudeTransform>
#include <osgUtil/CullVisitor>
#include <osg/PolygonMode>

namespace osg_terrain {

  class InstancesVisitor: public osg::NodeVisitor{
  public:
  InstancesVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {}
    virtual void apply(osg::Node &searchNode);
    int numInstances;
  private:
    void generateInstances(osg::Geometry *geom);
  };

  class BoundVisitor: public osg::NodeVisitor{
  public:
  BoundVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {}
    virtual void apply(osg::Node &searchNode);
  };

  class TerrainBoundCallback: public osg::Drawable::ComputeBoundingBoxCallback {
  public:
    TerrainBoundCallback(){}
    ~TerrainBoundCallback(){}
    osg::BoundingBox computeBound(const osg::Drawable &) const {
      return osg::BoundingBox(-600, -600, 0, 600, 600, 95);
    }
  };

  using namespace configmaps;
  using namespace mars;
  using namespace osg_material_manager;

  class PlaneTransform : public osg::Transform {
  public:
    virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix,
                                           osg::NodeVisitor* nv) const  {
      osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
      if (cv) {
        osg::Vec3 eyePointLocal = cv->getEyeLocal();
        eyePointLocal[0] = eyePointLocal[0]-fmod(eyePointLocal[0], 6.0);
        eyePointLocal[1] = eyePointLocal[1]-fmod(eyePointLocal[1], 6.0);
        eyePointLocal[2] = -20;
        matrix.preMultTranslate(eyePointLocal);
      }
      return true;
    }

    virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix,
                                           osg::NodeVisitor* nv) const {
      osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
      if (cv) {
        osg::Vec3 eyePointLocal = cv->getEyeLocal();
        eyePointLocal[0] = eyePointLocal[0]-fmod(eyePointLocal[0], 6.0);
        eyePointLocal[1] = eyePointLocal[1]-fmod(eyePointLocal[1], 6.0);
        eyePointLocal[2] = -20;
        matrix.postMultTranslate(-eyePointLocal);
      }
      return true;
    }
  };

  /** In the first version everthing is created statically by parsing a
   *  yaml file.
   */
  Terrain::Terrain(osg_material_manager::OsgMaterialManager *m) :
    materialManager(m) {
    osg::ref_ptr<osg::LOD> lod = new osg::LOD();
    ConfigMap map = ConfigMap::fromYamlFile("terrain.yml");
    osg::ref_ptr<MaterialNode> logMaterialGroup;

    bool haveLOD = map.hasKey("lod");
    osg::ref_ptr<osg::PositionAttitudeTransform> p;
    if(haveLOD) {
      ConfigVector::iterator it = map["lod"].begin();
      double start, end;
      std::string file;
      if(map.hasKey("material")) {
        ConfigMap mMap = map["material"];
        materialManager->createMaterial(mMap["name"], mMap);

        logMaterialGroup = materialManager->getNewMaterialGroup((std::string)mMap["name"]);
      }
      for(; it!=map["lod"].end(); ++it) {
        start = (*it)["start"];
        end = (*it)["end"];
        file << (*it)["file"];
        osg::ref_ptr<osg::Node> node;
        if(utils::getFilenameSuffix(file) == ".bobj") {
          node = graphics::GuiHelper::readBobjFromFile(file);
        }
        else {
          node = graphics::GuiHelper::readNodeFromFile(file);
        }

        InstancesVisitor visitor;
        visitor.numInstances = (*it)["instances"];
        node->accept(visitor);

        lod->addChild(node.get(), start, end);
      }
      for(it=map["pos"].begin(); it!=map["pos"].end(); ++it) {
        p = new osg::PositionAttitudeTransform();
        p->setPosition(osg::Vec3((double)(*it)["x"], (double)(*it)["y"], 0.0));
        p->setAttitude(osg::Quat((double)(*it)["a"], osg::Vec3(0, 0, 1)));
        fprintf(stderr, "add object: %g %g\n", (double)(*it)["x"], (double)(*it)["y"]);
        p->addChild(lod.get());
        if(logMaterialGroup.valid()) {
          logMaterialGroup->addChild(p.get());
        }
        else {
          this->addChild(p.get());
        }
      }
    }
    if(map.hasKey("plane")) {
      fprintf(stderr, "create plane object");
      osg::ref_ptr<PlaneTransform> p = new PlaneTransform();
      osg::ref_ptr<osg::Node> node = createPlane();
      node->setNodeMask(0xff | 0x1000);
      p->addChild(node.get());

      if(materialManager) {
        ConfigMap mMap = map["plane"]["material"];
        materialManager->createMaterial(mMap["name"], mMap);

        osg::ref_ptr<MaterialNode> mGroup = materialManager->getNewMaterialGroup(mMap["name"]);
        mGroup->addChild(p.get());
      }
      else {
        this->addChild(p.get());
      }
    }

    if(map.hasKey("heightmap")) {
      int width = map["heightmap"]["width"];
      int height = map["heightmap"]["height"];
      double scaleZ = map["heightmap"]["scaleZ"];
      int resolution = map["heightmap"]["resolution"];
      int depth = map["heightmap"]["depth"];
      bool wireframe = map["heightmap"]["wireframe"];
      vbt = new VertexBufferTerrain(width, height, scaleZ, resolution, depth);
      vbt->setInitialBound(osg::BoundingBox(0, 0, 0, width, height, scaleZ));
      vbt->setSelected(wireframe);
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;
      geode->addDrawable(vbt.get());
      p = new osg::PositionAttitudeTransform();
      p->setCullingActive(false);
      tPosX = -width*0.5;
      tPosY = -height*0.5;
      p->setPosition(osg::Vec3(tPosX, tPosY, -25));
      p->setNodeMask(0xff | 0x1000);
      p->addChild(geode.get());
      if(materialManager) {
        ConfigMap mMap = map["heightmap"]["material"];
        materialManager->createMaterial(mMap["name"], mMap);
        osg::ref_ptr<MaterialNode> mGroup = materialManager->getNewMaterialGroup(mMap["name"]);
        mGroup->addChild(p.get());
      }
      else {
        this->addChild(p.get());
      }
    }

    if(map.hasKey("terrain")) {
      fprintf(stderr, "create terrain object");
      //osg::ref_ptr<osg::PositionAttitudeTransform> p = new osg::PositionAttitudeTransform();
      osg::ref_ptr<PlaneTransform> p = new PlaneTransform();
      p->setCullingActive(false);
      osg::ref_ptr<osg::Node> node;
      std::string file = map["terrain"]["file"];
      if(utils::getFilenameSuffix(file) == ".bobj") {
        node = graphics::GuiHelper::readBobjFromFile(file);
      }
      else {
        node = graphics::GuiHelper::readNodeFromFile(file);
      }
      node->setNodeMask(0xff | 0x1000);

      BoundVisitor visitor;
      node->accept(visitor);

      //node->setInitialBound(osg::BoundingBox(-10, -10, 0, 10, 10, 40));
      osg::PolygonMode *polyModeObj = new osg::PolygonMode;
      polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
      bool wireframe = map["terrain"]["wireframe"];
      if(wireframe) {
        node->getOrCreateStateSet()->setAttribute( polyModeObj );
      }
      p->addChild(node.get());

      if(materialManager) {
        ConfigMap mMap = map["terrain"]["material"];
        materialManager->createMaterial(mMap["name"], mMap);

        osg::ref_ptr<MaterialNode> mGroup = materialManager->getNewMaterialGroup(mMap["name"]);
        mGroup->addChild(p.get());
      }
      else {
        this->addChild(p.get());
      }
    }
  }

  Terrain::~Terrain(void) {
  }

  osg::ref_ptr<osg::Node> Terrain::createPlane() {
    osg::ref_ptr<osg::Vec3Array> vertices(new osg::Vec3Array());
    osg::ref_ptr<osg::Vec2Array> texcoords(new osg::Vec2Array());
    osg::ref_ptr<osg::Vec3Array> normals(new osg::Vec3Array());
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();

    { // NOTE: we have to bind the normals per vertex for the stupid tangent generator of osg :/
      float x1 = 50/2.0, x2 = -x1;
      float y1 = 50/2.0, y2 = -y1;
      vertices->push_back(osg::Vec3(x2, y2, 0.0f));
      texcoords->push_back(osg::Vec2(x2, y2));
      normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
      vertices->push_back(osg::Vec3(x1, y2, 0.0f));
      texcoords->push_back(osg::Vec2(x1, y2));
      normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
      vertices->push_back(osg::Vec3(x1, y1, 0.0f));
      texcoords->push_back(osg::Vec2(x1, y1));
      normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
      vertices->push_back(osg::Vec3(x2, y1, 0.0f));
      texcoords->push_back(osg::Vec2(x2, y1));
      normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
    }

    geom->setVertexArray(vertices.get());
    geom->setTexCoordArray(DEFAULT_UV_UNIT,texcoords.get());
    geom->setNormalArray(normals.get());
    geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    // TODO: use addPrimitiveSet(DrawElementsUInt)
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,
                                              0, // index of first vertex
                                              vertices->size()));

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(geom.get());
    return geode.get();
  }

  void Terrain::setCameraPos(double x, double y, double z) {
    if(vbt) vbt->setCameraPosition(x-tPosX, y-tPosY);
  }

  void InstancesVisitor::apply(osg::Node &searchNode){
    // search for geometries and generate tangents for them
    osg::Geode* geode=dynamic_cast<osg::Geode*>(&searchNode);
    if(geode) {
      for(unsigned int i=0; i<geode->getNumDrawables(); ++i) {
        osg::Geometry* geom=dynamic_cast<osg::Geometry*>(geode->getDrawable(i));
        if(geom) {
          geom->setInitialBound(osg::BoundingBox(0, 0, 0, 2, 2, 1));
          generateInstances(geom);
        }
      }
    }

    traverse(searchNode);
  }

  void InstancesVisitor::generateInstances(osg::Geometry *geom) {
    // first turn on hardware instancing for every primitive set
    for (unsigned int i = 0; i < geom->getNumPrimitiveSets(); ++i) {
      geom->getPrimitiveSet(i)->setNumInstances(numInstances);
      fprintf(stderr, "set num instances: %d\n", numInstances);
    }
    // we need to turn off display lists for instancing to work
    geom->setUseDisplayList(false);
    geom->setUseVertexBufferObjects(true);
  }

   void BoundVisitor::apply(osg::Node &searchNode){
     // search for geometries and generate tangents for them
    osg::Geode* geode=dynamic_cast<osg::Geode*>(&searchNode);
    if(geode) {
      for(unsigned int i=0; i<geode->getNumDrawables(); ++i) {
        osg::Geometry* geom=dynamic_cast<osg::Geometry*>(geode->getDrawable(i));
        if(geom) {
          geom->setComputeBoundingBoxCallback(new TerrainBoundCallback());
          geom->setUseDisplayList(false);
          geom->setUseVertexBufferObjects(true);
        }
      }
    }

    traverse(searchNode);
  }
} // end of namespace: osg_terrain
