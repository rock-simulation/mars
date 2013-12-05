/*
 *  Copyright 2011, 2012 DFKI GmbH Robotics Innovation Center
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
 *  GraphicsManager.cpp
 *  Simulator
 *
 *  Created by borchers on 27.05.08.
 */

#include "GraphicsManager.h"
#include "config.h"

#include <osgDB/WriteFile>
#include <osg/Fog>
#include <osg/LightModel>

#include <osgParticle/FireEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/SmokeTrailEffect>
#include <osgParticle/FireEffect>

#include "3d_objects/GridPrimitive.h"
#include "3d_objects/DrawObject.h"
#include "3d_objects/CoordsPrimitive.h"
#include "3d_objects/AxisPrimitive.h"
#include "3d_objects/Clouds.h"

#include "2d_objects/HUDLabel.h"
#include "2d_objects/HUDTerminal.h"
#include "2d_objects/HUDLines.h"
#include "2d_objects/HUDTexture.h"

#include "wrapper/OSGLightStruct.h"
#include "wrapper/OSGMaterialStruct.h"
#include "wrapper/OSGDrawItem.h"
#include "wrapper/OSGHudElementStruct.h"

#include "GraphicsWidget.h"
#include "GraphicsViewer.h"
#include "HUD.h"

#include "wrapper/OSGNodeStruct.h"
#include "QtOsgMixGraphicsWidget.h"

#include <iostream>
#include <cassert>
#include <stdexcept>

#define SINGLE_THREADED

namespace mars {
  namespace graphics {

    using namespace std;
    using mars::utils::Vector;
    using mars::utils::Quaternion;
    using namespace mars::interfaces;

    static int ReceivesShadowTraversalMask = 0xffff;
    static int CastsShadowTraversalMask = 0xffff;


    GraphicsManager::GraphicsManager(lib_manager::LibManager *theManager,
                                     void *myQTWidget)
      : GraphicsManagerInterface(theManager),
        osgWidget(NULL),
        guiHelper(new GuiHelper(this)),
        next_hud_id(1),
        next_draw_object_id(1),
        next_window_id(1),
        nextPreviewID(1),
        viewer(0),
        scene(new osg::Group),
        shadowedScene(new osgShadow::ShadowedScene),
        lightGroup(new osg::Group),
        globalStateset(new osg::StateSet),
        grid(NULL),
        show_grid(false),
        showClouds_(false),
        show_coords(true),
        useFog(true),
        useNoise(false),
        cfg(0),
        ignore_next_resize(0),
        set_window_prop(0)

    {
      //osg::setNotifyLevel( osg::WARN );

      // first check if we have the cfg_manager lib

      if(libManager == NULL) return;

    }

    GraphicsManager::~GraphicsManager() {
      if(cfg) {
        string saveFile = configPath.sValue;
        saveFile.append("/mars_Graphics.yaml");
        cfg->writeConfig(saveFile.c_str(), "Graphics");
        libManager->releaseLibrary("cfg_manager");
      }
      fprintf(stderr, "Delete mars_graphics\n");
    }

#if USE_SM_SHADOW==1
    // TODO: not use global! but until shadows handled nicely...
    //osg::ref_ptr<osg::Camera> debugCam;
#endif
    void GraphicsManager::initializeOSG(void *data, bool createWindow) {
      cfg = libManager->getLibraryAs<cfg_manager::CFGManagerInterface>("cfg_manager");
      if(!cfg) {
        fprintf(stderr, "******* mars_graphics: couldn't find cfg_manager\n");
        return;
      }

      resources_path.propertyType = cfg_manager::stringProperty;
      resources_path.propertyIndex = 0;
      resources_path.sValue = ".";

      if(cfg) {
        configPath = cfg->getOrCreateProperty("Config", "config_path",
                                              string("."));

        string loadFile = configPath.sValue;
        loadFile.append("/mars_Graphics.yaml");
        cfg->loadConfig(loadFile.c_str());

        // have to handle multisampling here
        multisamples.propertyType = cfg_manager::intProperty;
        multisamples.propertyIndex = 0;
        multisamples.iValue = 0;
        if(cfg->getPropertyValue("Graphics", "num multisamples", "value",
                                 &multisamples.iValue)) {
          multisamples.paramId = cfg->getParamId("Graphics", "num multisamples");
        }
        else {
          multisamples.paramId = cfg->createParam(string("Graphics"),
                                                  string("num multisamples"),
                                                  cfg_manager::intParam);
          cfg->setProperty(multisamples);
        }
        cfg->registerToParam(multisamples.paramId,
                             dynamic_cast<cfg_manager::CFGClient*>(this));
        setMultisampling(multisamples.iValue);

        resources_path = cfg->getOrCreateProperty("Graphics", "resources_path",
                                                  string(MARS_GRAPHICS_DEFAULT_RESOURCES_PATH),
                                                  dynamic_cast<cfg_manager::CFGClient*>(this));

        noiseProp = cfg->getOrCreateProperty("Graphics", "useNoise",
                                             true, this);
        useNoise = noiseProp.bValue;
      }

      globalStateset->setGlobalDefaults();

      // with backface culling backfaces are not processed,
      // else front and back faces are always processed.
      // Its a good idea to turn this on for perfomance reasons,
      // 2D objects in 3D scene may want to overwrite this setting, or
      // walk through indices front to back and vice versa
      // to get two front faces.
      cull = new osg::CullFace();
      cull->setMode(osg::CullFace::BACK);

      { // setup LIGHT
        globalStateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
        globalStateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);

        globalStateset->setMode(GL_LIGHT0, osg::StateAttribute::OFF);
        globalStateset->setMode(GL_LIGHT1, osg::StateAttribute::OFF);
        globalStateset->setMode(GL_LIGHT2, osg::StateAttribute::OFF);
        globalStateset->setMode(GL_LIGHT3, osg::StateAttribute::OFF);
        globalStateset->setMode(GL_LIGHT4, osg::StateAttribute::OFF);
        globalStateset->setMode(GL_LIGHT5, osg::StateAttribute::OFF);
        globalStateset->setMode(GL_LIGHT6, osg::StateAttribute::OFF);
        globalStateset->setMode(GL_LIGHT7, osg::StateAttribute::OFF);
        globalStateset->setMode(GL_BLEND,osg::StateAttribute::OFF);
      }

      // background color for the scene
      graphicOptions.clearColor = mars::utils::Color(0.55, 0.67, 0.88, 1.0);

      { // setup FOG
        graphicOptions.fogColor = mars::utils::Color(0.2, 0.2, 0.2, 1.0);
        graphicOptions.fogEnabled = true;
        graphicOptions.fogDensity = 0.35;
        graphicOptions.fogStart = 10.0;
        graphicOptions.fogEnd = 30.0;

        osg::ref_ptr<osg::Fog> myFog = new osg::Fog;
        myFog->setMode(osg::Fog::LINEAR);
        myFog->setColor(toOSGVec4(graphicOptions.fogColor));
        myFog->setStart(graphicOptions.fogStart);
        myFog->setEnd(graphicOptions.fogEnd);
        myFog->setDensity(graphicOptions.fogDensity);
        globalStateset->setAttributeAndModes(myFog.get(), osg::StateAttribute::ON);
      }

      // some fixed function pipeline stuff...
      // i guess the default is smooth shading, that means
      // light influence is calculated per vertex and interpolated for fragments.
      osg::ref_ptr<osg::LightModel> myLightModel = new osg::LightModel;
      myLightModel->setTwoSided(false);
      globalStateset->setAttributeAndModes(myLightModel.get(), osg::StateAttribute::ON);

      // associate scene with global states
      scene->setStateSet(globalStateset.get());
      scene->addChild(lightGroup.get());
      scene->addChild(shadowedScene.get());

      // init light (osg can have only 8 lights enabled at a time)
      for (unsigned int i =0; i<8;i++) {
        lightmanager ltemp;
        ltemp.free=true;
        myLights.push_back(ltemp);
      }

      initDefaultLight();


      shadowedScene->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
      shadowedScene->setCastsShadowTraversalMask(CastsShadowTraversalMask);
      {
#if USE_LSPSM_SHADOW
        osg::ref_ptr<osgShadow::LightSpacePerspectiveShadowMapDB> sm =
          new osgShadow::LightSpacePerspectiveShadowMapDB;

        //sm->setDebugDraw(true);
        sm->setMinLightMargin( 10.0f );
        sm->setMaxFarPlane( 0.0f );
        sm->setTextureSize( osg::Vec2s( 2028, 2028 ) );
        sm->setShadowTextureCoordIndex( 6 );
        sm->setShadowTextureUnit( 6 );

        shadowedScene->setShadowTechnique( sm.get() );
#elif USE_PSSM_SHADOW
        osg::ref_ptr<osgShadow::ParallelSplitShadowMap> pssm =
          new osgShadow::ParallelSplitShadowMap(NULL,NUM_PSSM_SPLITS);

        pssm->enableShadowGLSLFiltering(false);
        pssm->setTextureResolution(2048);
        pssm->setMinNearDistanceForSplits(0);
        pssm->setMaxFarDistance(100);
        pssm->setMoveVCamBehindRCamFactor(0);
        //pssm->setPolygonOffset(osg::Vec2(-1.0,-4.0));

        shadowedScene->setShadowTechnique(pssm.get());
#elif USE_SM_SHADOW
        shadowMap = new osgShadow::ShadowMap;

        shadowedScene->setShadowTechnique(shadowMap.get());

        shadowMap->setTextureSize(osg::Vec2s(4096,4096));
        shadowMap->setTextureUnit(2);
        shadowMap->clearShaderList();
        //shadowMap->setAmbientBias(osg::Vec2(0.5f,0.5f));
        shadowMap->setPolygonOffset(osg::Vec2(-1.2,-1.2));
#endif
      }

      // TODO: check this out:
      //   i guess fire.rgb is a 1D texture
      //   there is something to generate these in OGLE
      //osg::ref_ptr<osgParticle::ParticleEffect> effectNode =
      //new osgParticle::FireEffect;
      //effectNode->setTextureFileName("fire.rgb");
      //effectNode->setIntensity(2.5);
      //effectNode->setScale(4);
      //scene->addChild(effectNode.get());

      grid = new GridPrimitive(osgWidget);
      showCoords();

      // reset number of frames
      framecount = 0;

      if(createWindow) {
        viewer = new GraphicsViewer((GuiEventInterface*)this);
        viewer->setKeyEventSetsDone(0);
#ifdef SINGLE_THREADED
        viewer->setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);
#else
        viewer->setThreadingModel(osgViewer::CompositeViewer::DrawThreadPerContext);
#endif
        new3DWindow(data);
      }

      //guiHelper->setGraphicsWidget(graphicsWindows[0]);
      setupCFG();

      if(backfaceCulling.bValue)
        globalStateset->setAttributeAndModes(cull, osg::StateAttribute::ON);
      else
        globalStateset->setAttributeAndModes(cull, osg::StateAttribute::OFF);

#if USE_SM_SHADOW==1
      //GraphicsWidget *gw = graphicsWindows[0];
      //debugCam = shadowMap->makeDebugHUD();
      //debugCam->setGraphicsContext(gw->getGraphicsWindow());
      //debugCam->setViewport(0,0,721,405);
      //gw->getView()->addSlave(debugCam.get(), false);
#endif
    }

    /**\brief resets scene */
    void GraphicsManager::reset(){
      //remove graphics stuff & rearrange light numbers
      for (unsigned int i = 0; i<myLights.size(); i++) {
        removeLight(i);
      }
      for (DrawObjects::iterator iter = drawObjects_.begin();
           iter != drawObjects_.end(); iter = drawObjects_.begin()) {
        removeDrawObject(iter->first);
      }
      clearDrawItems();
    }


    void GraphicsManager::addGraphicsUpdateInterface(GraphicsUpdateInterface *g) {
      graphicsUpdateObjects.push_back(g);
    }

    void GraphicsManager::removeGraphicsUpdateInterface(GraphicsUpdateInterface *g) {
      std::list<interfaces::GraphicsUpdateInterface*>::iterator it;
      it = find(graphicsUpdateObjects.begin(), graphicsUpdateObjects.end(), g);
      if(it!=graphicsUpdateObjects.end()) {
        graphicsUpdateObjects.erase(it);
      }
    }

    /**
     * sets the camera type
     * @param: type
     */
    void GraphicsManager::setCamera(int type){
      osgWidget->getCameraInterface()->setCamera(type);
    }

    /**
     * returns actual camera information
     */
    void GraphicsManager::getCameraInfo(mars::interfaces::cameraStruct *cs) const {
      osgWidget->getCameraInterface()->getCameraInfo(cs);
    }

    void* GraphicsManager::getScene() const {
      return (void*)scene.get();
    }

    void* GraphicsManager::getScene2() const {
      return (void*)dynamic_cast<osg::Node*>(shadowedScene.get());
    }

    void GraphicsManager::saveScene(const string &filename) const {
      osgDB::writeNodeFile(*(scene.get()), filename);
    }

    void GraphicsManager::exportScene(const string &filename) const {
      osgDB::writeNodeFile(*(scene.get()), filename.data());
    }

    void* GraphicsManager::getStateSet() const {
      return (void*)globalStateset.get();
    }

    void GraphicsManager::update(){
      //update drawElements
      for (unsigned int i=0; i<draws.size(); i++) {
        drawMapper &draw = draws[i];
        vector<draw_item> tmp_ditem;
        vector<osg::Node*> tmp_nodes;
        //update draws
        draw.ds.ptr_draw->update(&(draw.ds.drawItems));

        for (unsigned int j=0; j<draw.ds.drawItems.size(); j++) {
          draw_item &di = draw.ds.drawItems[j];

          if(di.draw_state == DRAW_STATE_ERASE) {
            scene->removeChild(draw.nodes[j]);
          }
          else if (di.draw_state == DRAW_STATE_CREATE) {
            std::string font_path = resources_path.sValue;
            font_path.append("/Fonts");
            osg::ref_ptr<osg::Group> osgNode = new OSGDrawItem(osgWidget, di,
                                                               font_path);
            scene->addChild(osgNode.get());

            di.draw_state = DRAW_UNKNOWN;
            tmp_ditem.push_back(di);
            tmp_nodes.push_back(osgNode.get());
          }
          else if (di.draw_state == DRAW_STATE_UPDATE) {
            assert(draws[i].nodes.size() > j);
            osg::Node *n = draws[i].nodes[j];
            OSGDrawItem *diWrapper = dynamic_cast<OSGDrawItem*>(n->asGroup()); // TODO: asGroup unneeded?
            assert(diWrapper != NULL); // TODO: handle this case better

            diWrapper->update(di);

            di.draw_state = DRAW_UNKNOWN;
            tmp_ditem.push_back(di);
            tmp_nodes.push_back(n);
          }
          else { // invalid draw state!
            di.draw_state = DRAW_UNKNOWN;
            tmp_ditem.push_back(di);
            tmp_nodes.push_back(draws[i].nodes[j]);
          }
        }

        draws[i].ds.drawItems.clear();
        draws[i].ds.drawItems = tmp_ditem;
        draws[i].nodes.clear();
        draws[i].nodes = tmp_nodes;
      }
    }

    const mars::interfaces::GraphicData GraphicsManager::getGraphicOptions(void) const {
      return graphicOptions;
    }

    void GraphicsManager::setGraphicOptions(const mars::interfaces::GraphicData &options) {
      osg::Fog *myFog;

      myFog = (osg::Fog*)globalStateset->getAttribute(osg::StateAttribute::FOG);

      graphicOptions = options;
      for(unsigned int i=0; i<graphicsWindows.size(); i++)
        graphicsWindows[i]->setClearColor(graphicOptions.clearColor);

      myFog->setColor(osg::Vec4(graphicOptions.fogColor.r,
                                graphicOptions.fogColor.g,
                                graphicOptions.fogColor.b, 1.0));
      myFog->setStart(graphicOptions.fogStart);
      myFog->setEnd(graphicOptions.fogEnd);
      myFog->setDensity(graphicOptions.fogDensity);

      if(graphicOptions.fogEnabled) {
        globalStateset->setMode(GL_FOG, osg::StateAttribute::ON);
        useFog = true;
      }
      else {
        globalStateset->setMode(GL_FOG, osg::StateAttribute::OFF);
        useFog = false;
      }

      map<unsigned long, osg::ref_ptr<OSGNodeStruct> >::iterator iter;

      for(iter=drawObjects_.begin(); iter!=drawObjects_.end(); ++iter)
        iter->second->object()->setUseFog(useFog);

    }

    void GraphicsManager::setWidget(GraphicsWidget *widget) {
      //guiHelper->setGraphicsWidget(widget);
    }

    void GraphicsManager::setTexture(unsigned long id, const string &filename) {
      std::vector<nodemanager>::iterator iter;

      for (iter = myNodes.begin(); iter != myNodes.end(); iter++) {
        if ((*iter).index == id) {
          osg::StateSet* state = (*iter).matrix->getChild(0)->getOrCreateStateSet();
          state->setTextureAttributeAndModes(0, GuiHelper::loadTexture(filename).get(),
                                             osg::StateAttribute::ON |
                                             osg::StateAttribute::PROTECTED);
          break;
        }
      }
    }

    unsigned long GraphicsManager::new3DWindow(void *myQTWidget, bool rtt,
                                               int width, int height, const std::string &name) {
      GraphicsWidget *gw;

      if (graphicsWindows.size() > 0) {
        gw = QtOsgMixGraphicsWidget::createInstance(myQTWidget, scene.get(),
                                                    next_window_id++, rtt);
        gw->initializeOSG(myQTWidget, graphicsWindows[0], width, height);
      }
      else {
        gw = QtOsgMixGraphicsWidget::createInstance(myQTWidget, scene.get(),
                                                    next_window_id++);
        gw->initializeOSG(myQTWidget, 0, width, height);
      }

      gw->setName(name);
      gw->setClearColor(graphicOptions.clearColor);
      viewer->addView(gw->getView());
      graphicsWindows.push_back(gw);

      if(!rtt) {
        gw->setGraphicsEventHandler((GraphicsEventInterface*)this);

        HUD *myHUD = new HUD(next_window_id);
        myHUD->init(gw->getGraphicsWindow());
        myHUD->setViewSize(1920, 1200);

        gw->setHUD(myHUD);

        // iterator over hudElements

        for(HUDElements::iterator iter = hudElements.begin();
            iter != hudElements.end(); iter++)
          gw->addHUDElement((*iter)->getHUDElement());

      }
      return next_window_id - 1;
    }

    void* GraphicsManager::getView(unsigned long id){

      GraphicsWidget* gw=getGraphicsWindow(id);

      if(gw == NULL){
        return gw;
      }
      return (void*) gw->getView();
    }

    GraphicsWindowInterface* GraphicsManager::get3DWindow(unsigned long id) const {
      std::vector<GraphicsWidget*>::const_iterator iter;

      for(iter=graphicsWindows.begin(); iter!=graphicsWindows.end(); iter++) {
        if((*iter)->getID() == id) {
          return (GraphicsWindowInterface*)(*iter);
        }
      }
      return 0;
    }

    GraphicsWindowInterface* GraphicsManager::get3DWindow(const std::string &name) const {
      std::vector<GraphicsWidget*>::const_iterator iter;

      for(iter=graphicsWindows.begin(); iter!=graphicsWindows.end(); iter++) {
        if((*iter)->getName().compare(name) == 0) {
          return (GraphicsWindowInterface*)(*iter);
        }
      }
      return 0;
    }




    GraphicsWidget* GraphicsManager::getGraphicsWindow(unsigned long id) const {
      std::vector<GraphicsWidget*>::const_iterator iter;

      for(iter=graphicsWindows.begin(); iter!=graphicsWindows.end(); iter++) {
        if((*iter)->getID() == id) {
          return *iter;
        }
      }
      return 0;
    }

    void GraphicsManager::getList3DWindowIDs(std::vector<unsigned long> *ids) const {
      std::vector<GraphicsWidget*>::const_iterator iter;

      for(iter=graphicsWindows.begin(); iter!=graphicsWindows.end(); iter++) {
        ids->push_back((*iter)->getID());
      }
    }

    void GraphicsManager::draw() {
      std::list<interfaces::GraphicsUpdateInterface*>::iterator it;
      std::vector<GraphicsWidget*>::iterator iter;

      for(it=graphicsUpdateObjects.begin();
          it!=graphicsUpdateObjects.end(); ++it) {
        (*it)->preGraphicsUpdate();
      }

      update();
      for(iter=graphicsWindows.begin(); iter!=graphicsWindows.end(); iter++) {
        (*iter)->updateView();
      }

      // Render a complete new frame.
      if(viewer) viewer->frame();
      ++framecount;
      for(it=graphicsUpdateObjects.begin();
          it!=graphicsUpdateObjects.end(); ++it) {
        (*it)->postGraphicsUpdate();
      }
    }

    void GraphicsManager::setGrabFrames(bool value) {
      graphicsWindows[0]->setGrabFrames(value);
      graphicsWindows[0]->setSaveFrames(value);
    }

    void GraphicsManager::setActiveWindow(unsigned long win_id) {
      get3DWindow(win_id)->grabFocus();
    }

    void* GraphicsManager::getQTWidget(unsigned long id) const {
      std::vector<GraphicsWidget*>::const_iterator iter;

      for (iter = graphicsWindows.begin(); iter != graphicsWindows.end(); iter++) {
        if ((*iter)->getID() == id) {
          return (*iter)->getWidget();
        }
      }
      return 0;
    }

    void GraphicsManager::showQTWidget(unsigned long id) {
      std::vector<GraphicsWidget*>::iterator iter;

      for(iter=graphicsWindows.begin(); iter!=graphicsWindows.end(); iter++) {
        if((*iter)->getID() == id) {
          (*iter)->showWidget();
        }
      }
    }

    void GraphicsManager::setGraphicsWindowGeometry(unsigned long id,
                                                    int top, int left,
                                                    int width, int height) {
      std::vector<GraphicsWidget*>::iterator iter;

      for(iter=graphicsWindows.begin(); iter!=graphicsWindows.end(); iter++) {
        if((*iter)->getID() == id) {
          (*iter)->setWGeometry(top, left, width, height);
        }
      }
    }

    void GraphicsManager::getGraphicsWindowGeometry(unsigned long id,
                                                    int *top, int *left,
                                                    int *width, int *height) const {
      std::vector<GraphicsWidget*>::const_iterator iter;

      for(iter=graphicsWindows.begin(); iter!=graphicsWindows.end(); iter++) {
        if((*iter)->getID() == id) {
          (*iter)->getWGeometry(top, left, width, height);
        }
      }
    }

    ////// DRAWOBJECTS

    unsigned long GraphicsManager::findCoreObject(unsigned long draw_id) const {
      map<unsigned long int, unsigned long int>::const_iterator it;
      it = DrawCoreIds.find(draw_id);
      if (it == DrawCoreIds.end())
        return 0;
      else
        return it->second;
    }


    OSGNodeStruct* GraphicsManager::findDrawObject(unsigned long id) const {
      map<unsigned long, osg::ref_ptr<OSGNodeStruct> >::const_iterator needle;
      needle = drawObjects_.find(id);
      if(needle == drawObjects_.end()) return NULL;
      else return needle->second.get();
    }

    unsigned long GraphicsManager::addDrawObject(const mars::interfaces::NodeData &snode,
                                                 bool activated) {
      unsigned long id = next_draw_object_id++;
      vector<mars::interfaces::LightData*> lightList;
      int mask = 0;

      getLights(&lightList);
      if(lightList.size() == 0) lightList.push_back(&defaultLight.lStruct);
      osg::ref_ptr<OSGNodeStruct> drawObject = new OSGNodeStruct(lightList, snode, false, id, marsShader.bValue, useFog, useNoise);
      osg::PositionAttitudeTransform *transform = drawObject->object()->getPosTransform();

      DrawCoreIds.insert(pair<unsigned long int, unsigned long int>(id, snode.index));
      drawObjects_[id] = drawObject;

      if(snode.isShadowCaster) {
        mask |= CastsShadowTraversalMask;
      }
      if(snode.isShadowReceiver) {
        mask |= ReceivesShadowTraversalMask;
      }
      transform->setNodeMask(transform->getNodeMask() | mask);

      // import an .STL file : we have to insert an additional transformation
      // in order to add the additional rotation by 90 degrees around the
      // x-axis (adding the rotation to "transform" does not help at all,
      // because the values of "transform" are constantly resetted by MARS
      // itself)
      if((snode.filename.substr(snode.filename.size()-4, 4) == ".STL") ||
         (snode.filename.substr(snode.filename.size()-4, 4) == ".stl")) {
        // create the new transformation to be added
        osg::ref_ptr<osg::PositionAttitudeTransform> transformSTL =
            new osg::PositionAttitudeTransform();

        // remove all child nodes from "transform" and add them to
        // "transformSTL"
        osg::Node* node = NULL;
        while (transform->getNumChildren() > 0) {
          node = transform->getChild(0);
          transformSTL->addChild(node);
          transform->removeChild(node);
        }

        // add "transformSTL" as child to "transform"
        transform->addChild(transformSTL);

        // calulate the quaternion for the rotation of 90 degrees around the
        // x-axis
        mars::utils::Quaternion offset =
            mars::utils::eulerToQuaternion(mars::utils::Vector(90.0, 0.0, 0.0));

        // set the orientation to the newly added transformation
        transformSTL->setAttitude(osg::Quat(offset.x(),
                                            offset.y(),
                                            offset.z(),
                                            offset.w()));
      }

      if(activated) {
        if(mask != 0) {
          shadowedScene->addChild(transform);
        }
        else {
          shadowedScene->addChild(transform);
        }
      }

      return id;
    }

    void GraphicsManager::removeDrawObject(unsigned long id) {
      OSGNodeStruct *ns = findDrawObject(id);
      if(ns == NULL) return;
      DrawObject *drawObject = ns->object();
      if (drawObject) {
        scene->removeChild(drawObject->getPosTransform());
        shadowedScene->removeChild(drawObject->getPosTransform());
        delete drawObject;
      }
      drawObjects_.erase(id);
    }

    void GraphicsManager::exportDrawObject(unsigned long id,
                                           const std::string &name) const {
      OSGNodeStruct *ns = findDrawObject(id);
      if(ns == NULL) return;
      ns->object()->exportModel(name);
    }

    void GraphicsManager::removeLayerFromDrawObjects(unsigned long window_id) {
      DrawObjects::iterator iter;
      unsigned int bit = 1 << (window_id-1);

      for (iter = drawObjects_.begin(); iter != drawObjects_.end(); iter++) {
        iter->second->object()->removeBits(bit);
      }
    }

    void GraphicsManager::setDrawObjectSelected(unsigned long id, bool val) {
      OSGNodeStruct *ns = findDrawObject(id);
      if(ns == NULL) return;

      std::vector<GraphicsEventClient*>::iterator jter;
      DrawObjectList::iterator drawit;

      ns->object()->setSelected(val);

      if(!val) {
        for(drawit=selectedObjects_.begin(); drawit!=selectedObjects_.end();
            ++drawit) {
          if(drawit->get() == ns) {
            selectedObjects_.erase(drawit);
            break;
          }
        }
      }

      for(jter=graphicsEventClientList.begin();
          jter!=graphicsEventClientList.end();
          ++jter) {
        (*jter)->selectEvent(findCoreObject(id), val);
      }
    }

    void GraphicsManager::setDrawObjectPos(unsigned long id, const Vector &pos) {
      OSGNodeStruct *ns = findDrawObject(id);
      if(ns != NULL) ns->object()->setPosition(pos);
    }
    void GraphicsManager::setDrawObjectRot(unsigned long id, const Quaternion &q) {
      OSGNodeStruct *ns = findDrawObject(id);
      if(ns != NULL) ns->object()->setQuaternion(q);
    }
    void GraphicsManager::setDrawObjectScale(unsigned long id, const Vector &ext) {
      OSGNodeStruct *ns = findDrawObject(id);
      if(ns != NULL) ns->object()->setScaledSize(ext);
    }
    void GraphicsManager::setDrawObjectMaterial(unsigned long id,
                                                const mars::interfaces::MaterialData &material) {
      OSGNodeStruct *ns = findDrawObject(id);
      if(ns != NULL) ns->object()->setMaterial(material, useFog, useNoise);
    }
    void GraphicsManager::setDrawObjectNodeMask(unsigned long id, unsigned int bits) {
      OSGNodeStruct *ns = findDrawObject(id);
      if(ns != NULL) ns->object()->setBits(bits);
    }

    void GraphicsManager::setBlending(unsigned long id, bool mode) {
      OSGNodeStruct *ns = findDrawObject(id);
      if(ns != NULL) ns->object()->setBlending(mode);
    }
    void GraphicsManager::setBumpMap(unsigned long id, const std::string &bumpMap) {
      OSGNodeStruct *ns = findDrawObject(id);
      if(ns != NULL) ns->object()->setBumpMap(bumpMap);
    }
    void GraphicsManager::setSelectable(unsigned long id, bool val) {
      OSGNodeStruct *ns = findDrawObject(id);
      if(ns != NULL) ns->object()->setSelectable(val);
    }
    void GraphicsManager::setDrawObjectRBN(unsigned long id, int val) {
      OSGNodeStruct *ns = findDrawObject(id);
      if(ns != NULL) ns->object()->setRenderBinNumber(val);
    }
    void GraphicsManager::setDrawObjectShow(unsigned long id, bool val) {
      OSGNodeStruct *ns = findDrawObject(id);
      if(ns != NULL) {
        if(val) {
          shadowedScene->addChild(ns->object()->getPosTransform());
        } else {
          scene->removeChild(ns->object()->getPosTransform());
          shadowedScene->removeChild(ns->object()->getPosTransform());
        }
      }
    }

    ///// DRAWITEMS

    /**
     * adds drawStruct items to the scene
     * @param drawStruct draw
     */
    void GraphicsManager::addDrawItems(drawStruct *draw) {
      //create a mapper
      drawMapper myMapper;
      myMapper.ds = *draw;
      draws.push_back(myMapper);
    }

    void GraphicsManager::removeDrawItems(DrawInterface *iface) {
      vector<drawMapper>::iterator it;

      for (it = draws.begin(); it != draws.end(); it++) {
        if (it->ds.ptr_draw != iface) continue;

        for(vector<osg::Node*>::iterator jt = it->nodes.begin();
            jt != it->nodes.end(); ++jt) {
          scene->removeChild(*jt);
        }
        it->nodes.clear();
        it->ds.drawItems.clear();
        draws.erase(it);
        break;
      }
    }

    void GraphicsManager::clearDrawItems(void) {
      //clear the list of draw items
      for(vector<drawMapper>::iterator it = draws.begin();
          it != draws.end(); it = draws.begin()) {
        removeDrawItems(it->ds.ptr_draw);
      }
      draws.clear();
    }

    ///// LIGHT

    void GraphicsManager::addLight(mars::interfaces::LightData &ls) {
      bool freeOne = false;
      unsigned int lightIndex = 0;

      // find a light unit, OpenGL has 8 available in fixed function pipeline
      for (unsigned int i =0; i<myLights.size(); i++) {
        if (myLights[i].free) {
          lightIndex = i;
          freeOne = true;
          break;
        }
      }
      // add light only if we found a free slot
      if (freeOne) {
        lightmanager lm;
        vector<mars::interfaces::LightData*> lightList;
        map<unsigned long, osg::ref_ptr<OSGNodeStruct> >::iterator iter;

        // set the free index
        ls.index = lightIndex;

        osg::ref_ptr<osg::LightSource> myLightSource = new OSGLightStruct(ls);

        //add to lightmanager for later editing possibility
        lm.light = myLightSource->getLight();
        lm.lightSource = myLightSource;
        lm.lStruct = ls;
        lm.lStruct.index = lightIndex;
        lm.free = false;

        lightGroup->addChild( myLightSource.get() );
        globalStateset->setMode(GL_LIGHT0+lightIndex, osg::StateAttribute::ON);
        myLightSource->setStateSetModes(*globalStateset, osg::StateAttribute::ON);

        myLights[lm.lStruct.index] = lm;

        // light changed for every draw object
        getLights(&lightList);
        for(iter=drawObjects_.begin(); iter!=drawObjects_.end(); ++iter)
          iter->second->object()->updateShader(lightList, true);
      }

      //else make a message (should be handled in another way, will be done later)
      else {
        cerr << "Light couldn't be added: No free lights available" << endl;
      }
      lightGroup->removeChild(defaultLight.lightSource.get());
    }

    void GraphicsManager::removeLight(unsigned int index) {
      if (index < myLights.size() && !myLights[index].free) {
        vector<mars::interfaces::LightData*> lightList;
        map<unsigned long, osg::ref_ptr<OSGNodeStruct> >::iterator iter;

        globalStateset->setMode(GL_LIGHT0+index, osg::StateAttribute::OFF);
        lightGroup->removeChild(myLights[index].lightSource.get());

        lightmanager temp;
        temp.free = true;
        myLights[index] = temp;

        getLights(&lightList);
        if(lightList.size() == 0) {
          lightGroup->addChild(defaultLight.lightSource.get());
          lightList.push_back(&defaultLight.lStruct);
        }

        for(iter=drawObjects_.begin(); iter!=drawObjects_.end(); ++iter)
          iter->second->object()->updateShader(lightList, true);
      }
    }

    void GraphicsManager::updateLight(unsigned int i) {
      OSGLightStruct *osgLight = dynamic_cast<OSGLightStruct*>(myLights[i].lightSource.get());
      if(osgLight != NULL)
        osgLight->update(myLights[i].lStruct);
      else
        fprintf(stderr, "GraphicsManager::updateLight -> no Light %u\n", i);
    }

    void GraphicsManager::getLights(vector<mars::interfaces::LightData*> *lightList) {
      lightList->clear();
      for (unsigned int i=0; i<myLights.size(); i++) {
        //return only the used lights
        if (!myLights[i].free) {
          lightList->push_back(&(myLights[i].lStruct));
        }
      }
    }

    void GraphicsManager::getLights(vector<mars::interfaces::LightData> *lightList) const {
      lightList->clear();
      for (unsigned int i=0; i<myLights.size(); i++) {
        //return only the used lights
        if (!myLights[i].free) {
          lightList->push_back(myLights[i].lStruct);
        }
      }
    }

    int GraphicsManager::getLightCount() const {
      // count used lights only
      int size = 0;
      for (unsigned int i=0; i<myLights.size(); i++)
        if (!myLights[i].free)
          size++;
      return size;
    }



    ///// DEFAULT 3D OBJECTS

    /** removes actual coordination frame from the scene*/
    void GraphicsManager::hideCoords(const Vector &pos) {
      (void) pos;
      if(positionedCoords.get()!=NULL)
        transformCoords->removeChild(positionedCoords.get());
      if(transformCoords.get()!=NULL)
        scene->removeChild(transformCoords.get());
    }

    /** removes the main coordination frame from the scene */
    void GraphicsManager::hideCoords() {
      scene->removeChild(coords.get());
      show_coords = false;
    }

    /** adds the main coordination frame to the scene */
    void GraphicsManager::showCoords(const Vector &pos, const Quaternion &rot,
                                     const Vector &size) {

      hideCoords(pos);

      string resPath = resources_path.sValue;

      positionedCoords = new CoordsPrimitive(osgWidget, size, resPath, true);
      transformCoords = new osg::PositionAttitudeTransform();
      transformCoords->addChild(positionedCoords.get());
      osg::Quat oquat;
      oquat.set( rot.x(),  rot.y(),  rot.z() , rot.w());
      transformCoords->setAttitude(oquat);
      transformCoords->setPosition(osg::Vec3(pos.x(),pos.y(),pos.z()));
      scene->addChild(transformCoords.get());

      show_coords = true;
    }

    /** adds a local coordination frame to the scene */
    void GraphicsManager::showCoords(){
      string resPath = resources_path.sValue;

      coords = new CoordsPrimitive(osgWidget, resPath);
      scene->addChild(coords.get());
      show_coords = true;
    }

    /**
     * closes the joint axis view
     */
    void GraphicsManager::closeAxis() {
      scene->removeChild(axisDrawer.get());
    }

    /**
     * draw joint axis, one red line from first node to anchor, one red line from
     * anchor to second node, 2 blue axis lines
     *
     * @param first the first node
     * @param second the anchor
     * @param third the second node
     * @param axis1 the first joint axis
     * @param axis2 the second joint axis
     */
    void GraphicsManager::drawAxis(
                                   const Vector &first, const Vector &second, const Vector &third,
                                   const Vector &axis1, const Vector &axis2) {
      //remove old axis
      if (axisDrawer!=NULL) {
        scene->removeChild(axisDrawer.get());
      }
      axisDrawer = new AxisPrimitive(first, second, third, axis1, axis2);
      scene->addChild(axisDrawer.get());
    }

    void GraphicsManager::showGrid(void) {
      if(!show_grid) scene->addChild(grid.get());
      show_grid = true;
    }

    void GraphicsManager::hideGrid(void) {
      if(show_grid) scene->removeChild(grid.get());
      show_grid = false;
    }

    void GraphicsManager::showClouds() {
      if(!showClouds_) {
        string tex_path = resources_path.sValue;
        tex_path.append("/Textures");
        clouds_ = new Clouds(tex_path);
        scene->addChild(clouds_.get());
      }
      showClouds_ = true;
    }

    void GraphicsManager::hideClouds() {
      if(showClouds_) {
        scene->removeChild(clouds_.get());
        clouds_.release();
      }
      showClouds_ = false;
    }

    ///// PREVIEW NODES

    int GraphicsManager::createPreviewNode(const vector<mars::interfaces::NodeData> &allNodes) {
      vector<mars::interfaces::LightData*> lightList;
      getLights(&lightList);

      if (allNodes[0].filename=="PRIMITIVE") {
        osg::ref_ptr<OSGNodeStruct> drawObject = new OSGNodeStruct(lightList,
                                                                   allNodes[0], true, nextPreviewID, marsShader.bValue, useFog, useNoise);
        previewNodes_[nextPreviewID] = drawObject;
        scene->addChild(drawObject->object()->getPosTransform());
      } else {
        unsigned int i=0;
        for(DrawObjects::iterator it = previewNodes_.begin();
            it != previewNodes_.end(); ++it) {
          osg::ref_ptr<OSGNodeStruct> drawObject = new OSGNodeStruct(lightList,
                                                                     allNodes[++i], true, nextPreviewID, marsShader.bValue, useFog, useNoise);
          previewNodes_[nextPreviewID] = drawObject;
          scene->addChild(drawObject->object()->getPosTransform());
        }
      }
      return nextPreviewID++;
    }

    void GraphicsManager::removePreviewNode(unsigned long id) {
      map<unsigned long, osg::ref_ptr<OSGNodeStruct> >::iterator needle = previewNodes_.find(id);
      if(needle == previewNodes_.end()) return;
      scene->removeChild(needle->second->object()->getTransform().get());
      previewNodes_.erase(needle);
    }

    /**
     * \throw std::runtime_error if action is \c PREVIEW_COLOR and mat is \c NULL.
     */
    void GraphicsManager::preview(int action, bool resize,
                                  const vector<mars::interfaces::NodeData> &allNodes,
                                  unsigned int num,
                                  const mars::interfaces::MaterialData *mat) {

      osg::ref_ptr<osg::Material> material = new osg::Material;
      switch (action) {
      case mars::interfaces::PREVIEW_CREATE:
        createPreviewNode(allNodes);
        break;
      case mars::interfaces::PREVIEW_EDIT: {
        unsigned int i=0;
        for(DrawObjects::iterator it = previewNodes_.begin();
            it != previewNodes_.end(); ++it) {
          it->second->edit(allNodes[++i], resize);
        }
        break;
      }
      case mars::interfaces::PREVIEW_CLOSE:
        for(DrawObjects::iterator it = previewNodes_.begin();
            it != previewNodes_.end(); ++it) {
          scene->removeChild(it->second->object()->getTransform().get());
        }
        previewNodes_.clear();
        break;
      case mars::interfaces::PREVIEW_COLOR:
        for(DrawObjects::iterator it = previewNodes_.begin();
            it != previewNodes_.end(); ++it) {
          if (mat == NULL) {
            throw std::runtime_error("ERROR: Got NULL pointer in "
                                     "GraphicsManager::preview(PREVIEW_COLOR)");
          }
          material = new OSGMaterialStruct(*mat);
          material->setTransparency(osg::Material::FRONT_AND_BACK, 0.8);
          it->second->getOrCreateStateSet()->setAttributeAndModes(material.get(),
                                                                  osg::StateAttribute::ON);
        }
        break;
      default:
        break;
      }
    }

    ////// HUD

    unsigned long GraphicsManager::addHUDElement(hudElementStruct *he) {
      unsigned long id = next_hud_id++;
      osg::ref_ptr<OSGHudElementStruct> elem = new OSGHudElementStruct(*he, resources_path.sValue, id);

      if (elem) {
        hudElements.push_back(elem);
        for (vector<GraphicsWidget*>::iterator iter = graphicsWindows.begin();
             iter!=graphicsWindows.end(); iter++) {
          (*iter)->addHUDElement(elem->getHUDElement());
        }
        return id;
      }

      return 0;
    }

    void GraphicsManager::removeHUDElement(unsigned long id) {
      HUDElements::iterator iter;
      HUDElement* elem = findHUDElement(id);

      if (elem) {
        for (vector<GraphicsWidget*>::iterator iter = graphicsWindows.begin();
             iter!=graphicsWindows.end(); iter++) {
          (*iter)->removeHUDElement(elem);
        }

        for (iter = hudElements.begin(); iter != hudElements.end(); iter++) {
          if ((*iter)->getHUDElement() == elem) {
            hudElements.erase(iter);
            break;
          }
        }
      }
    }

    HUDElement* GraphicsManager::findHUDElement(unsigned long id) const {
      HUDElements::const_iterator iter;
      //HUDTexture *elem;

      for (iter = hudElements.begin(); iter != hudElements.end(); iter++) {
        if ((*iter)->getHUDElement()->getID() == id) {
          return (*iter)->getHUDElement();
        }
      }
      return NULL;
    }

    void GraphicsManager::switchHUDElementVis(unsigned long id) {
      HUDTexture *elem = (HUDTexture*) findHUDElement(id);
      if(elem!=NULL) {
        elem->switchCullMask();
      }
    }

    void GraphicsManager::setHUDElementPos(unsigned long id, double x,
                                           double y) {
      HUDTexture *elem = (HUDTexture*) findHUDElement(id);
      if(elem!=NULL) {
        elem->setPos(x, y);
      }
    }

    void GraphicsManager::setHUDElementTexture(unsigned long id,
                                               std::string texturename) {
      HUDTexture *elem = (HUDTexture*) findHUDElement(id);
      if(elem!=NULL) {
        elem->setTexture(GuiHelper::loadTexture(texturename).get());
      }
    }
    void GraphicsManager::setHUDElementTextureData(unsigned long id,
                                                   void* data) {
      HUDTexture *elem = dynamic_cast<HUDTexture*>(findHUDElement(id));
      if(elem!=NULL) {
        elem->setImageData(data);
      }
    }
    void GraphicsManager::setHUDElementTextureRTT(unsigned long id,
                                                  unsigned long window_id,
                                                  bool depthComponent) {
      HUDTexture *elem = (HUDTexture*) findHUDElement(id);
      std::vector<GraphicsWidget*>::iterator jter;

      for(jter=graphicsWindows.begin(); jter!=graphicsWindows.end(); jter++) {
        if((*jter)->getID() == window_id) {
          if(elem!=NULL) {
            if(depthComponent)
              elem->setTexture((*jter)->getRTTDepthTexture());
            else
              elem->setTexture((*jter)->getRTTTexture());
          }
          break;
        }
      }
    }
    void GraphicsManager::setHUDElementLabel(unsigned long id,
                                             std::string text,
                                             double text_color[4]) {
      HUDLabel *elem = (HUDLabel*) findHUDElement(id);
      if(elem!=NULL) elem->setText(text, text_color);
    }
    void GraphicsManager::setHUDElementLines(unsigned long id,
                                             std::vector<double> *v,
                                             double color[4]) {
      HUDLines *elem = (HUDLines*) findHUDElement(id);
      if(elem!=NULL) elem->setLines(v, color);
    }

    ////// EVENTS

    void GraphicsManager::addGuiEventHandler(GuiEventInterface *_guiEventHandler) {
      std::vector<GuiEventInterface*>::iterator iter;
      bool found = false;

      for (iter = guiHandlerList.begin(); iter != guiHandlerList.end(); ++iter) {
        if((*iter) == _guiEventHandler) {
          found = true;
          break;
        }
      }
      if (!found) {
        guiHandlerList.push_back(_guiEventHandler);
      }
    }

    void GraphicsManager::removeGuiEventHandler(GuiEventInterface *_guiEventHandler) {
      std::vector<GuiEventInterface*>::iterator iter;

      for (iter = guiHandlerList.begin(); iter != guiHandlerList.end(); ++iter) {
        if ((*iter) == _guiEventHandler) {
          guiHandlerList.erase(iter);
          break;
        }
      }
    }

    void GraphicsManager::emitKeyDownEvent(int key, unsigned int modKey,
                                           unsigned long win_id) {
      std::vector<GuiEventInterface*>::iterator iter;

      for (iter = guiHandlerList.begin(); iter != guiHandlerList.end(); ++iter) {
        (*iter)->keyDownEvent(key, modKey, win_id);
      }
    }

    void GraphicsManager::emitKeyUpEvent(int key, unsigned int modKey,
                                         unsigned long win_id) {
      std::vector<GuiEventInterface*>::iterator iter;

      for (iter = guiHandlerList.begin(); iter != guiHandlerList.end(); ++iter) {
        (*iter)->keyUpEvent(key, modKey, win_id);
      }
    }

    void GraphicsManager::emitQuitEvent(unsigned long win_id) {
      if(win_id < 1) return;

      std::vector<GuiEventInterface*>::iterator iter;

      for (iter = guiHandlerList.begin(); iter != guiHandlerList.end(); ++iter) {
        (*iter)->quitEvent(win_id);
      }
    }

    void GraphicsManager::emitSetAppActive(unsigned long win_id) {
      if(win_id < 1) return;

      std::vector<GuiEventInterface*>::iterator iter;

      for (iter = guiHandlerList.begin(); iter != guiHandlerList.end(); ++iter) {
        (*iter)->setAppActive(win_id);
      }
    }

    void GraphicsManager::addEventClient(GraphicsEventClient* theClient) {
      std::vector<GraphicsEventClient*>::iterator iter;

      for(iter=graphicsEventClientList.begin();
          iter!=graphicsEventClientList.end();
          ++iter) {
        if(*iter == theClient) return;
      }
      graphicsEventClientList.push_back(theClient);
    }

    void GraphicsManager::removeEventClient(GraphicsEventClient* theClient) {
      std::vector<GraphicsEventClient*>::iterator iter;

      for(iter=graphicsEventClientList.begin();
          iter!=graphicsEventClientList.end();
          ++iter) {
        if(*iter == theClient) {
          graphicsEventClientList.erase(iter);
          return;
        }
      }
    }

    void GraphicsManager::emitNodeSelectionChange(unsigned long win_id, int mode) {
      if(win_id < 1 || mode == 0) return;
      std::vector<GraphicsEventClient*>::iterator jter;

      GraphicsWidget* gw = getGraphicsWindow(win_id);

      std::vector<osg::Node*> selectednodes = gw->getPickedObjects();
      if(selectednodes.empty()) return;

      DrawObjectList::iterator drawListIt;
      DrawObjects::iterator drawIt;
      std::vector<osg::Node*>::iterator nodeit;

      bool selection = true;

      switch(mode) {
      case 0:
        break;
        // Pickmode STANDARD
      case 1:
        /* Before attempting to add the object as newly selected, check
         *  if already selected. If yes, remove it from the list.
         */
        if(!selectedObjects_.empty()) {
          // scan for objects to potentially remove
          for(nodeit=selectednodes.begin(); nodeit!=selectednodes.end(); ++nodeit) {
            // and try to find them in the list of already selected objects
            for(drawListIt=selectedObjects_.begin(); drawListIt!=selectedObjects_.end();
                ++drawListIt) {
              /* In case we find the object, we have to remove the object
               *  from the list of selected objects
               */
              if((*drawListIt)->object()->containsNode((*nodeit))) {

                for(jter=graphicsEventClientList.begin();
                    jter!=graphicsEventClientList.end();
                    ++jter)
                  (*jter)->selectEvent(findCoreObject((*drawListIt)->object()->getID()), false);

                (*drawListIt)->object()->setSelected(false);
                selectedObjects_.erase(drawListIt);

                selection = false;

                if(selectedObjects_.empty())
                  break;
                else
                  drawListIt = selectedObjects_.begin();

              }
            }
            /* in case we previously erased all objects from the selectedobjects
             *  list, we want to stop searching
             */
            if(selectedObjects_.empty())
              break;
          }
        }
        /* If we didn't find our picked objects in the list of selected
         * objects, we can add them to our selections - if they are valid,
         * of course.
         */
        if(selection) {
          // scan for objects to potentially add
          for(nodeit=selectednodes.begin(); nodeit!=selectednodes.end(); ++nodeit) {
            // and try to verify their existance as drawobject
            for(drawIt=drawObjects_.begin(); drawIt!=drawObjects_.end(); ++drawIt) {
              /* In case we find the corresponding drawobject, we have to:
               * add them to the list of selected objects.
               */
              if(drawIt->second->object()->containsNode((*nodeit))) {
                drawIt->second->object()->setSelected(true);
                selectedObjects_.push_back(drawIt->second.get());

                for(jter=graphicsEventClientList.begin();
                    jter!=graphicsEventClientList.end();
                    ++jter)
                  (*jter)->selectEvent(findCoreObject(drawIt->second->object()->getID()), true);

                // increase nodeit to make sure we do not add this node again.
                //nodeit++;
                break;
              }
            }
            // nothing more to add? Fine, let's stop!
            if(nodeit == selectednodes.end())
              break;
          }
        }
        break;

        // Pickmode FORCE_ADD
      case 2:
        break;

        // Pickmode FORCE_REMOVE
      case 3:
        break;
      }

      gw->clearSelectionVectors();

    }

    void GraphicsManager::showNormals(bool val) {
      map<unsigned long, osg::ref_ptr<OSGNodeStruct> >::iterator iter;

      for(iter=drawObjects_.begin(); iter!=drawObjects_.end(); ++iter)
        iter->second->object()->showNormals(val);

    }

    void GraphicsManager::showRain(bool val) {
      if(val) {
        rain = new osgParticle::PrecipitationEffect;
        rain->setWind(osg::Vec3(1, 0, 0));
        rain->setParticleSpeed(0.4);
        rain->rain(0.6); // alternatively, use rain
        scene->addChild(rain.get());
      } else {
        scene->removeChild(rain.get());
      }
    }

    void GraphicsManager::showSnow(bool val) {
      if(val) {
        snow = new osgParticle::PrecipitationEffect;
        snow->setWind(osg::Vec3(1, 0, 0));
        snow->setParticleSpeed(0.4);
        snow->snow(0.4); // alternatively, use rain
        scene->addChild(snow.get());
      } else {
        scene->removeChild(snow.get());
      }
    }

    void GraphicsManager::setupCFG(void) {
      cfg_manager::CFGClient* cfgClient = dynamic_cast<cfg_manager::CFGClient*>(this);
      cfgW_top = cfg->getOrCreateProperty("Graphics", "window1Top", (int)40,
                                          cfgClient);

      cfgW_left = cfg->getOrCreateProperty("Graphics", "window1Left", (int)700,
                                           cfgClient);

      cfgW_width = cfg->getOrCreateProperty("Graphics", "window1Width", (int)720,
                                            cfgClient);

      cfgW_height = cfg->getOrCreateProperty("Graphics", "window1Height", (int)405,
                                             cfgClient);

      draw_normals = cfg->getOrCreateProperty("Graphics", "draw normals", false,
                                              cfgClient);

      brightness = cfg->getOrCreateProperty("Graphics", "brightness", 1.0,
                                            cfgClient);

      grab_frames = cfg->getOrCreateProperty("Graphics", "make movie", false,
                                             cfgClient);

      marsShader = cfg->getOrCreateProperty("Graphics", "marsShader", true,
                                            cfgClient);

      drawRain = cfg->getOrCreateProperty("Graphics", "drawRain", false,
                                          cfgClient);

      drawSnow = cfg->getOrCreateProperty("Graphics", "drawSnow", false,
                                          cfgClient);

      backfaceCulling = cfg->getOrCreateProperty("Graphics", "backfaceCulling",
                                                 true, cfgClient);

      setGraphicsWindowGeometry(1, cfgW_top.iValue, cfgW_left.iValue,
                                cfgW_width.iValue, cfgW_height.iValue);
      if(drawRain.bValue) showRain(true);
      if(drawSnow.bValue) showSnow(true);

    }

    void GraphicsManager::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {
      bool change_view = 0;

      if(set_window_prop) return;

      if(_property.paramId == cfgW_top.paramId) {
        cfgW_top.iValue = _property.iValue;
        change_view = 1;
      }

      else if(_property.paramId == cfgW_left.paramId) {
        cfgW_left.iValue = _property.iValue;
        change_view = 1;
      }

      else if(_property.paramId == cfgW_width.paramId) {
        cfgW_width.iValue = _property.iValue;
        change_view = 1;
      }

      else if(_property.paramId == cfgW_height.paramId) {
        cfgW_height.iValue = _property.iValue;
        change_view = 1;
      }

      if(change_view) {
        // we get four callback on the resize that we want to ignore
#ifdef QT_OSG_MIX
        ignore_next_resize += 1;
#else
        ignore_next_resize += 4;
#endif
        setGraphicsWindowGeometry(1, cfgW_top.iValue, cfgW_left.iValue,
                                  cfgW_width.iValue, cfgW_height.iValue);
        return;
      }

      if(_property.paramId == draw_normals.paramId) {
        showNormals(_property.bValue);
        return;
      }

      if(_property.paramId == drawRain.paramId) {
        showRain(_property.bValue);
        return;
      }

      if(_property.paramId == drawSnow.paramId) {
        showSnow(_property.bValue);
        return;
      }

      if(_property.paramId == multisamples.paramId) {
        setMultisampling(_property.iValue);
        return;
      }

      if(_property.paramId == noiseProp.paramId) {
        useNoise = noiseProp.bValue = _property.bValue;
        return;
      }

      if(_property.paramId == brightness.paramId) {
        setBrightness(_property.dValue);
        return;
      }

      if(_property.paramId == marsShader.paramId) {
        setUseShader(_property.bValue);
        return;
      }

      if(_property.paramId == backfaceCulling.paramId) {
        if((backfaceCulling.bValue = _property.bValue))
          globalStateset->setAttributeAndModes(cull, osg::StateAttribute::ON);
        else
          globalStateset->setAttributeAndModes(cull, osg::StateAttribute::OFF);
        return;
      }

      if(_property.paramId == grab_frames.paramId) {
        setGrabFrames(_property.bValue);
        return;
      }
    }

    void GraphicsManager::emitGeometryChange(unsigned long win_id, int left,
                                             int top, int width, int height) {

      bool update_cfg = false;
      if(win_id==1) {
        if(ignore_next_resize>0) {
          --ignore_next_resize;
          return;
        }

        if(top != cfgW_top.iValue) {
          cfgW_top.iValue = top;
          update_cfg = true;
        }
        if(left != cfgW_left.iValue) {
          cfgW_left.iValue = left;
          update_cfg = true;
        }
        if(width != cfgW_width.iValue) {
          cfgW_width.iValue = width;
          update_cfg = true;
        }
        if(height != cfgW_height.iValue) {
          cfgW_height.iValue = height;
          update_cfg = true;
        }
        if(update_cfg && cfg) {
          set_window_prop = true;
          cfg->setProperty(cfgW_top);
          cfg->setProperty(cfgW_left);
          cfg->setProperty(cfgW_width);
          cfg->setProperty(cfgW_height);
          set_window_prop = false;
        }
      }
    }

    void GraphicsManager::setMultisampling(int num_samples) {
      //Antialiasing
      osg::DisplaySettings::instance()->setNumMultiSamples(num_samples);
    }

    void GraphicsManager::setBrightness(double val) {
      map<unsigned long, osg::ref_ptr<OSGNodeStruct> >::iterator iter;

      for(iter=drawObjects_.begin(); iter!=drawObjects_.end(); ++iter)
        iter->second->object()->setBrightness((float)val);
    }

    void GraphicsManager::setUseShader(bool val) {
      map<unsigned long, osg::ref_ptr<OSGNodeStruct> >::iterator iter;

      for(iter=drawObjects_.begin(); iter!=drawObjects_.end(); ++iter)
        iter->second->object()->setUseMARSShader(val);
    }


    void GraphicsManager::initDefaultLight() {
      defaultLight.lStruct.pos = Vector(0.0, 0.0, 10.0);
      defaultLight.lStruct.ambient = mars::utils::Color(0.0, 0.0, 0.0, 1.0);
      defaultLight.lStruct.diffuse = mars::utils::Color(1.0, 1.0, 1.0, 1.0);
      defaultLight.lStruct.specular = mars::utils::Color(1.0, 1.0, 1.0, 1.0);
      defaultLight.lStruct.constantAttenuation = 0.0;
      defaultLight.lStruct.linearAttenuation = 0.0;
      defaultLight.lStruct.quadraticAttenuation = 0.00001;
      defaultLight.lStruct.directional = false;
      defaultLight.lStruct.type = mars::interfaces::OMNILIGHT;
      defaultLight.lStruct.index = 0;
      defaultLight.lStruct.angle = 0;
      defaultLight.lStruct.exponent = 0;

      osg::ref_ptr<osg::LightSource> myLightSource = new OSGLightStruct(defaultLight.lStruct);

      //add to lightmanager for later editing possibility
      defaultLight.light = myLightSource->getLight();
      defaultLight.lightSource = myLightSource;
      defaultLight.free = false;

      lightGroup->addChild( myLightSource.get() );
      globalStateset->setMode(GL_LIGHT0, osg::StateAttribute::ON);
      myLightSource->setStateSetModes(*globalStateset, osg::StateAttribute::ON);
    }

    void*  GraphicsManager::getWindowManager(int id){


      GraphicsWidget* gw=getGraphicsWindow(id);

      if(gw == NULL){
        std::cerr<<"window does not exist!"<<std::endl;
        return gw;
      }
      return (void*) gw->getOrCreateWindowManager();

    }


    bool GraphicsManager::coordsVisible(void) const {
      return show_coords;
    }

    bool GraphicsManager::gridVisible(void) const {
      return show_grid;
    }

    bool GraphicsManager::cloudsVisible(void) const {
      return showClouds_;
    }

    void GraphicsManager::collideSphere(unsigned long id, Vector pos,
                                        mars::interfaces::sReal radius) {
      OSGNodeStruct *ns = findDrawObject(id);
      if(ns == NULL) return;
      ns->object()->collideSphere(pos, radius);
    }

    const Vector& GraphicsManager::getDrawObjectPosition(unsigned long id) {
      OSGNodeStruct *ns = findDrawObject(id);
      static Vector dummy;
      if(ns == NULL) return dummy;
      return ns->object()->getPosition();
    }

    const Quaternion& GraphicsManager::getDrawObjectQuaternion(unsigned long id) {
      OSGNodeStruct *ns = findDrawObject(id);
      static Quaternion dummy;
      if(ns == NULL) return dummy;
      return ns->object()->getQuaternion();
    }

    interfaces::LoadMeshInterface* GraphicsManager::getLoadMeshInterface(void) {
      return guiHelper;
    }

    interfaces::LoadHeightmapInterface* GraphicsManager::getLoadHeightmapInterface(void) {
      return guiHelper;
    }

    void GraphicsManager::makeChild(unsigned long parentId,
                                    unsigned long childId) {
      OSGNodeStruct *parent = findDrawObject(parentId);
      OSGNodeStruct *child = findDrawObject(childId);
      osg::PositionAttitudeTransform *parentTransform;
      osg::PositionAttitudeTransform *childTransform;

      if(!parent || !child) return;

      parentTransform = parent->object()->getPosTransform();
      childTransform = child->object()->getPosTransform();

      parentTransform->addChild(childTransform);
      scene->removeChild(childTransform);
      shadowedScene->removeChild(childTransform);
    }


  } // end of namespace graphics
} // end of namespace mars

DESTROY_LIB(mars::graphics::GraphicsManager);
CREATE_LIB(mars::graphics::GraphicsManager);
