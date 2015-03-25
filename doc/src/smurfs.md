SMURF / SMURF Scenes {#smurfs}
==============================

## Overview

SMURF is the robot description format used by MARS and an extension of URDF: it contains more information than URDF, but maintains full compatibility. Thus, discarding the additional data, it's always possible to retrieve a valid URDF from a SMURF robot representation. Check the [SMURF parser](https://github.com/rock-simulation/smurf_parser) for details.

SMURF scenes (or SMURFS) is a scene format building on the definition of SMURF, but again extending it. While the original [MARS scene](@ref mars_scenes) essentially contained every geometrical object in the scene and it was possible to have multiple robots or objects of the environment mixed in one file, SMURF scenes demand objects in the scene to be grouped as simulation *entities*.  
Such entities can be anything from a heightmap representing the ground to a SMURF-defined robot to simply a light source. MARS has a component called `EntityFactoryManager` (EFM) where classes implementing the `EntityFactory` interface can register one or more entity types. The mars::smurf::SMURFLoader forwards the data in the form of a [`ConfigMap`](https://github.com/rock-simulation/configmaps) to the EFM, which in turn passes it to the respective factories. The config map data gets stored in the `SimEntity` for later use, which allows the creation of plugins to react to user-defined custom data.

There is a variety of [SMURF scenes based on SVG files](@ref smurfs_svg).

## SMURF

The Supplementable, Mostly Universal Robot Format is a flexible format to describe the entirety of a robot. It extends URDF - which it maintains as a separate file - by annotating the URDF data in additional [YAML](http://yaml.org) files.

You can find the SMURF format documentation on the [GitHub repository of the SMURF parser](http://github.com/rock-simulation/smurf_parser).

## SMURF Scenes

### Creating Entities

As mentioned above, SMURF scenes are built from putting together a list of entities. As most of SMURF itself (the only exception being the URDF file), SMURF scenes are encoded in [YAML](http://yaml.org) format. The smallest example we can conjure up has a list with just one element:

    entities:
      - name: Eve
        file: myrobot.smurf

This minimalist entry for our robot called 'Eve' contains its *name* (which will become the name of the `SimEntity` in MARS) and the URI of the SMURF *file*. As we specified no type, MARS will try to parse it from the file extension, which in the case of '.smurf' will translate to *smurf* and allow the correct `EntityFactory` to be used. However we can explicitly specify the type if we want:

    entities:
      - name: Eve
        file: myrobot.smurf
        type: smurf

This may be helpful for users not familiar with the SMURF scene format.  
If we want to get Eve some company, we can now simply add a second robot to our scene, like so:

    entities:
      - name: Eve
        file: myrobot.smurf
        type: smurf
      - name: Adam
        file: myrobot.smurf
        type: smurf

As you can see, it's entirely possible to put more than one robot based on the same SMURF file in one scene, as entities will be distinguished by their names.

### Placing Entities

There is a problem with the above scene containing Adam and Eve: they will both be created at the global origin. Biblical references aside, this is not very practical, as it will lead to collisions that are going to ruin your simulation day. We should therefore get some distance between the two models:

    # example.smurfs
    # SMURF scene example

    entities:
      - name: Eve
        type: smurf
        file: myrobot.smurf
        position: [5.0, 0, -0.8]    # [x, y, z]
        rotation: [1.5, 20.0, 0.0]  # euler angles: [x, y, z]
      - name: Adam
        type: smurf
        file: myrobot.smurf
        position: [-2.0, 3.5, 0]
        rotation: [0.7071, 0.0, -0.7071, 0.0]  # quaternion [w, x, y, z]

While position is always notated as a list of three floats, rotation can be provided as either Euler angles or a rotation quaternion - MARS will interpret the provided list of numbers according to its length. It's even possible only to specify one value (e.g. `rotation: [45.3]`), which will be interpreted as a rotation around the global Z axis.  

As you will also have noted, you can add comments anywhere in YAML using `#`.

### Anchoring Entities

Our two robots so far have been free to move around in our simulated world (though admittedly, there is no ground for them to move around on yet), but sometimes model of a robot or other entities for that matter are supposed to be static, or, "anchored to the world":

    # example.smurfs
    # SMURF scene example

    entities:
      - name: Eve
        type: smurf
        file: myrobot.smurf
        position: [5.0, 0, -0.8]    # [x, y, z]
        rotation: [1.5, 20.0, 0.0]  # euler angles: [x, y, z]
      - name: Adam
        type: smurf
        file: myrobot.smurf
        position: [-2.0, 3.5, 0]
        rotation: [0.7071, 0.0, -0.7071, 0.0]  # quaternion [w, x, y, z]
      - name: Lazy Bob
        file: lazybot.smurf
        anchor: world
        rotation: [15]

This `anchor: world` statement will lead to the root link of the underlying URDF model being statically anchored in the defined position and orientation. Any joints of the anchored model will of course be movable just as expected.  
It can be very helpful to anchor a robot in place while debugging the model, as it will not be affected just as badly by jerky movements caused by internal collisions or similar simulation errors. But since the borders between robots and non-robot-machines are fuzzy, it also allows to build stationary machinery of your simulation environment in the SMURF format, for instance doors, levers or traffic lights, which may not move with respect to the world, but may still have moving parts or sensors. Even fully static elements of the environment such as houses or trees can be represented in SMURF and anchored in the simulation.

### Using Old MARS Scenes

To stay backwards compatible, SMURF scenes allow to re-use old MARS scenes by simply adding them to the list of entities to be loaded. This is a very neat way to finally create some solid ground:

    # example.smurfs
    # SMURF scene example

    entities:
      - name: Eve
        type: smurf
        file: myrobot.smurf
        position: [5.0, 0, -0.8]    # [x, y, z]
        rotation: [1.5, 20.0, 0.0]  # euler angles: [x, y, z]
      - name: Adam
        type: smurf
        file: myrobot.smurf
        position: [-2.0, 3.5, 0]
        rotation: [0.7071, 0.0, -0.7071, 0.0]  # quaternion [w, x, y, z]
      - name: Lazy Bob
        file: lazybot.smurf
        anchor: world
        rotation: [15]
      - name: Earth
        file: infinite_plane.scn  # old MARS scene
        type: scn  # optional

Again, as with SMURF files, MARS will derive the type from the file extension, but it doesn't hurt to specify the type explicitly, either.

### Adding Custom Entities

SMURF scenes are easily extendable. For instance, you might feel that some nice background touch is missing in our creation and thus you wish to add an entity of type *rainbow*, for which you have just created an `EntityFactory`. Well, there you go:

    # example.smurfs
    # SMURF scene example

    entities:
      - name: Eve
        type: smurf
        file: myrobot.smurf
        position: [5.0, 0, -0.8]    # [x, y, z]
        rotation: [1.5, 20.0, 0.0]  # euler angles: [x, y, z]
      - name: Adam
        type: smurf
        file: myrobot.smurf
        position: [-2.0, 3.5, 0]
        rotation: [0.7071, 0.0, -0.7071, 0.0]  # quaternion [w, x, y, z]
      - name: Lazy Bob
        file: lazybot.smurf
        anchor: world
        rotation: [15]
      - name: Earth
        file: infinite_plane.scn  # old MARS scene
        type: scn  # optional
      - name: rainbow1
        file: environment/rainbow.yml
        type: rainbow
        position: [1500.0, 300.0, 270.0]
        rotation: [37.5]

Provided that you have entered your library with the `EntityFactory` for *rainbow*s in the other_libs.txt file in your MARS configuration folder, MARS will pass the information to your plugin and thus your new entity will be integrated in the scene.

  > **NOTE**: The parameters *position* and *rotation* are not required to define a valid `SimEntity`. You may create a rather abstract entity for which you don't need these variables. However if your entity is supposed to be located somewhere in the scene and you therefore want to create it at a provided position and orientation, you have to implement this functionality yourself in your `EntityFactory` class.

You will have recognized that the file extension for our custom rainbow entity  is '.yml'. We do not recommend to use file extensions as a way to substitute custom types, as types such as `my_special_kind_of_entity` are rather unwieldy in filenames. This is especially important since we encourage users to use specialized and verbose entity types, as this helps to mitigate the problem of type name conflicts.
