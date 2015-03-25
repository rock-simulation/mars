SMURF SVG Scenes {#smurfs_svg}
==============================

## Overview

MARS understands a variety of the SMURF scene format using [Scalable Vector Graphics (SVG)](http://en.wikipedia.org/wiki/Scalable_Vector_Graphics) to place entities. If your world environment is mostly two-dimensional, this is very handy as it allows to create scenes at the same time as creating nice maps of these scenes. A great free editor for vector graphics is [Inkscape](https://inkscape.org) (though we recommend saving your files not as 'inkscape svg', but plain svg).

## Structure of SMURF SVGs

SVG is an XML format of which SMURF SVGs use only a minor subset. As of now (March 2015), there are two main elements of the XML tree that are interpreted: `<svg>` which is the root element and `<g>`, which defines a group. While the former contains data on the overall scene, each group element represents an entity in the simulation and thus store entity data. A minimalist example would look like this:

    <svg
       id="my_svg_scene"
       width="10.0"
       height="10.0">
     <g
        transform="matrix(0,1,-1,0,44.2,0)"
        id="entity_1">
       <title>entity_library/my_entity.yml</title>
       <path
          d="m 0,30 1,0"
          id="entity_1_origin" />
     </g>
    </svg>

which admittedly is a lot to swallow for a minimalist example. We had to cannibalize a number of properties and tags to use SVG for SMURF scenes - let's walk through it step by step.

### `<svg>`

In order to correctly arrange the elements of a scene, we need to create a reference frame by defining the width and height of the svg element.

 property | use for SMURF scene
 -------- | -----------------------------
 id       | name of the scene
 width    | width of the scene in meters
 height   | height of the scene in meters

### `<g>`

`<g>` is used to group geometrical shapes as well as additional elements containing data on the group. Thus using groups to represent entities allows to visualize them in a way that the creator of the scene deems appropriate for its purpose and faithful to the actual appearance of the entity. That being said, there are a number of required elements.

property | use for SMURF scene
-------- | -----------------------------
id       | name of the entity
transform | transformation (matrix) defining the orientation of the entity

### `<title>`

The title contains the URI to the .yml file which contains the definition of the entity, i.e. the parameters and their values that define it.

### `<path>`

While the transform of the entity is stored in the *transform* property of the SMURF scene, we still need a way to also save the position of the entity. For this, we use a *path* element with two points, the first of which defining the origin of the entity.

property | use for SMURF scene
-------- | -----------------------------
id       | name of the entity's origin
d        | path definition with two points, the first one being the origin of the entity

As for the formatting of the `d` attribute, we recommend checking out [its description on the Mozilla Developer Network](https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/d) which is also a good source for info on other SVG elements.

  > **NOTE:** All x- and y-coordinate definition in SVG use a coordinate system with x pointing right and y pointing down. In order to avoid mirroring scenes and thus provide a WYSIWYG experience, MARS however interprets an SVG as x pointing right and Y pointing up. Keep that in mind when setting the values for x and y parameters. In fact, if you use an SVG editor such as Inkscape (and you really should, because if you don't, you might as well create .yml descriptions directly), the editor will take care of this for you.

### `<rect>`

Many entities have not only a position, but also a dynamically adjustable width and height. For such cases, our parser expects to find a `rect` element:

    <g>
      ...
      <rect
         width="5.0"
         height="3.5"
         x="3.0"
         y="4.7"
         id="entity_1_bbox" />
    </g>

A rectangle element essentially defines a bounding box for the entity.

property | use for SMURF scene
-------- | -----------------------------
id       | name of the bounding box element
x        | x position of the bbox
y        | y position of the bbox
width    | width of the entity
height   | height of the entity

Consequently, the attributes `width` and `height` get translated into the respective config parameters of the entity's `ConfigMap`.

## Custom properties in SVG scenes

We are planning to allow properties not only to be defined in the the references .yml file, but also to store entity properties directly in a `<desc>` element of the group defining an entity. As of now (March 2015), this functionality has not been implemented, except for one single property: "origin_z", which defines the Z component of an entity's position, like so:

    <g>
      ...
      <desc>origin_z: 0.0</desc>
    </g>

  > **NOTE:** The properties *origin_x*, *origin_y* and *origin_z* are properties currently being replaced by a single list-property of entities called *position*. We will maintain backwards-compatibility, though.

## Parser

The code parsing SMURF SVGs is currently located in the mars::smurf::SMURFLoader class.
