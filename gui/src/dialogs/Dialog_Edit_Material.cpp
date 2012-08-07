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

#include "Dialog_Edit_Material.h"

using namespace std;

namespace mars {
  namespace gui {

    Dialog_Edit_Material::Dialog_Edit_Material(main_gui::PropertyDialog* par,
                                               std::string name)
    {
      parent = par;
      std::map<QString, QVariant> attr;
      attr.insert(pair<QString, QVariant>(QString("decimals"), 9));
      attr.insert(pair<QString, QVariant>(QString("singleStep"), 0.1));
  
      top = 
        parent->addGenericProperty("../"+name+"Material ",
                                   QtVariantPropertyManager::groupTypeId(), 
                                   0);

      bothSides = 
        parent->addGenericProperty("../"+name+"Material /Edit both sides", 
                                   QVariant::Bool, true);
      qAmbient = 
        parent->addGenericProperty("../"+name+"Material /Ambient color/Front", 
                                   QVariant::Color, QColor(128,128,128,255));
      qAmbientBack = 
        parent->addGenericProperty("../"+name+"Material /Ambient color/Back", 
                                   QVariant::Color, QColor(128,128,128,255));
      qDiffuse =  
        parent->addGenericProperty("../"+name+"Material /Diffuse color/Front", 
                                   QVariant::Color, QColor(204,204,204,255));
      qDiffuseBack =  
        parent->addGenericProperty("../"+name+"Material /Diffuse color/Back", 
                                   QVariant::Color, QColor(204,204,204,255));  
      qSpecular =  
        parent->addGenericProperty("../"+name+"Material /Specular color/Front",
                                   QVariant::Color, QColor(0,0,0,255));
      qSpecularBack =  
        parent->addGenericProperty("../"+name+"Material /Specular color/Back", 
                                   QVariant::Color, QColor(0,0,0,255));
      qEmission =  
        parent->addGenericProperty("../"+name+"Material /Emission color/Front",
                                   QVariant::Color, QColor(0,0,0,255));
      qEmissionBack =  
        parent->addGenericProperty("../"+name+"Material /Emission color/Back", 
                                   QVariant::Color, QColor(0,0,0,255));
      transparency =  
        parent->addGenericProperty("../"+name+"Material /Transparency",
                                   QVariant::Double, 0.0, &attr);
      shininess =  
        parent->addGenericProperty("../"+name+"Material /Shininess", 
                                   QVariant::Double, 0.0, &attr);
      texture = 
        parent->addGenericProperty("../"+name+"Material /Texture", 
                                   VariantManager::filePathTypeId(), "");
      texture->setAttribute(QString("directory"), QString("."));
      bumpmap = 
        parent->addGenericProperty("../"+name+"Material /Bumpmap", 
                                   VariantManager::filePathTypeId(), "");
      bumpmap->setAttribute(QString("directory"), QString("."));

    }

    /**sets a new MaterialData and initializes it*/
    void Dialog_Edit_Material::setMaterial(interfaces::MaterialData *materialS){

      material = materialS;
  
      utils::Color ambient, ambientback, diffuse, diffuseback, specular, specularback, emission, emissionback;

      //set ambient
      ambient=material->ambientFront;
      ambientback=material->ambientBack;
      //set diffuse
      diffuse=material->diffuseFront;
      diffuseback=material->diffuseBack;
      //set specular
      specular=material->specularFront;
      specularback=material->specularBack;
      //set emission
      emission=material->emissionFront;
      emissionback=material->emissionBack;

      qAmbient->setValue(to_QColor(ambient));
      qDiffuse->setValue(to_QColor(diffuse));
      qSpecular->setValue(to_QColor(specular));
      qEmission->setValue(to_QColor(emission));
      qAmbientBack->setValue(to_QColor(ambientback));
      qDiffuseBack->setValue(to_QColor(diffuseback));
      qSpecularBack->setValue(to_QColor(specularback));
      qEmissionBack->setValue(to_QColor(emissionback));
  
      texture->setValue(QString::fromStdString(material->texturename));
      bumpmap->setValue(QString::fromStdString(material->bumpmap));
    }


    /**
     * sets the material information
     */
    void Dialog_Edit_Material::accept() {

      //set material information
      material->exists=true;

      material->bumpmap = bumpmap->value().toString().toStdString();
      material->texturename = texture->value().toString().toStdString();
      material->transparency = transparency->value().toDouble();
      material->shininess = shininess->value().toDouble();
  
      material->ambientFront = to_my_color(qAmbient->value().value<QColor>());
      material->diffuseFront = to_my_color(qDiffuse->value().value<QColor>());
      material->specularFront = to_my_color(qSpecular->value().value<QColor>());
      material->emissionFront = to_my_color(qEmission->value().value<QColor>());

      material->ambientBack = to_my_color(qAmbientBack->value().value<QColor>());
      material->diffuseBack = to_my_color(qDiffuseBack->value().value<QColor>());
      material->specularBack = to_my_color(qSpecularBack->value().value<QColor>());
      material->emissionBack = to_my_color(qEmissionBack->value().value<QColor>());
    }

    utils::Color Dialog_Edit_Material::to_my_color(QColor color)
    {
      utils::Color retval;
      retval.r = color.redF();
      retval.g = color.greenF();
      retval.b = color.blueF();
      retval.a = color.alphaF();
      return retval;
    }

    QColor Dialog_Edit_Material::to_QColor(utils::Color color)
    {
      QColor retval;
      retval.setRedF(color.r);
      retval.setGreenF(color.g);
      retval.setBlueF(color.b);
      retval.setAlphaF(color.a);
      return retval;
    }

    QtVariantProperty* Dialog_Edit_Material::getTopLevelProperty() {
      return top;
    }

    bool Dialog_Edit_Material::owns(QtProperty* property) {
      if (property == top || property == bothSides || property == transparency ||
          property == shininess || property == qAmbient || property == qDiffuse ||
          property == qSpecular || property == qEmission || property == qAmbientBack ||
          property == qDiffuseBack || property == qSpecularBack || property == qEmissionBack ||
          property == texture || property == bumpmap)
        return true;
      else
        return false;
    }

  } // end of namespace gui
} // end of namespace mars
