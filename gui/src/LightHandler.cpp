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

#include "LightHandler.h"

#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

using namespace std;

namespace mars {
  namespace gui {

    static int local_index = 0;

    LightHandler::LightHandler(QtVariantProperty *property, int ind,
                               main_gui::PropertyDialog *pd,
                               interfaces::ControlCenter *c) {
  
      control = c;
      topLevel = property;
      recompileShader = false;
      myLightIndex = ind;
      pDialog = pd;
      filled = false;
      if (myLightIndex < 0) {
        interfaces::LightData light;
        light.pos.z() = 3.00;
        light.lookAt.z() = -1.00;
        light.name = "NewLight" + QString::number(local_index).toStdString();
        light.constantAttenuation = 1.0000;
        light.quadraticAttenuation = 0.00002;
        light.angle = 180;
        light.directional = false;
        light.ambient = to_my_color(QColor(50,50,50,255));
        light.diffuse = to_my_color(QColor(255,255,255,255));
        light.specular = to_my_color(QColor(255,255,255,255));
        control->graphics->addLight(light);
        vector<interfaces::LightData*> tmp;
        control->graphics->getLights(&tmp);
        for (unsigned int i = 0; i <tmp.size(); i++)
          if (tmp[i]->name == "NewLight" + QString::number(local_index).toStdString()) {
            myLight = tmp[i];
            break;
          }
        local_index++;
      } else {
        vector<interfaces::LightData*> tmp;
        control->graphics->getLights(&tmp);
        for (unsigned int i = 0; i <tmp.size(); i++)
          if (tmp[i]->index == myLightIndex) {
            myLight = tmp[i];
            break;
          }
      }
  
      topLevel->setPropertyName(QString::number(myLight->index) + ":" + QString::fromStdString(myLight->name));
      propName = topLevel->propertyName().toStdString();
      fill();  
      on_switched_type();
      filled = true;
      control->graphics->updateLight(myLight->index);
    }



  
    LightHandler::~LightHandler() {
  
      pDialog->removeGenericProperty(topLevel);
      //delete topLevelNode;
    }


    void LightHandler::fill() {
      std::map<QString, QVariant> attr;
      attr.insert(pair<QString, QVariant>(QString("decimals"), 9));
      attr.insert(pair<QString, QVariant>(QString("singleStep"), 0.1));
      QStringList enumNames;
      enumNames << "Omnilight" << "Spotlight";
  
      name = pDialog->addGenericProperty("../" +propName+  "/Name", QVariant::String, 
                                         QString::fromStdString(myLight->name));
      type = pDialog->addGenericProperty("../" + propName+"/Type", QtVariantPropertyManager::enumTypeId(),
                                         myLight->type-1, NULL, &enumNames);
      geometry = pDialog->addGenericProperty("../"+propName+"/Geometry", QtVariantPropertyManager::groupTypeId(), 0);
      pos_x = pDialog->addGenericProperty("../"+propName+"/Geometry/Position/x", 
                                          QVariant::Double, myLight->pos.x(), &attr);
      pos_y = pDialog->addGenericProperty("../"+propName+"/Geometry/Position/y", 
                                          QVariant::Double, myLight->pos.y(), &attr);
      pos_z = pDialog->addGenericProperty("../"+propName+"/Geometry/Position/z", 
                                          QVariant::Double, myLight->pos.z(), &attr);
      orientation = pDialog->addGenericProperty("../"+propName+"/Geometry/LookAt", 
                                                QtVariantPropertyManager::groupTypeId(), 0);
      look_x = pDialog->addGenericProperty("../"+propName+"/Geometry/LookAt/x", 
                                           QVariant::Double, myLight->lookAt.x(), &attr);
      look_y = pDialog->addGenericProperty("../"+propName+"/Geometry/LookAt/y", 
                                           QVariant::Double, myLight->lookAt.y(), &attr);
      look_z = pDialog->addGenericProperty("../"+propName+"/Geometry/LookAt/z",
                                           QVariant::Double, myLight->lookAt.z(), &attr);
      constant = pDialog->addGenericProperty("../"+propName+"/Attenuation/Constant", 
                                             QVariant::Double, myLight->constantAttenuation, &attr);
      linear = pDialog->addGenericProperty("../"+propName+"/Attenuation/Linear", 
                                           QVariant::Double, myLight->linearAttenuation, &attr);
      quadratic = pDialog->addGenericProperty("../"+propName+"/Attenuation/Quadratic", 
                                              QVariant::Double, myLight->quadraticAttenuation, &attr);
      ambient = pDialog->addGenericProperty("../"+propName+"/Colors/Ambient", 
                                            QVariant::Color, to_QColor(myLight->ambient));
      diffuse = pDialog->addGenericProperty("../"+propName+"/Colors/Diffuse", 
                                            QVariant::Color, to_QColor(myLight->diffuse));
      specular = pDialog->addGenericProperty("../"+propName+"/Colors/Specular", 
                                             QVariant::Color, to_QColor(myLight->specular));
      cutoff = pDialog->addGenericProperty("../"+propName+"/Cutoff", 
                                           QVariant::Double, myLight->angle, &attr);
      exponent = pDialog->addGenericProperty("../"+propName+"/Exponent", 
                                             QVariant::Double, myLight->exponent, &attr);
      directional = pDialog->addGenericProperty("../"+propName+"/Directional", 
                                                QVariant::Bool, myLight->directional);  
  
    }



    void LightHandler::valueChanged(QtProperty *property, const QVariant &value) {
      if (filled == false) return;

      if (property == name)
        topLevel->setPropertyName(QString::number(myLight->index) + ":" + value.toString());

      if (property == type) {
        recompileShader = true;
        on_switched_type();
      } 

      update(myLight);
      control->graphics->updateLight(myLight->index, recompileShader);
      recompileShader = false;
    }



    void LightHandler::focusIn() {

    }
    void LightHandler::focusOut() {

    }

    unsigned int LightHandler::lightIndex() {
      return myLight->index;
    }



    void LightHandler::update(interfaces::LightData* light) {
      light->pos.x() = pos_x->value().toDouble();
      light->pos.y() = pos_y->value().toDouble();
      light->pos.z() = pos_z->value().toDouble();
      light->lookAt.x() = look_x->value().toDouble();
      light->lookAt.y() = look_y->value().toDouble();
      light->lookAt.z() = look_z->value().toDouble();
      light->linearAttenuation = linear->value().toDouble();
      light->constantAttenuation = constant->value().toDouble();
      light->quadraticAttenuation = quadratic->value().toDouble();
      light->type = type->value().toInt() + 1;
      light->exponent = exponent->value().toDouble();
      light->angle = cutoff->value().toDouble();
      if(light->directional != directional->value().toBool()) recompileShader = true;
      light->directional = directional->value().toBool();
      light->ambient = to_my_color(ambient->value().value<QColor>());
      light->diffuse = to_my_color(diffuse->value().value<QColor>());
      light->specular = to_my_color(specular->value().value<QColor>());
      light->name = name->value().toString().toStdString();
    }

    void LightHandler::on_switched_type() {
  
      if (type->value().toInt()+1== interfaces::SPOTLIGHT){
        geometry->addSubProperty(orientation);
        topLevel->addSubProperty(cutoff);
        topLevel->addSubProperty(exponent);
        //topLevel->addSubProperty(directional);
      }
      else{
        geometry->removeSubProperty(orientation);
        topLevel->removeSubProperty(cutoff);
        topLevel->removeSubProperty(exponent);
        //topLevel->removeSubProperty(directional);
      }
    }



    utils::Color LightHandler::to_my_color(QColor color) {
      utils::Color retval;
      retval.r = color.redF();
      retval.g = color.greenF();
      retval.b = color.blueF();
      retval.a = color.alphaF();
      return retval;
    }


    QColor LightHandler::to_QColor(utils::Color color)
    {
      QColor retval;
      retval.setRedF(color.r);
      retval.setGreenF(color.g);
      retval.setBlueF(color.b);
      retval.setAlphaF(color.a);
      return retval;
    }

  } // end of namespace gui
} // end of namespace mars
