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
      <materiallist>
      <graphicOptions>
    </SceneFile>
    
We will have a closer look at the items in these lists in the following.

### Nodes

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
    
| variable | description | possible values |
|----------|-------------|-----------------|
| origname | | 
| filename | | 
| index | | 
| groupid | | 
| physicmode | | 
| relativeid | | 
| position | | 
| rotation | | 
| extend | | 
| pivot | | 
| visualsize | | 
| movable | | 
| mass | | 
| density | | 
| material_id | |  
| coll_bitmask | |  


### Joints

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
    
| variable | description | possible values |
|----------|-------------|-----------------|
| index |  | 
| type |  | 
| nodeindex1 |  | 
| nodeindex2 |  | 
| anchorpos |  | 
| anchor |  | 
| axis1 |  | 
| angle1_offset |  | 


### Motors

Motors are associated with joints and can apply forces and torques respectively; for a *slider joint*, a force is applied while for a *hinge joint*, motors create a torque. As MARS uses the Open Dynamics Engine for its physics simulation, you can have a look at [ODE's documentation]{http://www.ode.org/ode-latest-userguide.html} to read further on different [joint and motor types]{http://www.ode.org/ode-latest-userguide.html#sec_7_0_0}.

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
  
| variable | description | possible values |
| index | index of the motor | 1..inf |
| jointIndex | index of the joint with which the motor is associated | 1..inf |
| axis | around/along which axis of its associated joint the motor turns/slides | 1..3 |
| maximumVelocity | maximum velocity with which the motor can move the joint | 0..inf |
| motorMaxForce | maximum force the motor can apply | 0..inf |
| type | motor type | 1: servo, 2: electric motor |
| p | P value of a PID controller | 0.. |
| i | I value of a PID controller |   |
| d | D value of a PID controller |  |
| min_val | minimum value (see below) |
| max_val | maximum value (see below) |
| value |  |


### Sensors


### Materials

    <material>
      <id>3</id>
      <diffuseFront>
        <a>1.0</a>
        <r>0.21402966976165771</r>
        <g>0.008978134021162987</g>
        <b>0.0</b>
      </diffuseFront>
      <specularFront>
        <a>1.0</a>
        <r>1.0</r>
        <g>1.0</g>
        <b>1.0</b>
      </specularFront>
      <shininess>25.0</shininess>
    </material>


### Graphic Options




