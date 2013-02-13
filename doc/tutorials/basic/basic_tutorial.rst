=======================
Basic Modeling Tutorial
=======================

Introduction
------------

In this tutorial you will learn how to build a simple model of a robot
in Blender and how to export it to MARS. You will also learn how to
then write a simple MARS plugin to control that robot.

Prerequisites
-------------

We assume you already have a MARS development environment setup in
{{{~/devel/}}} You should also have a recent version of Blender
installed (we use 2.62 in this tutorial)

Blender Basics
--------------

shortcuts and hotkeys
~~~~~~~~~~~~~~~~~~~~~
rotate
  middle mouse button
move
  shift + middle mouse button
zoom
  mouse wheel or Ctrl + middle mouse button
open object property window
  press n

rescaling objects
~~~~~~~~~~~~~~~~~

if you edit the dimensions of an object in "object mode" you might
notice that the scale changes automatically.  this is because in
object mode you do not really change the object's vertices.  to apply
the changes you did in "object mode" to the object's vertices you can
press Ctrl+a (with the mouse hovering over the 3D-View) and choose
scale.  you will notice that the scale in "object mode" is back to (1,
1, 1).  Alternatively you can select this option from the menu
object->apply->scale

.. image:: screenshots/object_menu.png
   :width: 100%

Building the Model
------------------

1. Start by creating a box of the dimensions 0.5 x 0.3 x 0.2 this can
   be done by directly editing the object properties "Dimensions" (1).

2. Rename the object (e.g. "Body" (2B)) this can be done in the object
   property window (2A)

   .. image:: screenshots/rename_object.png

3. Choose a color by selecting a diffuse and/or specular color in the
   material property window

   .. image:: screenshots/material.png

4. It is convinient to have the model on a different layer than the
   camera and lights.  Therefore we move it to a different layer by
   selecting everything belonging to the model (so far only the box)
   by "shift + right click"ing the obects or pressing "b" and draging
   a selection window around the objects.  Then press "m" and select a
   new layer to move the objects to that layer.

5. Create a sphere that will serve as a wheel.  In the Menu select
   "Add->Mesh->UV Sphere" and name the object (e.g., "Wheel.000").

   .. image:: screenshots/create_sphere.png

6. Scale the sphere to an apropriate size (we used 0.1 in all
   dimensions)

7. Position the sphere at a lower corner of the box be setting its
   location parameters (e.g., x=0.25; y=-0.15; z=-0.1)

8. Create a new material by pressing "new" in the material property
   window

   .. image:: screenshots/new_material.png

9. Set material properties for the wheel/sphere.

10. Set up parent-child relationship.  Select the wheel by
    right-clicking.  In the object property window under the
    "Relation" section select the box "Body" as parent.  NOTE: If you
    want to break up a parent-child relationship you should select the
    child object in the 3D-View and press "Alt+p" (while hovering the
    3D-View) and select "Clear and Keep Transformation".

11. The export scripts for MARS need you to define helper objects for
    every joint in your model. on these objects you can then also set
    joint and motor properties.  create a arbitrary object (we usually
    use slim cylinders because you can clearly see the rotation axis)
    and position it at the location of the joint.  It helps if the
    child object of the joint has the same origin and orientation as
    the joint helper object.  set its parent to "Body" and create two
    custom properties in the "object property" window.
    
    .. image:: screenshots/joint.png

    for the first custom property press edit to set the "property
    name" to "type" and the "property value" to "joint" for the second
    custom property press edit to set the "property name" to "node2"
    and the "property value" to "Wheel.000" this tells the export
    script that this is a helper object for a "joint" and that the
    object on the output shaft is the "Wheel.000" object.  The object
    on the input shaft it determined by the parent of the helper
    object itself so set that to "Body".

12. Duplicate the wheel by selecting it and pressing "Shift+d".  a
    duplicated object will inherit the location, orientation,
    material, and parent among other things.  By nameing the first
    wheel "Wheel.000" we ensure consistent nameing. blender
    automatically increases the ending to "Wheel.001" for the
    duplication. of course you can set the name to what ever you like.

13. Also create the last two remaining wheels by duplicating one of
    the first.  Position the four wheels at the lower four corners of
    the box.

14. Duplicate the helper object for each of the four wheels and do not
    forget to change the value of the "node2" custom property.

15. Turn the timeline into a text editor

    .. image:: screenshots/text_editor.png

16. By selecting the menu "text->Open Text Block" in the text editor
    open the "create_mars_props.py" script from
    ~/devel/mars/scripts/blender/

17. Select all objects of the model in the 3d-view and run the script
    by pressing "Run Script" in the text editor window.  You will see
    that this creates various custom properties on every object. you
    are free to edit these properties (e.g. set the mass)

18. By default each joint is automatically associated with a servo
    motor.  To change the motor type to a PID motor you need to create
    another custom property called "motor_type" and set its value to
    "2" (note that you might need to increase the "Max" value)

19. Load the "relative_mars_export.py" from
    ~/devel/mars/scripts/blender/ in the text editor window.

20. Add the custom properties "filename" and "path" to the World and
    fill in where you want to export the scene to.  Ideally, this
    should be an empty directory because bside the .scn file the .obj
    and .scene files are also expotred for debuging purposes.

    .. image:: screenshots/world_properties.png

21. For each wheel create two more custom properties: "physicMode" and
    "radius".  Set "physicMode" to "sphere" and "radius" to half the
    diameter (in our case 0.05)

22. Select all objects of the model and run the relative_mars_export
    script.

23. Do not forget to save your blender scene.

Congratulations!  You have your first blender created MARS scene.
However, if you load the scene in MARS and try to do a tank turn you
might notice that it does not work to well.  You would need to tune to
friction parameters for the wheels. Alternatively we add a third pair
of wheels to the model to show the workflow of editing an existing
scene.

24. Reopen your blender scene if you do not have it opened any more.

25. Duplicate a wheel pair (shift+d after selecting the parts) and
    move it to the middle of the body ("g" for grab; "x" to constrain
    the movement to the x-axis; "0.25" to move it to the middle)

    .. image:: screenshots/new_wheels.png

26. Adjust the custom property "node2" of the new joint helper objects
    to let them point to the new wheels.

27. Re-run the "create_mars_props.py" script.  You need to execute
    this script everytime you add or remove an object to/from the
    scene because it will assign unique ids to all objects and group
    objects belonging together.

28. Run the "relative_mars_export.py" script to update the scene.

29. Test in MARS!

