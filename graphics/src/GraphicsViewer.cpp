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

#include "GraphicsViewer.h"

namespace mars {
  namespace graphics {

    using namespace osgViewer;

    GraphicsViewer::GraphicsViewer(interfaces::GuiEventInterface *_guiEventHandler) {
      guiEventHandler = _guiEventHandler;
    }

    GraphicsViewer::~GraphicsViewer(void) {

    }

    void GraphicsViewer::eventTraversal() {
      if (_done) return;
    
      if (_views.empty()) return;
    
      double beginEventTraversal = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());

      // osg::notify(osg::NOTICE)<<"CompositeViewer::frameEventTraversal()."<<std::endl;
    
      // need to copy events from the GraphicsWindow's into local EventQueue;
    
      typedef std::map<osgViewer::View*, osgGA::EventQueue::Events> ViewEventsMap;
      ViewEventsMap viewEventsMap;
    
      Contexts contexts;
      getContexts(contexts);

      Scenes scenes;
      getScenes(scenes);

      osgViewer::View* masterView = getViewWithFocus() ? getViewWithFocus() : _views[0].get();
    
      osg::Camera* masterCamera = masterView->getCamera();
      osgGA::GUIEventAdapter* eventState = masterView->getEventQueue()->getCurrentEventState(); 
      osg::Matrix masterCameraVPW = masterCamera->getViewMatrix() * masterCamera->getProjectionMatrix();
      if (masterCamera->getViewport()) {
        osg::Viewport* viewport = masterCamera->getViewport();
        masterCameraVPW *= viewport->computeWindowMatrix();
      }

      for(Contexts::iterator citr = contexts.begin();
          citr != contexts.end();
          ++citr) {
        osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(*citr);
        if (gw) {
          gw->checkEvents();
      
          osgGA::EventQueue::Events gw_events;
          gw->getEventQueue()->takeEvents(gw_events);
      
          osgGA::EventQueue::Events::iterator itr;
          for(itr = gw_events.begin(); itr != gw_events.end(); ++itr) {
            osgGA::GUIEventAdapter* event = itr->get();
        
            //osg::notify(osg::NOTICE)<<"event->getGraphicsContext()="<<event->getGraphicsContext()<<std::endl;
        
            bool pointerEvent = false;
        
            float x = event->getX();
            float y = event->getY();
        
            bool invert_y = event->getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS;
            if (invert_y && gw->getTraits()) y = gw->getTraits()->height - y;
        
            switch(event->getEventType())
              {
              case(osgGA::GUIEventAdapter::RESIZE):
                //setCameraWithFocus(0);
                setCameraWithFocus(*((gw->getCameras()).begin()));
                masterView = getViewWithFocus();
                masterCamera = masterView->getCamera();
                break;
              case(osgGA::GUIEventAdapter::PUSH):
              case(osgGA::GUIEventAdapter::RELEASE):
              case(osgGA::GUIEventAdapter::DRAG):
              case(osgGA::GUIEventAdapter::MOVE): {
                pointerEvent = true;

                if (event->getEventType()!=osgGA::GUIEventAdapter::DRAG || !getCameraWithFocus()) {
                  osg::GraphicsContext::Cameras& cameras = gw->getCameras();
                  for(osg::GraphicsContext::Cameras::iterator citr = cameras.begin();
                      citr != cameras.end();
                      ++citr) {
                    osg::Camera* camera = *citr;
                    if (camera->getView() && 
                        camera->getAllowEventFocus() &&
                        camera->getRenderTargetImplementation()==osg::Camera::FRAME_BUFFER) {
                      osg::Viewport* viewport = camera ? camera->getViewport() : 0;
                      if (viewport && 
                          x >= viewport->x() && y >= viewport->y() &&
                          x <= (viewport->x()+viewport->width()) && y <= (viewport->y()+viewport->height()) ) {
                        setCameraWithFocus(camera);
                    
                        // If this camera is not a slave camera
                        if (camera->getView()->getCamera() == camera) {
                          eventState->setGraphicsContext(gw);
                          eventState->setInputRange( viewport->x(), viewport->y(), 
                                                     viewport->x()+viewport->width(),
                                                     viewport->y()+viewport->height());
                      
                        }
                        else {
                          eventState->setInputRange(-1.0, -1.0, 1.0, 1.0);
                        }
                    
                        if (getViewWithFocus()!=masterView) {
                          // need to reset the masterView
                          masterView = getViewWithFocus();
                          masterCamera = masterView->getCamera();
                          eventState = masterView->getEventQueue()->getCurrentEventState(); 
                          masterCameraVPW = masterCamera->getViewMatrix() * masterCamera->getProjectionMatrix();
                      
                          if (masterCamera->getViewport()) {
                            osg::Viewport* viewport = masterCamera->getViewport();
                            masterCameraVPW *= viewport->computeWindowMatrix();
                          }
                        }

                        // If this camera is not a slave camera
                        if (camera->getView()->getCamera() == camera) {
                          eventState->setGraphicsContext(gw);
                          eventState->setInputRange( viewport->x(), viewport->y(), 
                                                     viewport->x()+viewport->width(),
                                                     viewport->y()+viewport->height());
                      
                        }
                        else {
                          eventState->setInputRange(-1.0, -1.0, 1.0, 1.0);
                        }
                      }
                    }
                  }
                }
            
                break;
              }
              default:
                break;
              }
        
            if (pointerEvent) {
              if (getCameraWithFocus()) {
                osg::Viewport* viewport = getCameraWithFocus()->getViewport();
                osg::Matrix localCameraVPW = getCameraWithFocus()->getViewMatrix() * getCameraWithFocus()->getProjectionMatrix();
                if (viewport) localCameraVPW *= viewport->computeWindowMatrix();
            
                osg::Matrix matrix( osg::Matrix::inverse(localCameraVPW) * masterCameraVPW );
            
                osg::Vec3d new_coord = osg::Vec3d(x,y,0.0) * matrix;
            
                x = new_coord.x();
                y = new_coord.y();                                
            
                event->setInputRange(eventState->getXmin(), eventState->getYmin(), eventState->getXmax(), eventState->getYmax());
                event->setX(x);
                event->setY(y);
                event->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);
            
              }
              // pass along the new pointer events details to the eventState of the viewer 
              eventState->setX(x);
              eventState->setY(y);
              eventState->setButtonMask(event->getButtonMask());
              eventState->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);
          
            }
            else {
              event->setInputRange(eventState->getXmin(), eventState->getYmin(), eventState->getXmax(), eventState->getYmax());
              event->setX(eventState->getX());
              event->setY(eventState->getY());
              event->setButtonMask(eventState->getButtonMask());
              event->setMouseYOrientation(eventState->getMouseYOrientation());
            }
          }
      
          for(itr = gw_events.begin(); itr != gw_events.end(); ++itr) {
            osgGA::GUIEventAdapter* event = itr->get();
            switch(event->getEventType())
              {
              case(osgGA::GUIEventAdapter::CLOSE_WINDOW): {
                bool wasThreading = areThreadsRunning();
                if (wasThreading) stopThreading();
            
                gw->close();
            
                if (wasThreading) startThreading();
            
                break;
              }
              default:
                break;
              }
          }
      
          viewEventsMap[masterView].insert( viewEventsMap[masterView].end(), gw_events.begin(), gw_events.end() );
      
        }
      }
  
  
      // osg::notify(osg::NOTICE)<<"mouseEventState Xmin = "<<eventState->getXmin()<<" Ymin="<<eventState->getYmin()<<" xMax="<<eventState->getXmax()<<" Ymax="<<eventState->getYmax()<<std::endl;


      for(RefViews::iterator vitr = _views.begin(); vitr!=_views.end(); ++vitr) {
        View* view = vitr->get();
        view->getEventQueue()->frame( getFrameStamp()->getReferenceTime() );
        view->getEventQueue()->takeEvents(viewEventsMap[view]);
      }
    
  
      // osg::notify(osg::NOTICE)<<"Events "<<events.size()<<std::endl;
  
      if ((_keyEventSetsDone!=0) || _quitEventSetsDone) {
        for(ViewEventsMap::iterator veitr = viewEventsMap.begin();
            veitr != viewEventsMap.end();
            ++veitr) {
          for(osgGA::EventQueue::Events::iterator itr = veitr->second.begin();
              itr != veitr->second.end();
              ++itr) {
            osgGA::GUIEventAdapter* event = itr->get();
            switch(event->getEventType())
              {
              case(osgGA::GUIEventAdapter::KEYUP):
                if (_keyEventSetsDone && event->getKey()==_keyEventSetsDone) _done = true;
                break;
            
              case(osgGA::GUIEventAdapter::QUIT_APPLICATION):
                guiEventHandler->quitEvent(0);
                if (_quitEventSetsDone) _done = true;
                break;
            
              default:
                break;
              }
          }
        }
      }
  
      if (_done) return;
  
      if (_eventVisitor.valid()) {
        _eventVisitor->setFrameStamp(getFrameStamp());
        _eventVisitor->setTraversalNumber(getFrameStamp()->getFrameNumber());
    
        for(ViewEventsMap::iterator veitr = viewEventsMap.begin();
            veitr != viewEventsMap.end(); ++veitr) {
          View* view = veitr->first;
          _eventVisitor->setActionAdapter(view);
      
          if (view->getSceneData()) {            
            for(osgGA::EventQueue::Events::iterator itr = veitr->second.begin();
                itr != veitr->second.end();
                ++itr) {
              osgGA::GUIEventAdapter* event = itr->get();
          
              _eventVisitor->reset();
              _eventVisitor->addEvent( event );
          
              view->getSceneData()->accept(*_eventVisitor);
          
              // call any camera update callbacks, but only traverse that callback, don't traverse its subgraph
              // leave that to the scene update traversal.
              osg::NodeVisitor::TraversalMode tm = _eventVisitor->getTraversalMode();
              _eventVisitor->setTraversalMode(osg::NodeVisitor::TRAVERSE_NONE);
          
              if (view->getCamera() && view->getCamera()->getEventCallback()) view->getCamera()->accept(*_eventVisitor);
          
              for(unsigned int i=0; i<view->getNumSlaves(); ++i) {
                osg::Camera* camera = view->getSlave(i)._camera.get();
                if (camera && camera->getEventCallback()) camera->accept(*_eventVisitor);
              }
          
              _eventVisitor->setTraversalMode(tm);
          
            }
          }
        }
    
      }
  
      for(ViewEventsMap::iterator veitr = viewEventsMap.begin();
          veitr != viewEventsMap.end(); ++veitr) {
        View* view = veitr->first;
    
        for(osgGA::EventQueue::Events::iterator itr = veitr->second.begin();
            itr != veitr->second.end(); ++itr) {
          osgGA::GUIEventAdapter* event = itr->get();
      
          for(View::EventHandlers::iterator hitr = view->getEventHandlers().begin();
              hitr != view->getEventHandlers().end(); ++hitr) {
            (*hitr)->handleWithCheckAgainstIgnoreHandledEventsMask( *event, *view, 0, 0);
          }
        }
      }
  
      for(ViewEventsMap::iterator veitr = viewEventsMap.begin();
          veitr != viewEventsMap.end(); ++veitr) {
        View* view = veitr->first;
    
        for(osgGA::EventQueue::Events::iterator itr = veitr->second.begin();
            itr != veitr->second.end(); ++itr) {
          osgGA::GUIEventAdapter* event = itr->get();
      
          if (view->getCameraManipulator()) {
            view->getCameraManipulator()->handleWithCheckAgainstIgnoreHandledEventsMask( *event, *view);
          }
        }
      }
  
  
  
      if (getViewerStats() && getViewerStats()->collectStats("event")) {
        double endEventTraversal = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());
    
        // update current frames stats
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(), "Event traversal begin time", beginEventTraversal);
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(), "Event traversal end time", endEventTraversal);
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(), "Event traversal time taken", endEventTraversal-beginEventTraversal);
      } 
    }

  } // end of namespace graphics
} // end of namespace mars
