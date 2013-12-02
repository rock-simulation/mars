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

#ifndef MARS_INTERFACES_GRAPHICS_GUI_INTERFACE_H
#define MARS_INTERFACES_GRAPHICS_GUI_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "GraphicsGuiInterface.h"
#endif

#include <string>

#ifdef NO_TR1
#include <functional>
#else
#include <tr1/functional>
#endif

namespace osgWidget{
  class WindowManager;
};


namespace mars {
  namespace interfaces {

    /**
     * Interface for the osgWidget functionality. Every 3D Window inherits from this interface.
     * roughly spoken a window contains widgets. in widgets you can put you functionality.
     * there are 3 types of windows and one frame, its also a window, but should be movable and contain one window.
     */
    class GraphicsGuiInterface {
    public:
      /**
       * callback declaration for simple callback
       */
      typedef void (*guiClickCallBack)(double,double);
      /**
       *  declaration for the binding for more sophisticated callback
       */
#ifdef NO_TR1
      typedef std::function<void(double,double)> guiClickCallBackBind;
#else
      typedef std::tr1::function<void(double,double)> guiClickCallBackBind;
#endif

      GraphicsGuiInterface() {}
      virtual ~GraphicsGuiInterface() {}
      /**
       * if you want to create your own gui without this interface, feel free to do so 
       */
      virtual osgWidget::WindowManager* getOrCreateWindowManager() = 0;
      /**
       * Creates an osgWidget::Box. The parameter are for the type, how the box will order the widgets.
       * Types are VERTICAL = 0 and HORIZONTAL = 1
       * A Box is the most basic window, that contain your widgets
       */
      virtual int createBox(const std::string& name,int type=0)=0; 
      /**
       * Creates an osgWidget::Canvas. In a canvas you can put your widgets where ever you want. This means if you add a widget to the canvas you have to provide
       * the coordinates.
       */
      virtual int createCanvas(const std::string& name)=0; 
      /**
       *  An osgWidget::Table provides a basic grid layout. When adding the widget to the table you have to pass the place in the table as well. 
       */
      virtual int createTable(const std::string& name,int row, int colums)=0; 
      /**
       * creates a Frame that basically holds on window.  
       */
      virtual int createFrame(const std::string& name,
                              float x1, float y1,
                              float x2, float y2)=0; 
   
      /**
       *  add the window with the id to the windowmanager
       */
      virtual bool showWindow(int wndId)=0;  
      /**
       * removes the window from the windowmanager
       */
      virtual bool hideWindow(int wndId)=0;  
      /**
       * deletes your window and all containing widgets 
       */
      virtual bool deleteWindow(int wndId)=0;  
      /**
       * deletes one widget and his callbacks
       */
      virtual bool deleteWidget(int wdgId)=0;  
    
      /**
       *  sets the position of a window on the screen
       */
      virtual bool windowSetPosition(int wndId,float x,float y)=0;
      /**
       *  sets the color of a widget or a window
       */
      virtual bool setColor(int id,float r,float g,float b,float a)=0;
      /**
       *  adds window to a widget. if you remember the different window types (table, canvas, box)
       * their properties come into play here.  
       */
      virtual bool addWidgetToWindow(int window,int widget)=0;
      virtual bool addWidgetToWindow(int window,int widget,float x, float y)=0;
      virtual bool addWidgetToWindow(int window,int widget,int x, int y)=0;
  
      /**
       *  creates a basic widget, within the size you want it to be
       */
      virtual int createWidget(const std::string &name,float sizex,float sizey)=0;
      /**
       * creates a label,a widget with text in it. Don't forget to set the font color and maybe other imported thinks for you after creation 
       */
      virtual int createLabel(const std::string &name,const std::string &text)=0;
      /**
       * nearly like label, but you are able to input thinks into it
       */
      virtual int createInput(const std::string &name,const std::string &text,
                              int count)=0;
  
      /**
       *  should be clear what does this mean..
       *   Example:
       *     setFont(id,"fonts/arial.ttf");
       *     setFontSize(id,10);
       *     setColor(id, 0.8f, 0.8f, 0.8f, 0.6f);
       * 
       */
      virtual bool setFont(int id,const std::string &fontname)=0;
      virtual bool setFontColor(int id,float r, float g,float b,float a)=0;
      virtual bool setFontSize(int id,int size)=0;
  
      /**
       * sets the text of a lable
       */
      virtual bool setLabel(int id,const std::string& text)=0;
      /**
       * creates style, that you can apply to a widget or window, in order to set a lot of attributes, that are maybe not supported in this api
       */
      virtual bool createStyle(const std::string& name,const std::string &style)=0;
      /**
       * sets a style to a window or widget, that you created before with createStyle
       */
      virtual bool setStyle(int id,const std::string &styleName)=0;
      /**
       * sets the size of a widget
       */
      virtual bool setSize(int id,float x, float y)=0;
      /**
       *  used to add callback to a a widget.
       * use the first callback parameter for simple function call,
       * if this is not what you are searching for, because you want to call the member function of a instance, feel free to use second parameter and set first to NULL
       */
      virtual bool addMousePushEventCallback(int id,
                                             guiClickCallBack function,
                                             guiClickCallBackBind *bindptr=0)=0;
      virtual bool addMouseReleaseEventCallback(int id,
                                                guiClickCallBack function,
                                                guiClickCallBackBind *bindptr=0)=0;
      virtual bool addMouseEnterEventCallback(int id,
                                              guiClickCallBack function,
                                              guiClickCallBackBind *bindptr=0)=0;
      virtual bool addMouseLeaveEventCallback(int id,
                                              guiClickCallBack function,
                                              guiClickCallBackBind *bindptr=0)=0;
    
      /**
       *  sets the image of a widget
       */
      virtual bool setImage(int id,const std::string& path)=0;
      /**
       * sets the layer of a widget, please use the Layer enum for this
       */
      virtual bool setLayer(int id,int layer, int offset=0)=0;
      /**
       * returns you the current layer
       */
      virtual bool getLayer(int id,int &layer)=0;
      /**
       * sets the alignment for a widget, use the enums for this
       */
      virtual bool setAlignHorizontal (int id,int h)=0;
      virtual bool setAlignVertical (int id, int v)=0;
      virtual bool getAlignHorizontal(int id,int &h)=0;
      virtual bool getAlignVertical(int id, int &v)=0; 
  
      /**
       *like alignment but for windows, use also the same enums for this
       * */
      virtual bool setAnchorVertical(int id,int va)=0;
      virtual bool setAnchorHorizontal(int id,int ha)=0;
 
  
      virtual bool setCanFill(int id, bool state)=0;
      virtual bool setShadow(int id,float intensity)=0;
      /**
       * add bogus size to your widget
       */
      virtual bool addSize(int id, float x, float y)=0;
      /**
       * add color to setColor..
       */
      virtual bool addColor(int id,float r,float g,float b,float a)=0;
        
      /**
       * use this to define the layer, if you set it
       * */
      enum Layer {
        LAYER_TOP    = 100,
        LAYER_HIGH   = 75,
        LAYER_MIDDLE = 50,
        LAYER_LOW    = 25,
        LAYER_BG     = 0
      };

      /**
       * used for setAlignVertical and and  setAnchorVertical
       */
      enum VerticalAlignment {
        VA_CENTER,
        VA_TOP,
        VA_BOTTOM
      };

      /**
       * use this for setAnchorHorizontal and setAlignHorizontal
       */
      enum HorizontalAlignment {
        HA_CENTER,
        HA_LEFT,
        HA_RIGHT
      };
    
    protected:
    }; // end of class GraphicsGuiInterface

  } // end of namespace interfaces
} // end of namespace mars

#endif /* MARS_INTERFACES_GRAPHICS_GUI_INTERFACE_H */
