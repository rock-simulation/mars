Mars Scenes {#mars_scenes}
==============

## Overview

MARS possesses its own format to represent 3D environments with physical entities, the MARS scene. These files with the extension *.scene* are xml files and have the following principal structure:


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

    <node name="Lower.Leg.003">
      <origname>name(in_blender)</origname>
      <filename>name.obj</filename>
      <index>i</index>
      <groupid>g_id</groupid>
      <physicmode>box</physicmode>
      <relativeid>rel_id</relativeid>
      <position>
        <x>pos_x</x>
        <y>pos_y</y>
        <z>-0.49999985098838806</z>
      </position>
      <rotation>
        <x>rot_x</x>
        <y>rot_y</y>
        <z>rot_z</z>
        <w>1.0</w>
      </rotation>
      <extend>
        <x>ext_x</x>
        <y>ext_y</y>
        <z>ext_z</z>
      </extend>
      <pivot>
        <x>piv_x</x>
        <y>piv_y</y>
        <z>piv_z</z>
      </pivot>
      <visualsize>
        <x>vs_x</x>
        <y>vs_y</y>
        <z>vs_z</z>
      </visualsize>
      <movable>true/false</movable>
      <mass>m</mass>
      <density>rho</density>
      <material_id>mat_id</material_id>
      <coll_bitmask>65535</coll_bitmask>
    </node>
    
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
| coll_bitmask | ? | ? |  


## Joints

*Joints* are helper objects defining how *nodes* or groups of nodes are connected with each other.

    <joint name="H.Knee.000">
      <index>n</index>
      <type>jointType</type>
      <nodeindex1>n1</nodeindex1>
      <nodeindex2>n2</nodeindex2>
      <anchorpos>pos</anchorpos>
      <anchor>
        <x>anch_x</x>
        <y>anch_y</y>
        <z>anch_z</z>
      </anchor>
      <axis1>
        <x>a1_x</x>
        <y>a1_y</y>
        <z>a1_z</z>
      </axis1>
      <angle1_offset>offset</angle1_offset>
    </joint>
    
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


## Motors

Motors are associated with *joints* and can apply forces and torques respectively; for a *slider joint*, a force is applied while for a *hinge joint*, *motors* create a torque. As MARS uses the Open Dynamics Engine for its physics simulation, you can have a look at [ODE's documentation](http://www.ode.org/ode-latest-userguide.html) to read further on different [joint and motor types](http://www.ode.org/ode-latest-userguide.html#sec_7_0_0).

As for the decoding in MARS scenes, motors are described as follows:

    <motor name="H.Hip.000">
      <index>n</index>
      <jointIndex>j</jointIndex>
      <axis>1</axis>
      <maximumVelocity>v_max</maximumVelocity>
      <motorMaxForce>f_max</motorMaxForce>
      <type>t</type>
      <p>p</p>
      <i>i</i>
      <d>d</d>
      <min_val>minval</min_val>
      <max_val>maxval</max_val>
      <value>val</value>
    </motor>
  
| Variable | Description | Possible Values |
| -------- | ----------- | --------------- |
| index | index of the motor |  int ≥ 1 |
| jointIndex | index of the joint with which the motor is associated | int≥ 1 |
| axis | around/along which axis of its associated joint the motor turns/slides | 1 ≤ int ≤ 3 |
| maximumVelocity | maximum velocity with which the motor can move the joint | double |
| motorMaxForce | maximum force the motor can apply | double |
| type | motor type | 1: servo, 2: electric motor |
| p | P value of a PID controller | double |
| i | I value of a PID controller | double |
| d | D value of a PID controller | double |
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
      <index>i</index>
      <rate>r</rate>
      <id>id1</id>
      <id>id2</id>
      ...
    </sensor>
    
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
      <rate>40</rate>
      <sensorid>s_id1</sensorid>
      <sensorid>s_id2</sensorid>
      ...
      <motorid>m_id1</motorid>
      <motorid>m_id21</motorid>
      ...
    </controller>
    
    
| Variable | Description | Possible values |
| -------- | ----------- | --------------- |
| rate | operation rate of the *controller* | ? |
| sensorid | running index of an attached *sensor* | int ≥ 1 |
| motorid | running index of an attached *motor* | int ≥ 1 |



## Materials

    <material>
      <id>i</id>
      <diffuseFront>
        <a>a</a>
        <r>r</r>
        <g>g</g>
        <b>b</b>
      </diffuseFront>
      <specularFront>
        <a>a</a>
        <r>r</r>
        <g>g</g>
        <b>b</b>
      </specularFront>
      <shininess>shiny</shininess>
    </material>
    
    
| Variable | Description | Possible Values |
| -------- | ----------- | --------------- |
| id | running index of the materials | int ≥ 1 |
| diffuseFront | basic color of an object | a, r, g, b: 0 ≤ double ≤ 1 |
| specularFront | color of light reflected off an object | a, r, g, b: 0 ≤ double ≤ 1 |
| shininess | strength of light reflexion | 0 ≤ double ≤ 1 |


## Graphic Options

    <clearColor>
      <r>r</r>
      <g>g</g>
      <b>b</b>
      <a>a</a>
    </clearColor>
    <fogEnabled>true/false</fogEnabled>
    
| Variable | Description | Possible Values |
| -------- | ----------- | --------------- |
| clearColor | ? | ? |
| fogEnabled | whether or not there is fog in the simulation | true / false |   




