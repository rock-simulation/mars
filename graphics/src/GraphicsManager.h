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
 *  GraphicsManager.h
 *  Simulator
 *
 *  Created by borchers on 27.05.08.
 */

#ifndef MARS_GRAPHICS_MANAGER_H
#define MARS_GRAPHICS_MANAGER_H

#ifdef _PRINT_HEADER_
  #warning "GraphicsManager.h"
#endif


//OSG includes
#include <osg/Light>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/CompositeViewer>
#include <osg/CullFace>

#include <osgShadow/ShadowedScene>
#include <osgShadow/LightSpacePerspectiveShadowMap>
//#include <osgShadow/ParallelSplitShadowMap>
//#include <osgShadow/SoftShadowMap>
//#include <osgShadow/ShadowMap>

#include "ShadowMap.h"
#include "ParallelSplitShadowMap.h"

#include <osgParticle/PrecipitationEffect>

#include <mars_interfaces/MARSDefs.h>
#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>
#include <mars_interfaces/core_objects_exchange.h>
#include <mars_interfaces/GraphicData.h>
#include <mars_interfaces/LightData.h>
#include <mars_interfaces/MaterialData.h>
#include <mars_interfaces/cameraStruct.h>
#include <mars_interfaces/graphics/GraphicsEventInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>
#include <mars/cfg_manager/CFGClient.h>

#include <mars/osg_material_manager/OsgMaterialManager.h>
#include <mars/osg_material_manager/MaterialNode.h>
#include <mars/osg_material_manager/OsgMaterial.h>
#include <osg_frames/FramesFactory.hpp>

#include <mars/utils/Mutex.h>

// DataBroker includes
#include <mars/data_broker/ProducerInterface.h>
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/data_broker/DataPackageMapping.h>

#include "gui_helper_functions.h"

namespace mars {
  namespace graphics {

    class GraphicsWidget;
    class DrawObject;
    class OSGNodeStruct;
    class OSGHudElementStruct;
    class HUDElement;


    //mapping and control structs
    struct drawMapper {
      interfaces::drawStruct ds;
      std::vector<osg::Node*> nodes;
    };

    /**
     * internal struct to manage lights
     */
    struct lightmanager {
      osg::ref_ptr<osg::LightSource> lightSource;
      osg::ref_ptr<osg::Light> light;
      mars::interfaces::LightData lStruct;
      bool free;
    };

    typedef std::map< unsigned long, osg::ref_ptr<OSGNodeStruct> > DrawObjects;
    typedef std::list< osg::ref_ptr<OSGNodeStruct> > DrawObjectList;
    typedef std::list< osg::ref_ptr<OSGHudElementStruct> > HUDElements;

    class GraphicsManager : public interfaces::GraphicsManagerInterface,
                            public interfaces::GraphicsEventInterface,
                            public cfg_manager::CFGClient,
                            public data_broker::ProducerInterface
    {

    public:
      GraphicsManager(lib_manager::LibManager *theManager, void *QTWidget = 0);
      ~GraphicsManager();

      CREATE_MODULE_INFO();

      virtual void initializeOSG(void *data, bool createWindow=true) override;

      virtual void* getWindowManager(int id=1) override; // get osgWidget WindowManager*

      virtual void reset() override; ///< Resets scene.

      virtual void addGraphicsUpdateInterface(interfaces::GraphicsUpdateInterface *g) override;
      virtual void removeGraphicsUpdateInterface(interfaces::GraphicsUpdateInterface *g) override;

      virtual const mars::interfaces::GraphicData getGraphicOptions(void) const override;
      virtual void setGraphicOptions(const mars::interfaces::GraphicData &options,
                                     bool ignoreClearColor=false) override;

      virtual void addDrawItems(interfaces::drawStruct *draw) override; ///< Adds drawStruct items to the graphics scene.
      virtual void removeDrawItems(interfaces::DrawInterface *iface) override;
      virtual void clearDrawItems(void) override;

      virtual void addLight(mars::interfaces::LightData &ls) override; ///< adds a light to the scene
      virtual void removeLight(unsigned int index) override; ///< removes a light from the scene
      virtual void editLight(unsigned long id, const std::string &key,
                             const std::string &value) override;
      virtual void updateLight(unsigned int index, bool recompileShader=false) override;
      virtual void getLights(std::vector<mars::interfaces::LightData*> *lightList) override;
      virtual void getLights(std::vector<mars::interfaces::LightData> *lightList) const override;
      virtual int getLightCount(void) const override;

      virtual unsigned long addDrawObject(const mars::interfaces::NodeData &snode,
                                          bool activated = true) override;
      virtual unsigned long getDrawID(const std::string &name) const override;
      virtual void removeLayerFromDrawObjects(unsigned long window_id) override;
      virtual void removeDrawObject(unsigned long id) override;
      virtual void setDrawObjectPos(unsigned long id, const mars::utils::Vector &pos) override;
      virtual void setDrawObjectRot(unsigned long id, const mars::utils::Quaternion &q) override;
      virtual void setDrawObjectScale(unsigned long id, const mars::utils::Vector &scale) override;
      virtual void setDrawObjectScaledSize(unsigned long id, const mars::utils::Vector &ext) override;
      virtual void setDrawObjectMaterial(unsigned long id,
                                         const mars::interfaces::MaterialData &material) override;
      virtual void addMaterial(const interfaces::MaterialData &material) override;
      virtual void setDrawObjectNodeMask(unsigned long id, unsigned int bits) override;
      // deprecated function
      virtual void setBlending(unsigned long id, bool mode) override;
      virtual void setBumpMap(unsigned long id, const std::string &bumpMap) override;

      virtual void setDrawObjectSelected(unsigned long id, bool val) override;
      virtual void setDrawObjectShow(unsigned long id, bool val) override;
      virtual void setDrawObjectRBN(unsigned long id, int val) override;
      virtual void setSelectable(unsigned long id, bool val) override;
      virtual void exportDrawObject(unsigned long id,
                                    const std::string &name) const override;
      virtual void deactivate3DWindow(unsigned long id) override;
      virtual void activate3DWindow(unsigned long id) override;
      /** \brief creates a preview node */
      virtual void preview(int action, bool resize, const std::vector<mars::interfaces::NodeData> &allNodes,
                   unsigned int num = 0, const mars::interfaces::MaterialData *mat = 0) override;
      /**\brief removes a preview node */
      virtual void removePreviewNode(unsigned long id) override;

      virtual void setTexture(unsigned long id, const std::string &filename) override;

      /**\brief returns acutal camera information */
      virtual void getCameraInfo(mars::interfaces::cameraStruct *cs) const override;
      /**\brief sets the camera type */
      virtual void setCamera(int type) override;

      /**\brief returns the graphics scene */
      virtual void* getScene() const override;
      virtual void* getScene2() const override;
      /**\brief save the scene in an "obj" file for rendering */
      virtual void saveScene(const std::string &filename) const override;
      virtual void exportScene(const std::string &filename) const override;

      /**\brief returns the global state set */
      virtual void* getStateSet() const override;

      /**\brief close existing joint axis  */
      virtual void closeAxis() override;
      /**\brief draws 2 axis from first to second to third and 2 joint axis
         in the widget */
      virtual void drawAxis(const mars::utils::Vector &first,
                            const mars::utils::Vector &second,
                            const mars::utils::Vector &third,
                            const mars::utils::Vector &axis1,
                            const mars::utils::Vector &axis2) override;

      /**\brief adds the main coordination frame to the scene */
      virtual void showCoords() override;
      /**\brief adds a local coordination frame to the scene */
      virtual void showCoords(const mars::utils::Vector &pos,
                      const mars::utils::Quaternion &rot,
                      const mars::utils::Vector &size) override;
      /**\brief removes the main coordination frame from the scene */
      virtual void hideCoords() override;
      /**\brief removes actual coordination frame from the scene*/
      virtual void hideCoords(const mars::utils::Vector &pos) override;
      virtual bool coordsVisible(void) const override;

      virtual void showGrid() override;
      virtual void hideGrid() override;
      virtual bool gridVisible(void) const override;

      virtual void showClouds() override;
      virtual void hideClouds() override;
      virtual bool cloudsVisible(void) const override;

      virtual void update() override; //< updates graphics
      virtual void draw() override;
      virtual void lock() override;
      virtual void unlock() override;

      void setWidget(GraphicsWidget *widget);
      virtual void* getQTWidget(unsigned long id) const override;
      virtual void showQTWidget(unsigned long id) override;

      virtual unsigned long new3DWindow(void *myQTWidget = 0, bool rtt = 0,
                                        int width = 0, int height = 0, const std::string &name=std::string("")) override;
      virtual interfaces::GraphicsWindowInterface* get3DWindow(unsigned long id) const override;
      virtual void remove3DWindow(unsigned long id) override;

      /**
       * Return the first matching 3D windows with the given name, 0 otherwise
       */
      virtual interfaces::GraphicsWindowInterface* get3DWindow(const std::string &name) const override;

      virtual void getList3DWindowIDs(std::vector<unsigned long> *ids) const override;
      virtual void setGrabFrames(bool value) override;
      virtual void setGraphicsWindowGeometry(unsigned long id, int top,
                                             int left, int width, int height) override;
      virtual void getGraphicsWindowGeometry(unsigned long id,
                                             int *top, int *left,
                                             int *width, int *height) const override;
      virtual void setActiveWindow(unsigned long win_id) override;
      GraphicsWidget* getGraphicsWindow(unsigned long id) const;

      // HUD Interface:
      virtual unsigned long addHUDElement(interfaces::hudElementStruct *new_hud_element) override;
      void removeHUDElement(unsigned long id) override;
      virtual void switchHUDElementVis(unsigned long id) override;
      virtual void setHUDElementPos(unsigned long id, double x, double y) override;
      virtual void setHUDElementTexture(unsigned long id,
                                        std::string texturename) override;
      virtual void setHUDElementTextureData(unsigned long id, void* data) override;
      virtual void setHUDElementTextureRTT(unsigned long id,
                                           unsigned long window_id,
                                           bool depthComponent = false) override;
      virtual void setHUDElementLabel(unsigned long id, std::string text,
                                      double text_color[4]) override;
      virtual void setHUDElementLines(unsigned long id, std::vector<double> *v,
                                      double color[4]) override;

      virtual void addEventClient(interfaces::GraphicsEventClient* theClient) override;
      virtual void removeEventClient(interfaces::GraphicsEventClient* theClient) override;
      virtual void addGuiEventHandler(interfaces::GuiEventInterface *_guiEventHandler) override;
      virtual void removeGuiEventHandler(interfaces::GuiEventInterface *_guiEventHandler) override;
      virtual void emitKeyDownEvent(int key, unsigned int modKey, unsigned long win_id) override;
      virtual void emitKeyUpEvent(int key, unsigned int modKey, unsigned long win_id) override;
      virtual void emitQuitEvent(unsigned long win_id) override;
      virtual void emitSetAppActive(unsigned long win_id) override;
      virtual void emitNodeSelectionChange(unsigned long win_id, int mode) override;
      virtual void showNormals(bool val) override;
      void showRain(bool val);
      void showSnow(bool val);
      virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) override;
      virtual void emitGeometryChange(unsigned long win_id, int left, int top, int width, int height) override;
      // return the view of a window
      virtual  void* getView(unsigned long id=1) override;
      virtual void collideSphere(unsigned long id, mars::utils::Vector pos,
                                 mars::interfaces::sReal radius) override;
      virtual const mars::utils::Vector& getDrawObjectPosition(unsigned long id=0) override;
      virtual const mars::utils::Quaternion& getDrawObjectQuaternion(unsigned long id=0) override;

      virtual mars::interfaces::LoadMeshInterface* getLoadMeshInterface(void) override;
      virtual mars::interfaces::LoadHeightmapInterface* getLoadHeightmapInterface(void) override;

      virtual void makeChild(unsigned long parentId, unsigned long childId) override;
      virtual void attacheCamToNode(unsigned long winID, unsigned long drawID) override;

      /**
       * Sets the line laser
       * @pos: position of the laser
       * @normal: normalvector of the laser-plane
       * @color: color of the laser in RGB
       * @laser: Angle of the laser, as an direction-vector
       * @openingAngle: Opening angle of the laser; for complete laserLine, choose PI
       */
      virtual void setExperimentalLineLaser(utils::Vector pos, utils::Vector normal, utils::Vector color, utils::Vector laserAngle, float openingAngle) override;

      virtual void addOSGNode(void* node) override;
      virtual void removeOSGNode(void* node) override;
      virtual unsigned long addHUDOSGNode(void* node) override;

      void removeGraphicsWidget(unsigned long id);
      virtual bool isInitialized() const  override{return initialized;}
      osg_material_manager::MaterialNode* getMaterialNode(const std::string &name);
      void setDrawLineLaser(bool val);
      osg_material_manager::MaterialNode* getSharedStateGroup(unsigned long id);
      void setUseShadow(bool v);
      void setShadowSamples(int v);
      virtual std::vector<interfaces::MaterialData> getMaterialList() const override;
      virtual void editMaterial(std::string materialName, std::string key,
                                std::string value) override;
      virtual void setCameraDefaultView(int view) override;
      inline void setActiveWindow(GraphicsWidget *g) {activeWindow = g;}
      virtual void setDrawObjectBrightness(unsigned long id, double v) override;
      virtual void edit(const std::string &key, const std::string &value) override;
      virtual void edit(unsigned long widgetID, const std::string &key,
                        const std::string &value) override;
      osg::Vec3f getSelectedPos();
      void setShadowTechnique(std::string s);

        // ## DataBroker callbacks ##
        virtual void produceData(const data_broker::DataInfo &info,
                                 data_broker::DataPackage *package,
                                 int callbackParam) override;

    private:
        data_broker::DataBrokerInterface *dataBroker;
        data_broker::DataPackageMapping dbPackageMapping;
        double preTime, avgPreTime;
        double updateTime, avgUpdateTime;
        double lightTime, avgLightTime;
        double materialTime, avgMaterialTime;
        double frameTime, avgFrameTime;
        double postTime, avgPostTime;
        int avgTimeCount;
        mars::interfaces::GraphicData graphicOptions;

      //pointer to outer space
      GraphicsWidget *osgWidget; //pointer to the QT OSG Widget
      GuiHelper *guiHelper;

      unsigned long next_hud_id;
      unsigned long next_draw_object_id;
      unsigned long next_window_id;
      unsigned long nextPreviewID;

        utils::Mutex mutex;

      osg::ref_ptr<osgViewer::CompositeViewer> viewer;

      // includes osg::lights, osg::lightsource, lightstruct and flag to check if full
      std::vector<lightmanager> myLights;

      //static objects
      osg::ref_ptr<osg::Group> scene;
      osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene;

      //osg::ref_ptr<osg::Group> scene; //the graphcis scene
      osg::ref_ptr<osg::Group> lightGroup;
      osg::ref_ptr<osg::Group> axisDrawer;
      osg::ref_ptr<osg::Group> coords;
      osg::ref_ptr<osg::Group> positionedCoords;
      osg::ref_ptr<osg::PositionAttitudeTransform> transformCoords;
      osg::ref_ptr<osg::Group> transformCoordsGroup;
      osg::ref_ptr<osg::StateSet> globalStateset, shadowStateset;

      osg::ref_ptr<osgParticle::PrecipitationEffect> snow, rain;

      osg::ref_ptr<osg::Group> grid;
      bool show_grid;
      bool showClouds_;
      bool show_coords;

      lightmanager defaultLight;

      //HUDTerminal *myTerminal;
      //HUDLabel *myLabel;
      //HUDTexture *myTexture;
      void *image_data;
      double tex_x, tex_y;
      unsigned int framecount;
      bool useFog, useNoise, drawLineLaser;
      int hudWidth, hudHeight;

      std::vector<interfaces::GuiEventInterface*> guiHandlerList;
      std::vector<interfaces::GraphicsEventClient*> graphicsEventClientList;
      osg::ref_ptr<osg::Camera> hudCamera;

      std::map<unsigned long int, unsigned long int> DrawCoreIds;

      std::vector<nodemanager> myNodes;
      DrawObjects previewNodes_;
      DrawObjects drawObjects_;
      // object selection
      DrawObjectList selectedObjects_;
      std::list<interfaces::GraphicsUpdateInterface*> graphicsUpdateObjects;
      HUDElements hudElements;

      mars::interfaces::core_objects_exchange myNode; //for updating

      // mapper vectors
      std::vector<drawMapper> draws; //drawStructs
      std::vector<GraphicsWidget*> graphicsWindows;

      osg::ref_ptr<ShadowMap> shadowMap;
      osg::ref_ptr<ParallelSplitShadowMap> pssm;

      osg_frames::FramesFactory *framesFactory;


      /**\brief adds a preview node to the scene */
      int createPreviewNode(const std::vector<mars::interfaces::NodeData> &allNodes);

      OSGNodeStruct* findDrawObject(unsigned long id) const;
      HUDElement* findHUDElement(unsigned long id) const;

      // config stuff
      cfg_manager::CFGManagerInterface *cfg;
      cfg_manager::cfgPropertyStruct cfgW_top, cfgW_left, cfgW_height, cfgW_width;
      cfg_manager::cfgPropertyStruct draw_normals, drawRain, drawSnow,
        multisamples, noiseProp, brightness, noiseAmmount, marsShader, backfaceCulling,
        drawLineLaserProp, drawMainCamera, marsShadow, hudWidthProp,
        hudHeightProp, defaultMaxNumNodeLights, shadowTextureSize,
        showGridProp, showCoordsProp, showSelectionProp, vsyncProp, showFramesProp, scaleFramesProp,
        shadowTechnique;
      cfg_manager::cfgPropertyStruct grab_frames;
      cfg_manager::cfgPropertyStruct resources_path;
      cfg_manager::cfgPropertyStruct configPath;
      cfg_manager::cfgPropertyStruct shadowSamples;
      int ignore_next_resize;
      bool set_window_prop;
      osg::ref_ptr<osg::CullFace> cull;
      bool initialized;
      GraphicsWidget *activeWindow;
      osg_material_manager::OsgMaterialManager *materialManager;
      void setupCFG(void);

      unsigned long findCoreObject(unsigned long draw_id) const;
      void setMultisampling(int num_samples);
      void setBrightness(double val);
      void setNoiseAmmount(double val);
      void setUseNoise(bool val);
      void setUseShader(bool val);
      void showFrames(bool val);
      void scaleFrames(double x);

      void initDefaultLight();
      void setColor(utils::Color *c, const std::string &key,
                    const std::string &value);
      void initShadowMap();
      void initShadowPSSM();

    }; // end of class GraphicsManager

  } // end of namespace graphics
} // end of namespace mars

#endif  /* MARS_GRAPHICS_MANAGER_H */
