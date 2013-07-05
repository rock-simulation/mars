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

# global list of nodes and joints
nodeList = []
jointList = []

# global list of imported but not used meshes
unusedNodeList = []

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
    if len(bpy.context.selected_objects) > 0:
        bpy.ops.object.select_all()

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
    if len(bpy.context.selected_objects) > 0:
        bpy.ops.object.select_all()

    # select the node/object
    node.select = True

    # set the parent to be the currently active object
    bpy.context.scene.objects.active = node

    # set the origin of the mesh to the center of its
    # bounding box
    bpy.ops.object.origin_set(type="ORIGIN_GEOMETRY",
                              center="BOUNDS")

    return True


def parseNode(domElement, tmpDir):
    # read the config from the xml file
    config = getGenericConfig(domElement)

    check = False
    massDensity = False
    needMass = True

    # handle node name
    if not checkConfigParameter(config,"name"):
        return False
    name = config["name"]

    print("# Creating node <%s>" % name)

#    print("%s : %s" % (config["name"], config))

    # handle node mass
    if checkConfigParameter(config,"mass"):
        mass = float(config["mass"])
        # use some epsilon here
        if abs(mass) > 0.000000001:
            check = True

    if checkConfigParameter(config,"density"):
        density = float(config["density"])
        # use some epsilon here
        if abs(density) > 0.000000001:
            if check:
                massDensity = True;
            check = True;

    if checkConfigParameter(config,"noPhysical"):
        noPhysical = (config["noPhysical"] == "true")
        if noPhysical:
            needMass = False

    if checkConfigParameter(config,"movable"):
        movable = (config["movable"] == "true")
        if not movable:
            needMass = False

    if needMass:
        if not check:
            print("WARNING! No mass nor density given for node %s." % name)
        elif massDensity:
            print("WARNING! Mass and density given for node %s. Using only the mass." % name)
            density = 0.0

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

    if checkConfigParameter(config,"groupid"):
        groupID = int(config["groupid"])
    else:
        groupID = 0

    if checkConfigParameter(config,"index"):
        index = int(config["index"])

    if checkConfigParameter(config,"position"):
        position = config["position"]

    if checkConfigParameter(config,"pivot"):
        pivot = config["pivot"]

    if checkConfigParameter(config,"rotation"):
        rotation = config["rotation"]

    if checkConfigParameter(config,"extend"):
        ext = config["extend"]

    # handle relatvie positioning
    if checkConfigParameter(config,"relativeid"):
        relative_id = int(config["relativeid"])
        if relative_id:
            if checkConfigParamter(config,"mapIndex"):
                mapIndex = int(config["mapIndex"])
            #TODO: Do we need this?
            #if mapIndex and loadScene:
            #    relative_id = loadScene->getMappedID(relative_id, MAP_TYPE_NODE, mapIndex);

    # handle terrain info
    if checkConfigParameter(config,"t_srcname"):
      terrain = {}
      terrain["srcname"] = config["t_srcname"]

      if checkConfigParameter(config,"t_width"):
          terrain["targetWidth"] = float(config["t_width"])

      if checkConfigParameter(config,"t_height"):
          terrain["targetHeight"] = float(config["t_height"])

      if checkConfigParameter(config,"t_scale"):
          terrain["scale"] = float(config["t_scale"])

      if checkConfigParameter(config,"t_tex_scale"):
          terrain["texScale"] = float(config["t_tex_scale"])

    if checkConfigParameter(config, "visualposition"):
        visual_position = config["visualposition"]

    if checkConfigParameter(config, "visualrotation"):
        visual_rotation = config["visualrotation"]

    if checkConfigParameter(config,"visualsize"):
        visual_size = config["visualsize"]
    else:
      visual_size = ext

    # handle contact info
    c_params = {}

    if checkConfigParameter(config,"cmax_num_contacts"):
        c_params["max_num_contacts"] = float(config["cmax_num_contacts"])
        
    if checkConfigParameter(config,"cerp"):
        c_params["erp"] = float(config["cerp"])
        
    if checkConfigParameter(config,"ccfm"):
        c_params["cfm"] = float(config["ccfm"])

    if checkConfigParameter(config,"cfriction1"):
        c_params["friction1"] = float(config["cfriction1"])

    if checkConfigParameter(config,"cfriction2"):
        c_params["friction2"] = float(config["cfriction2"])

    if checkConfigParameter(config,"cmotion1"):
        c_params["motion1"] = float(config["cmotion1"])

    if checkConfigParameter(config,"cmotion2"):
        c_params["motion2"] = float(config["cmotion2"])

    if checkConfigParameter(config,"cfds1"):
        c_params["fds1"] = float(config["cfds1"])

    if checkConfigParameter(config,"cfds2"):
        c_params["fds2"] = float(config["cfds2"])

    if checkConfigParameter(config,"cbounce"):
        c_params["bounce"] = float(config["cbounce"])

    if checkConfigParameter(config,"cbounce_vel"):
        c_params["bounce_vel"] = float(config["cbounce_vel"])

    if checkConfigParameter(config,"capprox"):
        c_params["approx_pyramid"] = (config["capprox"] == "true")

    if checkConfigParameter(config,"coll_bitmask"):
        c_params["coll_bitmask"] = int(config["coll_bitmask"])

    if checkConfigParameter(config,"cfdir1"):
        c_params["friction_direction1"] = config["cfdir1"]

    if checkConfigParameter(config,"inertia"):
        inertia_set = (config["inertia"] == "true")

    inertia = [[0.0 for x in range(3)] for x in range(3)]

    if checkConfigParameter(config,"i00"):
        inertia[0][0] = float(config["i00"])

    if checkConfigParameter(config,"i01"):
        inertia[0][1] = float(config["i01"])

    if checkConfigParameter(config,"i02"):
         inertia[0][2] = float(config["i02"])

    if checkConfigParameter(config,"i10"):
         inertia[1][0] = float(config["i10"])

    if checkConfigParameter(config,"i11"):
         inertia[1][1] = float(config["i11"])

    if checkConfigParameter(config,"i12"):
         inertia[1][2] = float(config["i12"])

    if checkConfigParameter(config,"i20"):
         inertia[2][0] = float(config["i20"])

    if checkConfigParameter(config,"i21"):
         inertia[2][1] = float(config["i21"])

    if checkConfigParameter(config,"i22"):
         inertia[2][2] = float(config["i22"])

    if checkConfigParameter(config,"linear_damping"):
         linear_damping = float(config["linear_damping"])

    if checkConfigParameter(config,"angular_damping"):
         angular_damping = float(config["angular_damping"])

    if checkConfigParameter(config,"angular_low"):
         angular_low = float(config["angular_low"])

    if checkConfigParameter(config,"shadow_id"):
         shadow_id = int(config["shadow_id"])

    if checkConfigParameter(config,"shadowcaster"):
         isShadowCaster = (config["shadowcaster"] == "true")

    if checkConfigParameter(config,"shadowreceiver"):
         isShadowReceiver= (config["shadowreceiver"] == "true")

    #TODO: Do we need this?
    #if(!filenamePrefix.empty()) { 
    #   if(filename != "PRIMITIVE")
    #     handleFilenamePrefix(&filename, filenamePrefix);
    #   if(terrain) {
    #     handleFilenamePrefix(&terrain->srcname, filenamePrefix);
    #   }
    # }

    ######## LOAD THE NODE IN BLENDER ########

    # "pointer" to the newly created object
    node = None

    if filename == "PRIMITIVE":

        if typeName == "box":
            # create a new box as representation of the node
            bpy.ops.mesh.primitive_cube_add()
            # get the "pointer" to the new node
            for obj in bpy.data.objects:
                if obj.name == "Cube":
                    # set the name of the object
                    node = obj
                    # set the size of the cube
                    node.dimensions = ext

        elif typeName == "sphere":
            # create a new sphere as representation of the node
            bpy.ops.mesh.primitive_uv_sphere_add(size = ext.x)
            # get the "pointer" to the new node
            for obj in bpy.data.objects:
                if obj.name == "Sphere":
                    # set the name of the object
                    node = obj

        elif typeName == "reference":
            # TODO: is that really needed?
            print("Warning! Unhandled node type \'reference\'.")

        elif typeName == "mesh":
            # TODO: is that really needed?
            print("Warning! Unhandled node type \'mesh\'.")

        elif typeName == "cylinder":
            # create a new cylinder as representation of the node
            bpy.ops.mesh.primitive_cylinder_add(radius = ext.x, depth = ext.y)
            # get the "pointer" to the new node
            for obj in bpy.data.objects:
                if obj.name == "Cylinder":
                    # set the name of the object
                    node = obj

        elif typeName == "capsule":
            print("Warning! Node type \'capsule\' yet supported, using \'cylinder\' instead.")
            # create a new cylinder as representation of the node
            bpy.ops.mesh.primitive_cylinder_add(radius = ext.x, depth = ext.y)
            # get the "pointer" to the new node
            for obj in bpy.data.objects:
                if obj.name == "Cylinder":
                    # set the name of the object
                    node = obj

        elif typeName == "plane":
            # create a new plane as representation of the node
            bpy.ops.mesh.primitive_plane_add()
            # get the "pointer" to the new node
            for obj in bpy.data.objects:
                if obj.name == "Plane":
                    # set the name of the object
                    node = obj
                    # set the size of the cube
                    node.dimensions = ext

        else:
            print("Cannot find primitive type: %s" % origName) 

    elif physicMode == "terrain":
        # TODO: Creating terrain in Blender ...
        print("Warning! \'Terrain\' nodes are not handled right now!")

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
            # store the names of all objects before importing
            old_object_list = bpy.data.objects.keys()

            # import the .obj file
            bpy.ops.import_scene.obj(filepath=tmpDir+os.sep+filename)

            # store the names of all objects after importing
            new_object_list = bpy.data.objects.keys()

            # if there were added multiple meshes from one .obj file
            if len(new_object_list) - len(old_object_list) > 1:
                # put the newly imported objects into the "unused" list
                for nodeName in new_object_list:
                    if nodeName not in old_object_list:
                        # add the name of the imported object to the list of
                        # unused nodes
                        unusedNodeList.append(nodeName)

                        # center the origin of the loaded node to the center
                        # of its bounding box
                        tmp = bpy.data.objects[nodeName]
                        centerNodeOrigin(tmp)

                    # get the currently added object
                    if filename in nodeName or name in nodeName:
                        node = bpy.data.objects[nodeName]
                        # remove the current object from the unused list
                        unusedNodeList.remove(nodeName)

            # if there was added just one mesh
            else:
                # get the currently added object
                for nodeName in new_object_list:
                    if filename in nodeName or name in nodeName:
                        node = bpy.data.objects[nodeName]

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

        # add each item of 'config' as a custom property to the node
        for (key, value) in config.items():
            node[key] = value

        # store the index of the node as custom property
        node["id"] = index

        # store the group index as custom property
        node["group"] = groupID

        # set the object type to be a node
        node["type"] = "body"

        # set the position of the object
        node.location = position + rotation * visual_position

        # set the rotation of the object
        # TODO: why do we need this offset rotation?!?
        rotation_offset = mathutils.Euler((math.pi/2.0, 0.0, 0.0)).to_quaternion()

        node.rotation_mode = "QUATERNION"
        node.rotation_quaternion = rotation * visual_rotation * rotation_offset

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

    nodeIndex1 = None
    nodeIndex2 = None

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
        index = int(config["index"])

    if checkConfigParameter(config,"nodeindex1"):
        nodeIndex1 = int(config["nodeindex1"])
        if not nodeIndex1:
            print("JointData: no first node attached to joint \'%s\'" % name);

    if checkConfigParameter(config,"nodeindex2"):
        nodeIndex2 = int(config["nodeindex2"])

    # handle axis 1
    if checkConfigParameter(config,"axis1"):
        axis1 = config["axis1"]

    if checkConfigParameter(config,"lowStopAxis1"):
        lowStopAxis1 = float(config["lowStopAxis1"])

    if checkConfigParameter(config,"highStopAxis1"):
        highStopAxis1 = float(config["highStopAxis1"])

    if checkConfigParameter(config,"damping_const_constraint_axis1"):
        damping_const_constraint_axis1 = float(config["damping_const_constraint_axis1"])

    if checkConfigParameter(config,"spring_const_constraint_axis1"):
        spring_const_constraint_axis1 = float(config["spring_const_constraint_axis1"])

    if checkConfigParameter(config,"angle1_offset"):
        angle1_offset = float(config["angle1_offset"])

    # handle axis 2
    if checkConfigParameter(config,"axis2"):
        axis2 = config["axis2"]

    if checkConfigParameter(config,"lowStopAxis2"):
        lowStopAxis2 = float(config["lowStopAxis2"])

    if checkConfigParameter(config,"highStopAxis2"):
        highStopAxis2 = float(config["highStopAxis2"])

    if checkConfigParameter(config,"damping_const_constraint_axis2"):
        damping_const_constraint_axis2 = float(config["damping_const_constraint_axis2"])

    if checkConfigParameter(config,"spring_const_constraint_axis2"):
        spring_const_constraint_axis2 = float(config["spring_const_constraint_axis2"])

    if checkConfigParameter(config,"angle2_offset"):
        angle2_offset = float(config["angle2_offset"])

    if checkConfigParameter(config,"anchorpos"):
        anchorPos = int(config["anchorpos"])

    if checkConfigParameter(config,"anchor"):
        anchor = config["anchor"]


    ######## LOAD THE JOINT IN BLENDER ########

    if type != jointTypes.index("fixed"):

        # "pointer" to the newly created object
        joint = None

        # create a new cylinder as representation of the joint        
        bpy.ops.mesh.primitive_cylinder_add(radius=0.01, depth=0.2)

        for obj in bpy.data.objects:
            if obj.name == "Cylinder":
                joint = obj

        if joint:
            # add the node to the global node list
            jointList.append(joint)

            # set the name of the object
            joint.name = name

            # add each item of 'config' as a custom property to the joint
            for (key, value) in config.items():
                joint[key] = value

            # store the index of the joint as custom property
            joint["id"] = index

            # set the object type to be a joint
            joint["type"] = "joint"

            # set the color of the joint helper object to green
            if "green" not in bpy.data.materials:
                # create new "green" material
                mat = bpy.data.materials.new("green")
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
                mat = bpy.data.materials["green"]

            joint.data.materials.append(mat)

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

            if len(children) == 1:
                # set the parent-child relationship
                node2 = children[0]
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

    # clean up all the unnecessary white spaces
    removeWhitespaceNodes(dom)

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

