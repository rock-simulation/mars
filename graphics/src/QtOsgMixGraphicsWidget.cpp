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
 * QtOsgMixAdapter.cpp
 *
 *  Created on: 08.08.2011
 *      Author: daniel
 */

#include <QTimer>
#include <QKeyEvent>
#include <QApplication>
#include <QtGui>
#include <QWidget>
#include <QKeyEvent>

#include "QtOsgMixGraphicsWidget.h"
#include "HUD.h"

// X defines these macros, and they conflict with QTEvent namespace
// evil macros get replaced pre compiling and result for example in: QEvent::6
#ifdef KeyPress
#undef KeyPress
#endif
#ifdef KeyRelease
#undef KeyRelease
#endif

namespace mars {
  namespace graphics {

    using Qt::WindowFlags;

    QtOsgMixGraphicsWidget* QtOsgMixGraphicsWidget::createInstance(
                                                                   void *parent, osg::Group *scene, unsigned long id, bool rtt_widget,
                                                                   Qt::WindowFlags f) {

      if(parent) {
        return new QtOsgMixGraphicsWidget(parent, scene, id, rtt_widget, f);
      }

      //#ifdef USE_COCOA
      QWidget *newWidget = new QWidget();
      //newWidget->setGeometry(0, 0, 500, 500);
      //newWidget->show();
      return new QtOsgMixGraphicsWidget(newWidget, scene, id, rtt_widget, f);  
      //#else
      return new QtOsgMixGraphicsWidget(parent, scene, id, rtt_widget, f);
      //#endif

    }



    void QtOsgMixGraphicsWidget::initialize() {
      //this->setGeometry(50, 50, 720, 405);
      this->setMouseTracking(true);
      setAttribute(Qt::WA_PaintOnScreen);
      setAttribute(Qt::WA_OpaquePaintEvent);
      setFocusPolicy(Qt::ClickFocus);
      window()->installEventFilter(this);
    }

    osg::ref_ptr<osg::GraphicsContext> QtOsgMixGraphicsWidget::createWidgetContext(
                                                                                   void* parent,
                                                                                   osg::ref_ptr<osg::GraphicsContext::Traits> traits) {
      traits->windowDecoration = false;

#if defined(__APPLE__)
#if defined(USE_COCOA)
      wdata =  new WindowData(WindowData::CreateOnlyView);
      traits->inheritedWindowData = wdata;
      haveNSView = false;
#else
      traits->inheritedWindowData = new WindowData(HIViewGetWindow((HIViewRef)winId()));
#endif
#else // all others
      traits->inheritedWindowData = new WindowData(winId());
#endif // __APPLE__

      osg::DisplaySettings* ds = osg::DisplaySettings::instance();
      if (ds->getStereo()) {
        switch(ds->getStereoMode()) {
        case(osg::DisplaySettings::QUAD_BUFFER):
          traits->quadBufferStereo = true;
          break;
        case(osg::DisplaySettings::VERTICAL_INTERLACE):
        case(osg::DisplaySettings::CHECKERBOARD):
        case(osg::DisplaySettings::HORIZONTAL_INTERLACE):
          traits->stencil = 8;
          break;
        default: break;
        }
      }

      osg::ref_ptr<osg::GraphicsContext> gc =
        osg::GraphicsContext::createGraphicsContext(traits.get());

      if (parent) {
        traits->x = ((QWidget*)parent)->x();
        traits->y = ((QWidget*)parent)->y();
        traits->width = ((QWidget*)parent)->width();
        traits->height = ((QWidget*)parent)->height();
      }
      else {
        traits->x = x();
        traits->y = y();
        traits->width = width();
        traits->height = height();
      }

      return gc;
    }

    void QtOsgMixGraphicsWidget::setWGeometry(int top, int left, int width, int height) {
      window()->setGeometry(left, top, width, height);
    }

    void QtOsgMixGraphicsWidget::getWGeometry(int *top, int *left, int *width, int *height) const {
      *top  = window()->y();
      *left = window()->x();
      *width  = this->width();
      *height = this->height();
    }

    void QtOsgMixGraphicsWidget::setWidgetFullscreen(bool val) {
      if(val) {
        window()->showFullScreen();
        window()->setCursor(QCursor(Qt::BlankCursor));
      } else {
        window()->showNormal();
        window()->setCursor(QCursor(Qt::ArrowCursor));
      }
    }

    void* QtOsgMixGraphicsWidget::getWidget() {
      //#ifdef USE_COCOA
      return parent();
      //#else
      //return (void*)((QWidget*)this);
      //#endif
    }

    void QtOsgMixGraphicsWidget::showWidget() {
      show();
    }

    void QtOsgMixGraphicsWidget::updateView() {
#if defined(__APPLE__) && defined(USE_COCOA)
      if(!haveNSView && !isRTTWidget) {
        //NSView* thisWindow = (NSView*)winId();
        NSView* osgWindow = wdata->getCreatedNSView();
        if(osgWindow) {
          setCocoaView((void*)osgWindow);
          haveNSView = true;
          setGeometry(0, 0, 0, 0);

          setGeometry(0, 0, parentWidget()->width(),
                      parentWidget()->height());
        }
      }
#endif
      GraphicsWidget::updateView();
    }

    void QtOsgMixGraphicsWidget::resizeEvent( QResizeEvent * event ) {
      const QSize & geometrySize = event->size();
      if(graphicsWindow) {
        graphicsWindow->resized(
                                window()->geometry().x(), window()->geometry().y(),
                                geometrySize.width(), geometrySize.height());
        graphicsCamera->setViewport(0, 0, geometrySize.width(), geometrySize.height());
        if(hudCamera) hudCamera->setViewport(0, 0, geometrySize.width(), geometrySize.height());
        if(myHUD) myHUD->resize(geometrySize.width(), geometrySize.height());
        postDrawCallback->setSize(geometrySize.width(), geometrySize.height());
      }
    }

    void QtOsgMixGraphicsWidget::moveEvent( QMoveEvent * event ) {
      if(graphicsWindow) {
        graphicsWindow->getEventQueue()->windowResize(
                                                      window()->geometry().x(), window()->geometry().y(),
                                                      window()->width(), window()->height());
      }
    }

    static int qtToOsgKey(QKeyEvent* e) {
      switch(e->key()) {
      case Qt::Key_Shift:
        return osgGA::GUIEventAdapter::KEY_Shift_L;
      case Qt::Key_Alt:
        return osgGA::GUIEventAdapter::KEY_Alt_L;
      case Qt::Key_Meta:
        return osgGA::GUIEventAdapter::KEY_Meta_L;
      case Qt::Key_Control:
        return osgGA::GUIEventAdapter::KEY_Control_L;
      case Qt::Key_Up:
        return osgGA::GUIEventAdapter::KEY_Up;
      case Qt::Key_Down:
        return osgGA::GUIEventAdapter::KEY_Down;
      case Qt::Key_Left:
        return osgGA::GUIEventAdapter::KEY_Left;
      case Qt::Key_Right:
        return osgGA::GUIEventAdapter::KEY_Right;
      default:
        return (osgGA::GUIEventAdapter::KeySymbol)*e->text().toAscii().data();
      }
    }

    void QtOsgMixGraphicsWidget::keyPressEvent(QKeyEvent* e) {
#ifndef WIN32
      view->getEventQueue()->keyPress(qtToOsgKey(e));
#endif
    }

    void QtOsgMixGraphicsWidget::keyReleaseEvent(QKeyEvent* e) {
#ifndef WIN32
      view->getEventQueue()->keyRelease(qtToOsgKey(e));
#endif
    }

    void QtOsgMixGraphicsWidget::hideEvent(QHideEvent * event) {
      CPP_UNUSED(event);
    }

    void QtOsgMixGraphicsWidget::showEvent(QShowEvent * event) {
#ifdef USE_COCOA
      haveNSView = false;
#endif
    }

    void QtOsgMixGraphicsWidget::closeEvent( QCloseEvent * event ) {
      event->accept();
      graphicsWindow->getEventQueue()->closeWindow();
      // this should also be done if the QCloseEvent is accepted
      // don't know why there are problems at the moment
      hide();
    }

    void QtOsgMixGraphicsWidget::mouseMoveEvent(QMouseEvent* e) {
#ifndef WIN32
      view->getEventQueue()->mouseMotion(e->x(), e->y());
#endif
    }

    void QtOsgMixGraphicsWidget::mousePressEvent(QMouseEvent* e) {
      int button = 0;
      switch(e->button()) {
      case(Qt::LeftButton): button = 1; break;
      case(Qt::MidButton): button = 2; break;
      case(Qt::RightButton): button = 3; break;
      case(Qt::NoButton): button = 0; break;
      default: button = 0; break;
      }
#ifndef WIN32
      view->getEventQueue()->mouseButtonPress(e->x(), e->y(), button);
#endif
      grabKeyboard();
    }

    void QtOsgMixGraphicsWidget::mouseReleaseEvent(QMouseEvent* e) {
      int button = 0;
      switch(e->button())
        {
        case(Qt::LeftButton): button = 1; break;
        case(Qt::MidButton): button = 2; break;
        case(Qt::RightButton): button = 3; break;
        case(Qt::NoButton): button = 0; break;
        default: button = 0; break;
        }

#ifndef WIN32
      view->getEventQueue()->mouseButtonRelease(e->x(), e->y(), button);
#endif
      releaseKeyboard();
    }

    void QtOsgMixGraphicsWidget::wheelEvent(QWheelEvent* event)
    {
      if(event->delta()>0){
        view->getEventQueue()->mouseScroll(osgGA::GUIEventAdapter::SCROLL_UP);
      } 
      else{
        view->getEventQueue()->mouseScroll(osgGA::GUIEventAdapter::SCROLL_DOWN);
      }
    }


    void QtOsgMixGraphicsWidget::paintEvent( QPaintEvent * event ) {
      (void) event;
      if(graphicsWindow && 0) {
        graphicsCamera->setViewport(0, 0, width(), height());
        if(hudCamera) hudCamera->setViewport(0, 0, width(), height());
        if(myHUD) myHUD->resize(width(), height());
      }
    }

    bool QtOsgMixGraphicsWidget::eventFilter(QObject *obj, QEvent *event) {
      if (obj != parent()) {
        return false;
      }
      else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);

        if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
          /*
          qDebug() << "QOSGWidget::eventFilter:  TAB Press on "
                   << qPrintable(objectName());
          */
          // Empirically have found that it's not necessary to call
          // keyPressEvent on tab press ... my guess is that OSG ignores it.
          keyPressEvent( ke );

          // Return false so that the parent QWidget will process the tab.
          return false;
        }
        else {
          /*
          qDebug() << "QOSGWidget::eventFilter:  KeyPress on "
                   << qPrintable(objectName());
          */
          keyPressEvent( ke );
          // event handled, return true because parent does not have to see
          // this event
          return false;
        }
      }
      else if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);

        if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
          /*
          qDebug() << "QOSGWidget::eventFilter:  TAB Release on "
                   << qPrintable(objectName());
          */
          keyReleaseEvent( ke );
          // Return false so that the parent QWidget will process the tab..
          return false;
        }
        else {
          /*
          qDebug() << "QOSGWidget::eventFilter:  KeyRelease on "
                   << qPrintable(objectName());
          */
          keyReleaseEvent( ke );
          // event handled, return true because parent does not have to see
          // this event
          return false;
        }
      }
      else if (event->type() == QEvent::Move) {
        QMoveEvent *re = static_cast<QMoveEvent *>(event);
        moveEvent(re);
        return false;
      } else if (event->type() == QEvent::Resize) {
        QResizeEvent *re = static_cast<QResizeEvent *>(event);
        setGeometry(0, 0, re->size().width(), re->size().height());
        // if(re) resizeEvent(re);

        // event handled, return true because parent does not have to see
        // this event ???
        return false;
      }
      else if (event->type() == QEvent::Close) {
        QCloseEvent *ce = static_cast<QCloseEvent *>(event);
        closeEvent( ce );
      }

      return false;
    }

  } // end of namespace graphics
} // end of namespace mars
