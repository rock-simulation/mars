Basic Python {#tutorial_basic_python}
============

## Introduction

A first version of using python scripts to control the simulation is
implemented with the ´PythonMars´ plugin. It allows to read sensors
and write motor valus via python. This way it is possible to change the
controller while the simulation is running. Additionally, text can be
rendered onto the 3D window and lines and pointclouds can be drawn into
the simulation.

## Prerequisites

We assume you already have a MARS development environment setup in
´MARS_DEV_ROOT´. Within the documentation we use ´MARS_DEV_ROOT´ as alias to
the folder where you installed MARS. You should also have a simulation scene
or SMURF file to load into the simulation.
Open a terminal and source your environment:

    cd MARS_DEV_ROOT
    . env.sh

## Install

To install the ´PythonMars´ plugin via pybob use:

    bob-install simulation/mars/plugins/PythonMars

To use the plugin you can create your own configuration folder. Just copy
the "mars_default" configuration to "mars_python", rename your example
additional lib configuration file "other_libs.txt.example" into
"other_libs.txt" and add ´PythonMars´ to the list in the file. Additionally,
you have to rename the "core_libs.txt.example" into "core_libs.txt", too.
This will cause your plugin to be loaded upon startup of MARS if  ´mars_app´
is started from within that configuration folder. Alternatively, you can start
´mars_app´ from anywhere and pass the configuration folder:
´mars_app -C path_to_configuration´.

    cd MARS_DEV_ROOT/install/configuration
    cp -r mars_default mars_python
    cd mars_python
    cp other_libs.txt.example other_libs.txt
    cp core_libs.txt.example core_libs.txt
    echo "PythonMars" >> other_libs.txt

Content of other_libs.txt:

    ...
    connexion_plugin
    data_broker_gui
    cfg_manager_gui
    lib_manager_gui
    PythonMars

The plugin needs an additional configuration file giving the path from where
to load the ´mars_plugin.py´ file.

    echo "pypath: [python]" > pypath.yml

Content of pypath.yml:

    pypath: [python]

Then we create the folder ´python´ and create the mars_plugin.py:

    mkdir python
    touch python/mars_plugin.py

Add the following code to the file:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.py}
from mars_interface import *

def init():
    clearDict()
    requestSensor("position")
    requestSensor("laser")
    setConfig("Graphics", "showCoords", 0)
    setConfig("Scene", "skydome_enabled", 1)
    setConfig("Simulator", "calc_ms", 20)
    setUpdateTime(40)
    setConfig("Robo", "behavior", 0)
    requestConfig("Robo", "behavior")

    clearLines("debug")
    appendLines("debug", 0., 0., 0.2)
    appendLines("debug", 0., 0., 0.2)
    configureLines("debug", 3.0, 0, 1, 0)
    return sendDict()

def update(marsData):
    clearDict()
    distance = [0, 0, 0, 0, 0, 0, 0, 0]
    x = 0.;
    y = 0.;
    if "Sensors" in marsData:
        if "laser" in marsData["Sensors"]:
            if len(marsData["Sensors"]["laser"]) == 8:
                for i in range(8):
                    distance[i] = marsData["Sensors"]["laser"][i]
        if "position" in marsData["Sensors"]:
            x = marsData["Sensors"]["position"][0]
            y = marsData["Sensors"]["position"][1]

    if distance[3] < 1.0 or distance[0] < 0.4:
      setMotor("motor_right", 12.0)
    elif distance[0] > 0.7:
      setMotor("motor_right", 4.8)
    else:
      setMotor("motor_right", 5.)
    setMotor("motor_left", 5.)
    if timing(1):
        logMessage("behavior: " + str(marsData["Config"]["Robo"]["behavior"]))

    clearLines("debug")
    appendLines("debug", x, y, 0.2)
    appendLines("debug", 0., 0., 0.2)
    
    return sendDict()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Start the simulation while loading the scene:

    mars_app -s robo.scn
