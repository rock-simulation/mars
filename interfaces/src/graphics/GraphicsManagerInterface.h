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

#ifndef MARS_INTERFACES_GRAPHICS_MANAGER_INTERFACE_H
#define MARS_INTERFACES_GRAPHICS_MANAGER_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "GraphicsManagerInterface.h"
#endif

#include "GraphicsUpdateInterface.h"
#include "GraphicsCameraInterface.h"
#include "GraphicsWindowInterface.h"
#include "GuiEventInterface.h"
#include "GraphicsEventClient.h"
#include "draw_structs.h"
#include "../NodeData.h"
#include "../GraphicData.h"
#include "../LightData.h"
#include "../MaterialData.h"
#include "../sim/LoadCenter.h"

#include <mars/lib_manager/LibInterface.h>
#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>

#include <vector>
#include <string>

namespace mars {
  namespace interfaces {

    class LoadMeshInterface;
    class LoadHeightfieldInterface;


    class GraphicsManagerInterface : public mars::lib_manager::LibInterface {

    public:

      virtual void* getWindowManager(int id) =0;
      // virtual void* getCameraInterface(int id);
      
      GraphicsManagerInterface(mars::lib_manager::LibManager *theManager)
        : lib_manager::LibInterface(theManager)
      {}
      virtual ~GraphicsManagerInterface()
      {}

      int getLibVersion() const
      { return 1; }
      const std::string getLibName() const
      { return std::string("mars_graphics"); }
      virtual void createModuleInfo() {}

      virtual void addDrawItems(drawStruct *draw) = 0; ///< Adds \c drawStruct items to the graphics scene
      virtual void removeDrawItems(DrawInterface *iface) = 0;
      virtual void clearDrawItems(void) = 0;

      virtual void addLight(LightData &ls) = 0; ///< Adds a light to the scene.

      virtual void addGraphicsUpdateInterface(GraphicsUpdateInterface *g) = 0;
      virtual void removeGraphicsUpdateInterface(GraphicsUpdateInterface *g) = 0;
      virtual unsigned long addDrawObject(const NodeData &snode,
                                          bool activated = true) = 0;
      virtual void removeDrawObject(unsigned long id) = 0;
      virtual void setDrawObjectPos(unsigned long id,
                                    const mars::utils::Vector &pos) = 0;
      virtual void setDrawObjectRot(unsigned long id,
                                    const mars::utils::Quaternion &q) = 0;
      virtual void setDrawObjectScale(unsigned long id,
                                      const mars::utils::Vector &ext) = 0;
      virtual void setDrawObjectMaterial(unsigned long id, 
                                         const MaterialData &material) = 0;
      virtual void setDrawObjectNodeMask(unsigned long id, unsigned int bits) = 0;

      virtual void closeAxis() = 0; ///< Closes existing joint axis.

      virtual void drawAxis(const mars::utils::Vector &first,
                            const mars::utils::Vector &second, 
                            const mars::utils::Vector &third,
                            const mars::utils::Vector &axis1,
                            const mars::utils::Vector &axis2) = 0; ///< Draws 2 axes from first to second to third and 2 joint axes in the widget.

      virtual void getCameraInfo(cameraStruct *cs) const = 0;  ///< Returns current camera information.

      virtual void* getStateSet() const = 0; ///< Provides access to global state set.

      virtual void* getScene() const = 0; ///< Provides access to the graphics scene.
      virtual void* getScene2() const = 0; ///< ?

      virtual void hideCoords() = 0; ///< Removes the main coordination frame from the scene.

      virtual void hideCoords(const mars::utils::Vector &pos) = 0; ///< Removes current coordination frame from the scene.

      virtual void showClouds() = 0;
      virtual void hideClouds() = 0;

      virtual void preview(int action, bool resize, 
                           const std::vector<NodeData> &allNodes,
                           unsigned int num = 0,
                           const MaterialData *mat = 0) = 0; ///< Creates a preview node.

      virtual void removeLight(unsigned int index) = 0; ///< Removes a light from the scene.

      virtual void removePreviewNode(unsigned long id) = 0; ///< Removes a preview node.

      virtual void reset() = 0; ///< Resets scene.

      virtual void setCamera(int type) = 0; ///< Sets the camera type.

      virtual void showCoords() = 0; ///< Adds the main coordination frame to the scene.

      virtual void showCoords(const mars::utils::Vector &pos,
                              const mars::utils::Quaternion &rot,
                              const mars::utils::Vector &size) = 0; ///< Adds a local coordination frame to the scene.

      virtual bool coordsVisible(void) const = 0;
      virtual bool gridVisible(void) const = 0;
      virtual bool cloudsVisible(void) const = 0;

      virtual void update() = 0; ///< Update the scene.
      virtual void saveScene(const std::string &filename) const = 0;
      virtual const GraphicData getGraphicOptions(void) const = 0;
      virtual void setGraphicOptions(const GraphicData &options) = 0;
      virtual void showGrid(void) = 0;
      virtual void hideGrid(void) = 0;
      virtual void updateLight(unsigned int index) = 0;
      virtual void getLights(std::vector<LightData*> *lightList) = 0;
      virtual void getLights(std::vector<LightData> *lightList) const = 0;
      virtual int getLightCount(void) const = 0;
      virtual void exportScene(const std::string &filename) const = 0;
      virtual void setTexture(unsigned long id, const std::string &filename) = 0;
      virtual unsigned long new3DWindow(void *myQTWidget = 0, bool rtt = 0,
                                        int width = 0, int height = 0, const std::string &name = std::string("")) = 0;
      virtual void setGrabFrames(bool value) = 0;
      virtual GraphicsWindowInterface* get3DWindow(unsigned long id) const = 0; ///< Return the first matching 3D windows with the given name, 0 otherwise.
      virtual GraphicsWindowInterface* get3DWindow(const std::string &name) const=0;
      virtual void getList3DWindowIDs(std::vector<unsigned long> *ids) const = 0;
      virtual void removeLayerFromDrawObjects(unsigned long window_id) = 0;
      
      // HUD Interface:
      virtual unsigned long addHUDElement(hudElementStruct *new_hud_element) = 0;
      virtual void removeHUDElement(unsigned long id) = 0;
      virtual void switchHUDElementVis(unsigned long id) = 0;
      virtual void setHUDElementPos(unsigned long id, double x, double y) = 0;
      virtual void setHUDElementTextureData(unsigned long id, void* data) = 0;
      virtual void setHUDElementTextureRTT(unsigned long id,
                                           unsigned long window_id,
                                           bool depthComponent = false) = 0;
      virtual void setHUDElementTexture(unsigned long id,
                                        std::string texturename) = 0;
      virtual void setHUDElementLabel(unsigned long id, std::string text,
                                      double text_color[4]) = 0;
      virtual void setHUDElementLines(unsigned long id, std::vector<double> *v,
                                      double color[4]) = 0;
      virtual void* getQTWidget(unsigned long id) const = 0;
      virtual void showQTWidget(unsigned long id) = 0;
      virtual void addGuiEventHandler(GuiEventInterface *_guiEventHandler) = 0;
      virtual void removeGuiEventHandler(GuiEventInterface *_guiEventHandler) = 0;
      virtual void exportDrawObject(unsigned long id, 
                                    const std::string &name) const = 0;
      virtual void setBlending(unsigned long id, bool mode) = 0;
      virtual void setBumpMap(unsigned long id, const std::string &bumpMap) = 0;
      virtual void setGraphicsWindowGeometry(unsigned long id, int top,
                                             int left, int width, int height) = 0;
      virtual void getGraphicsWindowGeometry(unsigned long id, 
                                             int *top, int *left, 
                                             int *width, int *height) const = 0;
      virtual void setActiveWindow(unsigned long win_id) = 0;
      virtual void setDrawObjectSelected(unsigned long id, bool val) = 0;
      virtual void setDrawObjectShow(unsigned long id, bool val) = 0;
      virtual void setDrawObjectRBN(unsigned long id, int val) = 0;
      virtual void addEventClient(GraphicsEventClient* theClient) = 0;
      virtual void removeEventClient(GraphicsEventClient* theClient) = 0;
      virtual void setSelectable(unsigned long id, bool val) = 0;
      virtual void showNormals(bool val) = 0;
      virtual void* getView(unsigned long id=1)=0; ///< Returns the view of a window. The first window has id 1, this is also the default value. Return 0 if the window does not exist.
      virtual void collideSphere(unsigned long id, mars::utils::Vector pos,
                                 sReal radius) = 0;
      virtual const utils::Vector& getDrawObjectPosition(unsigned long id=0) = 0;
      virtual const utils::Quaternion& getDrawObjectQuaternion(unsigned long id=0) = 0;
      
      virtual void draw() = 0;
      virtual void initializeOSG(void *data, bool createWindow=true) = 0;
      virtual LoadMeshInterface* getLoadMeshInterface(void) = 0;
      virtual LoadHeightmapInterface* getLoadHeightmapInterface(void) = 0;
      
      virtual void makeChild(unsigned long parentId, unsigned long childId) = 0;

    }; // end of class GraphicsManagerInterface

  } // end of namespace interfaces
} // end of namespace mars

#endif /* MARS_INTERFACES_GRAPHICS_MANAGER_INTERFACE_H */
