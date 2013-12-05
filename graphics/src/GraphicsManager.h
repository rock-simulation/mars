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
#include <osg/CullFace>

#include <osgShadow/ShadowedScene>
#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgShadow/ParallelSplitShadowMap>
#include <osgShadow/ShadowVolume>
#include <osgShadow/SoftShadowMap>
#include <osgShadow/ShadowMap>

#include <osgParticle/PrecipitationEffect>

#include <mars/interfaces/MARSDefs.h>
#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>
#include <mars/interfaces/core_objects_exchange.h>
#include <mars/interfaces/GraphicData.h>
#include <mars/interfaces/LightData.h>
#include <mars/interfaces/MaterialData.h>
#include <mars/interfaces/cameraStruct.h>
#include <mars/interfaces/graphics/GraphicsEventInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>
#include <mars/cfg_manager/CFGClient.h>

#include "gui_helper_functions.h"


#define USE_LSPSM_SHADOW 0
#define USE_PSSM_SHADOW 0
#define USE_SM_SHADOW 0
#define NUM_PSSM_SPLITS 3


namespace mars {
  namespace graphics {

    class GraphicsWidget;
    class GraphicsViewer;
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
                            public cfg_manager::CFGClient {

    public:
      GraphicsManager(lib_manager::LibManager *theManager, void *QTWidget = 0);
      ~GraphicsManager();

      CREATE_MODULE_INFO();

      virtual void initializeOSG(void *data, bool createWindow=true);

      virtual void* getWindowManager(int id=1); // get osgWidget WindowManager*

      virtual void reset(); ///< Resets scene.

      virtual void addGraphicsUpdateInterface(interfaces::GraphicsUpdateInterface *g);
      virtual void removeGraphicsUpdateInterface(interfaces::GraphicsUpdateInterface *g);

      virtual const mars::interfaces::GraphicData getGraphicOptions(void) const;
      virtual void setGraphicOptions(const mars::interfaces::GraphicData &options);

      virtual void addDrawItems(interfaces::drawStruct *draw); ///< Adds drawStruct items to the graphics scene.
      virtual void removeDrawItems(interfaces::DrawInterface *iface);
      virtual void clearDrawItems(void);

      virtual void addLight(mars::interfaces::LightData &ls); ///< adds a light to the scene
      virtual void removeLight(unsigned int index); ///< removes a light from the scene
      virtual void updateLight(unsigned int index);
      virtual void getLights(std::vector<mars::interfaces::LightData*> *lightList);
      virtual void getLights(std::vector<mars::interfaces::LightData> *lightList) const;
      virtual int getLightCount(void) const;

      virtual unsigned long addDrawObject(const mars::interfaces::NodeData &snode,
                                          bool activated = true);
      virtual void removeLayerFromDrawObjects(unsigned long window_id);
      virtual void removeDrawObject(unsigned long id);
      virtual void setDrawObjectPos(unsigned long id, const mars::utils::Vector &pos);
      virtual void setDrawObjectRot(unsigned long id, const mars::utils::Quaternion &q);
      virtual void setDrawObjectScale(unsigned long id, const mars::utils::Vector &ext);
      virtual void setDrawObjectMaterial(unsigned long id,
                                         const mars::interfaces::MaterialData &material);
      virtual void setDrawObjectNodeMask(unsigned long id, unsigned int bits);
      virtual void setBlending(unsigned long id, bool mode);
      virtual void setBumpMap(unsigned long id, const std::string &bumpMap);
      virtual void setDrawObjectSelected(unsigned long id, bool val);
      virtual void setDrawObjectShow(unsigned long id, bool val);
      virtual void setDrawObjectRBN(unsigned long id, int val);
      virtual void setSelectable(unsigned long id, bool val);
      virtual void exportDrawObject(unsigned long id,
                                    const std::string &name) const;

      /** \brief creates a preview node */
      void preview(int action, bool resize, const std::vector<mars::interfaces::NodeData> &allNodes,
                   unsigned int num = 0, const mars::interfaces::MaterialData *mat = 0);
      /**\brief removes a preview node */
      void removePreviewNode(unsigned long id);

      virtual void setTexture(unsigned long id, const std::string &filename);

      /**\brief returns acutal camera information */
      virtual void getCameraInfo(mars::interfaces::cameraStruct *cs) const;
      /**\brief sets the camera type */
      virtual void setCamera(int type);

      /**\brief returns the graphics scene */
      void* getScene() const;
      void* getScene2() const;
      /**\brief save the scene in an "obj" file for rendering */
      void saveScene(const std::string &filename) const;
      void exportScene(const std::string &filename) const;

      /**\brief returns the global state set */
      void* getStateSet() const;

      /**\brief close existing joint axis  */
      void closeAxis();
      /**\brief draws 2 axis from first to second to third and 2 joint axis
         in the widget */
      void drawAxis(const mars::utils::Vector &first,
                    const mars::utils::Vector &second,
                    const mars::utils::Vector &third,
                    const mars::utils::Vector &axis1,
                    const mars::utils::Vector &axis2);

      /**\brief adds the main coordination frame to the scene */
      void showCoords();
      /**\brief adds a local coordination frame to the scene */
      void showCoords(const mars::utils::Vector &pos,
                      const mars::utils::Quaternion &rot,
                      const mars::utils::Vector &size);
      /**\brief removes the main coordination frame from the scene */
      void hideCoords();
      /**\brief removes actual coordination frame from the scene*/
      void hideCoords(const mars::utils::Vector &pos);
      bool coordsVisible(void) const;

      void showGrid();
      void hideGrid();
      bool gridVisible(void) const;

      void showClouds();
      void hideClouds();
      bool cloudsVisible(void) const;

      virtual void update(); //< updates graphics
      virtual void draw();

      void setWidget(GraphicsWidget *widget);
      virtual void* getQTWidget(unsigned long id) const;
      virtual void showQTWidget(unsigned long id);

      virtual unsigned long new3DWindow(void *myQTWidget = 0, bool rtt = 0,
                                        int width = 0, int height = 0, const std::string &name=std::string(""));
      virtual interfaces::GraphicsWindowInterface* get3DWindow(unsigned long id) const;

      /**
       * Return the first matching 3D windows with the given name, 0 otherwise
       */
      virtual interfaces::GraphicsWindowInterface* get3DWindow(const std::string &name) const;

      virtual void getList3DWindowIDs(std::vector<unsigned long> *ids) const;
      virtual void setGrabFrames(bool value);
      virtual void setGraphicsWindowGeometry(unsigned long id, int top,
                                             int left, int width, int height);
      virtual void getGraphicsWindowGeometry(unsigned long id,
                                             int *top, int *left,
                                             int *width, int *height) const;
      virtual void setActiveWindow(unsigned long win_id);
      GraphicsWidget* getGraphicsWindow(unsigned long id) const;

      GraphicsViewer* getGraphicsViewer(void) const {return viewer;}

      // HUD Interface:
      virtual unsigned long addHUDElement(interfaces::hudElementStruct *new_hud_element);
      void removeHUDElement(unsigned long id);
      virtual void switchHUDElementVis(unsigned long id);
      virtual void setHUDElementPos(unsigned long id, double x, double y);
      virtual void setHUDElementTexture(unsigned long id,
                                        std::string texturename);
      virtual void setHUDElementTextureData(unsigned long id, void* data);
      virtual void setHUDElementTextureRTT(unsigned long id,
                                           unsigned long window_id,
                                           bool depthComponent = false);
      virtual void setHUDElementLabel(unsigned long id, std::string text,
                                      double text_color[4]);
      virtual void setHUDElementLines(unsigned long id, std::vector<double> *v,
                                      double color[4]);

      virtual void addEventClient(interfaces::GraphicsEventClient* theClient);
      virtual void removeEventClient(interfaces::GraphicsEventClient* theClient);
      virtual void addGuiEventHandler(interfaces::GuiEventInterface *_guiEventHandler);
      virtual void removeGuiEventHandler(interfaces::GuiEventInterface *_guiEventHandler);
      virtual void emitKeyDownEvent(int key, unsigned int modKey, unsigned long win_id);
      virtual void emitKeyUpEvent(int key, unsigned int modKey, unsigned long win_id);
      virtual void emitQuitEvent(unsigned long win_id);
      virtual void emitSetAppActive(unsigned long win_id);
      virtual void emitNodeSelectionChange(unsigned long win_id, int mode);
      virtual void showNormals(bool val);
      virtual void showRain(bool val);
      virtual void showSnow(bool val);
      virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property);
      virtual void emitGeometryChange(unsigned long win_id, int left, int top, int width, int height);
      // return the view of a window
      virtual  void* getView(unsigned long id=1);
      virtual void collideSphere(unsigned long id, mars::utils::Vector pos,
                                 mars::interfaces::sReal radius);
      virtual const mars::utils::Vector& getDrawObjectPosition(unsigned long id=0);
      virtual const mars::utils::Quaternion& getDrawObjectQuaternion(unsigned long id=0);

      virtual mars::interfaces::LoadMeshInterface* getLoadMeshInterface(void);
      virtual mars::interfaces::LoadHeightmapInterface* getLoadHeightmapInterface(void);

      virtual void makeChild(unsigned long parentId, unsigned long childId);
      
    private:

      mars::interfaces::GraphicData graphicOptions;

      //pointer to outer space
      GraphicsWidget *osgWidget; //pointer to the QT OSG Widget
      GuiHelper *guiHelper;

      unsigned long next_hud_id;
      unsigned long next_draw_object_id;
      unsigned long next_window_id;
      unsigned long nextPreviewID;

      GraphicsViewer *viewer;

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
      osg::ref_ptr<osg::StateSet> globalStateset;

      osg::ref_ptr<osgParticle::PrecipitationEffect> snow, rain;

      osg::ref_ptr<osg::Group> grid;
      bool show_grid;
      osg::ref_ptr<osg::Group> clouds_;
      bool showClouds_;
      bool show_coords;

      lightmanager defaultLight;

      //HUDTerminal *myTerminal;
      //HUDLabel *myLabel;
      //HUDTexture *myTexture;
      void *image_data;
      double tex_x, tex_y;
      unsigned int framecount;
      bool useFog, useNoise;

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

#if USE_SM_SHADOW==1
      osg::ref_ptr<osgShadow::ShadowMap> shadowMap;
#endif

      /**\brief adds a preview node to the scene */
      int createPreviewNode(const std::vector<mars::interfaces::NodeData> &allNodes);

      OSGNodeStruct* findDrawObject(unsigned long id) const;
      HUDElement* findHUDElement(unsigned long id) const;

      // config stuff
      cfg_manager::CFGManagerInterface *cfg;
      cfg_manager::cfgPropertyStruct cfgW_top, cfgW_left, cfgW_height, cfgW_width;
      cfg_manager::cfgPropertyStruct draw_normals, drawRain, drawSnow, multisamples, noiseProp,
        brightness, marsShader, backfaceCulling;
      cfg_manager::cfgPropertyStruct grab_frames;
      cfg_manager::cfgPropertyStruct resources_path;
      cfg_manager::cfgPropertyStruct configPath;
      int ignore_next_resize;
      bool set_window_prop;
      osg::ref_ptr<osg::CullFace> cull;


      void setupCFG(void);

      unsigned long findCoreObject(unsigned long draw_id) const;
      void setMultisampling(int num_samples);
      void setBrightness(double val);
      void setUseShader(bool val);

      void initDefaultLight();

    }; // end of class GraphicsManager

  } // end of namespace graphics
} // end of namespace mars

#endif  /* MARS_GRAPHICS_MANAGER_H */
