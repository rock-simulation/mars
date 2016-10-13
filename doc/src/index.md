MARS (Machina Arte Robotum Simulans) is a cross-platform simulation and visualisation tool created for robotics research developed at the [Robotics Innovation Center of the German Research Center for Artificial Intelligence (DFKI-RIC)](http://robotik.dfki-bremen.de/en/startpage.html) and the [University of Bremen](http://www.informatik.uni-bremen.de/robotik/index_en.php). It runs on (Ubuntu) Linux, Mac and Windows and consists of a core framework containing all main simulation components, a GUI (based on [Qt]()), 3D visualization (using [OSG](http://www.openscenegraph.org)) and a physics core (based on [ODE](http://www.ode.org)).  
MARS is designed in a modular manner and can be used very flexibly, e.g. by running the physics simulation without visualization and GUI.
It is possible to extend MARS writing your own plugins and many plugins introducing various functionality such as HUDs or custom ground reaction forces already exist - and it's easy to write your own.

## Mailing Lists

If you have questions about MARS not suited for GitHub issues or would like to receive updates on upcoming releases, you can [subscribe to the *mars users mailing list*](http://www.dfki.de/mailman/cgi-bin/listinfo/mars-users).

## Installation

We don't regularly provide binaries for MARS, thus installing it requires downloading the source and building it locally. To simplify this task, we have created a set of python install scripts that will do all the work of downloading and installing dependencies, cloning the MARS repository and building for you. They have their own repository here on GitHub: [PyBob for MARS installation](https://github.com/rock-simulation/pybob)

## Documentation

You can find MARS' documentation on its [GitHub Page](http://rock-simulation.github.io/mars).

## License

MARS is distributed under the [Lesser GNU Public License (LGPL)](https://www.gnu.org/licenses/lgpl.html).
