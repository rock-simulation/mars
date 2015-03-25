Supported File Formats {#supported_file_formats}
=======================

MARS is compatible with the following formats:
 - [URDF](http://wiki.ros.org/urdf)
 - [SMURF](http://github.com/rock-simulation/smurf_parser)
 - [SMURF Scenes (SMURFS)](@ref smurfs)
 - [SMURF SVG Scenes (SMURFS SVG)](@ref smurfs_svg)
 - [MARS Scenes](@ref mars_scenes)
 - [OBJ](http://en.wikipedia.org/wiki/Wavefront_.obj_file)
 - [STL](http://en.wikipedia.org/wiki/STL_%28file_format%29)


## URDF

The [Unified Robot Description Format](http://wiki.ros.org/urdf) is a format to describe the kinematics of a robot developed by the [ROS](http://www.ros.org/) community. MARS can load a URDF, however this will only lead to a robot's links and joints be created in simulation, but no motors or sensors will be attached.

URDF is therefore best used in MARS to create passive objects.


## SMURF

The Supplementable, Mostly Universal Robot Format is a flexible format to describe the entirety of a robot. It extends URDF - which it maintains as a separate file - by annotating the URDF data in additional [YAML](http://yaml.org) files.

You can find the SMURF format documentation on the [GitHub repository of the SMURF parser](http://github.com/rock-simulation/smurf_parser).

## SMURF Scenes (SMURFS)

[SMURF scenes](@ref smurfs) are a scene format based on SMURF.

## SMURF SVG Scenes (SMURFS SVG)

[SMURF SVG scenes](@ref smurfs_svg) use SVG files to position entities defined in YAML.

## MARS Scenes

[MARS Scenes](@ref mars_scenes) are the original 3D scene format used with MARS. They are deprecated, but MARS is still compatible with them (March 2015).


## OBJ / STL files

.obj and .stl files can be directly imported as meshes in MARS. We do not recommend this functionality unless you know what you're doing, but it can be very useful for debugging purposes.
