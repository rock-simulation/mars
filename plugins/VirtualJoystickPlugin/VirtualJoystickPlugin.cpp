 /**
 * \file VirtualJoystickPlugin.cpp
 * \author Michael Rohn
 * \brief A Plugin to map a virtual JoyStick to motors in the sim
 */

#include "VirtualJoystickPlugin.h"

VirtualJoystickPlugin::VirtualJoystickPlugin(lib_manager::LibManager *theManager)
  : QObject(),
    lib_manager::LibInterface(theManager) {

  leftValue  = 0.0;
  rightValue = 0.0;
  joystick = NULL;
  dataPackageId = 0;
  gui = libManager->getLibraryAs<mars::main_gui::GuiInterface>(std::string("main_gui"));
  // add options to the menu
  if(gui)
    gui->addGenericMenuAction("../Plugins/Joystick", 1, this, 0);

  dataBroker = libManager->getLibraryAs<mars::data_broker::DataBrokerInterface>(std::string("data_broker"));
  dataPackage.add("values/x", 0.0);
  dataPackage.add("values/y", 0.0);

  cfg = libManager->getLibraryAs<mars::cfg_manager::CFGManagerInterface>(std::string("cfg_manager"));
  if(cfg) {
    cfg->getOrCreateProperty("VirtualJoystick", "x", leftValue);
    cfg->getOrCreateProperty("VirtualJoystick", "y", rightValue);
  }
}

VirtualJoystickPlugin::~VirtualJoystickPlugin(void) {
  //closeWidget();
  if(gui) libManager->releaseLibrary(std::string("main_gui"));
  if(dataBroker) libManager->releaseLibrary(std::string("data_broker"));
  if(cfg) libManager->releaseLibrary(std::string("cfg_manager"));
}

void VirtualJoystickPlugin::newSpeed(double leftValue, double rightValue) {
  this->leftValue  = leftValue;
  this->rightValue = rightValue;

  if(dataBroker) {
    dataPackage.set(0, leftValue);
    dataPackage.set(1, rightValue);
    if(!dataPackageId) {
      dataPackageId = dataBroker->pushData("joystick", "values", dataPackage,
                                           NULL,
                                           mars::data_broker::DATA_PACKAGE_READ_FLAG);
    }
    else {
      dataBroker->pushData(dataPackageId, dataPackage, NULL);
    }
  }
  if(cfg) {
    cfg->setProperty("VirtualJoystick", "x", leftValue);
    cfg->setProperty("VirtualJoystick", "y", rightValue);
  }
}


void VirtualJoystickPlugin::getNewValue(double* leftValue, double* rightValue) {
  *leftValue  = this->leftValue;
  *rightValue = this->rightValue;
}

void VirtualJoystickPlugin::menuAction(int action, bool checked) {
  (void) checked;

  switch (action) {
  case 1:
    if (!joystick) {
      joystick = new JoystickWidget();
      gui->addDockWidget((void*)joystick);
      joystick->setMaxSpeed(12.0);

      connect(joystick, SIGNAL(newSpeed(double, double)), this, SLOT(newSpeed(double, double)));

      connect(joystick, SIGNAL(hideSignal()), this, SLOT(hideWidget()));
      connect(joystick, SIGNAL(closeSignal()), this, SLOT(closeWidget()));
    }
    else {
      closeWidget();//myWidget->hide();
    }
    break;
  }
}

void VirtualJoystickPlugin::hideWidget(void) {
}

void VirtualJoystickPlugin::closeWidget(void) {
  if (joystick) {
    gui->removeDockWidget((void*)joystick);
    joystick = NULL;
  }
}

DESTROY_LIB(VirtualJoystickPlugin);
CREATE_LIB(VirtualJoystickPlugin);
