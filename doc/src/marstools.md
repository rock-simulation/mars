MARStools Blender ADD-ON {#marstools}
===================

## Overview

The MARStools are an Add-On for Blender which simplify editing robot models and environments to be used in the MARS simulation.

## Installation

The MARStools provide a simple way of installation via the *install.sh* shell script.

### Linux

### OSX

### Windows


## Robot Model

### Joints

    revolute - a hinge joint that rotates along the axis and has a limited range specified by the upper and lower limits.
continuous - a continuous hinge joint that rotates around the axis and has no upper and lower limits
prismatic - a sliding joint that slides along the axis, and has a limited range specified by the upper and lower limits.
fixed - This is not really a joint because it cannot move. All degrees of freedom are locked. This type of joint does not require the axis, calibration, dynamics, limits or safety_controller.
floating - This joint allows motion for all 6 degrees of freedom.
planar - This joint allows motion in a plane perpendicular to the axis. 





## Exporting your model

Model data can be exported with the MARStools in a number of formats: YAML, URDF, SMURF


### URDF

URDF, the "Unified Robot Description Format", is an XML-Format limited to providing information about a robot's kinematics. Other types of information, such as referring to motors, sensors etc., is mostly ignored or limited to specific ROS solutions. While closely affiliated with the ROS framework, URDF is not part of ROS (any more) and the accompanying tools for parsing etc. can be used independently. The complete definition of URDF can be found here: ![http://wiki.ros.org/urdf/XML]. Regarding its core functionality, i.e., defining kinematics, URDF is further limited as it does not allow to specify parallel linkages (joint loops).


## The MARStools Robot Representation

To facilitate simple im- and export of robot models in various formats, the MARStools use a file-format-independent Python dictionary representation to store robot data. This dictionary representation is loosely based on URDF/SDF as well as MARS' and 'Blender's own naming conventions. It is layed out in the following in YAML-notation:

model
- {link}
	- filename: str
	- [pose]: (d, d, d, d, d, d, d)
	- {visual}:
		- {visual_1}
			- [pose]: (d, d, d, d, d, d, d) #x, y, z, w, x, y, z
			- {material}:
				- name: str
				- [diffuseColor]: (d, d, d, d)
				- [ambientColor]: (d, d, d, d)
				- [emissionColor]: (d, d, d, d)
				- [emissionColorcolor]: (d, d, d, d)
				- transparency: d
			- {geometry}:
				- type: str ("box" | "sphere" | "cylinder" | "plane" | "mesh")
				- radius: d #sphere
				- [size]: (d, d, d) #box
				- radius, height: d, d #cylinder
				- [size]: (d, d, d) #mesh
				- [size]: (d, d) #plane
		- {visual_2}
			- ...
		- ...
	- {collision}
		- {collision_1}
			- bitmask: int
			- {geometry}:
				- type: str ("box" | "sphere" | "cylinder" | "plane" | "mesh")
				- radius: d #sphere
				- [size]: (d, d, d) #box
				- radius, height: d, d #cylinder
				- [size]: (d, d, d) #mesh
				- [size]: (d, d) #plane
				- filename #mesh
			- [pose]: (d, d, d, d, d, d, d)
			- max_contacts: int
		- {collision_2}
			- ...
		- ...
	- {inertial}:
		- mass: d
		- [inertia]: (d, d, d, d, d, d) #ixx, ixy, ixz, iyx, iyy, iyz
- {joint}
	- parent: str
	- child: str
	- jointType: str ("hinge", "continuous", "linear")
- ["sensor"]
	- link: str
	- sensorType: str
- ["motor"]
- ["controller"]
- {group}:
	- [group1]: (str, ...) #names of links
	- ...


int: integer
d: double
str: string

## Blender Transformations

== Rotation Matrices ==

In Blender, every object possesses four different transformation matrices to save it current location, rotation and scale in space. These are:

- matrix_basis
- matrix_local
- matrix_world
- matrix_parent_inverse

These matrices make the parent-child relationships possible that objects can be structurally ordered with in Blender. Thus very different behavior can be observed for these matrices depending on whether they belong to a *global* or *child* object. Global objects, i.e. objects which do not have a parent, their location, rotation and scale are fully defined in relation to the world, thus their matrix_basis, matrix_local and matrix_world are all equal. As they possess no parent, the parent inverse matrix is the identity transform. In objects which are children of other objects, i.e., which have a parent, the three matrices serve different functions, as outlined below.

=== matrix_basis ===

matrix_basis is what is displayed in Blender's "Transform" settings in the sidebar. This is often a point of confusion because it does *not* necessarily refer to the object's actual position and orientation in space. For global objects it does.

=== matrix_local ===

The local matrix defines the transformation from the parent world transform to the child transform. This means that if the local matrix of a child is applied to its basis, it would reside at its parent's origin.

=== matrix_world ===

The absolute transform of the object in the world. This is the location, orientation and scale at which the object is displayed in blender.

=== matrix_parent_inverse ===

The parent inverse matrix is set at the time of parenting and *never changes afterwards*, no matter what transformations are applied to the parent or the child (in fact, it doesn't even change after a parent-child relationship is cancelled and is simply ignored by Blender until the object becomes a child again, in which case it is simply overwritten). It is the transformation which, if applied to the child, reverses the change of the origin of the child's coordinate system that resulted from establishing the parent-child relationship. It is thus equal to the inverse of the parent's *world* transform. This last point is important! As the parent could be a child of another object, the local and basis transforms would not reflect the change of origin of the parent, thus the world transform has to be use to derive the parent inverse.

=== Summary ===

- matrix_basis: the object's "own" transform in its own coordinate system
- matrix_local: the transform which brings an object from it's parent's origin to its position in the world
- matrix_world: the absolute transform of the object with respect to the world
- matrix_parent_inverse: the inverse of the parent's world transform at time of parenting


== Location & Transformation ==
