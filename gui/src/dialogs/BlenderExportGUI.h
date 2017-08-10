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
 * \file BlenderExportGUI.h
 * \author Malte Römmermann
 * \brief "BlenderExportGUI" is a template for the widget interface of the MARS GUI
 **/

#ifndef BLENDER_EXPORT_GUI_H
#define BLENDER_EXPORT_GUI_H

#ifdef _PRINT_HEADER_
#warning "BlenderExportGUI.h"
#endif

#include <mars/main_gui/BaseWidget.h>
#include <mars/main_gui/PropertyDialog.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/data_broker/ReceiverInterface.h>

namespace mars {

  namespace main_gui {
    class GuiInterface;
  }

  namespace gui {

    class BlenderExportGUI : public main_gui::BaseWidget,
                             public main_gui::PropertyCallback,
                             public data_broker::ReceiverInterface {
      Q_OBJECT

      public:
      BlenderExportGUI(interfaces::ControlCenter *c, main_gui::GuiInterface *gui);
      ~BlenderExportGUI();

      virtual void writeGenericData(unsigned long id, void *data) const;

      main_gui::PropertyDialog *pDialog;
      void receiveData(const mars::data_broker::DataInfo &info,
                       const mars::data_broker::DataPackage &data_package,
                       int callbackParam);

    public slots:
      void startButton(void);
      void stopButton(void);
      void resetButton(void);

   signals:
      void closeSignal(void* widget);

    private:
      interfaces::ControlCenter *control;
      main_gui::GuiInterface *mainGui;
      mutable unsigned long frame;
      int state;
      FILE *fileHandle;
      unsigned long generic_id;
      std::string rPath;

      QtVariantProperty *status;

      QtVariantProperty *filename;

      void closeEvent(QCloseEvent *event);

    protected slots:
      virtual void valueChanged(QtProperty *property, const QVariant &value);
    };

  } // end of namespace gui
} // end of namespace mars

#endif /* BLENDER_EXPORT_GUI_H */
