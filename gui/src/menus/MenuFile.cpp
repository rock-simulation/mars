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
#include <mars/lib_manager/LibManager.h>
#include <QtGui>

namespace mars {
  namespace gui {

    /**
     * \brief MenuFile creates the menus and menu items in the File menu of the simulation.
     */
    MenuFile::MenuFile(interfaces::ControlCenter *c,
                       main_gui::GuiInterface *gui, std::string resPath,
                       lib_manager::LibManager *theManager) :
      control(c), mainGui(gui), libManager(theManager) {
  
      std::string tmp1;

      tmp1 = resPath + "/images";
      tmp1.append("/new_scene.png");
      mainGui->addGenericMenuAction("../File/New Scene", GUI_ACTION_NEW_SCENE,
                                    (main_gui::MenuInterface*)this, 0, tmp1, true);

      tmp1 = resPath + "/images";
      tmp1.append("/open_scene.png");
      mainGui->addGenericMenuAction("../File/Open Scene", GUI_ACTION_OPEN_SCENE,
                                    (main_gui::MenuInterface*)this, 0, tmp1, true);

      tmp1 = resPath + "/images";
      tmp1.append("/save_scene.png");
      mainGui->addGenericMenuAction("../File/Save Scene", GUI_ACTION_SAVE_SCENE,
                                    (main_gui::MenuInterface*)this, 0, tmp1, true);

      tmp1 = resPath + "/images";
      tmp1.append("/reset.png");
      mainGui->addGenericMenuAction("../File/Reset Scene", GUI_ACTION_RESET_SCENE,
                                    (main_gui::MenuInterface*)this, 0, tmp1, true);

      // add separator
      mainGui->addGenericMenuAction("../File/", 0, NULL, 0, "", 0, -1);

      mainGui->addGenericMenuAction("../File/Export Scene", GUI_ACTION_EXPORT_SCENE, 
                                    (main_gui::MenuInterface*)this, 0);

      mainGui->addGenericMenuAction("../File/Load Plugin", GUI_ACTION_OPEN_PLUGIN,
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
      case GUI_ACTION_OPEN_PLUGIN: menu_openPlugin(); break;
      }
    }

    void MenuFile::menu_exportScene(void) {
      control->sim->exportScene();
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
      const char *fileTypes = "All supported files (*.scn *.zip *.scene)"
        ";;MARS zipped scene files (*.scn *.zip)"
        ";;MARS scene files (*.scene)"
        ";;All files (*.*)";
      QString fileName = QFileDialog::getOpenFileName(0, "Open Scene", ".", 
                                                      fileTypes);

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

    void MenuFile::menu_openPlugin() {
      bool wasRunning = false;
      if (control->sim->isSimRunning()) {
        control->sim->startStopTrigger();
        wasRunning = true;
      }
      QFileDialog *fd = new QFileDialog(0);
      //fd->setWindowTitle(tr("Select plugin to load"));
      fd->setDirectory(QString("."));
      fd->setAcceptMode(QFileDialog::AcceptOpen);
      fd->setFileMode(QFileDialog::ExistingFile);
      //  fd->setFilter("Plugin Files (*.mars)");
#ifdef WIN32
      fd->setFilter("Shared Object (*.dll)");
      fd->setDefaultSuffix("dll");
#else
#ifdef __APPLE__
      fd->setFilter("Shared Object (*.dylib)");
      fd->setDefaultSuffix("dylib");
#else
      fd->setFilter("Shared Object (*.so)");
      fd->setDefaultSuffix("so");
#endif
#endif
      fd->setViewMode(QFileDialog::Detail);
      fd->setLabelText(QFileDialog::Accept, (QString)"Load Plugin");
      fd->setLabelText(QFileDialog::Reject, (QString)"Cancel");
      QStringList fileNames;
      if (fd->exec()) {
        fileNames = fd->selectedFiles();
      }
      if (!fileNames.isEmpty()) {
        std::cout << "File Open: " << fileNames[0].toStdString() << std::endl;
        libManager->loadLibrary(fileNames[0].toStdString());
        delete fd;
      } else {
        if (wasRunning) {
          control->sim->startStopTrigger();
          delete fd;
        }
      }
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
