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

#include "BlenderExportGUI.h"

#include <mars/main_gui/GuiInterface.h>

#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/core_objects_exchange.h>

#include <QHBoxLayout>

namespace mars {
  namespace gui {

    BlenderExportGUI::BlenderExportGUI(interfaces::ControlCenter* c,
                                       main_gui::GuiInterface* gui) :
      main_gui::BaseWidget(0, c->cfg, "BlenderExportGUI"),
      pDialog(new main_gui::PropertyDialog(NULL)) {

      control = c;
      mainGui = gui;
  
      state = 0;
      frame = 0;
      fileHandle = 0;

      this->setWindowTitle(tr("Blender Export"));

      QHBoxLayout *hLayout = new QHBoxLayout;
      this->setLayout(hLayout);
      hLayout->addWidget(pDialog);

      pDialog->setPropCallback(this);
      pDialog->setViewButtonVisibility(false);

      filename = pDialog->addGenericProperty("../Export to", VariantManager::filePathTypeId(), "");
      filename->setAttribute(QString::fromStdString("filter"), "QDIR"); // for choosing a directory
      filename->setAttribute(QString::fromStdString("directory"), "."); // open in current directory

      status = pDialog->addGenericProperty("../State", QVariant::String, "No file selected!");
      status->setEnabled(false);

      pDialog->clearButtonBox();
      pDialog->addGenericButton("Reset", this, SLOT(resetButton()));
      pDialog->addGenericButton("Start", this, SLOT(startButton()));
      pDialog->addGenericButton("Stop", this, SLOT(stopButton()));  

      // ToDo: register to physic timer update with framerate
    }


    BlenderExportGUI::~BlenderExportGUI(void) {
      delete pDialog;
      if(fileHandle) fclose(fileHandle);
    }

    void BlenderExportGUI::valueChanged(QtProperty* property,
                                        const QVariant& value) {
  
      if (property != filename) return;
      if (value.toString().isEmpty()) return;

      status->setValue("File to export selected!");
      state = 1;
    }

    void BlenderExportGUI::startButton(void) {
      if(state == 1) {
        if(fileHandle) fclose(fileHandle);
        std::string filename_str = filename->value().toString().toStdString() +std::string("/export.dat");
        if ((fileHandle = fopen(filename_str.data(), "w")) == 0) {
          status->setValue("Cannot open file. Aborted.");
          status = 0;
        }
        control->nodes->exportGraphicNodesByID(filename->value().toString().toStdString());
        state = 2;
      }
      if(state == 2) {
        state = 3;
        writeGenericData(0, NULL);
        status->setValue(tr("Exporting started..."));
      }
    }

    void BlenderExportGUI::stopButton(void) {
      if(state == 3) {
        state = 2;
        status->setValue(tr("Exporting stopped."));
      }
    }

    void BlenderExportGUI::resetButton(void) {
      state = 0;
      status->setValue(tr("No file selected!"));
      filename->setValue("");
    }


    void BlenderExportGUI::writeGenericData(unsigned long id, 
                                            void* data) const {
      (void)id;
      (void)data;
      if(state == 3) {
        std::vector<interfaces::core_objects_exchange> objectList;
        std::vector<interfaces::core_objects_exchange>::iterator iter;
        interfaces::NodeData theNode;
        utils::Quaternion q;
        utils::Vector pos;
        control->nodes->getListNodes(&objectList);
    
        fprintf(fileHandle, "frame %lu\n", frame++);

        for(iter=objectList.begin(); iter!=objectList.end(); ++iter) {
          theNode = control->nodes->getFullNode((*iter).index);
          q = (*iter).rot * theNode.visual_offset_rot;

          //pos = QVRotate((*iter).rot, theNode.visual_offset_pos);
          pos = ((*iter).rot * theNode.visual_offset_pos);
          pos += (*iter).pos;
          fprintf(fileHandle, "node%lu %g %g %g %g %g %g %g\n",
                  (*iter).index, pos.x(), pos.y(),
                  pos.z(), q.w(), q.x(), q.y(), q.z());
        }
      }
    }

    void BlenderExportGUI::closeEvent(QCloseEvent *event) {
      (void)event;
      emit closeSignal(this);
    }

  } // end of namespace gui
} // end of namespace mars
