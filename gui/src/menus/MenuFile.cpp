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
 * \file MenuFile.h
 * \author Vladimir Komsiyski
 * \brief MenuFile creates the menus and menu items in the File menu of the simulation.
 */

#include "MenuFile.h"

#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/main_gui/GuiInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/core_objects_exchange.h>
#include <lib_manager/LibManager.hpp>
#include <QtGui>

#include <QMessageBox>
#include <QFileDialog>

#include <fstream>

namespace mars {
  namespace gui {

    /**
     * \brief MenuFile creates the menus and menu items in the File menu of the simulation.
     */
    MenuFile::MenuFile(interfaces::ControlCenter *c,
                       main_gui::GuiInterface *gui, std::string resPath,
                       lib_manager::LibManager *theManager)
      : libManager(theManager), mainGui(gui), control(c) {

      std::string tmp1;

      tmp1 = resPath + "/images";
      tmp1.append("/open_scene.png");
      mainGui->addGenericMenuAction("../File/Open", GUI_ACTION_OPEN_SCENE,
                                    (main_gui::MenuInterface*)this, 0, tmp1, true);

      tmp1 = resPath + "/images";
      tmp1.append("/new_scene.png");
      mainGui->addGenericMenuAction("../File/Clear Simulation", GUI_ACTION_NEW_SCENE,
                                    (main_gui::MenuInterface*)this, 0, tmp1, true);


      tmp1 = resPath + "/images";
      tmp1.append("/reset.png");
      mainGui->addGenericMenuAction("../File/Reset Simulation", GUI_ACTION_RESET_SCENE,
                                    (main_gui::MenuInterface*)this, 0, tmp1, true);

      tmp1 = resPath + "/images";
      tmp1.append("/save_scene.png");
      mainGui->addGenericMenuAction("../File/Export/MARS Scene", GUI_ACTION_SAVE_SCENE,
                                    (main_gui::MenuInterface*)this, 0, tmp1, true);


      // add separator
      mainGui->addGenericMenuAction("../File/", 0, NULL, 0, "", 0, -1);

      mainGui->addGenericMenuAction("../File/Export/OSG-OBJ Model", GUI_ACTION_EXPORT_SCENE,
                                    (main_gui::MenuInterface*)this, 0);

      mainGui->addGenericMenuAction("../File/Export/State", GUI_ACTION_EXPORT_STATE,
                                    (main_gui::MenuInterface*)this, 0);
      mainGui->addGenericMenuAction("../File/Import/State", GUI_ACTION_IMPORT_STATE,
                                    (main_gui::MenuInterface*)this, 0);
    }

    MenuFile::~MenuFile() {

    }

    void MenuFile::menuAction(int action, bool checked)
    {
      (void)checked;
      switch (action) {
      case GUI_ACTION_OPEN_SCENE: menu_openSceneFile(); break;
      case GUI_ACTION_SAVE_SCENE: menu_saveSceneFile(); break;
      case GUI_ACTION_RESET_SCENE: menu_resetScene(); break;
      case GUI_ACTION_NEW_SCENE: menu_newScene(); break;
      case GUI_ACTION_EXPORT_SCENE: menu_exportScene(); break;
      case GUI_ACTION_EXPORT_STATE: menu_exportState(); break;
      case GUI_ACTION_IMPORT_STATE: menu_importState(); break;
      }
    }

    void MenuFile::menu_exportScene(void) {
      control->sim->exportScene();
    }

    void MenuFile::menu_exportState(void) {
        std::vector<interfaces::core_objects_exchange> objectList;
        std::vector<interfaces::core_objects_exchange>::iterator iter;
        interfaces::NodeData theNode;
        utils::Quaternion q;
        utils::Vector pos;

        QString fileName = QFileDialog::getSaveFileName(0, "Save State", ".", "MARS State Files (*.state)");

        if (fileName.isEmpty())
            return;

        control->nodes->getListNodes(&objectList);

        FILE *fileHandle = fopen(fileName.toStdString().c_str(), "w");

        for(iter=objectList.begin(); iter!=objectList.end(); ++iter) {
            theNode = control->nodes->getFullNode((*iter).index);
            q = (*iter).rot * theNode.visual_offset_rot;

            //pos = QVRotate((*iter).rot, theNode.visual_offset_pos);
            pos = ((*iter).rot * theNode.visual_offset_pos);
            pos += (*iter).pos;
            fprintf(fileHandle, "n %s %g %g %g %g %g %g %g\n",
                    (*iter).name.c_str(), pos.x(), pos.y(),
                    pos.z(), q.w(), q.x(), q.y(), q.z());
        }
        control->motors->getListMotors(&objectList);
        for(iter=objectList.begin(); iter!=objectList.end(); ++iter) {
            fprintf(fileHandle, "m %s %g\n", (*iter).name.c_str(), (*iter).value);
        }
        fclose(fileHandle);
    }

      void MenuFile::menu_importState(void) {
        QString fileName = QFileDialog::getOpenFileName(0, "Save State", ".", "MARS State Files (*.state)");

        if (fileName.isEmpty())
            return;

        std::ifstream fileHandle(fileName.toStdString());
        std::string line;
        std::string c, name;
        double x, y, z, qx, qy, qz, qw, v;
        interfaces::NodeId id;
        while(std::getline(fileHandle, line))
        {
            if(line[0] == 'n')
            {
                std::istringstream iss(line);
                iss >> c >> name >> x >> y >> z >> qw >> qx >> qy >> qz;
                id = control->nodes->getID(name);
                control->nodes->setPosition(id, utils::Vector(x, y, z));
                control->nodes->setRotation(id, utils::Quaternion(qw, qx, qy, qz));
            }
            else if(line[0] == 'm')
            {
                std::istringstream iss(line);
                iss >> c >> name >> v;
                id = control->motors->getID(name);
                control->motors->setMotorValue(id, v);
            }
        }
    }

    void MenuFile::menu_resetScene(void) {
      control->sim->resetSim();
    }

    void MenuFile::menu_newScene(void) {
      switch (QMessageBox::information( 0, "Simulation", "WARNING:\n"
                                        "All Objects will be deleted!",
                                        "OK", "Cancel", 0, // OK == button 0
                                        1) ) // Cancel == button 1
        {
        case 0: //OK clicked, delete all stuff
          //graphicsFactory->reset();
          control->sim->newWorld(true);
          break;
        case 1: // Cancel clicked, Alt-C or Escape pressed
          // don't exit
          break;
        }
    }


    /**
     * opens a saved file
     */
    void MenuFile::menu_openSceneFile() {
      const char *fileTypes =
          "All supported files (*.scn *.zip *.scene *.smurf *smurfs *.svg *.zsmurf *.zsmurfs *.urdf)"
              ";;MARS zipped scene files (*.scn *.zip)"
              ";;MARS scene files (*.scene)"
              ";;URDF files (*.urdf)"
              ";;SMURF files (*.smurf)"
              ";;zipped SMURF files (*.zsmurf)"
              ";;SMURF scene files (*.smurfs)"
              ";;SMURF svg scene (*.svg)"
              ";;zipped SMURF scene files (*.smurfs)"
              ";;YAML files (*.yaml *.yml)"
              ";;All files (*.*)";
      QString fileName = QFileDialog::getOpenFileName(0, "Open Scene", ".", fileTypes);

      if (fileName.isEmpty())
        return;
      bool wasrunning = false;
      if (control->sim->isSimRunning()) {
        control->sim->startStopTrigger();
        wasrunning = true;
      }
      control->sim->loadScene(fileName.toStdString(), false);
      if(wasrunning) control->sim->startStopTrigger();
    }

    /**
     * saves a scene
     */
    void MenuFile::menu_saveSceneFile() {
      //graphicsFactory->saveScene("scene.obj");
      bool wasRunning = false;
      if (control->sim->isSimRunning()) {
        control->sim->startStopTrigger();
        wasRunning = true;
      }

      QString fileName = QFileDialog::getSaveFileName(0, "Save File",
                                                      "../",
                                                      "MARS Scene Files (*.scn)");
      if (fileName.isEmpty()) {
        std::cout << "File Save: no file selected" << std::endl;
      } else {
        control->sim->saveScene(fileName.toStdString(), wasRunning);
        if (wasRunning) {
          control->sim->startStopTrigger();
        }
      }

    }

  } // end of namespace gui
} // end of namespace mars
