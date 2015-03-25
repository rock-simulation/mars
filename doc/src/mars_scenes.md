MARS Scenes {#mars_scenes}
===========

## Overview

MARS scenes are MARS's original scene format encoded in XML. They have been replaced with the introduction of the [SMURF and SMURF scene](@ref smurfs) formats, but we have maintained backwards compatibility and it is still possible to load MARS scenes.

If for some reasons it is not possible for you to use a SMURF/URDF representation of a robot with MARS, you can still create MARS scenes with [Blender](http://www.blender.org) using the export scripts and following the [tutorials](@ref tutorials). You can even combine existing MARS scenes with the SMURF scenes format.

In general, we do not recommend the use of MARS scenes, as we will not be working on MARS scenes any more in future development and we may even have to break with backwards compatibility at some point (though we will tag a MARS release before that happens, if it ever does happen).

  > **NOTE:** [Phobos](https://github.com/rock-simulation/phobos) has an option to import MARS scenes as of version 0.6.5., so that it should be very little work to transform old robot representations. As of version 0.7, this also includes MARS scenes with multiple "entities", such as a heightmap and a robot.

## XML Definition
MARS scene files possess the extension *.scene* and are XML files with following principal structure:


    <?xml version="1.0"?>
    <!DOCTYPE dfkiMarsSceneFile PUBLIC '-//DFKI/RIC/MARS SceneFile 1.0//EN' ''>
    <SceneFile>
      <version>0.2</version>
      <nodelist>
      <jointlist>
      <motorlist>
      <sensorlist>
      <controllerlist>
      <materiallist>
      <graphicOptions>
    </SceneFile>

You may have noted that the lists of items correspond to the types of objects needed to represent a robot graphically as well as physically. However, MARS scenes can not merely be used for robots, but also to define an environment a robot can operate in. As it is possible to load multiple MARS scenes in the same MARS simulation, it thus makes sense to save robots and environment in different scene files. This makes it a lot easier to edit the files and to prevent errors in one element of a simulation interfering with another part. It also enables the user to test the same robot in various environments without having to re-edit any of the underlying scenes.

When you load multiple scenes into MARS, make sure that they are all in the same scale.

We will have a closer look at the items in these lists in the following.

## Nodes

*Nodes* are all visible objects in MARS, whether they are physical objects or not. Now

--> show notedata.h

    <node name="node_name">
      <origname>
      <filename>
      <index>
      <groupid>
      <physicmode>
      <relativeid>
      <position>
        <x>
        <y>
        <z>
      <rotation>
        <x>
        <y>
        <z>
        <w>
      <extend>
        <x>
        <y>
        <z>
      <pivot>
        <x>
        <y>
        <z>
      <visualsize>
        <x>
        <y>
        <z>
      <movable>
      <mass>
      <density>
      <material_id>
      <coll_bitmask>

| Variable | Description | Possible Values |
|----------|-------------|-----------------|
| origname | name used in Blender to represent the object | any String |
| filename | name of the *.obj or *.bobj file associated with this note | any valid file path |
| index | running index of the nodes | int ≥ 1 |
| groupid | id of the group to which a node belongs | int ≥ 1 |
| physicmode | primitive object by which the node is represented in physics | box, sphere, capsule, cylinder, plane, terrain, mesh  |
| relativeid | ? | ? |
| position | position of the node in x, y and z coordinates | double |
| rotation | rotation of the node in quaternion format | double |
| extend | extend of the representing primitive (see physicmode) | double |
| pivot | ? | ? |
| visualsize | ? | ? |
| movable | whether or not the object is fixed in the world | true / false |
| mass | mass of the object in kg | double |
| density | density of the object in kg/m³ | double  |
| material_id | index of the associated material | int ≥ 1
| coll_bitmask | bitmask to define groups of collision objects | int < 65536 |
| t_srcname | source name for terrain | string |
| t_width | width of terrain | double |
| t_height | height of terrain | double |
| t_scale | scale of terrain | double |
| t_tex_scale | texture scale for terrain |
| t_tex_scale_x | texture scale for terrain |
| t_tex_scale_y | texture scale for terrain |
| visualposition | offset of position of visual representation | vector |
| visualrotation | rotational offset of visual representation | quaternion |
| visualsize | size of visual representation | double |
| cmax_num_contacts | maximum number of contacts in ODE of this node | int |
| cerp |  | double |
| ccfm |  | double |
| cfriction1 |  | double |
| cfriction2 |  | double |
| cmotion1 |  | double |
| cmotion2 |  | double |
| cfds1 |  | double |
| cfds2 |  | double |
| cbounce |  | double |
| cbounce_vel |  | double |
| capprox | use of simplified friction pyramid | bool |
| inertia | whether or not the defined inertias are used | bool |
| i00 |  | double |
| i01 |  | double |
| i02 |  | double |
| i10 |  | double |
| i11 |  | double |
| i12 |  | double |
| i20 |  | double |
| i21 |  | double |
| i22 |  | double |
| linear_damping | damping of linear motion | double |
| angular_damping | damping of angular motion | double |
| angular_low |  | double |
| shadow_id | shadow id object is associated with | int |
| shadowcaster | objects casts shadows | bool |
| shadowreceiver | objects subject to shadows | bool |


## Joints

*Joints* are helper objects defining how *nodes* or groups of nodes are connected with each other.

    <joint name="joint_name">
      <index>
      <type>
      <nodeindex1>
      <nodeindex2>
      <anchorpos>
      <anchor>
        <x>
        <y>
        <z>
      <spring_constant>
      <damping_constant>
      <axis1>
        <x>
        <y>
        <z>
      <angle1_offset>
      <lowStopAxis1>
      <highStopAxis1>
      <damping_const_constraint_axis1>
      <spring_const_constraint_axis1>
      <axis2>
        <x>
        <y>
        <z>
      <angle1_offset>
      <lowStopAxis2>
      <highStopAxis2>
      <damping_const_constraint_axis2>
      <spring_const_constraint_axis2>

| Variable | Description | Possible Values |
|----------|-------------|-----------------|
| index | running index of the *joints* | int ≥ 1 |
| type | type of the joint | hinge, slider |
| nodeindex1 | index of first attached *node* | valid *node* id |
| nodeindex2 | index of second attached *node* | valid *node* id |
| anchorpos | *node* to which the *joint* is anchored | valid *node* id |
| anchor | pos to which the *joint* is anchored | position of *node* id |
| axis1 | axis around / along which joint moves | x, y, z: double |
| angle1_offset | initial offset of the joint | double |
| lowStopAxis1 |  |
| highStopAxis1 |  |
| damping_const_constraint_axis1 |  |
| spring_const_constraint_axis1 |  |
| axis2 |  |
| angle1_offset |  |
| lowStopAxis2 |  |
| highStopAxis2 |  |
| damping_const_constraint_axis2 |   |
| spring_const_constraint_axis2 |  |


## Motors

Motors are associated with *joints* and can apply forces and torques respectively; for a *slider joint*, a force is applied while for a *hinge joint*, *motors* create a torque. As MARS uses the Open Dynamics Engine for its physics simulation, you can have a look at [ODE's documentation](http://www.ode.org/ode-latest-userguide.html) to read further on different [joint and motor types](http://www.ode.org/ode-latest-userguide.html#sec_7_0_0).

As for the decoding in MARS scenes, motors are described as follows:

    <motor name="H.Hip.000">
      <index>
      <jointIndex>
      <jointIndex2>
      <axis>
      <maximumVelocity>
      <motorMaxForce>
      <type>
      <p>
      <i>
      <d>
      <Km>
      <Kn>
      <Ra>
      <Lm>
      <Jm>
      <Rm>
      <U>
      <gear>
      <max_current>
      <r_current>
      <min_val>
      <max_val>
      <value>

| Variable | Description | Possible Values |
| -------- | ----------- | --------------- |
| index | index of the motor |  int ≥ 1 |
| jointIndex | index of the joint with which the motor is associated | int≥ 1 |
| jointIndex2 | ? | int |
| axis | around/along which axis of its associated joint the motor turns/slides | 1 ≤ int ≤ 3 |
| maximumVelocity | maximum velocity with which the motor can move the joint | double |
| motorMaxForce | maximum force the motor can apply | double |
| type | motor type | 1: servo, 2: electric motor |
| p | P value of a PID controller | double |
| i | I value of a PID controller | double |
| d | D value of a PID controller | double |
| Km |  |
| Kn |  |
| Ra |  |
| Lm |  |
| Jm |   |
| Rm |   |
| U |  |
| gear |  |
| max_current |  |
| r_current |  |
| min_val | minimum value (see below) | ? |
| max_val | maximum value (see below) | ? |
| value | ? | ? |


## Sensors

There are a number of different *sensors* in MARS which can be attached to *nodes*, *joints* or *motors* in order to get readings of certain variables. The valid types are:

| Type | Data yielded |
|------|-------------|
| Camera | ? |
| Joint6DOF | ? |
| JointArray | ? |
| JointAVGTorque | ? |
| JointLoad | load of associated joint |
| JointPosition | position of associated joint |
| JointTorque | torque of associated joint |
| JointVelocity | velocity of associated joint |
| MotorCurrent | current of associated motor |
| NodeAngularVelocity | angular velocity of associated node |
| NodeArray | ? |
| NodeCOM | center of mass (COM) of associated node |
| NodeContactForce | contact force of associated node |
| NodePosition | position of associated node |
| NodeRotation | rotation of associated node |
| NodeVelocity | velocity of associated node  |
| RayGrid | collision point cloud of ray grid traced in defined direction |
| Ray | collision point of ray traced in defined direction |
| ScanningSonar | ? |



*Sensors* are defined in a scene file as follows:

    <sensor name="sensorName" type="sensorType">
      <index>
      <rate>
      <dylib_path>
      <id>
      ...
      <id>

| Variable | Description | Possible Values |
| -------- | ----------- | --------------- |
| sensor.name | name of the *sensor*| any valid string |
| sensor.type | type of the *sensor* | any valid *sensor* type with respect to id |
| index | running index of the *sensor* | int ≥ 1 |
| rate | reading rate of *sensor* | double |
| id | id of *node*, *joint* or *motor* | any valid id; can be list of multiple ids |


## Controllers

Controllers ...

    <controller>
      <rate>
      <sensorid>
      <sensorid>
      ...
      <motorid>
      <motorid>
      ...


| Variable | Description | Possible values |
| -------- | ----------- | --------------- |
| rate | operation rate of the *controller* | ? |
| sensorid | running index of an attached *sensor* | int ≥ 1 |
| motorid | running index of an attached *motor* | int ≥ 1 |



## Materials

    <material>
      <id>
      <ambientFront>
        <a>
        <r>
        <g>
        <b>
      <diffuseFront>
        <a>
        <r>
        <g>
        <b>
      <specularFront>
        <a>
        <r>
        <g>
        <b>
      <emissionFront>
        <a>
        <r>
        <g>
        <b>
      <ambientBack>
        <a>
        <r>
        <g>
        <b>
      <diffuseBack>
        <a>
        <r>
        <g>
        <b>
      <specularBack>
        <a>
        <r>
        <g>
        <b>
      <emissionBack>
        <a>
        <r>
        <g>
        <b>
      <transparency>
      <shininess>
      <texturename>
      <reflect>
      <bumpmap>
      <tex_scale>
      <brightness>
      <getLight>
      <cullMask>


| Variable | Description | Possible Values |
| -------- | ----------- | --------------- |
| id | running index of the materials | int ≥ 1 |
| ambientFront | ? | a, r, g, b: 0 ≤ double ≤ 1 |
| diffuseFront | basic color of an object | a, r, g, b: 0 ≤ double ≤ 1 |
| emissionFront | color of light relfected off of object front faces | a, r, g, b: 0 ≤ double ≤ 1 |
| specularFront | color of light reflection on object's front faces | a, r, g, b: 0 ≤ double ≤ 1 |
| ambientBack | ? | a, r, g, b: 0 ≤ double ≤ 1 |
| diffuseBack | basic color of object back faces | a, r, g, b: 0 ≤ double ≤ 1 |
| specularBack | color of light reflection on an object's back faces | a, r, g, b: 0 ≤ double ≤ 1 |
| emissionBack | color of light reflection on object's front faces | a, r, g, b: 0 ≤ double ≤ 1 |
| shininess | strength of light reflexion | 0 ≤ double ≤ 1 |


## Graphic Options

    <clearColor>
      <r>
      <g>
      <b>
      <a>
    <fogEnabled>
    <fogDensity>
    <fogStart>
    <fogEnd>
    <fogColor>
      <r>
      <g>
      <b>
      <a>

| Variable | Description | Possible Values |
| -------- | ----------- | --------------- |
| clearColor | ? | ? |
| fogEnabled | whether or not there is fog in the simulation | true / false |
| fogDensity | density of fog |  |
| fogStart | distance where the fog starts |  |
| fogEnd | distance where the fog ends  |  |
| fogColor| rgba values to define fog color |  |
