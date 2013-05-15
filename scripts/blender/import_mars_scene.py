import os
import sys
import math
import shutil
import zipfile
import textwrap

import xml.dom.minidom

import bpy
import mathutils

# allowed node types
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
    if (not child):
        return None
    # if it is a text node, we just return the contained string
    elif (child.nodeType == xml.dom.minidom.Node.TEXT_NODE):
        return child.nodeValue

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
 #           print("element [%s : %s]" % (key, value))
            if key not in config:
                config[key] = value
            else:
                print("Warning! Key '%s' already exists!" % child.tagName)
        child = child.nextSibling

    return config


def checkConfigParameter(config, key):
    if key not in config.keys():
        print("WARNING! Node config does not contain parameter \'%s\'!" % key)
        return False
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
        typeName = config["physicmode"].strip()
        if typeName in nodeTypes:
            physicMode = nodeTypes.index(typeName)
        else:
            print("ERROR! Could not get type for node: %s" % name)

    if checkConfigParameter(config,"origname"):
        origName = config["origname"].strip()

    if checkConfigParameter(config,"filename"):
        filename = config["filename"].strip()

    if filename == "PRIMITIVE":
        if not origName:
            origName = nodeTypes[physicMode]
        elif origName != nodeTypes[physicMode]:
            tmp = nodeTypes[physicMode]
            print("WARNING! Origname set to \"%s\" for primitive in node \"%s\" with physicMode \"%s\"" % (origName, name, tmp))

    if checkConfigParameter(config,"groupid"):
        groupID = int(config["groupid"])

    if checkConfigParameter(config,"index"):
        index = int(config["index"])

    if checkConfigParameter(config,"position"):
        pos = config["position"]

    if checkConfigParameter(config,"pivot"):
        pivot = config["pivot"]

    if checkConfigParameter(config,"rotation"):
        rot = config["rotation"]

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
        visual_offset_pos = config["visualposition"]

    if checkConfigParameter(config, "visualrotation"):
        visual_offset_rot = config["visualrotation"]

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

    if filename != "PRIMITIVE":
        # import the respective .obj file
        bpy.ops.import_scene.obj(filepath=tmpDir+os.sep+filename)
        
        # rename the currently loaded object
        for obj in bpy.data.objects:
            if filename in obj.name:
                # set the name of the object
                obj.name = name

                # set the size of the object
                print("visualsize = %s" % visual_size)
                obj.dimensions = mathutils.Vector((float(visual_size['x']),\
                                                   float(visual_size['z']),\
                                                   float(visual_size['y'])))

                # set the position of the object
                print("pos = %s" % pos)
                position = mathutils.Vector((float(pos["x"]),\
                                             float(pos["y"]),\
                                             float(pos["z"])))

                print("rot = %s" % rot)
                rotation = mathutils.Quaternion((float(rot["w"]),\
                                                 float(rot["x"]),\
                                                 float(rot["y"]),\
                                                 float(rot["z"])))

                print("visual_offset_pos = %s" % visual_offset_pos)
                visual_position = mathutils.Vector((float(visual_offset_pos["x"]),\
                                                    float(visual_offset_pos["y"]),\
                                                    float(visual_offset_pos["z"])))

                obj.location = position + rotation * visual_position

                # set the rotation of the object
                print("rot = %s" % rot)
                rotation = mathutils.Quaternion((float(rot["w"]),\
                                                 float(rot["x"]),\
                                                 float(rot["y"]),\
                                                 float(rot["z"])))

                print("visual_offset_rot = %s" % visual_offset_rot)
                visual_rotation = mathutils.Quaternion((float(visual_offset_rot["w"]),\
                                                        float(visual_offset_rot["x"]),\
                                                        float(visual_offset_rot["y"]),\
                                                        float(visual_offset_rot["z"])))

                # TODO: why do we need this offset rotation?!?
                rotation_offset = mathutils.Euler((math.pi/2.0, 0.0, 0.0)).to_quaternion()

                obj.rotation_mode = "QUATERNION"
                obj.rotation_quaternion = rotation * visual_rotation * rotation_offset

    return True


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

