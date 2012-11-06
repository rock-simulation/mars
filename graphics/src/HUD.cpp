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

 /**
 * \file HUD.cpp
 * \author Malte Roemmermann
 * \brief The "HUD" class contains all necessary methods for rendering
 * data into a texture.
 */

#include "HUD.h"

#include <cstdio>

namespace mars {
  namespace graphics {
    
    using mars::utils::Vector;
    using mars::utils::Quaternion;
    using mars::utils::Color;
    using mars::interfaces::sReal;

    HUD::HUD(unsigned long id) {
      this->id = id;
      hudCamera = new osg::Camera();
      hudTerminalList = new osg::Group;
      scaleTransform = new osg::MatrixTransform;
      scaleTransform->setMatrix(osg::Matrix::scale(1.0, 1.0, 1.0));
      scaleTransform->addChild(hudTerminalList.get());
      cull_mask = 0;
    }

    HUD::~HUD(void) {
    }
  
    void HUD::init(osgViewer::GraphicsWindow* gw) {
      swidth = 1.0;
      sheight = 1.0;
      initialize(gw);
    }

    void HUD::init(sReal width, sReal height) {
      this->swidth = width;
      this->sheight = height;
      initialize(0);
    }

    void HUD::initialize(osgViewer::GraphicsWindow* gw) {
      //sReal ratio = 9.0/16.0;//((sReal)myWindow->getTraits()->height /
      // (sReal)myWindow->getTraits()->width);

      //width = myWindow->getTraits()->width*swidth;
      //height = myWindow->getTraits()->height*sheight;
      width = (unsigned int)(1024);//*swidth);
      height = (unsigned int)(1024);//*ratio*sheight);
      hudCamera->setGraphicsContext(gw);
      //hudCamera->setInheritanceMask(0x0); // <-- this seems to be bad
      //hudCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      hudCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
      //hudCamera->setClearMask(0);
      hudCamera->setClearColor(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
      //hudCamera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
      hudCamera->setViewport(0, 0, width/4, height/8);
      //hudCamera->setRenderOrder(osg::Camera::PRE_RENDER);
      hudCamera->setRenderOrder(osg::Camera::POST_RENDER, 10);
#ifdef WIN32
      //hudCamera->setRenderTargetImplementation(osg::Camera::PIXEL_BUFFER);
#else
      //hudCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
#endif
      hudCamera->setProjectionMatrix(osg::Matrix::ortho2D(0,width,0,height));
      hudCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
      hudCamera->setViewMatrix(osg::Matrix::identity());
  
      hudCamera->getOrCreateStateSet()->setMode(GL_LIGHTING,
                                                osg::StateAttribute::OFF |
                                                osg::StateAttribute::OVERRIDE |
                                                osg::StateAttribute::PROTECTED);
      hudCamera->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,
                                                osg::StateAttribute::OFF |
                                                osg::StateAttribute::OVERRIDE |
                                                osg::StateAttribute::PROTECTED);
      hudCamera->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN |
                                                         osg::StateAttribute::OVERRIDE |
                                                         osg::StateAttribute::PROTECTED);
      hudCamera->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON |
                                                osg::StateAttribute::OVERRIDE |
                                                osg::StateAttribute::PROTECTED);
      hudCamera->setAllowEventFocus(false);

      //hudCamera->setRenderer(new osgViewer::Renderer(hudCamera));
      hudCamera->addChild(scaleTransform.get());
    }

    osg::ref_ptr<osg::Texture2D> HUD::getTexture(void) {
      return 0;//hudTexture;
    }

    osg::ref_ptr<osg::Camera> HUD::getCamera(void) {
      return hudCamera;
    }

    void HUD::setViewSize(double width, double height) {
      view_width = width;
      view_height = height;
    }

    void HUD::resize(double width, double height) {
      /*
        std::vector<HUDElement*>::iterator iter;
  
        for(iter=elements.begin(); iter<elements.end(); iter++) {
        (*iter)->resize(width, height);
        }
      */
      double scale_x = width / view_width;
      double scale_y = height / view_height;
      scaleTransform->setMatrix(osg::Matrix::scale(scale_x, scale_y, 1.0));

      hudCamera->setViewport(0, 0, width, height);
      hudCamera->setProjectionMatrix(osg::Matrix::ortho2D(0,width,0,height));
    }

    void HUD::setCullMask(unsigned int cull_mask) {
      hudCamera->setCullMask(cull_mask);
      this->cull_mask = cull_mask;
    }

    void HUD::addHUDElement(HUDElement *elem) {
      elements.push_back(elem);
      hudTerminalList->addChild(elem->getNode());  
    }

    void HUD::removeHUDElement(HUDElement *elem) {
      std::vector<HUDElement*>::iterator iter;
  
      for(iter=elements.begin(); iter<elements.end(); iter++) {
        if((*iter) == elem) {
          elements.erase(iter);
          break;
        }
      }

      hudTerminalList->removeChild(elem->getNode());  
    }

    void HUD::switchCullElement(int key) {
      unsigned int id = 0;

      switch(key) {
      case '1' : id = 0; break;
      case '2' : id = 1; break;
      case '3' : id = 2; break;
      case '4' : id = 3; break;
      case '5' : id = 4; break;
      case '6' : id = 5; break;
      case '7' : id = 6; break;
      case '8' : id = 7; break;
      case '9' : id = 8; break;
      }

      if(id < elements.size()) {
        elements[id]->xorCullMask(cull_mask);
      }
    }

    void HUD::switchElementVis(int num_element) {
      if(num_element <= (int)elements.size() && num_element > 0) {
        elements[num_element-1]->xorCullMask(cull_mask);
      }
    }

  } // end of namespace graphics
} // end of namespace mars
