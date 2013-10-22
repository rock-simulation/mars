# MARS Simulator

## About

MARS (Machina Arte Robotum Simulans) is a simulation and visualisation tool for developing control algorithms and designing robots. It consists of a core framework containing all main simulation components, a GUI, OpenGL based visualisation and a physics core that is currently based on the [Open Dynamics Engine](http://www.ode.org). The software is designed in a modular manner by making high use of interfaces. In this way many parts of MARS can be also used standalone (e.g. the physic simulation can be used without any visualization or GUI).

## News

## Documentation

* \subpage installation
* \subpage tutorials

## Launch

After MARS is installed you can find a mars_default configuration folder in $MARS_DEV_ROOT/install/configuration. To launch MARS open a Terminal (on Windows an msys shell) and type:

    cd $MARS_DEV_ROOT
    . env.sh
    cd install/configuration/mars_default
    mars_app
    
If it's not clear to you what these commands do, check out the [MARS environment and build tutorial](tutorials/basic/build_tutorial.html).

