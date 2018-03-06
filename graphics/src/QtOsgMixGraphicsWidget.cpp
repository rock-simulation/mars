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
#include <QVBoxLayout>
#include <osgQt/GraphicsWindowQt>

#include "QtOsgMixGraphicsWidget.h"
#include "HUD.h"
#include "GraphicsManager.h"

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

    // Store current active window, which we change if we got an mouse in event
    // on another graphics window. This is needed because the focus handling isn't
    // working correctly and the mouse event are all captured by the first window.
    QtOsgMixGraphicsWidget* QtOsgMixGraphicsWidget::activeWindow = NULL;
    QtOsgMixGraphicsWidget* QtOsgMixGraphicsWidget::eventInWindow = NULL;

    using Qt::WindowFlags;

    QtOsgMixGraphicsWidget* QtOsgMixGraphicsWidget::createInstance(
                                                                   void *parent, osg::Group *scene, unsigned long id, bool rtt_widget,
                                                                   Qt::WindowFlags f, GraphicsManager *gm) {
      return new QtOsgMixGraphicsWidget(parent, scene, id, rtt_widget, f, gm);
    }



    void QtOsgMixGraphicsWidget::initialize() {
      //this->setGeometry(50, 50, 720, 405);
      this->setMouseTracking(true);
      setAttribute(Qt::WA_PaintOnScreen);
      setAttribute(Qt::WA_OpaquePaintEvent);
      window()->setAttribute(Qt::WA_DeleteOnClose);
      setFocusPolicy(Qt::ClickFocus);
      window()->installEventFilter(this);
#if (QT_VERSION < QT_VERSION_CHECK(5, 2, 0))
      retinaScale = 1.0;
#else
      retinaScale = devicePixelRatio();
#endif
      fprintf(stderr, "retina scale: %g\n", retinaScale);
    }

    osg::ref_ptr<osg::GraphicsContext> QtOsgMixGraphicsWidget::createWidgetContext(
                                                                                   void* parent,
                                                                                   osg::ref_ptr<osg::GraphicsContext::Traits> traits) {
      traits->windowDecoration = false;

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

      osgQt::GraphicsWindowQt *gc = new osgQt::GraphicsWindowQt(traits.get());
      QVBoxLayout *l = new QVBoxLayout(this);
      l->addWidget(gc->getGLWidget());
      gc->getGLWidget()->setMouseTracking(false);
      eventChild = gc->getGLWidget();
      eventChild->installEventFilter(this);
      l->setContentsMargins(0, 0, 0, 0);


      if (parent) {
        traits->x = ((QWidget*)parent)->x()*retinaScale;
        traits->y = ((QWidget*)parent)->y()*retinaScale;
        traits->width = ((QWidget*)parent)->width()*retinaScale;
        traits->height = ((QWidget*)parent)->height()*retinaScale;
      }
      else {
        traits->x = x()*retinaScale;
        traits->y = y()*retinaScale;
        traits->width = width()*retinaScale;
        traits->height = height()*retinaScale;
      }
      return gc;
    }

    void QtOsgMixGraphicsWidget::setWGeometry(int top, int left, int width, int height) {

      widgetX = left;
      widgetY = top;
      widgetWidth = width;
      widgetHeight = height;
      window()->setGeometry(widgetX/retinaScale, widgetY/retinaScale,
                            widgetWidth/retinaScale, widgetHeight/retinaScale);
      applyResize();
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
      return (void*)((QWidget*)this);
    }

    void QtOsgMixGraphicsWidget::showWidget() {
      show();
    }

    void QtOsgMixGraphicsWidget::updateView() {
      GraphicsWidget::updateView();
    }

    void QtOsgMixGraphicsWidget::resizeEvent( QResizeEvent * event ) {
      const QSize & geometrySize = event->size();
      if(graphicsWindow) {
        //fprintf(stderr, "resize: %d %d %d %d\n", window()->geometry().x(),
        //        window()->geometry().y(), geometrySize.width(), geometrySize.height());
        graphicsWindow->getEventQueue()->windowResize(
                                                      window()->geometry().x()*retinaScale, window()->geometry().y()*retinaScale,
                                                      window()->width()*retinaScale, window()->height()*retinaScale);
      }
    }

    void QtOsgMixGraphicsWidget::moveEvent( QMoveEvent * event ) {
      if(graphicsWindow) {
        graphicsWindow->getEventQueue()->windowResize(
                                                      window()->geometry().x()*retinaScale, window()->geometry().y()*retinaScale,
                                                      window()->width()*retinaScale, window()->height()*retinaScale);
      }
    }

    void QtOsgMixGraphicsWidget::focusInEvent( QFocusEvent *event) {
      gm->setActiveWindow(this);
      gm->setActiveWindow(widgetID);
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
      case Qt::Key_Delete:
        return osgGA::GUIEventAdapter::KEY_Delete;
      default:
        return (osgGA::GUIEventAdapter::KeySymbol)*e->text().toLatin1().data();
      }
    }

    void QtOsgMixGraphicsWidget::keyPressEvent(QKeyEvent* e) {
      // switch focus if needed
      if(activeWindow != eventInWindow) {
        activeWindow = eventInWindow;
        activeWindow->focusInEvent(NULL);
        activeWindow->keyPressEvent(e);
        return;
      }
      // todo: check if this windows handling is still correct
      //#ifndef WIN32

      view->getEventQueue()->keyPress(qtToOsgKey(e));
      //#endif
    }

    void QtOsgMixGraphicsWidget::keyReleaseEvent(QKeyEvent* e) {
      //#ifndef WIN32
      view->getEventQueue()->keyRelease(qtToOsgKey(e));
      //#endif
    }

    void QtOsgMixGraphicsWidget::hideEvent(QHideEvent * event) {
      CPP_UNUSED(event);
    }

    void QtOsgMixGraphicsWidget::showEvent(QShowEvent * event) {
    }

    void QtOsgMixGraphicsWidget::closeEvent( QCloseEvent * event ) {
      fprintf(stderr, "receive close event\n");
      event->accept();
      //graphicsWindow->getEventQueue()->closeWindow();
      // this should also be done if the QCloseEvent is accepted
      // don't know why there are problems at the moment
      //hide();
    }

    void QtOsgMixGraphicsWidget::mouseMoveEvent(QMouseEvent* e) {
#ifndef WIN32
#ifdef __APPLE__
      view->getEventQueue()->mouseMotion(e->x()*retinaScale, -e->y()*retinaScale);
#else
      view->getEventQueue()->mouseMotion(e->x()*retinaScale, -e->y()*retinaScale);
#endif
#endif
    }

    void QtOsgMixGraphicsWidget::mousePressEvent(QMouseEvent* e) {
      // switch focus if needed
      if(activeWindow != eventInWindow) {
        activeWindow = eventInWindow;
        activeWindow->focusInEvent(NULL);
        activeWindow->mousePressEvent(e);
        return;
      }
      int button = 0;

      switch(e->button()) {
      case(Qt::LeftButton): button = 1; break;
      case(Qt::MidButton): button = 2; break;
      case(Qt::RightButton): button = 3; break;
      case(Qt::NoButton): button = 0; break;
      default: button = 0; break;
      }
#ifndef WIN32
#ifdef __APPLE__
      view->getEventQueue()->mouseButtonPress(e->x()*retinaScale, -e->y()*retinaScale, button);
#else
      view->getEventQueue()->mouseButtonPress(e->x()*retinaScale, -e->y()*retinaScale, button);
#endif
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
#ifdef __APPLE__
      view->getEventQueue()->mouseButtonRelease(e->x()*retinaScale, -e->y()*retinaScale, button);
#else
      view->getEventQueue()->mouseButtonRelease(e->x()*retinaScale, -e->y()*retinaScale, button);
#endif
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
      if(event->type() == QEvent::Enter) {
        eventInWindow = this;
        return false;
      }
      if (obj != parent() && obj != this && obj != eventChild) {

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
          return true;
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
          return true;
        }
      }
      else if (event->type() == QEvent::MouseButtonPress) {
        mousePressEvent(static_cast<QMouseEvent *>(event));
        return true;
      }
      else if (event->type() == QEvent::MouseButtonRelease) {
        mouseReleaseEvent(static_cast<QMouseEvent *>(event));
        return true;
      }
      else if (event->type() == QEvent::MouseMove) {
        mouseMoveEvent(static_cast<QMouseEvent *>(event));
        return true;
      }
      else if (event->type() == QEvent::Move) {
        QMoveEvent *re = static_cast<QMoveEvent *>(event);
        moveEvent(re);
        return true;
      } else if (event->type() == QEvent::Resize) {
        QResizeEvent *re = static_cast<QResizeEvent *>(event);
        setGeometry(0, 0, re->size().width(), re->size().height());
        // if(re) resizeEvent(re);

        // event handled, return true because parent does not have to see
        // this event ???
        return true;
      }
      else if (event->type() == QEvent::Close) {
        //QCloseEvent *ce = static_cast<QCloseEvent *>(event);
        //closeEvent( ce );
        return false;
      }
      return true;
    }

  } // end of namespace graphics
} // end of namespace mars
