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

    QtOsgMixGraphicsWidget* QtOsgMixGraphicsWidget::createInstance(
                                                                   void *parent, osg::Group *scene, unsigned long id, bool rtt_widget,
                                                                   GraphicsManager *gm) {
      return new QtOsgMixGraphicsWidget(parent, scene, id, rtt_widget, gm);
    }



    void QtOsgMixGraphicsWidget::initialize() {
    }

    osg::ref_ptr<osg::GraphicsContext> QtOsgMixGraphicsWidget::createWidgetContext(
                                                                                   void* parent,
                                                                                   osg::ref_ptr<osg::GraphicsContext::Traits> traits) {
#if (QT_VERSION < QT_VERSION_CHECK(5, 2, 0))
      retinaScale = 1.0;
#else
      retinaScale = devicePixelRatio();
#endif
      fprintf(stderr, "retina scale: %g\n", retinaScale);
      traits->windowDecoration = true;

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

      osgQt::GraphicsWindowQt *gc = new osgQt::GraphicsWindowQt(traits.get(), (QWidget*)parent);
      childWidget = (QWidget*)gc->getGLWidget();
      eventChild = gc->getGLWidget();
      eventChild->installEventFilter(this);

      if (parent) {
        traits->x = ((QWidget*)parent)->x()*retinaScale;
        traits->y = ((QWidget*)parent)->y()*retinaScale;
        traits->width = ((QWidget*)parent)->width()*retinaScale;
        traits->height = ((QWidget*)parent)->height()*retinaScale;
      }
      else {
        traits->x = childWidget->x()*retinaScale;
        traits->y = childWidget->y()*retinaScale;
        traits->width = childWidget->width()*retinaScale;
        traits->height = childWidget->height()*retinaScale;
      }
      widgetWidth = traits->width;
      widgetHeight = traits->height;
      return gc;
    }

    void QtOsgMixGraphicsWidget::setWGeometry(int top, int left, int width, int height) {

      widgetX = left;
      widgetY = top;
      widgetWidth = width;
      widgetHeight = height;
      if(childWidget) {
        childWidget->window()->setGeometry(widgetX/retinaScale, widgetY/retinaScale,
                                 widgetWidth/retinaScale, widgetHeight/retinaScale);
      }
      applyResize();
    }

    void QtOsgMixGraphicsWidget::getWGeometry(int *top, int *left, int *width, int *height) const {
      *top  = childWidget->window()->y();
      *left = childWidget->window()->x();
      *width  = childWidget->width();
      *height = childWidget->height();
    }

    void QtOsgMixGraphicsWidget::setWidgetFullscreen(bool val) {
      if(val) {
        childWidget->window()->showFullScreen();
        childWidget->window()->setCursor(QCursor(Qt::BlankCursor));
      } else {
        childWidget->window()->showNormal();
        childWidget->window()->setCursor(QCursor(Qt::ArrowCursor));
      }
    }

    void* QtOsgMixGraphicsWidget::getWidget() {
      return (void*)childWidget;
    }

    void QtOsgMixGraphicsWidget::showWidget() {
      childWidget->show();
    }

    void QtOsgMixGraphicsWidget::updateView() {
      GraphicsWidget::updateView();
    }

    void QtOsgMixGraphicsWidget::focusInEvent( QFocusEvent *event) {
      gm->setActiveWindow(widgetID);
    }

    void QtOsgMixGraphicsWidget::hideEvent(QHideEvent * event) {
      fprintf(stderr, "hide event\n");
      CPP_UNUSED(event);
    }

    void QtOsgMixGraphicsWidget::showEvent(QShowEvent * event) {
      fprintf(stderr, "show event\n");
    }

    void QtOsgMixGraphicsWidget::closeEvent( QCloseEvent * event ) {
      fprintf(stderr, "receive close event\n");
      event->accept();
      //graphicsWindow->getEventQueue()->closeWindow();
      // this should also be done if the QCloseEvent is accepted
      // don't know why there are problems at the moment
      //hide();
    }

    void QtOsgMixGraphicsWidget::mousePressEvent(QMouseEvent* e) {
      // switch focus if needed
      if(activeWindow != eventInWindow) {
        activeWindow = eventInWindow;
        activeWindow->focusInEvent(NULL);
        activeWindow->mousePressEvent(e);
      }
      return;
    }



    bool QtOsgMixGraphicsWidget::eventFilter(QObject *obj, QEvent *event) {
      if(event->type() == QEvent::Enter) {
        eventInWindow = this;
        return false;
      }
      else if (event->type() == QEvent::MouseButtonPress) {
        mousePressEvent(static_cast<QMouseEvent *>(event));
        return false;
      }
      else if (event->type() == QEvent::Close) {
        //QCloseEvent *ce = static_cast<QCloseEvent *>(event);
        //closeEvent( ce );
        return false;
      }
      return false;
    }

  } // end of namespace graphics
} // end of namespace mars
