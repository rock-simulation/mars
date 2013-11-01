Advanced Modeling {#tutorial_advanced_modelling}
==============

![](../../../src/images/tutorials/advanced_modeling/robot.png)


## Introduction

In this tutorial you will learn how to build model of a robot in Blender which has a more complicated structure than the car used in the basic modeling tutorial.


## Prerequisites

You should have a recent version of Blender installed (we use 2.62 in this tutorial) and access to the scripts in your MARS development folder. 


## Hierarchy and Object Types

In MARS, models are hierarchically structured to attach different parts of the model to one another via joints. A very flat hierarchy was used in the basic modeling example, where the body of the modeled car was *parent* to each of the four wheels. This same principle applies, but with more levels, when more complicated models are created in MARS.

In general, objects in MARS can be either *bodies* or *joints*. A *body* is any object in the simulated world and can be either solid (and thus collide with other objects) or merely visible. A *joint* is not a physical object (nor normally visible), but a helper object defining how to connect bodies. This means that in order for bodies to have a constraint movement with respect to each other (other than collision), joints have to specify those contraints. If bodies are supposed to be fully constrained, they are combined in *groups*, which are derived from the parent and child relationships of the objects.


## Modeling a walking robot

Let's implement all these elements in a model with a slighty more complex hierarchy. To keep things simple, we will only use *hinge* joints represented by cylinders as in the basic modeling tutorial and model the rather clumsy looking fellow shown above. You can of course choose another design of your own fancy as long as the overall morphology stays the same.

As a starting point, we model a main body (we chose a flat cylinder) and attach limbs to it that each possess two segments. Remember to use the *Add -> Mesh* menu in Blender. Via joints, we connect the main body with the upper segment and the upper segment with the lower one:

![](../../../src/images/tutorials/advanced_modeling/building_arm.png)

MARS does not take into account how far apart objects connected by joints are, thus to properly visually represent a joint, it is often helpful to add another object to "hide" that the objects are seemingly connected only by air. In our robot, we chose spheres for that purpose.

What is important now is to get the hierarchy in this system right. Since the lower leg is connected to the upper one, it has to be moved as well when the upper leg rotates around its joint to the main body. This can by setting the upper leg the *parent* of the lower leg, and the main body the *parent* of the upper leg. This effectively creates a hierarchy as shown here:

![](../../../src/images/tutorials/advanced_modeling/hierarchy.png)

Create the custom properties for the joints:

    - motor_type: 1 (1 simulates a servo motor, 2 a simple electric motor)
    - type: joint
    - node2: here it's easiest to choose the spheres we used to represent the joints with visually
    
Also, you can add a nice ball-shaped feet that will make collision-detection more accurate later on and set the custom properties *physicMode* to *sphere* as well as *radius* to the correct value like you did in the basic modeling tutorial. You can then run the *create_mars_props.py* script.    

Now duplicate the leg (*Shift + D*) and rearrange the copies nicely around your body:

![](../../../src/images/tutorials/advanced_modeling/overview.png)

Don't forget to again change the *node2* custom properties of the joints after duplicating the legs. If everything worked according to plan, you should have a hierarchy of objects in Blender looking somewhat like this, with the joint helper objects named "H.*":

![](../../../src/images/tutorials/advanced_modeling/blender_model_structure.png)


## Collision

At this point you might be wondering: "Wait a second, all these solid bodies we created are penetrating each other, is this not going to be a problem for collision in MARS?" In fact, this is not a problem, as objects grouped together cannot collide and may therefore safely overlap. Neither do objets connected via joints show collision behaviour. However, this is only true for direct connections, but not true over more than one hierarchical level. Thus the main body and the upper leg of our robot cannot collide, but the main body and the lower leg do collide. This is the reason why it's important to give the objects helping to visually represent the joint the same parent as the joint itself (and the leg to which the joint is attached).

## Final Remarks

And that's it, you've successfully built a walking robot. We'll deal with how to actually make it walk in another tutorial, but you can use MARS' motor controller window to manually play around with the robot and check whether everything worked out or not.

If you have taken a closer look at the custom properties created by the *create_mars_props* script you will have noticed that the hierarchy shown in the Blender model was mirrored in the groups: each section of the robot belonged to a separate group. Technically, the hierarchy introduced above is not strict in the sense that it is internally represented as connected groups rather than hierarchical levels. However, the *create_mars_props* script uses the hierarchy to properly subdivide everything in groups, thus if you have a centipede-like robot where each part is equally structurally equivalent, you will still have to order them manually into a nested hierarchy. The only alternative is to assign the group numbers manually before running the export script.




