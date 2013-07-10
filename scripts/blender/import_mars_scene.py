import os
import sys
import math
import shutil
import zipfile
import textwrap

import xml.dom.minidom

import bpy
import mathutils

EPSILON = sys.float_info.epsilon

# definition of node types
nodeTypes = ["undefined",
             "mesh",
             "box",
             "sphere",
             "capsule",
             "cylinder",
             "plane",
             "terrain",
             "reference"
            ]

# definition of joint types
jointTypes = ["undefined",
              "hinge",
              "hinge2",
              "slider",
              "ball",
              "universal",
              "fixed",
              "istruct-spine"
             ]

# global list of nodes, joints and materials
nodeList = []
jointList = []
materialList = []

# global list of imported but not used meshes
unusedNodeList = []

# map of keyword differences between MARS .scn and Blender
scnToBlenderKeyMap = {"groupid" : "group", # node
                      "index" : "id", # node
                      "type" : "jointType", # joint
                      "id" : "marsID" # material
                     }

# delete all objects, meshes and material from Blender
def cleanUpScene():
    # select all objects
    bpy.ops.object.select_all(action="SELECT")

    # and delete them
    bpy.ops.object.delete()

    # after that we have to clean up all loaded meshes (unfortunately
    # this is not done automatically)
    for mesh in bpy.data.meshes:
        bpy.data.meshes.remove(mesh)

    # and all materials
    for material in bpy.data.materials:
        bpy.data.materials.remove(material)


# clean up all the "empty" whitespace nodes
def removeWhitespaceNodes(parent, unlink=True):
    remove_list = []

    for child in parent.childNodes:
        if child.nodeType == xml.dom.minidom.Node.TEXT_NODE and \
           not child.data.strip():
            remove_list.append(child)
        elif child.hasChildNodes():
            removeWhitespaceNodes(child, unlink)

    for node in remove_list:
        node.parentNode.removeChild(node)
        if unlink:
            node.unlink()


# convert the given xml structure into a python dict
def getGenericConfig(parent):
    child = parent.firstChild
    if not child:
        return None
    # if it is a text node, we just return the contained value
    elif (child.nodeType == xml.dom.minidom.Node.TEXT_NODE):
        # check if it is an integer?
        try:
            return int(child.nodeValue)
        except ValueError:
            pass

        # or a float?
        try:
            return float(child.nodeValue)
        except ValueError:
            pass

        # if it is a string, remove leading and trailing
        # white space characters
        string = child.nodeValue.strip()

        # see if it is the boolean 'True'?
        if string in ["true", "True"]:
            string = True

        # or the boolean 'False'?
        if string in ["false", "False"]:
            string = False

        return string

    config = {}

    # check for attributes ...
    if parent.hasAttributes():
        for key, value in parent.attributes.items():
#            print("attrib [%s : %s]" % (key, value))
            if key not in config:
                config[key] = value
            else:
                print("Warning! Key '%s' already exists!" % key)

    # and check the appending child nodes
    while child is not None:
        if (child.nodeType == xml.dom.minidom.Node.ELEMENT_NODE):
            key   = child.tagName
            value = getGenericConfig(child)
#            print("element [%s : %s]" % (key, value))
            if key not in config:
                config[key] = value
            else:
                print("Warning! Key '%s' already exists!" % child.tagName)
        child = child.nextSibling

    # check if it is a vector
    if isinstance(config,dict) and sorted(config.keys()) == ["x","y","z"]:
        # and transform it into a Blender Vector
        config = mathutils.Vector((config["x"],
                                   config["y"],
                                   config["z"]))

    # check if it is a quaternion
    if isinstance(config,dict) and sorted(config.keys()) == ["w","x","y","z"]:
      # and transform it into a Blender Quaternion
      config = mathutils.Quaternion((config["w"],
                                     config["x"],
                                     config["y"],
                                     config["z"]))

    # check if it is a color
    if isinstance(config,dict) and sorted(config.keys()) == ["a","b","g","r"]:
        # and transform it into a Blender Color
        config = mathutils.Color((config["r"],
                                  config["g"],
                                  config["b"]))

    return config


def checkConfigParameter(config, key):
    if key not in config.keys():
        print("WARNING! Config does not contain parameter \'%s\'!" % key)
        return False
    return True


def setParentChild(parent, child):
    if parent not in nodeList:
        print("WARNING! Unable to set parent-child relationship! Parent non-existing! Parent must be a node!")
        return False

    if child not in nodeList and child not in jointList:
        print("WARNING! Unable to set parent-child relationship! Child non-existing! Child must be either a node or a joint!")
        return False

    # de-select all objects
    bpy.ops.object.select_all(action="DESELECT")

    # select the child
    child.select = True

    # select the parent
    parent.select = True

    # set the parent to be the currently active object
    bpy.context.scene.objects.active = parent

    # set the parent-child relationship
    bpy.ops.object.parent_set(type="OBJECT")

    return True


def centerNodeOrigin(node):
    if node.name not in bpy.data.objects:
        print("WARNING! Unable to center the origin of node <%s>! Node does not exist!" % node.name)
        return False

    # de-select all objects
    bpy.ops.object.select_all(action="DESELECT")

    # select the node/object
    node.select = True

    # set the parent to be the currently active object
    bpy.context.scene.objects.active = node

    # set the origin of the mesh to the center of its
    # bounding box
    bpy.ops.object.origin_set(type="ORIGIN_GEOMETRY",
                              center="BOUNDS")

    return True


def calculateCenter(boundingBox):
    c = mathutils.Vector()
    for v in boundingBox:
        for i in range(3):
            c[i] += v[i]
    return c / 8.0


def parseMaterial(domElement):
    # read the config from the xml file
    config = getGenericConfig(domElement)

    # check for the material index
    if not checkConfigParameter(config,"id"):
        return False
    marsID = config["id"]

    # create a name out of the material index (not needed in MARS
    # but here in Blender)
    name = "mars_material_%u" % marsID

    print("# Creating material <%s>" % name)

    # check if a material with the same name already does exist
    if name in bpy.data.materials:
        # when it exists remove it (just to wipe the plate clear)
        tmp = bpy.data.materials[name]
        bpy.data.materials.remove(tmp)

    # create a new material
    material = bpy.data.materials.new(name)

    # add the material to the global list
    materialList.append(material)

    # add each item of 'config' as a custom property to the material
    for (key, value) in config.items():
        if key in scnToBlenderKeyMap:
            material[scnToBlenderKeyMap[key]] = value
        else:
            material[key] = value

    # set the diffuse color
    if checkConfigParameter(config,"diffuseFront"):
        material.diffuse_color = config["diffuseFront"]

    # set the specular color
    if checkConfigParameter(config,"specularFront"):
        material.specular_color = config["specularFront"]

    # set the shininess
    if checkConfigParameter(config,"shininess"):
        material.specular_hardness = 2 * config["shininess"]

    return True


def parseNode(domElement, tmpDir):
    # read the config from the xml file
    config = getGenericConfig(domElement)

    # handle node name
    if not checkConfigParameter(config,"name"):
        return False
    name = config["name"]

    print("# Creating node <%s>" % name)

#    print("%s : %s" % (config["name"], config))

    # handle node physic mode
    if checkConfigParameter(config,"physicmode"):
        typeName = config["physicmode"]
        if typeName in nodeTypes:
            physicMode = nodeTypes.index(typeName)
        else:
            print("ERROR! Could not get type for node: %s" % name)

    if checkConfigParameter(config,"origname"):
        origName = config["origname"]

    if checkConfigParameter(config,"filename"):
        filename = config["filename"]

    if filename == "PRIMITIVE":
        if not origName:
            origName = nodeTypes[physicMode]
        elif origName != nodeTypes[physicMode]:
            tmp = nodeTypes[physicMode]
            print("WARNING! Origname set to \"%s\" for primitive in node \"%s\" with physicMode \"%s\"" % (origName, name, tmp))

    if checkConfigParameter(config,"index"):
        index = int(config["index"])

    if checkConfigParameter(config,"position"):
        position = config["position"]
    else:
        position = mathutils.Vector()

    if checkConfigParameter(config,"rotation"):
        rotation = config["rotation"]
    else:
        rotation = mathtutils.Quaternion()
        rotation.identity()

    if checkConfigParameter(config,"extend"):
        extend = config["extend"]
    else:
        extend = mathutils.Vector()

    if checkConfigParameter(config,"pivot"):
        pivot = config["pivot"]
    else:
        pivot = mathutils.Vector()

    if checkConfigParameter(config, "visualposition"):
        visual_position = config["visualposition"]
    else:
        visual_position = mathutils.Vector()

    if checkConfigParameter(config, "visualrotation"):
        visual_rotation = config["visualrotation"]
    else:
        visual_rotation = mathutils.Quaternion()
        visual_rotation.identity()

    if checkConfigParameter(config,"visualsize"):
        visual_size = config["visualsize"]
    else:
        visual_size = extend

    ######## LOAD THE NODE IN BLENDER ########

    # "pointer" to the newly created object
    node = None

    if filename == "PRIMITIVE":

        if typeName == "box":
            # create a new box as representation of the node
            bpy.ops.mesh.primitive_cube_add()
            # get the "pointer" to the new node
            node = bpy.context.selected_objects[0]
            # set the size of the cube
            node.dimensions = extend

        elif typeName == "sphere":
            # create a new sphere as representation of the node
            bpy.ops.mesh.primitive_uv_sphere_add(size = extend.x)
            # get the "pointer" to the new node
            node = bpy.context.selected_objects[0]

        elif typeName == "reference":
            # TODO: is that really needed?
            print("Warning! Unhandled node type \'reference\'.")

        elif typeName == "mesh":
            # TODO: is that really needed?
            print("Warning! Unhandled node type \'mesh\'.")

        elif typeName == "cylinder":
            # create a new cylinder as representation of the node
            bpy.ops.mesh.primitive_cylinder_add(radius = extend.x, depth = extend.y)
            # get the "pointer" to the new node
            node = bpy.context.selected_objects[0]

        elif typeName == "capsule":
            print("Warning! Node type \'capsule\' yet supported, using \'cylinder\' instead.")
            # create a new cylinder as representation of the node
            bpy.ops.mesh.primitive_cylinder_add(radius = extend.x, depth = extend.y)
            # get the "pointer" to the new node
            node = bpy.context.selected_objects[0]

        elif typeName == "plane":
            # create a new plane as representation of the node
            bpy.ops.mesh.primitive_plane_add()
            # get the "pointer" to the new node
            node = bpy.context.selected_objects[0]
            # set the size of the cube
            node.dimensions = extend

        else:
            print("Cannot find primitive type: %s" % origName) 

    elif physicMode == "terrain":
        # TODO: Creating terrain in Blender ...
        print("Warning! \'Terrain\' nodes are not supported right now! Using a small box as a placeholder!")
        # create a new box as placeholder for the "terrain" node
        bpy.ops.mesh.primitive_cube_add()
        # get the "pointer" to the new node
        node = bpy.context.selected_objects[0]
        # set the size of the cube
        node.dimensions = mathutils.Vector((0.1,0.1,0.1))

    # we have to load the node from an import file
    else:
        # check whether the object was previously already imported
        # and is not used
        for nodeName in unusedNodeList:
            if filename in nodeName or name in nodeName:
                node = bpy.data.objects[nodeName]
                # remove the current object from the unused list
                unusedNodeList.remove(nodeName)

        # if not, import the respective .obj file
        if not node:
            # import the .obj file (after importing a .obj file the newly
            # added objects are selected by Blender)
            bpy.ops.import_scene.obj(filepath=tmpDir+os.sep+filename)

            # if there were added multiple meshes from one .obj file
            if len(bpy.context.selected_objects) > 1:
                # store the list of newly added meshes
                new_object_list = bpy.context.selected_objects
                
                # put the newly imported objects into the "unused" list
                for tmp in new_object_list:
                    # add the name of the imported object to the list of
                    # unused nodes
                    unusedNodeList.append(tmp.name)

                    # center the origin of the loaded node to the center
                    # of its bounding box
                    centerNodeOrigin(tmp)

                    # get the currently added object
                    if filename in tmp.name or name in tmp.name:
                        node = bpy.data.objects[tmp.name]
                        # remove the current object from the unused list
                        unusedNodeList.remove(tmp.name)

            # if there was added just one mesh
            else:
                # get the newly added node as the only selected object
                node = bpy.context.selected_objects[0]

        else:
            print("WARNING! Mesh \'%s\' already imported! Skipping second import!" % name)

        # set the size of the object
        #TODO: find out why the inversion of y- and z-axis is required
        node.dimensions = mathutils.Vector((visual_size.x,
                                            visual_size.z,
                                            visual_size.y))

    # Set the parameter, orientation and position of the new node
    if node:

        # add the node to the global node list
        nodeList.append(node)

        # set the name of the object
        node.name = name

        # set the object type to be a node
        node["type"] = "body"

        # if there is no "groupid" in the config set the default value of zero
        if not checkConfigParameter(config,"groupid"):
            node["group"] = 0

        # add each item of 'config' as a custom property to the node
        for (key, value) in config.items():
            if key in scnToBlenderKeyMap:
                node[scnToBlenderKeyMap[key]] = value
            else:
                node[key] = value

        # if the node should be positioned "relative" to some other node
        if checkConfigParameter(config,"relativeid"):
            # get the ID of this "relative" node
            relativeID = config["relativeid"]

            # find the "relative" node
            relative = None
            for tmp in nodeList:
                if tmp["id"] == relativeID:
                    relative = tmp

            # if the "relative" node was found
            if relative:
                # TODO: why do we have to remove the previous applied rotation?
                inverted_rotation_offset = mathutils.Euler((-math.pi/2.0, 0.0, 0.0)).to_quaternion()

                # calculate the absolute position based on the "relative" node's
                # position, the center of its bounding box (could be different
                # than its orign) and the given position
                position = relative.location + \
                           relative.rotation_quaternion * calculateCenter(relative.bound_box) + \
                           relative.rotation_quaternion * inverted_rotation_offset * position

                # calculate the absolute orientation based on the "relative" node's
                # orientation and the given orientation
                rotation = relative.rotation_quaternion * inverted_rotation_offset * rotation
            else:
                print("WARNING! Could not find relative node (id: %u)!" % relativeID)

        # set the position of the object
        node.location = position + rotation * (visual_position - pivot)

        # TODO: why do we need this offset rotation?!?
        rotation_offset = mathutils.Euler((math.pi/2.0, 0.0, 0.0)).to_quaternion()

        # set the rotation of the object
        node.rotation_mode = "QUATERNION"
        node.rotation_quaternion = rotation * visual_rotation * rotation_offset

        # if a node is linked to a material
        if checkConfigParameter(config,"material_id"):
            # get the node ID
            materialID = config["material_id"]

            # find the corresponding material
            for material in materialList:
                if material["marsID"] == materialID:
                    # and add the new material to the node
                    node.active_material = material

    else:
        print("ERROR! Something went wrong while creating node \'%s\'" % name)
        return False

    return True


def parseJoint(domElement):
    # read the config from the xml file
    config = getGenericConfig(domElement)

    # handle joint name
    if not checkConfigParameter(config,"name"):
        return False
    name = config["name"]

    print("# Creating joint <%s>" % name)

    # handle joint type
    if checkConfigParameter(config,"type"):
        typeName = config["type"]
        if typeName in jointTypes:
            type = jointTypes.index(typeName)
            if type == 0:
                print("JointData: type given is undefined for joint \'%s\'" % name)
        else:
            print("JointData: no type given for joint \'%s\'" % name)

    if checkConfigParameter(config,"index"):
        index = config["index"]

    if checkConfigParameter(config,"nodeindex1"):
        nodeIndex1 = config["nodeindex1"]
        if not nodeIndex1:
            print("JointData: no first node attached to joint \'%s\'" % name);

    if checkConfigParameter(config,"nodeindex2"):
        nodeIndex2 = config["nodeindex2"]

    # handle axis 1
    if checkConfigParameter(config,"axis1"):
        axis1 = config["axis1"]

    if checkConfigParameter(config,"anchorpos"):
        anchorPos = config["anchorpos"]

    if checkConfigParameter(config,"anchor"):
        anchor = config["anchor"]


    ######## LOAD THE JOINT IN BLENDER ########

    if type != jointTypes.index("fixed"):

        # create a new cylinder as representation of the joint        
        bpy.ops.mesh.primitive_cylinder_add(radius=0.01, depth=0.2)

        # get the "pointer" to the new joint
        joint = bpy.context.selected_objects[0]

        # add the node to the global node list
        jointList.append(joint)

        # set the name of the object
        joint.name = name

        # set the object type to be a joint
        joint["type"] = "joint"

        # add each item of 'config' as a custom property to the joint
        for (key, value) in config.items():
            if key in scnToBlenderKeyMap:
                joint[scnToBlenderKeyMap[key]] = value
            else:
                joint[key] = value

        # set the color of the joint helper object to green
        if "joint" not in bpy.data.materials:
            # create new "green" material
            mat = bpy.data.materials.new("joint")
            mat.diffuse_color = mathutils.Color((0.0,
                                                 1.0,
                                                 0.0))
            mat.diffuse_shader = "LAMBERT"
            mat.diffuse_intensity = 0.6
            mat.specular_color = mathutils.Color((0.208,
                                                  0.208,
                                                  0.208))
            mat.specular_shader = "COOKTORR"
            mat.specular_intensity = 0.5
            mat.alpha = 1.0
            mat.ambient = 1.0
        else:
            mat = bpy.data.materials["joint"]

        joint.active_material = mat

        # check whether 'axis1' is valid and the type is not 'fixed'
        if axis1.length_squared < EPSILON and type != 6:
            print("ERROR! Cannot create joint \'%s\' without axis1" % name)
            #TODO: remove created cylinder
            return False

        # find the corresponding two nodes
        node1 = None
        node2 = None

        for tmp in nodeList:
            # check for thr right "ids"
            if tmp["id"] == nodeIndex1:
                node1 = tmp
            if tmp["id"] == nodeIndex2:
                node2 = tmp

        # determine the anchor position of the joint
        if anchorPos == 1: # "node1"
            joint.location = node1.location
        elif anchorPos == 2: # "node2"
            joint.location = node2.location
        elif anchorPos == 3: # "center"
            joint.location = (node1.location + node2.location) / 2.0
        elif anchorPos == 4: # "custom"
            joint.location = anchor
        else:
            #TODO: What position should be set in this case?
            print("WARNING! Wrong anchor position for joint \'%s\'" % name)

        # set the orientation of the joint
        z_axis = mathutils.Vector((0.0,0.0,1.0))
        joint.rotation_mode = "QUATERNION"
        joint.rotation_quaternion = z_axis.rotation_difference(axis1)

        # setting up the node hierarchy (between parent and child node)
        if node1 and node2:
            # set the parent-child relationship
            setParentChild(node1,node2)

        # setting up the node hierarchy (between parent node and joint helper)
        if node1:
            # set the parent-child relationship
            setParentChild(node1,joint)

        # store the pointer to the second joint node as custom property
        if node2:
            joint["node2"] = node2.name

    # if it is a 'fixed' joint, we don't create a helper object, we just create
    # the parent-child-relationship and set the groupID of both objects accordingly
    else:
        # find the corresponding two nodes
        node1 = None
        node2 = None

        for tmp in nodeList:
            # check for thr right "ids"
            if tmp["id"] == nodeIndex1:
                node1 = tmp
            if tmp["id"] == nodeIndex2:
                node2 = tmp

        # setting up the node hierarchy (between parent and child node)
        if node1 and node2:
            # set the parent-child relationship
            setParentChild(node1,node2)

            # check whether the groupID of both nodes differ (highly probable,
            # if used in combination with a joint!)
            if node1["group"] != node2["group"]:
                # some helper variables for the groupIDs of node1 and node2
                groupID1 = node1["group"]
                groupID2 = node2["group"]
                # see if there are other nodes with the same groupID as the child
                for tmp in nodeList:
                    if tmp["group"] == groupID2:
                        tmp["group"] = groupID1

    return True


def checkGroupIDs():
    print("# Checking group IDs")
    
    # check all nodes ...
    for node1 in nodeList:
        # objects with group ID zero are ignored because they are handled
        # seperately by MARS (not as one object consisting of multiple nodes)
        if node1["group"] == 0:
            continue
        
        # put all nodes with the same group ID together in a list
        group = []
        for node2 in nodeList:
            # check for matching group IDs
            if node2["group"] == node1["group"]:
                group.append(node2)

        # if there are other nodes with the same group ID
        if len(group) > 1:
            # see if the nodes with the same group ID are either parents
            # or children of a joint
            parents  = []
            children = []
            for node2 in group:
                for joint in jointList:
                    if joint["node2"] == node2.name and node2 not in children:
                        children.append(node2)
                    if joint.parent == node2 and node2 not in parents:
                        parents.append(node2)

            # if there is only one child, we use this as parent for the whole
            # group (would make sense)
            if len(children) == 1:
                # set the parent-child relationship
                node2 = children[0]
                setParentChild(node2,node1)
            # if there are no children, we use the "first" parent as "group parent"
            elif len(children) == 0 and len(parents) > 0:
                # set the parent-child relationship
                node2 = parents[0]
                setParentChild(node2,node1)
            else:
                print("WARNING! Unable to set parent-child relationship for node <%s> while checking group IDs!" % node1.name)


def main(fileDir, filename):
    tmpDir = os.path.join(fileDir, "tmp")

    # change the current directory
    if os.path.isdir(fileDir) :
        os.chdir(fileDir)
    else:
        print("ERROR! File path (%s) does not exist!" % fileDir)
        sys.exit(1)

    # if there is already a "tmp" directory delete it
    shutil.rmtree(tmpDir, ignore_errors=True)
    
    # extract the .scn file to the "tmp" directory
    zipfile.ZipFile(os.path.join(fileDir, filename), "r").extractall(tmpDir)

    # path to the .scene file
    scenepath = os.path.join(tmpDir, filename.replace('.scn', '.scene'))

    # if the .scene file exists, read all of its content
    if os.path.isfile(scenepath) :
        dom = xml.dom.minidom.parse(scenepath)
    else:
        print("ERROR! Couldn't find .scene file (%s)!" % scenepath)
        sys.exit(1)

    # --- DO THE PARSING HERE!!! ---

    # before we start, wipe the plate clean
    cleanUpScene()

    # clean up all the unnecessary white spaces
    removeWhitespaceNodes(dom)

    # parsing all materials
    materials = dom.getElementsByTagName("material")
    for material in materials :
        if not parseMaterial(material):
            print("Error while parsing material!")
            sys.exit(1)

    # parsing all nodes
    nodes = dom.getElementsByTagName("node")
    for node in nodes :
        if not parseNode(node, tmpDir):
            print("Error while parsing node!")
            sys.exit(1)

    # clean up if some unused meshes were loaded
    if len(unusedNodeList) > 0:
        print("WARNING! Not all imported meshes are in use!")
        #TODO: remove unneeded objects/meshes
        pass

    # parsing all joints
    joints = dom.getElementsByTagName("joint")
    for joint in joints :
        if not parseJoint(joint):
            print("Error while parsing joint!")
            sys.exit(1)

    # check for nodes with the same group ID
    checkGroupIDs()

    #cleaning up afterwards
    shutil.rmtree(tmpDir)


if __name__ == '__main__' :
    if len(sys.argv) != 2:
        print("USAGE: %s <fullpath_to_scn_file>" % sys.argv[0])
        sys.exit(1)

    if not os.path.isfile(sys.argv[1]):
        print('USAGE: %s <fullpath_to_scn_file>' % sys.argv[0])
        print('  Error: "%s" is not an existing file!' % sys.argv[1])
        sys.exit(1)

    fileDirectory, filename = os.path.split(sys.argv[1])

    main(fileDirectory, filename)

