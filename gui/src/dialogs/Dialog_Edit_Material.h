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

#ifndef DIALOG_EDIT_MATERIAL_H
#define DIALOG_EDIT_MATERIAL_H

#ifdef _PRINT_HEADER_
#warning "Dialog_Edit_Material.h"
#endif

#include <mars/interfaces/MaterialData.h>
#include <mars/main_gui/PropertyDialog.h>


namespace mars {
  namespace gui {

    /*
     * \brief Creates the "Create Material" Dialog window.
     */
    class Dialog_Edit_Material : public QObject {
      Q_OBJECT
  
      public:
      /** \brief Creates the Dialog */
      Dialog_Edit_Material(main_gui::PropertyDialog *p, 
                           std::string name = "");
	
      /** \brief sets the material struct*/
      void setMaterial(interfaces::MaterialData *materialS);
      bool owns(QtProperty *property);
  
      QtVariantProperty* getTopLevelProperty();

    public slots:
      /**
       * sets the material settings to the given struct
       */
      void accept();
  
    private:
      utils::Color to_my_color(QColor color);
      QColor to_QColor(utils::Color color);

      interfaces::MaterialData *material;

      QtVariantProperty *top;
      QtVariantProperty* bothSides;
      QtVariantProperty* transparency;
      QtVariantProperty* shininess;
      QtVariantProperty* qAmbient;
      QtVariantProperty* qDiffuse;
      QtVariantProperty* qSpecular;
      QtVariantProperty* qEmission;
      QtVariantProperty* qAmbientBack;
      QtVariantProperty* qDiffuseBack;
      QtVariantProperty* qSpecularBack;
      QtVariantProperty* qEmissionBack;
      QtVariantProperty* texture;
      QtVariantProperty* bumpmap;
 
      main_gui::PropertyDialog* parent;

    };

  } // end of namespace gui
} // end of namespace mars

#endif
