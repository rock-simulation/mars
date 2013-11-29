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


#include "MenuSimulation.h"

#include <mars/main_gui/GuiInterface.h>

#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>

#include <QMessageBox>

namespace mars {
  namespace gui {

    MenuSimulation::MenuSimulation(interfaces::ControlCenter* c,
                                   main_gui::GuiInterface *gui,
                                   std::string resPath) : control(c),
                                                          mainGui(gui) {

      dn = NULL; // nodes
      dj = NULL; // joints
      dl = NULL; // lights
      dm = NULL; // motors
      ds = NULL; // sensors
      dc = NULL; // controllers

      daf = NULL; // dialog add force
      dat = NULL; // dialog apply torque
      dim = NULL; // dialog import mesh 

      dd = NULL; // distance
      nst = NULL; // selection
      djoy = NULL; // joystick
      dmc = NULL; // motor control

      dre = NULL; // dialog rescale environment
      dgo = NULL; // dialog graphics options



      std::string tmp1;
  
      tmp1 = resPath + "/images";
      tmp1.append("/play_button.png");
      mainGui->addGenericMenuAction("../Controls/Start", GUI_ACTION_SIM_START,
                                    (main_gui::MenuInterface*)this,
                                    'R', tmp1, true);

      tmp1 = resPath + "/images";
      tmp1.append("/pause_button.png");
      mainGui->addGenericMenuAction("../Controls/Pause", GUI_ACTION_SIM_PAUSE,
                                    (main_gui::MenuInterface*)this,
                                    'P', tmp1, true);

      tmp1 = resPath + "/images";
      tmp1.append("/step_button.png");
      mainGui->addGenericMenuAction("../Controls/Single Step",
                                    GUI_ACTION_SIM_STEP,
                                    (main_gui::MenuInterface*)this,
                                    'T', tmp1, true);

      // add separator
      mainGui->addGenericMenuAction("../Controls/", 0, NULL, 0, "", 0, -1);  
      mainGui->addGenericMenuAction("../Controls/Joystick", 
                                    GUI_ACTION_JOYSTICK,
                                    (main_gui::MenuInterface*)this, 0);
      mainGui->addGenericMenuAction("../Controls/Object Distance",
                                    GUI_ACTION_DISTANCE,
                                    (main_gui::MenuInterface*)this, 0);
      mainGui->addGenericMenuAction("../Controls/Node Selection",
                                    GUI_ACTION_SELECTION,
                                    (main_gui::MenuInterface*)this, 0);
      mainGui->addGenericMenuAction("../Controls/Motor Control",
                                    GUI_ACTION_MOTOR_CONTROL,
                                    (main_gui::MenuInterface*)this, 0);


      mainGui->addGenericMenuAction("../Simulation/Nodes", GUI_ACTION_NODE_TREE,
                                    (main_gui::MenuInterface*)this, 
                                    QKeySequence("CTRL+N")[0]);
      mainGui->addGenericMenuAction("../Simulation/Joints", GUI_ACTION_JOINT_TREE,
                                    (main_gui::MenuInterface*)this, 
                                    QKeySequence("CTRL+J")[0]);
      mainGui->addGenericMenuAction("../Simulation/Lights", GUI_ACTION_LIGHT_TREE,
                                    (main_gui::MenuInterface*)this, 
                                    QKeySequence("CTRL+L")[0]);
      mainGui->addGenericMenuAction("../Simulation/Motors", GUI_ACTION_MOTOR_TREE,
                                    (main_gui::MenuInterface*)this, 
                                    QKeySequence("CTRL+M")[0]);
      mainGui->addGenericMenuAction("../Simulation/Sensors", GUI_ACTION_SENSOR_TREE,
                                    (main_gui::MenuInterface*)this, 
                                    QKeySequence("CTRL+E")[0]);
      mainGui->addGenericMenuAction("../Simulation/Controllers",
                                    GUI_ACTION_CONTROLLER_TREE,
                                    (main_gui::MenuInterface*)this, 
                                    QKeySequence("CTRL+O")[0]);

      mainGui->addGenericMenuAction("../Simulation/", 0, NULL, 0, "", 0, -1); // separator
      mainGui->addGenericMenuAction("../Simulation/Apply Force",
                                    GUI_ACTION_APPLY_FORCE,
                                    (main_gui::MenuInterface*)this, 0);
      mainGui->addGenericMenuAction("../Simulation/Apply Torque",
                                    GUI_ACTION_APPLY_TORQUE,
                                    (main_gui::MenuInterface*)this, 0);
      mainGui->addGenericMenuAction("../Simulation/Rescale Environment",
                                    GUI_ACTION_RESCALE_ENV,
                                    (main_gui::MenuInterface*) this, 0);

      mainGui->addGenericMenuAction("../Simulation/", 0, NULL, 0, "", 0, -1); // separator
      mainGui->addGenericMenuAction("../Simulation/Import Mesh",
                                    GUI_ACTION_IMPORT_MESH,
                                    (main_gui::MenuInterface*)this, 0);


      mainGui->addGenericMenuAction("../Simulation/", 0, NULL, 0, "", 0, -1); // separator
      mainGui->addGenericMenuAction("../Simulation/Graphics Options",
                                    GUI_ACTION_EDIT_GRAPHICS,
                                    (main_gui::MenuInterface*) this, 0);
  
      if (control->cfg) {
        cfg_manager::cfgPropertyStruct r_path;
        r_path = control->cfg->getOrCreateProperty("MarsGui", "resources_path",
                                                   std::string("."));
        resourcesPath = r_path.sValue;
      }
      resourcesPath.append("/images");
    }


    MenuSimulation::~MenuSimulation() {
    }

    void MenuSimulation::menuAction(int action, bool checked) {
      (void)checked;

      switch (action) {
      case GUI_ACTION_NODE_TREE: menu_nodes(); break;
      case GUI_ACTION_JOINT_TREE: menu_joints(); break;
      case GUI_ACTION_LIGHT_TREE: menu_lights(); break;
      case GUI_ACTION_MOTOR_TREE: menu_motors(); break;
      case GUI_ACTION_SENSOR_TREE: menu_sensors(); break;
      case GUI_ACTION_CONTROLLER_TREE: menu_controllers(); break;
      case GUI_ACTION_DISTANCE: menu_distance(); break;
      case GUI_ACTION_SELECTION: menu_selection(); break;
      case GUI_ACTION_JOYSTICK: menu_joystick();  break;
      case GUI_ACTION_MOTOR_CONTROL:  menu_motorControl(); break;
      case GUI_ACTION_IMPORT_MESH: menu_importMesh(); break;
      case GUI_ACTION_APPLY_FORCE: menu_applyForce(); break;
      case GUI_ACTION_APPLY_TORQUE: menu_applyTorque(); break;
      case GUI_ACTION_EDIT_GRAPHICS: menu_graphicsOptions(); break;
      case GUI_ACTION_RESCALE_ENV: menu_rescaleEnvironment(); break;
      case GUI_ACTION_SIM_START:
        control->sim->StartSimulation();
        /*
        if(!(control->sim->isSimRunning()))
          control->sim->startStopTrigger();
        */
        break;
      case GUI_ACTION_SIM_PAUSE:
        control->sim->StopSimulation();
        /*
        if(control->sim->isSimRunning())
          control->sim->startStopTrigger();
        */
        break;
      case GUI_ACTION_SIM_STEP:
        if(!(control->sim->isSimRunning()))
          control->sim->singleStep();
        break;
      default: break;
      }
    }


    void MenuSimulation::menu_nodes() {
      if (dn != NULL) {
        if (dn->pDialog) {
          dn->pDialog->close();
        }
        delete dn;
        dn = NULL;
      }
      dn = new DialogNodes(control, mainGui);
      mainGui->addDockWidget(dn);
      dn->show();
    }



    void MenuSimulation::menu_joints() {
      if (dj != NULL) {
          dj->close();
      }
      else {
        dj = new DialogJoints(control, mainGui, resourcesPath);
        connect(dj, SIGNAL(closeSignal(void*)),
                this, SLOT(closeWidget(void*)));
        mainGui->addDockWidget(dj);
        dj->show();
      }
    }



    void MenuSimulation::menu_lights() {
      if (dl != NULL) {
        if (dl->pDialog) {
          dl->pDialog->close();
        }
        delete dl;
        dl = NULL;
      }
      dl = new DialogLights(control, mainGui);
      mainGui->addDockWidget(dl->pDialog);
      dl->show();
    }

    void MenuSimulation::menu_motors() {
      if (dm != NULL) {
        if (dm->pDialog) {
          dm->pDialog->close();
        }
        delete dm;
        dm = NULL;
      }
      dm = new DialogMotors(control, mainGui);
      mainGui->addDockWidget(dm->pDialog);
      dm->show();
    }


    void MenuSimulation::menu_sensors() {
      if (ds != NULL) {
        if (ds->pDialog) {
          ds->pDialog->close();
        }
        delete ds;
        ds = NULL;
      }
      ds = new DialogSensors(control, mainGui);
      mainGui->addDockWidget(ds->pDialog);
      ds->show();
    }

    void MenuSimulation::menu_controllers() {
      if (dc != NULL) {
        if (dc->pDialog)
          dc->pDialog->close();
        delete dc;
        dc = NULL;
      }
      dc = new DialogControllers(control, mainGui);
      mainGui->addDockWidget(dc->pDialog);
      dc->show();
    }


    void MenuSimulation::menu_graphicsOptions() {
      //if dialog already exists close it and delete its memory
      if (dgo != NULL) {
        if(dgo->pDialog) 
          dgo->pDialog->close();
        delete dgo;
        dgo = NULL;
      }  
      //create new dialog
      if (control->graphics) {
        dgo = new Dialog_Graphics_Options(control, mainGui);
        mainGui->addDockWidget((void*)dgo->pDialog);
        dgo->show();
      } else {
        QMessageBox::information(0,
                                 "Simulation",
                                 "Graphics are not initialized!",
                                 "OK", 0); // ok == button 0
      }
    }


    void MenuSimulation::menu_rescaleEnvironment(){
      //if dialog already exists close it and delete its memory
      if (dre != NULL) {
        if(dre->pDialog) 
          dre->pDialog->close();
        delete dre;
        dre = NULL;
      }
      if (control->nodes->getNodeCount() > 0) {
        //create new dialog and show it
        dre = new Dialog_Rescale_Environment(control, mainGui);
        mainGui->addDockWidget((void*)dre->pDialog);
        dre->show();
      } else {
        QMessageBox::information( 0, "Simulation",
                                  "Please create a node before starting the environment rescaling",
                                  "OK", 0); // ok == button 0
      }

    }

    void MenuSimulation::menu_motorControl() {
      if (dmc != NULL) {
        dmc->close();
      }
      else {
        if (control->motors->getMotorCount() <= 0) {
          QMessageBox::information(0, "Simulation",
                                   "Create a motor first!", "OK", 0);
          return;
        }

        dmc = new Dialog_Motor_Control(control);
        mainGui->addDockWidget((void*)dmc);
        connect(dmc, SIGNAL(closeSignal(void*)),
                this, SLOT(closeWidget(void*)));
        dmc->show();
      }
    }

    void MenuSimulation::menu_joystick() {
      if (djoy != NULL) {
        djoy->close();
      }
      else {
        djoy = new DialogJoystick(control);
        mainGui->addDockWidget(djoy);
        connect(djoy, SIGNAL(closeSignal(void*)),
                this, SLOT(closeWidget(void*)));
      djoy->show();
      menu_selection();
      djoy->linkSelectionDialog(this->nst);
      }
    }

    void MenuSimulation::menu_distance() {
      if (dd != NULL) {
        dd->close();
      }
      else {
        dd = new DialogDistance(control);
        mainGui->addDockWidget(dd);
        connect(dd, SIGNAL(closeSignal(void*)),
                this, SLOT(closeWidget(void*)));
        dd->show();
      }
    }


    void MenuSimulation::menu_selection() {
      if (nst != NULL) {
        nst->close();
      }
      else {
        nst = new NodeSelectionTree(control);
        mainGui->addDockWidget(nst);
        connect(nst, SIGNAL(closeSignal(void*)),
                this, SLOT(closeWidget(void*)));
        nst->show();
      }
    }


    void MenuSimulation::menu_applyForce() {
      //close and delete existing dialog
      if (daf != NULL) {
        if(daf->pDialog) 
          daf->pDialog->close();
        delete daf;
        daf = NULL;
      }
      if (control->nodes->getNodeCount() > 0) {
        // create new dialog
        daf = new Dialog_Add_Force(control, mainGui);
        mainGui->addDockWidget((void*)daf->pDialog);
        daf->show();
      } else {
        QMessageBox::information(0, "Simulation", "Please create a node first",
                                 "OK", 0); // ok == button 0
      }
    }


    void MenuSimulation::menu_applyTorque() {
      //close and delete existing dialog
      if (dat != NULL) {
        if(dat->pDialog) 
          dat->pDialog->close();
        delete dat;
        dat = NULL;
      }
      if (control->nodes->getNodeCount() > 0) {
        // create new dialog
        dat = new Dialog_Add_Torque(control, mainGui);
        mainGui->addDockWidget((void*)dat->pDialog);
        dat->show();
      } else {
        QMessageBox::information(0, "Simulation", "Please create a node first",
                                 "OK", 0); // ok == button 0
      }
    }

    void MenuSimulation::menu_importMesh() {
      //close and delete existing dialog
      if (dim != NULL) {
        if(dim->pDialog) 
          dim->pDialog->close();
        delete dim;
        dim = NULL;
      }
      //create dialog
      dim = new Dialog_Import_Mesh(control, mainGui);
      mainGui->addDockWidget((void*)dim->pDialog);
      dim->show();
    }

    void MenuSimulation::closeWidget(void* widget) {
      void **toClose = NULL;

      if(widget == dd) toClose = (void**)&dd;
      else if(widget == djoy) toClose = (void**)&djoy;
      else if(widget == nst) toClose = (void**)&nst;
      else if(widget == dmc) toClose = (void**)&dmc;
      else if(widget == dj) toClose = (void**)&dj;

      if(toClose && *toClose) {
        mainGui->removeDockWidget(*toClose);
        delete (QObject*)*toClose;
        *toClose = NULL;
      }

    }

  } // end of namespace gui
} // end of namespace mars
