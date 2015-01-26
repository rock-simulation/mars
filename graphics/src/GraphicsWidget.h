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

#ifndef MARS_GRAPHICS_WIDGET_H
#define MARS_GRAPHICS_WIDGET_H

#ifdef _PRINT_HEADER_
  #warning "GraphicsWidget.h"
#endif

#include "gui_helper_functions.h"
#include "GraphicsCamera.h"
#include "PostDrawCallback.h"

#include <mars/interfaces/MARSDefs.h>
#include <mars/utils/Vector.h>
#include <mars/interfaces/graphics/GraphicsEventInterface.h>
#include <mars/interfaces/graphics/GraphicsGuiInterface.h>

#include <osgViewer/Viewer>
#include <osgWidget/WindowManager>
#include <osgWidget/ViewerEventHandlers>
#include <osgWidget/Util>
#include <osgGA/StateSetManipulator>
#include <osgDB/ReadFile>

#ifdef NO_TR1
#include <memory>
#else
#include <tr1/memory>
#endif

namespace mars {
  namespace graphics {

    class GraphicsManager;
    class HUD;
    class HUDElement;
    const unsigned int MASK_2D = 0xF0000000;
    
    /**
     * Widget with OpenGL context and event handling.
     */
    class GraphicsWidget : public osgGA::GUIEventHandler,
                           public interfaces::GraphicsWindowInterface,
                           public interfaces::GraphicsGuiInterface {
    public:
      GraphicsWidget(void *parent, osg::Group *scene,
                     unsigned long id, bool hasRTTWidget = 0,
                     int f=0, GraphicsManager *gm=0);
      ~GraphicsWidget();
      void initializeOSG(void *data = 0, GraphicsWidget* shared = 0,
                         int width = 0, int height = 0);

      unsigned long getID(void);

      /**\brief returns actual mouse position */
      mars::utils::Vector getMousePos();

      virtual void setWGeometry(int top, int left, int width, int height) {};
      virtual void getWGeometry(int *top, int *left,
                                int *width, int *height) {};
      void setFullscreen(bool val, int display = 1);

      osgViewer::View* getView(void);

      interfaces::GraphicsCameraInterface* getCameraInterface(void) const;
      osg::ref_ptr<osg::Camera> getMainCamera();

      osgViewer::GraphicsWindow* getGraphicsWindow();
      const osgViewer::GraphicsWindow* getGraphicsWindow() const;

      osg::Texture2D* getRTTTexture(void);
      osg::Texture2D* getRTTDepthTexture(void);

      std::vector<osg::Node*> getPickedObjects();
      void clearSelectionVectors(void);

      void setGraphicsEventHandler(interfaces::GraphicsEventInterface *graphicsEventHandler);
      void addGraphicsEventHandler(interfaces::GraphicsEventInterface *graphicsEventHandler);

      virtual osgWidget::WindowManager* getOrCreateWindowManager();
      void setHUD(HUD *theHUD);
      void addHUDElement(HUDElement *elem);
      void removeHUDElement(HUDElement* elem);
      void switchHudElemtVis(int num_element);

      /**\brief sets the clear color */
      void setClearColor(mars::utils::Color color);

      void setGrabFrames(bool grab);
      void setSaveFrames(bool grab);

      virtual void* getWidget() {return NULL;}
      virtual void showWidget() {};

      virtual void updateView();

      virtual int createBox(const std::string& name,int type=0);
      virtual int createCanvas(const std::string& name);
      virtual int createFrame(const std::string& name,float x1, float y1,float x2,float y2); 
   
      virtual bool showWindow(int wndId);
      virtual bool hideWindow(int wndId);  
      virtual bool deleteWindow(int wndId);  
      virtual bool deleteWidget(int wdgId);  
   
      virtual void setName(const std::string &name){
        this->name = name;
      }

      virtual const std::string getName() const{
        return name;
      }

      virtual bool windowSetPosition(int wndId,float x,float y);
      virtual int createWidget(const std::string &name,float sizex,float sizey);
      virtual bool setColor(int id,float r,float g,float b,float a);
      virtual bool addWidgetToWindow(int window,int widget);
      virtual bool addWidgetToWindow(int window,int widget,float x, float y);
      virtual bool addWidgetToWindow(int window,int widget,int x, int y);
  
      virtual int createLabel(const std::string &name,const std::string &text);
      virtual int createInput(const std::string &name,const std::string &text,int count);
      virtual bool setLabel(int id,const std::string& text);
     
      virtual bool setFont(int id,const std::string &fontname);
      virtual bool setFontColor(int id,float r, float g,float b,float a);
      virtual bool setFontSize(int id,int size);
      virtual bool createStyle(const std::string& name,const std::string &style);
      virtual bool setStyle(int id,const std::string &styleName);
      virtual bool setSize(int id,float x, float y);
      virtual bool addMousePushEventCallback(int id, guiClickCallBack function,guiClickCallBackBind *bindptr=0);
      virtual bool addMouseReleaseEventCallback(int id, guiClickCallBack function,guiClickCallBackBind *bindptr=0);
      virtual bool addMouseEnterEventCallback(int id, guiClickCallBack function,guiClickCallBackBind *bindptr=0);
      virtual bool addMouseLeaveEventCallback(int id, guiClickCallBack function,guiClickCallBackBind *bindptr=0);
    
      bool addEventToWidget(int id,guiClickCallBack function,guiClickCallBackBind *bindptr, osgWidget::EventType type);
    
      virtual bool setImage(int id,const std::string& path);
      virtual int createTable(const std::string& name,int row, int colums); 
      virtual bool setLayer(int id,int layer, int offset=0);
      virtual bool getLayer(int id,int &layer);
      virtual bool setAlignHorizontal(int id,int h);
      virtual bool setAlignVertical(int id, int v); 
      virtual bool getAlignHorizontal(int id,int &h);
      virtual bool getAlignVertical(int id, int &v); 
      virtual bool setAnchorVertical(int id,int va);
      virtual bool setAnchorHorizontal(int id,int ha);
 
      virtual bool setCanFill(int id, bool state);
      virtual bool setShadow(int id,float intensity);
      virtual bool addSize(int id, float x, float y);
      virtual bool addColor(int id,float r,float g,float b,float a);
      /**
       * This function copies the image data in the given buffer.
       * It assumes that the buffer ist correctly initalized
       * with a char array of the size width * height * 4
       * 
       * @param buffer buffer in which the image gets copied
       * @param width returns the width of the image
       * @param height returns the height of the image
       * */
      virtual void getImageData(char *buffer, int &width, int &height);
      virtual void getImageData(void **data, int &width, int &height);
      
      /**
       * This function copies the depth image in the given buffer.
       * It assumes that the buffer ist correctly initalized
       * with a double array of the size width * height
       * 
       * @param buffer buffer in which the image gets copied
       * @param width returns the width of the image
       * @param height returns the height of the image
       * */
      virtual void getRTTDepthData(float *buffer, int &width, int &height);
      virtual void getRTTDepthData(float **data, int &width, int &height);
  
      virtual osg::Group* getScene(){
        return scene;
      }
      virtual void setScene(osg::Group *s) {
        scene = s;
      }

      virtual void setHUDViewOffsets(double x1, double y1,
                                     double x2, double y2);
    
    protected:
      // the widget size
      int widgetWidth, widgetHeight, widgetX, widgetY;

      // protected for osg reference counter
  
      bool manageClickEvent(osgWidget::Event& event);
      std::string name; 
      osgWidget::Window* getWindowById(int wndId);
      osgWidget::Widget* getWidgetById(int wdId);
     
      int addOsgWidget(osgWidget::Widget *wid);
      int addOsgWindow(osgWidget::Window* wnd);
      osgWidget::WindowManager* _osgWidgetWindowManager;
      //   typedef std::map<int,void* > WindowCallackMapType;
      //  WindowCallackMapType _osgWindowIdMap;
     
      typedef std::map<int,osg::ref_ptr<osgWidget::Window> > WindowIdMapType;
      WindowIdMapType _osgWindowIdMap;
      typedef std::map<int,osg::ref_ptr<osgWidget::Widget> > WidgetIdMapType;
      WidgetIdMapType _osgWidgetIdMap;
     
#ifdef NO_TR1
      typedef std::pair<guiClickCallBack,std::shared_ptr<guiClickCallBackBind> > WidgetCallBackPairType;
#else
      typedef std::pair<guiClickCallBack,std::tr1::shared_ptr<guiClickCallBackBind> > WidgetCallBackPairType;
#endif
      typedef std::list<WidgetCallBackPairType > WidgetCallBackList;
      typedef std::map<int,WidgetCallBackList > WidgetCallBackMapType;
      WidgetCallBackMapType _widgetCallBackMap;
     
      unsigned int _osgWidgetWindowCnt;     
     
      // holds a single view on a scene, this view may be composed of one or more slave cameras
      osg::ref_ptr<osgViewer::View> view;
      // root of the scene
      osg::Group *scene;

      // toggle for render to texture
      bool isRTTWidget;

      // access to creating and managing graphics window and events
      osg::ref_ptr<osgViewer::GraphicsWindow> graphicsWindow;
      // the OpenGL/OSG camera
      GraphicsCamera* graphicsCamera;
      // handles some events
      std::vector<interfaces::GraphicsEventInterface *> graphicsEventHandler;

      // 2D display on top of the scene
      HUD *myHUD;
      // camera for the 2D display
      osg::ref_ptr<osg::Camera> hudCamera;

      // called post drawing
      PostDrawCallback *postDrawCallback;

    private:
      // the widget id
      unsigned long widgetID;

      // toggle for fullscreen display
      bool isFullscreen;

      // toggle for stereo display
      bool isStereoDisplay;
      // eye separation for stereo display
      float cameraEyeSeparation;

      // toggle for HUD display
      bool isHUDShown;

      // toggles for the mouse state
      bool isMouseButtonDown, isMouseMoving;
      // last mouse position from event queue
      int mouseX, mouseY;

      // destination texture if isRTTWidget==true
      osg::ref_ptr<osg::Texture2D> rttTexture;
      // destination image if isRTTWidget==true
      osg::ref_ptr<osg::Image> rttImage;

      // destination texture if isRTTWidget==true
      osg::ref_ptr<osg::Texture2D> rttDepthTexture;
      // destination image if isRTTWidget==true
      osg::ref_ptr<osg::Image> rttDepthImage;

      // list of picked objects
      std::vector<osg::Node*> pickedObjects;
      enum PickMode { DISABLED, STANDARD, FORCE_ADD, FORCE_REMOVE };
      PickMode pickmode;
      GraphicsManager *gm;

      virtual void initialize() {};
      virtual osg::ref_ptr<osg::GraphicsContext> createWidgetContext(
                                                                     void* parent, osg::ref_ptr<osg::GraphicsContext::Traits> traits);
      void createContext(void* parent, GraphicsWidget* shared, int width, int height);

      // implements osgGA::GUIEventHandler::handle
      bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
      // some event handlers...
      bool handleKeyDownEvent(const osgGA::GUIEventAdapter &ea);

      bool handleKeyUpEvent(const osgGA::GUIEventAdapter &ea);
      bool handlePushEvent(const osgGA::GUIEventAdapter &ea);
      bool handleMoveEvent(const osgGA::GUIEventAdapter &ea);
      bool handleDragEvent(const osgGA::GUIEventAdapter &ea);
      bool handleScrollEvent(const osgGA::GUIEventAdapter &ea);

      bool handleResizeEvent(const osgGA::GUIEventAdapter &ea);
      bool handleReleaseEvent(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
      void sendKeyDownEvent(const osgGA::GUIEventAdapter &ea);
      void sendKeyUpEvent(const osgGA::GUIEventAdapter &ea);
      void translateKey(int &key, unsigned int &mod);

      bool pick(const double x, const double y);

      virtual void setWidgetFullscreen(bool val) {};

      void grabFocus();

      void applyResize();

    }; // end of class GraphicsWidget

  } // end of namespace graphics 
} // end of namespace mars

#endif // GRAPHICS_WIDGET_H
