Structural Overview {#structural_overview}
==========================

## Overview

MARS makes use of multiple libraries, most importantly *Open Scene Graph* (OSG) for its visualisation, *Open Dynamics Engine* (ODE) for the physical simulation and *Qt* for the GUI. The following is an attempt to give a rough overview of how these libraries are integrated in MARS and how MARS' own classes work together while the program is running.

Typically, MARS will either be started up with a plugin loading a scene or started up empty with the subsequent manual loading of a scene via the file menu. In both cases, the command

    control->sim->loadScene(fileName)
    
will be executed. To understand what is happening here, we first have to take a look at what \c control and \c sim mean in this context.

\c Control is a pointer to an instance of mars::interfaces::ControlCenter and a member of an instance of mars::sim::Simulator, which is the main class of the simulation. The mars::interfaces::ControlCenter interface provides access to a number of crucial interfaces of the simulation via its members. They are:

* cfg
* nodes
* joints
* motors
* controllers
* sensors
* sim
* graphics
* entities
* dataBroker
* loadCenter

So ironically, the member \c control of the mars::sim::Simulator class provides access to the mars::interfaces::SimulatorInterface the class implements. However, the usual case is that you will access the \c control interface from the plugin you're developing. We will later get back to other typical examples of how to use these interfaces, but let's for now stick with what happens upon execution of the above \c loadScene command.

## Loading / Reloading Scenes

What happens when the \c loadScene method is called depends on whether or not the simulation is running at the time. As MARS uses multithreading, if it is running, it simply pushes the specified file into a list of files to be loaded when appropriate. Eventually, when these requests are being processed, MARS will execute mars::sim::Simulator::loadScene_internal. If MARS is not running, this method will be called directly.

\c loadScene_internal calls the method \c loadFile from the \c loadSceneInterface member interface of the \c loadCenter interface.
