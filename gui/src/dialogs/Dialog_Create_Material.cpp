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

#include "Dialog_Create_Material.h"

using namespace std;

namespace mars {
  namespace gui {

    Dialog_Create_Material::Dialog_Create_Material(main_gui::PropertyDialog* p,
                                                   std::string name )
    {
      parent = p;
      std::map<QString, QVariant> attr;
      attr.insert(pair<QString, QVariant>(QString("decimals"), 9));
      attr.insert(pair<QString, QVariant>(QString("singleStep"), 0.1));
      top = 
        parent->addGenericProperty("../"+name+"Material",
                                   QtVariantPropertyManager::groupTypeId(), 
                                   0);

      bothSides = 
        parent->addGenericProperty("../"+name+"Material/Edit both sides", 
                                   QVariant::Bool, true);
      qAmbient = 
        parent->addGenericProperty("../"+name+"Material/Ambient color/Front", 
                                   QVariant::Color, QColor(128,128,128,255));
      qAmbientBack = 
        parent->addGenericProperty("../"+name+"Material/Ambient color/Back", 
                                   QVariant::Color, QColor(128,128,128,255));
      qDiffuse =  
        parent->addGenericProperty("../"+name+"Material/Diffuse color/Front", 
                                   QVariant::Color, QColor(204,204,204,255));
      qDiffuseBack =  
        parent->addGenericProperty("../"+name+"Material/Diffuse color/Back", 
                                   QVariant::Color, QColor(204,204,204,255));  
      qSpecular =  
        parent->addGenericProperty("../"+name+"Material/Specular color/Front",
                                   QVariant::Color, QColor(0,0,0,255));
      qSpecularBack =  
        parent->addGenericProperty("../"+name+"Material/Specular color/Back", 
                                   QVariant::Color, QColor(0,0,0,255));
      qEmission =  
        parent->addGenericProperty("../"+name+"Material/Emission color/Front",
                                   QVariant::Color, QColor(0,0,0,255));
      qEmissionBack =  
        parent->addGenericProperty("../"+name+"Material/Emission color/Back", 
                                   QVariant::Color, QColor(0,0,0,255));
      transparency =  
        parent->addGenericProperty("../"+name+"Material/Transparency",
                                   QVariant::Double, 0.0, &attr);
      shininess =  
        parent->addGenericProperty("../"+name+"Material/Shininess", 
                                   QVariant::Double, 0.0, &attr);
      texture = 
        parent->addGenericProperty("../"+name+"Material/Texture", 
                                   VariantManager::filePathTypeId(), "");
      texture->setAttribute(QString("directory"), QString("."));
      bumpmap = 
        parent->addGenericProperty("../"+name+"Material/Bumpmap", 
                                   VariantManager::filePathTypeId(), "");
      bumpmap->setAttribute(QString("directory"), QString("."));
    }

    /**sets a new MaterialData and initializes it*/
    void Dialog_Create_Material::setMaterial(interfaces::MaterialData *materialS){
  
      material = materialS;
  
      utils::Color ambient, ambientback, diffuse, diffuseback, specular, specularback, emission, emissionback;
      //set ambient
      ambient.r=0.5;
      ambient.g=0.5;
      ambient.b=0.5;
      ambient.a=1.0;
      ambientback.r=0.5;
      ambientback.g=0.5;
      ambientback.b=0.5;
      ambientback.a=1.0;  
      //set diffuse
      diffuse.r=0.8;
      diffuse.g=0.8;
      diffuse.b=0.8;
      diffuse.a=1.0;
      diffuseback.r=0.8;
      diffuseback.g=0.8;
      diffuseback.b=0.8;
      diffuseback.a=1.0; 
      //set specular
      specular.r=0.0;
      specular.g=0.0;
      specular.b=0.0;
      specular.a=1.0;
      specularback.r=0.0;
      specularback.g=0.0;
      specularback.b=0.0;
      specularback.a=1.0;
      //set emission
      emission.r=0.0;
      emission.g=0.0;
      emission.b=0.0;
      emission.a=1.0;
      emissionback.r=0.0;
      emissionback.g=0.0;
      emissionback.b=0.0;
      emissionback.a=1.0;
  
      //initialize standard values
      material->transparency = 0.0;
      material->shininess = 0.0;
      material->ambientFront = ambient;
      material->diffuseFront = diffuse;
      material->specularFront = specular;
      material->emissionFront = emission;
      material->ambientBack = ambient;
      material->diffuseBack = diffuse;
      material->specularBack = specular;
      material->emissionBack = emission;
      material->texturename = "";
      material->bumpmap = "";
      material->reflect = 0;
    }


    /**
     * sets the material information
     */
    void Dialog_Create_Material::accept() {
  
      //set material information
      material->exists=true;
  
      material->texturename = texture->value().toString().toStdString();
      material->bumpmap = bumpmap->value().toString().toStdString();
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

    utils::Color Dialog_Create_Material::to_my_color(QColor color)
    {
      utils::Color retval;
      retval.r = color.redF();
      retval.g = color.greenF();
      retval.b = color.blueF();
      retval.a = color.alphaF();
      return retval;
    }


    QtVariantProperty* Dialog_Create_Material::getTopLevelProperty() {
      return top;
    }

  } // end of namespace gui
} // end of namespace mars
