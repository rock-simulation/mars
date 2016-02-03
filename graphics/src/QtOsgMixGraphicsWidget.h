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
 * QtOsgMixAdapter.h
 *
 *  Created on: 08.08.2011
 *      Author: daniel
 */

#ifndef QTOSGMIXADAPTER_H
#define QTOSGMIXADAPTER_H

#ifdef __APPLE__
#ifdef __OBJC__
#else
#ifdef Q_FORWARD_DECLARE_OBJC_CLASS
#undef Q_FORWARD_DECLARE_OBJC_CLASS
#endif
#define Q_FORWARD_DECLARE_OBJC_CLASS(classname) class classname;
#endif
#endif // __APPLE__

#include "GraphicsWidget.h"

#include <QWidget>


#if defined(WIN32) && !defined(__CYGWIN__)
  #include <osgViewer/api/Win32/GraphicsWindowWin32>
#elif defined(__APPLE__)  // Assume using Carbon on Mac.
  #include <osgViewer/api/Cocoa/GraphicsWindowCocoa>
#else // all other unix
  #include <osgViewer/api/X11/GraphicsWindowX11>
#endif

namespace mars {
  namespace graphics {

    class GraphicsManager;

#if defined(WIN32) && !defined(__CYGWIN__)
  typedef HWND WindowHandle;
  typedef osgViewer::GraphicsWindowWin32::WindowData WindowData;
#elif defined(__APPLE__)
    typedef osgViewer::GraphicsWindowCocoa::WindowData WindowData;
#else // all other unix
  typedef Window WindowHandle;
  typedef osgViewer::GraphicsWindowX11::WindowData WindowData;
#endif

    /**
     * A GraphicsWidget using QWidget
     */
    class QtOsgMixGraphicsWidget: public QWidget, public GraphicsWidget
    {
      Q_OBJECT; // with this we need a generated moc file
    public:
      QtOsgMixGraphicsWidget(void *parent, osg::Group *scene,
                             unsigned long id, bool rtt_widget = 0,
                             Qt::WindowFlags f=0, GraphicsManager *gm = 0)
        : QWidget((QWidget*) parent, f),
        GraphicsWidget(parent, scene, id, rtt_widget, f, gm) {
#ifdef __APPLE__
          haveNSView = false;
#endif
        }

      // Prevent flicker on Windows Qt
      QPaintEngine* paintEngine () const { return 0; }

      virtual void initialize();
      virtual osg::ref_ptr<osg::GraphicsContext> createWidgetContext(void *parent, osg::ref_ptr<osg::GraphicsContext::Traits> traits);

      virtual void setWGeometry(int top, int left, int width, int height);
      virtual void getWGeometry(int *top, int *left, int *width, int *height) const;
      virtual void setWidgetFullscreen(bool val);

      virtual void* getWidget();
      virtual void showWidget();

      virtual void updateView();

      virtual void keyPressEvent(QKeyEvent *e);
      virtual void keyReleaseEvent(QKeyEvent *e);
      virtual void hideEvent(QHideEvent *event);
      virtual void closeEvent(QCloseEvent *event);
      virtual void showEvent(QShowEvent *event);
      virtual void mouseMoveEvent(QMouseEvent *e);
      virtual void mousePressEvent(QMouseEvent *e);
      virtual void mouseReleaseEvent(QMouseEvent *e);
      virtual void moveEvent(QMoveEvent *event );
      virtual void wheelEvent(QWheelEvent *event);
      virtual void resizeEvent(QResizeEvent *event);
      virtual void paintEvent(QPaintEvent *event);
      virtual bool eventFilter(QObject *obj, QEvent *event);
      virtual void focusInEvent(QFocusEvent *event);

      static QtOsgMixGraphicsWidget* createInstance(void *parent,
                                                    osg::Group *scene,
                                                    unsigned long id,
                                                    bool rtt_widget = 0,
                                                    Qt::WindowFlags f=0,
                                                    GraphicsManager *gm = 0);

    protected:
      virtual ~QtOsgMixGraphicsWidget() {};

#ifdef __APPLE__
    private:
      WindowData *wdata;
      bool haveNSView;
#endif
    }; // end of class QtOsgMixGraphicsWidget

  } // end of namespace graphics
} // end of namespace mars

#endif /* QTOSGMIXADAPTER_H */
