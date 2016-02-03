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

#include "Dialog_Graphics_Options.h"

#include <mars/main_gui/GuiInterface.h>

#include <mars/interfaces/graphics/GraphicsCameraInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

using namespace std;

namespace mars {
  namespace gui {

    Dialog_Graphics_Options::Dialog_Graphics_Options(interfaces::ControlCenter *c,
                                                     main_gui::GuiInterface *gui) : 
      main_gui::BaseWidget(0, c->cfg, "Dialog_Graphics_Options"),
      pDialog(new main_gui::PropertyDialog(0)), mainGui(gui) {

      control = c;
      pDialog->setAttribute(Qt::WA_DeleteOnClose);
      pDialog->setWindowTitle(tr("Graphics Options"));
      pDialog->setPropCallback(dynamic_cast<PropertyCallback*>(this));
      QObject::connect(pDialog, SIGNAL(closeSignal()), this, SLOT(closeDialog()));

      filled = false;

      gs_backup = control->graphics->getGraphicOptions();

      std::vector<unsigned long> ids;
      control->graphics->getList3DWindowIDs(&ids);
      for (unsigned int i = 0; i < ids.size(); i++) {
        QtVariantProperty *tmp_win = 
          pDialog->addGenericProperty("../3D Window " + QString::number(ids[i]).toStdString(), 
                                      QVariant::Bool, true);
        winIds.push_back(tmp_win);
        cam_backup_struct tmp_cam_backup;
        tmp_cam_backup.g_cam = control->graphics->get3DWindow(ids[i])->getCameraInterface();
        tmp_cam_backup.g_cam->getViewport(&tmp_cam_backup.rx_backup, &tmp_cam_backup.ry_backup, 
                                          &tmp_cam_backup.tx_backup, &tmp_cam_backup.ty_backup,
                                          &tmp_cam_backup.tz_backup, &tmp_cam_backup.rz_backup); 
        tmp_cam_backup.camera_backup = tmp_cam_backup.g_cam->getCamera() - 1;
        cam_backup.push_back(tmp_cam_backup);
      }
  
      coords_backup = control->graphics->coordsVisible();
      grid_backup = control->graphics->gridVisible();
  
      std::map<QString, QVariant> attr;
      attr.insert(pair<QString, QVariant>(QString("decimals"), 9));
      attr.insert(pair<QString, QVariant>(QString("singleStep"), 0.1));

      QStringList enumNames;
      enumNames << "OSG" << "ODE" << "MICHA" << "ISO";
      camera = pDialog->addGenericProperty("../Camera" , QtVariantPropertyManager::enumTypeId(),
                                           0, NULL, &enumNames);
      enumNames.clear();
      enumNames << "Custom" << "Perspective" << "Orthogonal" ;
      cam_type = pDialog->addGenericProperty("../Camera/Type", QtVariantPropertyManager::enumTypeId(),
                                             0, NULL, &enumNames);
      enumNames.clear();
      enumNames << "Custom" << "Left" << "Right" << "Front" << "Rear" << "Top" << "Bottom";
      /*
      viewport = pDialog->addGenericProperty("../Camera/Viewport", QtVariantPropertyManager::enumTypeId(),
                                             0, NULL, &enumNames);
      */
      pos = pDialog->addGenericProperty("../Camera/Viewport/Position", QtVariantPropertyManager::groupTypeId(), 0);
      rot = pDialog->addGenericProperty("../Camera/Viewport/Rotation", QtVariantPropertyManager::groupTypeId(), 0);
  
      pos_x = pDialog->addGenericProperty("../Camera/Viewport/Position/x", QVariant::Double, 0, &attr);
      pos_y = pDialog->addGenericProperty("../Camera/Viewport/Position/y", QVariant::Double, 0, &attr);
      pos_z = pDialog->addGenericProperty("../Camera/Viewport/Position/z", QVariant::Double, 0, &attr);
      rot_x = pDialog->addGenericProperty("../Camera/Viewport/Rotation/x", QVariant::Double, 0, &attr);
      rot_y = pDialog->addGenericProperty("../Camera/Viewport/Rotation/y", QVariant::Double, 0, &attr);
      rot_z = pDialog->addGenericProperty("../Camera/Viewport/Rotation/z", QVariant::Double, 0, &attr);

      attr.clear();
      attr.insert(pair<QString, QVariant>(QString("minimum"), 0));
      attr.insert(pair<QString, QVariant>(QString("singleStep"), 10));
      clearColor = pDialog->addGenericProperty("../Clear Color", QVariant::Color,
                                               to_QColor(gs_backup.clearColor));
      fogEnabled = pDialog->addGenericProperty("../Fog", QVariant::Bool, gs_backup.fogEnabled);
      fogDensity = pDialog->addGenericProperty("../Fog/Density", QVariant::Double, gs_backup.fogDensity, &attr);
      fogStart = pDialog->addGenericProperty("../Fog/Start", QVariant::Double, gs_backup.fogStart);
      fogEnd = pDialog->addGenericProperty("../Fog/End", QVariant::Double, gs_backup.fogEnd);
      fogColor = pDialog->addGenericProperty("../Fog/Color", QVariant::Color,
                                             to_QColor(gs_backup.fogColor));
      /*
      coords = pDialog->addGenericProperty("../Coordinates", QVariant::Bool, coords_backup);
      grid = pDialog->addGenericProperty("../Grid", QVariant::Bool, grid_backup);
      */

      on_change_window(0);
      on_change_fog(fogEnabled->value().toBool());

      filled = true;
    }

    Dialog_Graphics_Options::~Dialog_Graphics_Options() {
    }

    void Dialog_Graphics_Options::update(void) {
      interfaces::GraphicData gs_new;
      gs_new.clearColor = to_my_color(clearColor->value().value<QColor>());
      gs_new.fogEnabled = fogEnabled->value().toBool();
      gs_new.fogStart = fogStart->value().toDouble();
      gs_new.fogEnd = fogEnd->value().toDouble();
      gs_new.fogDensity = fogDensity->value().toDouble();
      gs_new.fogColor = to_my_color(fogColor->value().value<QColor>());
  
      control->graphics->setGraphicOptions(gs_new);
    }

    void Dialog_Graphics_Options::on_change_window(int index) {
      filled = false;
      pos_x->setValue(cam_backup[index].tx_backup);
      pos_y->setValue(cam_backup[index].ty_backup);
      pos_z->setValue(cam_backup[index].tz_backup);
      rot_x->setValue(cam_backup[index].rx_backup);
      rot_y->setValue(cam_backup[index].ry_backup);
      rot_z->setValue(cam_backup[index].rz_backup);
      camera->setValue(cam_backup[index].camera_backup);
      filled = true;
    }

    void Dialog_Graphics_Options::on_change_fog(bool enabled) {
      if (enabled) {
        fogEnabled->addSubProperty(fogDensity);
        fogEnabled->addSubProperty(fogStart);
        fogEnabled->addSubProperty(fogEnd);
        fogEnabled->addSubProperty(fogColor);
      } else {
        fogEnabled->removeSubProperty(fogDensity);
        fogEnabled->removeSubProperty(fogStart);
        fogEnabled->removeSubProperty(fogEnd);
        fogEnabled->removeSubProperty(fogColor);
      }
    }    


    void Dialog_Graphics_Options::on_change_viewport(int index) {
      for (unsigned int i = 0; i < winIds.size(); i++)
        if (winIds[i]->value().toBool() == true) {
          interfaces::GraphicsCameraInterface *g_cam = cam_backup[i].g_cam;
          switch (index) {
          case 1: g_cam->context_setCamPredefLeft(); break;
          case 2: g_cam->context_setCamPredefRight(); break; 
          case 3: g_cam->context_setCamPredefFront(); break; 
          case 4: g_cam->context_setCamPredefRear(); break; 
          case 5: g_cam->context_setCamPredefTop(); break; 
          case 6: g_cam->context_setCamPredefBottom(); break; 
          default: break;
          }
      
          if (index) {
            viewport->removeSubProperty(rot);
            viewport->removeSubProperty(pos);
          } else {
            viewport->addSubProperty(pos);
            viewport->addSubProperty(rot);
            double rx, ry, tx, ty, tz, rz;
            g_cam->getViewport(&rx, &ry, &tx, &ty, &tz, &rz);
            rot_x->setValue(rx); rot_y->setValue(ry); rot_z->setValue(rz);
            pos_x->setValue(tx); pos_y->setValue(ty); pos_z->setValue(tz);
          } 
        }
    }

    void Dialog_Graphics_Options::on_change_camera_type(int index) {
      for (unsigned int i = 0; i < winIds.size(); i++)
        if (winIds[i]->value().toBool() == true) {
          interfaces::GraphicsCameraInterface *g_cam = cam_backup[i].g_cam;
          switch (index) {
          case 1: g_cam->changeCameraTypeToPerspective(); break;
          case 2: g_cam->changeCameraTypeToOrtho(); break;
          default: break;
          }
        }
    }


    void Dialog_Graphics_Options::accept() {
      if (pDialog) pDialog->close();
    }


    void Dialog_Graphics_Options::reject() {
      control->graphics->setGraphicOptions(gs_backup);

      coords_backup ? control->graphics->showCoords() : control->graphics->hideCoords();
      grid_backup ? control->graphics->showGrid() : control->graphics->hideGrid();

      for (unsigned int i = 0; i < winIds.size(); i++) {
        interfaces::GraphicsCameraInterface *g_cam = cam_backup[i].g_cam;
        g_cam->setCamera(cam_backup[i].camera_backup+1);
        g_cam->updateViewport(cam_backup[i].rx_backup, cam_backup[i].ry_backup, 
                              cam_backup[i].tx_backup, cam_backup[i].ty_backup, 
                              cam_backup[i].tz_backup, cam_backup[i].rz_backup, 1); 
      }
  
      if (pDialog) pDialog->close();
    }


    void Dialog_Graphics_Options::valueChanged(QtProperty* property, const QVariant& value) {
  
      if (filled == false)
        return;

      for (unsigned int i = 0; i < winIds.size(); i++)
        if (property == winIds[i]) {
          on_change_window(value.toInt());
          update();
          return;
        }

      if (property == fogEnabled) {
        on_change_fog(value.toBool());
        update();
      }

      else if (property == coords)
        value.toBool() ? control->graphics->showCoords() : control->graphics->hideCoords();

      else if (property == clouds)
        value.toBool() ? control->graphics->showClouds() : control->graphics->hideClouds();

      else if (property == grid)
        value.toBool() ? control->graphics->showGrid() : control->graphics->hideGrid();

      else if (property == camera) {
        for (unsigned int i = 0; i < winIds.size(); i++)
          if (winIds[i]->value().toBool() == true)
            cam_backup[i].g_cam->setCamera(value.toInt()+1);
      }
  
      else if (property == viewport)
        on_change_viewport(value.toInt());

      else if (property == cam_type)
        on_change_camera_type(value.toInt());

      else if (property == rot_x || property == rot_y || property == rot_z ||
               property == pos_x || property == pos_y || property == pos_z) {
        for (unsigned int i = 0; i < winIds.size(); i++)
          if (winIds[i]->value().toBool() == true)
            cam_backup[i].g_cam->updateViewport(rot_x->value().toDouble(), rot_y->value().toDouble(), 
                                                pos_x->value().toDouble(), pos_y->value().toDouble(), 
                                                pos_z->value().toDouble(), rot_z->value().toDouble(), 1);
      }
  
      else 
        update();
    }


    void Dialog_Graphics_Options::closeDialog() {
      if (pDialog) {
        mainGui->removeDockWidget((void*)pDialog);
        delete pDialog;
        pDialog = NULL;
      }
    }


    utils::Color Dialog_Graphics_Options::to_my_color(QColor color)
    {
      utils::Color retval;
      retval.r = color.redF();
      retval.g = color.greenF();
      retval.b = color.blueF();
      retval.a = color.alphaF();
      return retval;
    }

    QColor Dialog_Graphics_Options::to_QColor(utils::Color color)
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
