Supported 3D Formats {#supported_3d_formats}
=======================

MARS is compatible with the following formats:
 - [URDF](http://wiki.ros.org/urdf)
 - [SMURF](http://github.com/rock-simulation/smurf_parser)/[SMURFs](@ref smurfs)
 - [MARS Scenes](@ref mars_scenes)
 - [OBJ](http://en.wikipedia.org/wiki/Wavefront_.obj_file)
 - [STL](http://en.wikipedia.org/wiki/STL_%28file_format%29)


## URDF

The [Unified Robot Description Format](http://wiki.ros.org/urdf) is a format to describe the kinematics of a robot developed by the [ROS](http://www.ros.org/) community. MARS can load a URDF, however this will only lead to a robot's links and joints be created in simulation, but no motors or sensors will be attached.

URDF is therefore best used in MARS to create passive objects.


## SMURF/SMURFs

The Supplementable, Mostly Universal Robot Format is a flexible format to describe the entirety of a robot. It extends URDF - which it maintains as a separate file - by annotating the URDF data in additional [YAML](http://yaml.org) files.

You can find the SMURF format documentation on the [GitHub repository of the SMURF parser](http://github.com/rock-simulation/smurf_parser).


## MARS Scenes

MARS Scenes are the original 3D scene format used with MARS and since we have maintained backwards compatibility, it is still possible to load MARS scenes. If for some reasons it is not possible for you to use a SMURF/URDF representation of a robot with MARS, you can still create MARS scenes with [Blender](http://www.blender.org) using the export scripts and following the [tutorials](@ref tutorials). However, we do not recommend this option, as we will not be working on MARS scenes any more in future development and we may even have to break with backwards compatibility at some point (though we will tag a MARS release before that happens, if it ever does happen).


## OBJ / STL files

.obj and .stl files can be directly imported as meshes in MARS. We do not recommend this functionality unless you know what you're doing, but it can be very useful for debugging purposes.
