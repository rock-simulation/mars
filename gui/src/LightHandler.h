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
 * \file "LightHandler.h"
 * \brief Provides the GUI properties for handling a light - creating and editing
 */
#ifndef LIGHT_HANDLER_H
#define LIGHT_HANDLER_H

#ifdef _PRINT_HEADER_
#warning "LightHandler.h"
#endif

#include <mars/main_gui/PropertyDialog.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/LightData.h>

namespace mars {
  namespace gui {

    /**
     * \brief Provides the GUI properties for handling a light - creating and editing
     */
    class LightHandler : public QObject {
      Q_OBJECT
      public:

      LightHandler(QtVariantProperty* property, int ind,
                   main_gui::PropertyDialog *pd, interfaces::ControlCenter *c);
	   
      ~LightHandler();

      void valueChanged(QtProperty *property, const QVariant &value);
      void focusIn();
      void focusOut();

      unsigned int lightIndex();

    private:
      int myLightIndex;
      std::string propName;
      bool filled;
      bool recompileShader;
      interfaces::ControlCenter *control;
      std::vector<struct interfaces::LightData*> allLights;
      struct interfaces::LightData* myLight;

      main_gui::PropertyDialog *pDialog;
  
      QtVariantProperty* topLevel;
      QtVariantProperty* general, *geometry, *orientation, *name, *type;
      QtVariantProperty* pos_x, *pos_y, *pos_z, *look_x, *look_y, *look_z;
      QtVariantProperty* constant, *linear, *quadratic, *exponent;
      QtVariantProperty* ambient, *specular, *diffuse, *directional, *cutoff;

      void fill();
      utils::Color to_my_color(QColor color);
      QColor to_QColor(utils::Color color);
      void update(interfaces::LightData* light);
      void on_switched_type();

    };

  } // end of namespace gui
} // end of namespace mars

#endif
