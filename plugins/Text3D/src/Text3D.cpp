/*
 *  Copyright 2014, DFKI GmbH Robotics Innovation Center
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
 * \file Text3D.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */


#include "Text3D.h"
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/data_broker/DataPackage.h>

#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

namespace mars {
  namespace plugins {
    namespace Text3D {

      using namespace mars::utils;
      using namespace mars::interfaces;

      Text3D::Text3D(lib_manager::LibManager *theManager)
        : MarsPluginTemplate(theManager, "Text3D"), maskId(0) {
      }

      void Text3D::init() {

        // load the mask id form file
        FILE *f = fopen("./id.txt", "r");
        if(f) {
          fscanf(f, "%d", &maskId);
          fclose(f);
          maskId = 1 << (maskId - 1);
        }

        // Register for node information:
        /*
          std::string groupName, dataName;
          control->nodes->getDataBrokerNames(id, &groupName, &dataName);
          control->dataBroker->registerTimedReceiver(this, groupName, dataName, "mars_sim/simTimer", 10, 0);
        */

        /* get or create cfg_param
           example = control->cfg->getOrCreateProperty("plugin", "example",
           0.0, this);
        */
        textFactory = libManager->getLibraryAs<osg_text::TextFactoryInterface>("osg_text_factory");
        if(textFactory) {
          mars::utils::ConfigMap map;
          map = mars::utils::ConfigMap::fromYamlFile("Text3DConfig.yml");
          mars::utils::ConfigVector::iterator it;
          osg_text::TextInterface *text;
          if(map.find("Labels") != map.end()) {
            for(it = map["Labels"].begin(); it!=map["Labels"].end(); ++it) {
              TextData *td = new TextData;
              td->name = it->children["name"][0].getString();
              td->value = it->children["value"][0].getString();
              td->posX = it->children["posX"][0].getDouble();
              td->posY = it->children["posY"][0].getDouble();
              double fixedWidth = it->children["fixedWidth"][0].getDouble();
              double fixedHeight = it->children["fixedHeight"][0].getDouble();
              bool drawFrame = it->children["frame"][0].getBool();
              td->text = textFactory->createText(td->value, 40,
                                                osg_text::Color(1.0, 1.0, 1.0, 1.0),
                                                td->posX, td->posY);
              if(drawFrame) {
                td->text->setBackgroundColor(osg_text::Color(0.0, 0.5, 0.0, 0.5));
                td->text->setBorderWidth(4.0);
              }
              else {
                td->text->setBackgroundColor(osg_text::Color(0.0, 0.0, 0.0, 0.0));
                td->text->setBorderWidth(0.0);
              }
              td->text->setBorderColor(osg_text::Color(1.0, 1.0, 1.0, 0.5));

              td->text->setPadding(10., 10., 10., 10.);
              td->text->setFixedWidth(fixedWidth);
              td->text->setFixedHeight(fixedHeight);
              example = control->cfg->getOrCreateProperty("Text3D",
                                                          td->name+"/value",
                                                          td->value, this);
              td->vId = example.paramId;
              example = control->cfg->getOrCreateProperty("Text3D",
                                                          td->name+"/posX",
                                                          td->posX, this);
              td->pxId = example.paramId;
              example = control->cfg->getOrCreateProperty("Text3D",
                                                          td->name+"/posY",
                                                          td->posY, this);
              td->pyId = example.paramId;
              int mask = it->children["mask"][0].getInt();
              example = control->cfg->getOrCreateProperty("Text3D",
                                                          td->name+"/mask",
                                                          mask, this);
              td->maskId = example.paramId;

              td->hudID = control->graphics->addHUDOSGNode(td->text->getOSGNode());
              td->vis = mask & maskId;
              if(!td->vis) {
                control->graphics->switchHUDElementVis(td->hudID);
              }

              textMap[td->vId] = td;
              textMap[td->pxId] = td;
              textMap[td->pyId] = td;
              textMap[td->maskId] = td;
            }
          }
        }
        else {
          fprintf(stderr, "can not load osg_text_factory\n");
        }
      }

      void Text3D::reset() {
      }

      Text3D::~Text3D() {
      }


      void Text3D::update(sReal time_ms) {
      }

      void Text3D::receiveData(const data_broker::DataInfo& info,
                               const data_broker::DataPackage& package,
                               int id) {
        // package.get("force1/x", force);
      }

      void Text3D::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {
        std::map<cfg_manager::cfgParamId, TextData*>::iterator it;

        if((it = textMap.find(_property.paramId)) != textMap.end()) {
          if(it->second->vId == _property.paramId) {
            it->second->text->setText(_property.sValue);
          }
          else if(it->second->pxId == _property.paramId) {
            it->second->posX = _property.dValue;
            it->second->text->setPosition(it->second->posX, it->second->posY);
          }
          else if(it->second->pyId == _property.paramId) {
            it->second->posY = _property.dValue;
            it->second->text->setPosition(it->second->posX, it->second->posY);
          }
          else if(it->second->maskId == _property.paramId) {
            int vis = maskId & _property.iValue;
            if(vis && !it->second->vis) {
              it->second->vis = true;
              control->graphics->switchHUDElementVis(it->second->hudID);
            }
            else if(!vis && it->second->vis) {
              it->second->vis = false;
              control->graphics->switchHUDElementVis(it->second->hudID);
            }
          }

        }
      }

    } // end of namespace Text3D
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::Text3D::Text3D);
CREATE_LIB(mars::plugins::Text3D::Text3D);
