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

#include "CameraConfiguratorGUI.h"
#include <iostream>

#include <mars/interfaces/sim/NodeManagerInterface.h>

using namespace std;

namespace mars {
  namespace gui {

    CameraConfiguratorGUI::CameraConfiguratorGUI(interfaces::ControlCenter* c) :
      main_gui::BaseWidget(0, c->cfg, "CameraConfiguratorGUI"),
      pDialog(new main_gui::PropertyDialog(NULL)) , set_frustum(false) {
      control = c;
      pDialog->setAttribute(Qt::WA_DeleteOnClose);
      camera = 0;
      first_camera = true;
      take_events = false;
      filled = false;
  
      pDialog->setWindowTitle(tr("Camera Configuration"));
      pDialog->setPropCallback(this);
  
      QStringList enumNames;
      std::map<QString, QVariant> attr;
      attr.insert(pair<QString, QVariant>(QString::fromStdString("singleStep"), 0.1));
      attr.insert(pair<QString, QVariant>(QString::fromStdString("minimum"), -360.0));
      attr.insert(pair<QString, QVariant>(QString::fromStdString("maximum"), 360.0));
      attr.insert(pair<QString, QVariant>(QString::fromStdString("decimals"), 6));

      nodeElem = new NodeElem;
      nodeElem->id = 0;
      nodeElem->name = std::string("");
      nodeElem->index = 0;
      nodes.push_back(nodeElem);

      state = pDialog->addGenericProperty("../State", QVariant::String, "Camera Free!");
      //state->setEnabled(false);
      winIDCombo = pDialog->addGenericProperty("../Window", QtVariantPropertyManager::enumTypeId(),
                                               QVariant(0), NULL, &enumNames);
      enumNames << "None selected";
      nodeIDCombo = pDialog->addGenericProperty("../Node", QtVariantPropertyManager::enumTypeId(),
                                                QVariant(0), NULL, &enumNames);
      lockPos = pDialog->addGenericProperty("../Lock Position", QVariant::Bool, false);
      xPosBox = pDialog->addGenericProperty("../Lock Position/Offset X", QVariant::Double, 0.0, &attr);
      yPosBox = pDialog->addGenericProperty("../Lock Position/Offset Y", QVariant::Double, 0.0, &attr);
      zPosBox = pDialog->addGenericProperty("../Lock Position/Offset Z", QVariant::Double, 0.0, &attr);
      //xPosBox->setEnabled(false); yPosBox->setEnabled(false); zPosBox->setEnabled(false);
      lockRot = pDialog->addGenericProperty("../Lock Rotation", QVariant::Bool, false);
      xRotBox = pDialog->addGenericProperty("../Lock Rotation/Offset Alpha", QVariant::Double, 0.0, &attr);
      yRotBox = pDialog->addGenericProperty("../Lock Rotation/Offset Beta",  QVariant::Double, 0.0, &attr);
      zRotBox = pDialog->addGenericProperty("../Lock Rotation/Offset Gamma", QVariant::Double, 0.0, &attr);
      //xRotBox->setEnabled(false); yRotBox->setEnabled(false); zRotBox->setEnabled(false);
      frustum = pDialog->addGenericProperty("../Frustum Settings", QVariant::Bool, false);
      frt_left   = pDialog->addGenericProperty("../Frustum Settings/Left",   QVariant::Double, 0.0, &attr);
      frt_right  = pDialog->addGenericProperty("../Frustum Settings/Right",  QVariant::Double, 0.0, &attr);
      frt_top    = pDialog->addGenericProperty("../Frustum Settings/Top",    QVariant::Double, 0.0, &attr);
      frt_bottom = pDialog->addGenericProperty("../Frustum Settings/Bottom", QVariant::Double, 0.0, &attr);
      frt_near   = pDialog->addGenericProperty("../Frustum Settings/Near",   QVariant::Double, 0.0, &attr);
      frt_far    = pDialog->addGenericProperty("../Frustum Settings/Far",    QVariant::Double, 0.0, &attr);
      //frt_left->setEnabled(false); frt_right->setEnabled(false); frt_top->setEnabled(false);
      //frt_bottom->setEnabled(false); frt_near->setEnabled(false); frt_far->setEnabled(false);
      save_config = pDialog->addGenericProperty("../Save Configuration", VariantManager::filePathTypeId(), "");
      load_config = pDialog->addGenericProperty("../Load Configuration", VariantManager::filePathTypeId(), "");
  
      pDialog->clearButtonBox();
      pDialog->addGenericButton("Save", this, SLOT(saveConfig()));
      pDialog->addGenericButton("Load", this, SLOT(loadConfig()));

      startTimer(500);
      take_events = true;
      filled = true;
    }

    CameraConfiguratorGUI::~CameraConfiguratorGUI(void) {
    }

    void CameraConfiguratorGUI::accept() {}

    void CameraConfiguratorGUI::reject() {}

    void CameraConfiguratorGUI::valueChanged(QtProperty *property, const QVariant &value)
    {
      if (filled == false) return;
 
      if (property == winIDCombo)
        cameraSelected(value.toInt());
      else if (property == nodeIDCombo) 
        checkStateChanged(lockPos->value().toBool());
      else if (property == lockRot)
        checkRotationChanged(value.toBool());
      else if (property == lockPos) 
        checkStateChanged(value.toBool());
      else if (property == xPosBox || property == yPosBox || property == zPosBox)
        setOffsetPos();
      else if (property == xRotBox || property == yRotBox || property == zRotBox) 
        setOffsetRot();
      else if (property == frustum)
        checkFrustumChanged(value.toBool());
      else if (property == frt_left || property == frt_right || property == frt_top || 
               property == frt_bottom || property == frt_near || property == frt_far)
        if(set_frustum)
          setFrustum();
    
    
      (void)value;
    }

    void CameraConfiguratorGUI::cameraSelected(int index) {
      camera = index;
      updateGUI();
    }
  
    void CameraConfiguratorGUI::checkStateChanged(bool checked) {
      bool da = checked;
      if (take_events) {
        nodeMutex.lock();  
        cameras[camera]->setLockID(nodes[nodeIDCombo->value().toInt()]->id);
        nodeMutex.unlock();
        da = cameras[camera]->setLockCamera(checked);
      }
      if(da) 
        state->setValue(tr("Camera Locked!"));
      else 
        state->setValue(tr("Camera Free!"));
      //xPosBox->setEnabled(checked); yPosBox->setEnabled(checked); zPosBox->setEnabled(checked);
    } 

    void CameraConfiguratorGUI::checkRotationChanged(bool checked) {
      if (take_events) {
        cameras[camera]->setLockRotation(checked);
        //xRotBox->setEnabled(checked); yRotBox->setEnabled(checked); zRotBox->setEnabled(checked);
      }
    }

    void CameraConfiguratorGUI::setOffsetPos(void) {
      cameras[camera]->setOffsetPos(utils::Vector(xPosBox->value().toDouble(),
                                                 yPosBox->value().toDouble(),
                                                 zPosBox->value().toDouble()));
    }


    void CameraConfiguratorGUI::setOffsetRot(void) {
      utils::sRotation rot = (utils::sRotation) {xRotBox->value().toDouble(), 
                                               yRotBox->value().toDouble(), 
                                               zRotBox->value().toDouble()};
      cameras[camera]->setOffsetRot(rot);
    }

    void CameraConfiguratorGUI::checkFrustumChanged(bool checked) {
      set_frustum = checked;

      updateGUI();
    }


    void CameraConfiguratorGUI::setFrustum(void) {
      std::vector<double> frustum;
      frustum.push_back(frt_left->value().toDouble());
      frustum.push_back(frt_right->value().toDouble());
      frustum.push_back(frt_top->value().toDouble());
      frustum.push_back(frt_bottom->value().toDouble());
      frustum.push_back(frt_near->value().toDouble());
      frustum.push_back(frt_far->value().toDouble());
      cameras[camera]->setFrustum(frustum);
    }


    void CameraConfiguratorGUI::saveConfig() {
      QString filename = save_config->value().toString();
      if (!filename.isEmpty()) {
        cameras[camera]->saveConfig(filename.toStdString());
        state->setValue("Configuration Saved!");
      }
      else 
        state->setValue("No file specified!");
    }

    void CameraConfiguratorGUI::loadConfig() {
      QString filename = load_config->value().toString();
      if (!filename.isEmpty()) {
        cameras[camera]->loadConfig(filename.toStdString());
        state->setValue("Configuration Loaded!");
      }
      else 
        state->setValue("No file specified!");
    }



    void CameraConfiguratorGUI::updateGUI(void) {
      if(camera < cameras.size() && take_events) {

        if (frustum->value().toBool()) {
          std::vector<double> frust = cameras[camera]->getFrustumSettings();
          updateFRTBoxes(frust);
        } else {
          std::vector<double> dummy;
          updateFRTBoxes(dummy);
        }

        take_events = false;
        const utils::Vector* pos = cameras[camera]->getOffsetPos();
        xPosBox->setValue(pos->x());
        yPosBox->setValue(pos->y());
        zPosBox->setValue(pos->z());

        const utils::sRotation* rot = cameras[camera]->getOffsetRot();
        xRotBox->setValue(rot->alpha);
        yRotBox->setValue(rot->beta);
        zRotBox->setValue(rot->gamma);
  
        lockPos->setValue(cameras[camera]->getLockCamera());
        lockRot->setValue(cameras[camera]->getLockRotation());
  
        unsigned long id = cameras[camera]->getLockID();

        nodeMutex.lock();
        std::vector<NodeElem*>::iterator iter;
    
        for(iter=nodes.begin(); iter!=nodes.end(); ++iter) {
          if((*iter)->id == id) {
            nodeIDCombo->setValue((*iter)->index);
            take_events = true;
            nodeMutex.unlock();
            return;
          }
        }
        nodeIDCombo->setValue(0);
        nodeMutex.unlock();

        take_events = true;
      }
    }

    void CameraConfiguratorGUI::updateFRTBoxes(std::vector<double> frustum) {

      boxmutex.lock();

      if (frustum.size()) {
        frt_left->setValue(frustum.at(0));
        frt_right->setValue(frustum.at(1));
        frt_top->setValue(frustum.at(2));
        frt_bottom->setValue(frustum.at(3));
        frt_near->setValue(frustum.at(4));
        frt_far->setValue(frustum.at(5));
      } else {
        frt_left->setValue(0);
        frt_right->setValue(0);
        frt_top->setValue(0);
        frt_bottom->setValue(0);
        frt_near->setValue(0);
        frt_far->setValue(0);
      }
      for(int i = 0; i < (int)frustum.size(); i++) {
        fprintf(stderr, "Index#%i is %f\n", i, frustum.at(i));
      }

      boxmutex.unlock();
    }

    void CameraConfiguratorGUI::addCamera(CameraConfig* camera) {
      QStringList enumNames = winIDCombo->attributeValue("enumNames").toStringList();
      char text[255];
      //generate name for label
      sprintf(text, "WinID: %lu", camera->getWindowID());
      // insert into select box
      enumNames << QString(text);
      winIDCombo->setAttribute("enumNames", enumNames);
      // and push_back to list
      cameras.push_back(camera);
      //fprintf(stderr, "\n%s\n", text);
      if(first_camera) {
        timerEvent(0);
        updateGUI();
        first_camera = false;
      }
    }

    void CameraConfiguratorGUI::removeCamera(CameraConfig* camera) {
      unsigned int i = 0;
      std::vector<CameraConfig*>::iterator iter;
      QStringList enumNames = winIDCombo->attributeValue("enumNames").toStringList();
      bool found;

      // first find the correct index
      for(iter=cameras.begin(); iter!=cameras.end(); ++iter, ++i)
        if((*iter) == camera) break;

      if(iter!=cameras.end()) {
        // we have the camera in the list and can remove it
        cameras.erase(iter);
        // select box remove i
        found = false;
        if(this->camera == i) {
          found = true;
          this->camera = 0;
        }
        enumNames.removeAt(i);
        winIDCombo->setAttribute("enumNames", enumNames);
        if(found) winIDCombo->setValue(this->camera);
      }
    }

    void CameraConfiguratorGUI::timerEvent(QTimerEvent* event) {
      (void) event;
      std::vector<interfaces::core_objects_exchange> objectList;
      std::vector<interfaces::core_objects_exchange>::iterator iter;
      std::vector<NodeElem*>::iterator jter;
      std::vector<NodeElem*> addList;
      std::vector<NodeElem*> deleteList;
      NodeElem* newElem;
      unsigned long current_id = nodes[nodeIDCombo->value().toInt()]->id;
      int index = 1, switch_id = 0, count_delete = 0;
      bool found, first = 1;
      QStringList enumNames = nodeIDCombo->attributeValue("enumNames").toStringList();

      addList.push_back(nodeElem);

      control->nodes->getListNodes(&objectList);

      // first we delete the nodes that are not in the list
      for(jter=nodes.begin(); jter!=nodes.end(); ++jter) {
        if(first) first = 0;
        else {
          found = false;
          for(iter=objectList.begin(); iter!=objectList.end(); ++iter) {
            if((*jter)->id == (*iter).index) {
              found = true;
              (*jter)->index = index++;
              addList.push_back((*jter));
              objectList.erase(iter);          
              break;
            }
          }
          if(!found) {
            if((*jter)->id == current_id) switch_id = 1;
            // wir brauchen eine delete list weil wir von hinten l�schen m�ssen
            enumNames.removeAt((*jter)->index - count_delete++);
            nodeIDCombo->setAttribute("enumNames", enumNames);
            delete (*jter);
          }
        }
      }

      // then we add the nodes that are still in the object list
      for(iter=objectList.begin(); iter!=objectList.end(); ++iter) {
        newElem = new NodeElem;
        newElem->id = (*iter).index;
        newElem->name = (*iter).name;
        newElem->index = index++;
        addList.push_back(newElem);
        enumNames << QString::fromStdString((*iter).name);
        nodeIDCombo->setAttribute("enumNames", enumNames);
      }
      objectList.clear();

      nodeMutex.lock();
      nodes = addList;
      if(switch_id) nodeIDCombo->setValue(0);
      nodeMutex.unlock();
    }

  } // end of namespace gui
} // end of namespace mars
