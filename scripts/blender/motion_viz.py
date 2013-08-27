import os
import sys
import math
#import numpy
import time

import bpy
import mathutils

# get the filepath for the currently running script # (WARNING! It
# is problematic if another version of the "import_mars_scene" or
# the "motion_viz" script are "internal" available; you can check
# that by looking into "bpy.data.texts", if there are multiple
# versions of one of those scripts, simply delete all versions and
# load the script anew!)
scriptPath = bpy.data.texts['motion_viz.py'].filepath
# split the path into file name and file directory
scriptDir, scriptFile = os.path.split(scriptPath)
# append the directory to the current system path
sys.path.append(scriptDir)
# import the "import_mars_scene" (should be in the same directory
# as this script!)
import import_mars_scene

# global list of nodes
nodeList = []

# global list of parentnodes
parentList = {}

# global list of childrennodes
childrenList = {}

# mapping dict for markernames to nodenames
mappingMarkerToNode = {}

# mapping dict for nodenames to markernames
mappingNodeToMarker = {}

# gloabal list of inactive nodes
inactiveList = []

# parser for the yaml-Files
cfg = None

# parser for the motion-Files
mvp = None

# boolean to check if there is a new config file
newConfig = False

# the positions in experimentfile are to huge so we have to scale them down
scale = 0.001


# loads an experiment from a yaml-file
def loadExperiment(filename):
    global cfg

    # create the yaml-parser object and parse the file
    cfg = configFileParser(filename)

    group = "mviz"
    paramName = "motionfile"
    value = [""]
    # check if the config-file has an entry for the experimentdatafile
    if cfg.getOrCreateProperty(group,paramName,value):
        motionFile = value[0]
        print("motionFile: "+str(motionFile))
        motionFileSave = motionFile
        scene = bpy.context.scene
        
        global newConfig
        # if we have a new ymal-file we have to clean up the scene
        if newConfig:
            cleanUpScene()
        
        # check if the experimentfilename has changed    
        if hasattr(bpy.context.scene,"Experimentfile") and not newConfig:
            if motionFile != scene.Experimentfile:
                cfg.setPropertyValue(group,paramName,scene.Experimentfile)
                motionFile = scene.Experimentfile
        else:
            bpy.types.Scene.Experimentfile = bpy.props.StringProperty(name="Experimentfile", default=motionFile)

        # check if the scenefilename has changed
        paramName = "scenefile"
        sceneFile = ""
        value.clear()
        value.append("")
        if cfg.getOrCreateProperty(group,paramName,value):
            sceneFile = value[0]    
        if hasattr(bpy.context.scene,"MarsScene") and not newConfig:
            if sceneFile != scene.MarsScene:
                cfg.setPropertyValue(group,paramName,scene.MarsScene)
                sceneFile = scene.MarsScene
        else:
            bpy.types.Scene.MarsScene = bpy.props.StringProperty(name="MarsScene", default=sceneFile)
            
        newConfig = False

        # create the parser for the experimentfiles and parse the experimentfile
        global mvp
        mvp = motion_viz_parser(motionFile)
        markerEnumItems = []

        if hasattr(bpy.context.scene,"NodeProp") and \
           len(scene.NodeProp) != len(mvp.markerNames):
            # clear all Lists if there not as many properties as marker
            clearLists()
            # selected wrong experimentfile for the current configfile
            if len(scene.NodeProp) > 0:
                paramName = "motionfile"
                cfg.setPropertyValue(group,paramName,motionFileSave)
                scene.Experimentfile = motionFileSave
                cleanUpScene()
                print("The selected Experimentfile doesnt work with this configfile")
                return False

        # go through all markers and get the node properties
        for marker in mvp.markerNames:
            markerEnumItems.append((marker,marker,marker))
            nodeParam = ""
            remove = False
            
            # if there are already custom properties check if the config-file has to be updated
            if hasattr(bpy.context.scene,"NodeProp") and (len(scene.NodeProp) == len(mvp.markerNames)):
                nodeParam = scene.NodeProp[getNodeIndex(marker)]

                paramName = marker+"/parent"
                value.clear()
                value.append("")
                cfg.getOrCreateProperty(group,paramName,value)
                parentList[marker] = nodeParam.parent
                if value[0] != nodeParam.parent and (value[0] != "" or nodeParam.parent != "<none>"):
                    cfg.setPropertyValue(group,paramName,nodeParam.parent)
                    remove = True

                paramName = marker+"/child"
                value.clear()
                value.append("")
                cfg.getOrCreateProperty(group,paramName,value)
                childrenList[marker] = nodeParam.child
                if value[0] != nodeParam.child and (value[0] != "" or nodeParam.child != "<none>"):
                    cfg.setPropertyValue(group,paramName,nodeParam.child)
                    remove = True
                    
                paramName = marker+"/mapping"
                value.clear()
                value.append("")
                cfg.getOrCreateProperty(group,paramName,value)
                mappingMarkerToNode[marker] = nodeParam.mapping  
                mappingNodeToMarker[nodeParam.mapping] = marker 
                if value[0] != nodeParam.mapping:
                    cfg.setPropertyValue(group,paramName,nodeParam.mapping)
                    remove = True

                paramName = marker+"/visualoffset"
                value.clear()
                value.append("0.0,0.0,0.0")
                cfg.getOrCreateProperty(group,paramName,value)
                if value[0] != nodeParam.visualOffset:
                    cfg.setPropertyValue(group,paramName,nodeParam.visualOffset)
                    remove = True

                paramName = marker+"/active"
                value.clear()
                value.append(True)
                cfg.getOrCreateProperty(group,paramName,value)
                if value[0] != nodeParam.active:
                    cfg.setPropertyValue(group,paramName,nodeParam.active)
                    remove = True
                if nodeParam.active:
                    if marker in inactiveList:
                        inactiveList.remove(marker)
                else:
                    inactiveList.append(marker)
            else:
                paramName = marker+"/parent"
                value.clear()
                value.append("")
                if cfg.getOrCreateProperty(group,paramName,value):
                    if value[0] != "":
                        parentList[marker] = value[0]
                    else:
                        parentList[marker] = "<none>"
                        
                paramName = marker+"/child"
                value.clear()
                value.append("")
                if cfg.getOrCreateProperty(group,paramName,value):
                    if value[0] != "":
                        childrenList[marker] = value[0]
                    else:
                        childrenList[marker] = "<none>"
                        
                paramName = marker+"/visualoffset"
                value.clear()
                value.append("0.0,0.0,0.0")
                cfg.getOrCreateProperty(group,paramName,value)

                paramName = marker+"/mapping"
                value.clear()
                value.append("")
                if cfg.getOrCreateProperty(group,paramName,value):
                    if value[0] != "":
                        mappingMarkerToNode[marker] = value[0]  
                        mappingNodeToMarker[value[0]] = marker 
                    else:
                        mappingMarkerToNode[marker] = marker
                        mappingNodeToMarker[marker] = marker

                paramName = marker+"/active"
                value.clear()
                value.append(True)
                if cfg.getOrCreateProperty(group,paramName,value):
                    if not bool(value[0]):
                        inactiveList.append(marker)
            # if some of the properties has changed the old node has to be removed and a new created        
            if remove:
                removeObject(marker)
                    
        # search for object which have to be removed (unused nodes)
        removeOldObjects()

        # add an item for no object
        markerEnumItems.append(("<none>","<none>","<none>"))
        # add the marker names to the dropdownboxes
        if len(bpy.types.NodeProperties.parent[1]["items"]) != len(markerEnumItems):
            bpy.types.NodeProperties.parent = bpy.props.EnumProperty(items=markerEnumItems)
        if len(bpy.types.NodeProperties.child[1]["items"]) != len(markerEnumItems):
            bpy.types.NodeProperties.child = bpy.props.EnumProperty(items=markerEnumItems)
        return True
    else:
        return False


# load mars scene
def loadMarsScene(SceneFile):
    if SceneFile == "":
        return False
    nodeList.clear()
    path,file = os.path.split(SceneFile)
    try:
        import_mars_scene.main(path, file)
    except:
        print("Scene import error")


# create helper objects for each marker in blender
def createHelperObjects():
    
    if not hasattr(bpy.context.scene,"NodeProp"):
        bpy.types.Scene.NodeProp = bpy.props.CollectionProperty(type=NodeProperties)
    for marker in mvp.markerNames:
        if marker in bpy.data.objects:
            continue
        group = "mviz"
        paramName = marker+"/mapping"
        value = []
         
        cfg.getPropertyValue(group,paramName,value)
        if len(value) > 0 and value[0] in bpy.data.objects:
            nodeList.append(bpy.data.objects[value[0]])
        else:
            # if the marker has a parent- or child-node the helperobject has to be a cylinder 
            if (marker in parentList and parentList[marker] != "<none>") or \
               (marker in childrenList and childrenList[marker] != "<none>"):
                ext = mathutils.Vector()
                # the radius the cylinder should have
                ext.x = 10*scale
                # the height the cylinder should have
                ext.y = 1
                
                # compute the rotation-quaternion for the cylinder so the cylinder points to
                # the parent- or child-node
                
                # get the startposition of the marker
                currVec = mvp.markerPositions[marker][0]
                # initialize the variables for the rotation and position
                rotation = mathutils.Quaternion()
                pos = mathutils.Vector()
                # if the marker has got a child the cylinder should look to the child-node
                if marker in childrenList and childrenList[marker] != "<none>":
                    childVec = mvp.markerPositions[mappingNodeToMarker[childrenList[marker]]][0]
                    # get the vector which points in the direction the cylinder should point to
#                    dirrVec = currVec - childVec
                    dirrVec = [a - b for a, b in zip(currVec,childVec)]
                    # get the rotation-quaternion
                    rotation = quatFromTwoVectors(mathutils.Vector((0.0,0.0,1.0)),dirrVec)
                    # move the cylinder have the way of the direction-vector so everything sticks together
#                    pos = mvp.markerPositions[marker][0]-0.5*dirrVec
                    pos = [a - 0.5*b for a, b in zip(mvp.markerPositions[marker][0],dirrVec)]
                else:
                    parentVec = mvp.markerPositions[mappingNodeToMarker[parentList[marker]]][0]
                    # get the vector which points in the direction the cylinder should point to
#                    dirrVec = parentVec - currVec
                    dirrVec = [a - b for a, b in zip(parentVec,currVec)]
                    # get the rotation-quaternion
                    rotation = quatFromTwoVectors(mathutils.Vector((0.0,0.0,1.0)),dirrVec)
                    # move the cylinder have the way of the direction-vector so everything sticks together
#                    pos = mvp.markerPositions[marker][0]+0.5*dirrVec
                    pos = [a + 0.5*b for a, b in zip(mvp.markerPositions[marker][0],dirrVec)]

                addVisualOffset(marker,pos)

                # create the cylinder in blender
                createPrimitive(mappingMarkerToNode[marker],"cylinder",pos,ext)
                # set the rotation and the height of the new cylinder
                node = nodeList[len(nodeList)-1]
                node.rotation_mode = "QUATERNION"
#                node.scale.z = numpy.linalg.norm(dirrVec)
                node.scale.z = math.sqrt(sum(x*x for x in dirrVec))
                node.rotation_quaternion = rotation
            else:
                # if the marker has no parents or children the helperobject should be a sphere
                ext = mathutils.Vector()
                ext.x = 10*scale
                pos = mvp.markerPositions[marker][0]
                addVisualOffset(marker,pos)
                createPrimitive(mappingMarkerToNode[marker],"sphere",pos,ext)
                
        # create custom property
        createCustomProperty(marker)

    return True


# animate the helperobjects with the information of the experimentdata            
def createAnimation():
    # get the fps
    fps = float(bpy.context.scene.render.fps)
    # get the total count of frames
#    bpy.context.scene.frame_end = mvp.times[len(mvp.times)-1]*fps
    print("###")
    print(mvp.times)
    print(len(mvp.times))
    print(mvp.times[-1])
    print("###")
    bpy.context.scene.frame_end = mvp.times[len(mvp.times)-1]*fps
    currIndex = 0
    # delete all old Keyframes
    for node in nodeList:
        node.animation_data_clear()

    for frame in range(bpy.context.scene.frame_end):
        # get how much times passed
        currTime = frame/fps
        timeDiff = 1000.0 
        # search for the right time in the experimentdata
        while math.fabs(mvp.times[currIndex]-currTime) < timeDiff and \
              currIndex < len(mvp.times):
            timeDiff = math.fabs(mvp.times[currIndex]-currTime)
            currIndex += 1
        # set the scene frame to our currentframe
        bpy.context.scene.frame_set(frame)
        # go throug all nodes(marker) and set postion, rotation and height for the new time
        for node in nodeList:
            if node.name in inactiveList:
                continue
            if (node.name in parentList and parentList[node.name] != "<none>") or \
               (node.name in childrenList and childrenList[node.name] != "<none>"):
                # if the node is has a parent or child node we have to update the rotation and height
                currVec = mvp.markerPositions[mappingNodeToMarker[node.name]][currIndex]
                rotation = mathutils.Quaternion()
#                pos = numpy.array([0.0,0.0,0.0])
                pos = mathutils.Vector()
                # if the marker has got a child the cylinder should look to the child-node
                if node.name in childrenList and childrenList[node.name] != "<none>":
                    childVec = mvp.markerPositions[mappingNodeToMarker[childrenList[node.name]]][currIndex]
                    # get the vector which points in the direction the cylinder should point to
#                    dirrVec = currVec - childVec
                    dirrVec = [a - b for a, b in zip(currVec,childVec)]
                    # get the rotation-quaternion
#                    rotation = quatFromTwoVectors(numpy.array([0.0,0.0,1.0]),dirrVec)
                    rotation = quatFromTwoVectors(mathutils.Vector((0.0,0.0,1.0)),dirrVec)
                    # move the cylinder have the way of the direction-vector so everything sticks together
#                    pos = mvp.markerPositions[mappingNodeToMarker[node.name]][currIndex]-0.5*dirrVec
                    pos = [a - 0.5*b for a, b in zip(mvp.markerPositions[mappingNodeToMarker[node.name]][currIndex],dirrVec)]

                else:
                    parentVec = mvp.markerPositions[mappingNodeToMarker[parentList[node.name]]][currIndex]
                    # get the vector which points in the direction the cylinder should point to
#                    dirrVec = parentVec - currVec
                    dirrVec = [a - b for a, b in zip(parentVec,currVec)]
                    # get the rotation-quaternion
#                    rotation = quatFromTwoVectors(numpy.array([0.0,0.0,1.0]),dirrVec)
                    rotation = quatFromTwoVectors(mathutils.Vector((0.0,0.0,1.0)),dirrVec)
                    # move the cylinder have the way of the direction-vector so everything sticks together
#                    pos = mvp.markerPositions[mappingNodeToMarker[node.name]][currIndex]+0.5*dirrVec
                    pos = [a + 0.5*b for a, b in zip(mvp.markerPositions[mappingNodeToMarker[node.name]][currIndex],dirrVec)]

                addVisualOffset(mappingNodeToMarker[node.name],pos)
                # update position, rotation and height
#                node.scale.z = numpy.linalg.norm(dirrVec)
                node.scale.z = math.sqrt(sum(x*x for x in dirrVec))
                node.rotation_quaternion = rotation
                node.location = pos
            else:
                # the node is a sphere -> only update position
                pos = mvp.markerPositions[mappingNodeToMarker[node.name]][currIndex]
                addVisualOffset(mappingNodeToMarker[node.name],pos)

                node.location = pos
            # insert the new keyframes in the timeline
            node.keyframe_insert(data_path='location', frame=(frame))
            node.keyframe_insert(data_path='rotation_quaternion', frame=(frame))
            node.keyframe_insert(data_path='scale', frame=(frame))

    return True


def clearLists():
    nodeList.clear()
    parentList.clear()
    childrenList.clear()
    mappingMarkerToNode.clear()
    mappingNodeToMarker.clear()
    inactiveList.clear()


# correct the position with the visual offset
def addVisualOffset(marker, pos):
    value = []
    if cfg.getPropertyValue("mviz",marker+"/visualoffset",value):
        x,y,z = value[0].split(",")
        pos[0] += float(x)
        pos[1] += float(y)
        pos[2] += float(z)
        return True
    return False


# create a quaternion which describes the rotation from vec1 to vec2
def quatFromTwoVectors(vec1, vec2):
    # initialize the result quaternion
    q = mathutils.Quaternion()
    # get the vector we rotate around (vector that stands upright on the 2 given vectors)
#    a = numpy.cross(vec1, vec2)
    a = mathutils.Vector((vec1[1]*vec2[2] - vec1[2]*vec2[1], \
                          vec1[2]*vec2[0] - vec1[0]*vec2[2], \
                          vec1[0]*vec2[1] - vec1[1]*vec2[0]))
    
    # assign our rotation-vector to our quaternion
    q.x = a[0]
    q.y = a[1]
    q.z = a[2]
    # compute the angle
#    q.w = math.sqrt((numpy.linalg.norm(vec1) ** 2) * (numpy.linalg.norm(vec2) ** 2)) + numpy.dot(vec1, vec2)
    q.w = math.sqrt(sum(x*x for x in vec1) * sum(x*x for x in vec2)) + sum(x1 * x2 for x1, x2 in zip(vec1, vec2))
    # normalize the quaternion
    q.normalize()
    return q


# get node by name
def getNodeIndex(name):
    if not hasattr(bpy.context.scene,"NodeProp"):
        return -1
    for index in range(len(bpy.context.scene.NodeProp)):
        if bpy.context.scene.NodeProp[index].name == name:
            return index

    return -1


# remove all helperobjects from the blenderscene
def removeAllObjects():
    # select all "MESH" objects
    for object in bpy.data.objects:
        if object.type == "MESH":
            bpy.data.objects[object.name].select = True

    # remove all selected
    bpy.ops.object.delete()     
    # remove the meshes, they have no users anymore
    for item in bpy.data.meshes:
        bpy.data.meshes.remove(item)
    # and all materials
    for material in bpy.data.materials:
        bpy.data.materials.remove(material)

    nodeList.clear()

    return True


# remove all unused objects
def removeOldObjects():
    # select the object we have to remove
    for object in bpy.data.objects:
        if object.name not in mvp.markerNames and object.type == "MESH":
            bpy.data.objects[object.name].select = True
            if object in nodeList:
                nodeList.remove(object)
        else:
            bpy.data.objects[object.name].select = False
    # remove all selected
    bpy.ops.object.delete()     
    # remove the meshes, they have no users anymore
    for item in bpy.data.meshes:
        if item.users == 0:
            bpy.data.meshes.remove(item)

    # and all materials
    for material in bpy.data.materials:
        if material.users == 0:
            bpy.data.materials.remove(material)

    return True


# remove the the object with the given name
def removeObject(name):
    # select the object we have to remove
    for object in bpy.data.objects:
        if object.name == name:
            bpy.data.objects[object.name].select = True
            if object in nodeList:
                nodeList.remove(object)
        else:
            bpy.data.objects[object.name].select = False
    # remove all selected
    bpy.ops.object.delete()     

    # remove the meshes, they have no users anymore
    for item in bpy.data.meshes:
        if item.users == 0:
            bpy.data.meshes.remove(item)

    # and all materials
    for material in bpy.data.materials:
        if material.users == 0:
            bpy.data.materials.remove(material)

    return True


def cleanUpScene():
    removeAllObjects()
    if hasattr(bpy.context.scene,"NodeProp"):
        bpy.context.scene.NodeProp.clear()


# create a primitive object in blender
def createPrimitive(name, typeName, pos, extend):
    node = None
    if typeName == "cylinder":
        # create a new cylinder as representation of the node
        bpy.ops.mesh.primitive_cylinder_add(radius = extend.x, depth = extend.y)
        # get the "pointer" to the new node
        node = bpy.context.selected_objects[0]     
    elif typeName == "sphere":
        # create a new sphere as representation of the node
        bpy.ops.mesh.primitive_uv_sphere_add(size = extend.x)
        # get the "pointer" to the new node
        node = bpy.context.selected_objects[0]

    if node:
        # add the node to the global node list
        nodeList.append(node)
        # set the name of the object
        node.name = name
        # set the position of the object
        node.location = pos


def createCustomProperty(marker):
    # create custom property by reading the config-file and putting the information into the property collection
    if getNodeIndex(marker) == -1:
        group = "mviz"
        paramName = marker+"/parent"
        value = []
        nodeP = bpy.context.scene.NodeProp.add()
        nodeP.name = marker
        if cfg.getPropertyValue(group,paramName,value):
            if value[0] != "":
                nodeP.parent = value[0]
            else:
                nodeP.parent = "<none>"
        paramName = marker+"/child"
        if cfg.getPropertyValue(group,paramName,value):
            if value[0] != "":
                nodeP.child = value[0]
            else:
                nodeP.child = "<none>"
        paramName = marker+"/visualoffset"
        if cfg.getPropertyValue(group,paramName,value):
            nodeP.visualOffset = value[0]
        paramName = marker+"/mapping"
        if cfg.getPropertyValue(group,paramName,value):
            nodeP.mapping = value[0]
        paramName = marker+"/active"
        if cfg.getPropertyValue(group,paramName,value):
            nodeP.active = bool(value[0])


# define a nested property
class NodeProperties(bpy.types.PropertyGroup):
    # name of the marker
    name   = bpy.props.StringProperty(name="Name", default="")
    # name of the parent marker
    parent = bpy.props.EnumProperty(items=[])
    # name of the child marker
    child  = bpy.props.EnumProperty(items=[])
    # marker active or not
    active = bpy.props.BoolProperty(name="Active", default=True)
    # name of the node
    mapping= bpy.props.StringProperty(name="Mapping", default="")
    # visual position offset
    visualOffset = bpy.props.StringProperty(name="VisualOffset", default="0.0,0.0,0.0")


# Dialog to select a new config file
class LoadYAMLFileDialog(bpy.types.Operator):
    bl_idname = "load_yaml_file.yaml"
    bl_label = "..."

    # creating property for storing the path to the .yaml file
    filepath = bpy.props.StringProperty(subtype="FILE_PATH")

    # set a filter to only consider .yaml files (only used internally)
    filter_glob = bpy.props.StringProperty(default="*.yaml",options={'HIDDEN'})

    @classmethod
    def poll(cls, context):
        return context is not None

    def execute(self, context):
        main(self.filepath)

        return {'FINISHED'}


    def invoke(self, context, event):
        # create the open file dialog
        context.window_manager.fileselect_add(self)

        return {'RUNNING_MODAL'}


# Property GUI Class
class NodePropertiesPanel(bpy.types.Panel):
    # Button to select a new Marsscene file
    class Browse_SceneFile_Button(bpy.types.Operator):
        bl_idname = "select_mars_scene.scn"
        bl_label = "..."
    
        # creating property for storing the path to the .scn file
        filepath = bpy.props.StringProperty(subtype="FILE_PATH")
    
        # set a filter to only consider .scn files (only used internally)
        filter_glob = bpy.props.StringProperty(default="*.scn",options={'HIDDEN'})
    
        @classmethod
        def poll(cls, context):
            return context is not None
    
        def execute(self, context):
            # get the chosen file path
            context.scene.MarsScene = self.filepath
            main(context.scene.ConfigFile)
            return {'FINISHED'}
    
        def invoke(self, context, event):
            # create the open file dialog
            context.window_manager.fileselect_add(self)
    
            return {'RUNNING_MODAL'}
            
    # Button to select a new Experiment file
    class Browse_ExperimentFile_Button(bpy.types.Operator):
        bl_idname = "select_experiment.tsv"
        bl_label = "..."
    
        # creating property for storing the path to the .tsv file
        filepath = bpy.props.StringProperty(subtype="FILE_PATH")
    
        # set a filter to only consider .tsv files (only used internally)
        filter_glob = bpy.props.StringProperty(default="*.tsv",options={'HIDDEN'})
    
        @classmethod
        def poll(cls, context):
            return context is not None
    
        def execute(self, context):
            # get the chosen file path
            context.scene.Experimentfile = self.filepath
            main(context.scene.ConfigFile)
            return {'FINISHED'}
    
        def invoke(self, context, event):
            # create the open file dialog
            context.window_manager.fileselect_add(self)
    
            return {'RUNNING_MODAL'}

    # Dialog to select a new config file
    class sceneUpadte_Button(bpy.types.Operator):
        bl_idname = "scene.update"
        bl_label = "Update"
    
        @classmethod
        def poll(cls, context):
            return context is not None
    
        def execute(self, context):
            main(context.scene.ConfigFile)
                
            return {'FINISHED'}

    # register the dialogclasses
    bpy.utils.register_class(Browse_SceneFile_Button)
    bpy.utils.register_class(Browse_ExperimentFile_Button)
    bpy.utils.register_class(sceneUpadte_Button)

    bl_label = "MotionViz Properties"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context     = "world"

    @classmethod
    def poll(cls, context):
        return (context is not None)

    def draw(self, context):
        layout = self.layout
        row = layout.row()
        row.operator("scene.update")
        row = layout.row()
        row.label("Configfile")
        row.prop(context.scene,"ConfigFile","")
        row.operator("load_yaml_file.yaml")
        row = layout.row()
        row.label("Experimentfile")
        row.prop(context.scene,"Experimentfile","")
        row.operator("select_experiment.tsv")
        row = layout.row()
        row.label("MarsScene")
        row.prop(context.scene,"MarsScene","")
        row.operator("select_mars_scene.scn")
        for node in context.scene.NodeProp:
            box = layout.box()
            box.label(node.name)
            row = box.row()
            row.label("Parent")
            row.prop(node,"parent","")
            row = box.row()
            row.label("Child")
            row.prop(node,"child","")
            row = box.row()
            row.label("Mapping")
            row.prop_search(node,"mapping",search_data = bpy.context.scene,search_property = 'objects',text = "")
            #row.prop(node,"mapping","")
            row = box.row()
            row.label("active")
            row.prop(node,"active")
            row = box.row()
            row.label("Visualoffset")
            row.prop(node,"visualOffset","")


# class for parsing the experimentdata
class motion_viz_parser:

    # variables for the experimentdata
    frameCount = 0
    cameraCount = 0
    markerCount = 0
    frequency = 0.0
    analogCount = 0
    analogFrequency = 0.0
    description = ""
    timeStamp = ""
    dataIncluded = ""
    markerNames = []
    markerPositions = {}
    times = []

    def __init__(self,filename):
        self.parseFile(filename)


    def clearData(self):
        self.markerNames.clear()
        self.markerPositions.clear()
        self.frameCount = 0
        self.cameraCount = 0
        self.markerCount = 0
        self.frequency = 0.0
        self.analogCount = 0
        self.analogFrequency = 0.0
        self.description = ""
        self.timeStamp = ""
        self.dataIncluded = ""
        self.times = []

        return True


    def parseFile(self, filename):
        if not os.path.exists(filename):
            print("file not exists: "+filename)
            return False
        printResults = False

        # clear the variables
        self.clearData()
    
        # create helperlists
        tmpListTimes = []
        # read the file
        f = open(filename, 'r')
        lines = f.readlines()
        
        for s in lines:
            # split the current line in "words"
            line=s.split()
            if len(line) < 2:
                continue
            if line[0].isdigit():
                tmpListTimes.append(float(line[1]))
                for i in range(len(self.markerNames)):
                    curr_line_index = 2+i*3
                    x = 0.0
                    y = 0.0
                    z = 0.0
                    if line[curr_line_index] == "null" and \
                       line[curr_line_index+1]=="null" and \
                       line[curr_line_index+2]=="null":
                        # if the current line is "null null null" we have a invalid position
                        # this have to be corrected later. Setting x,y and z to nan so we can find this entry
                        x = float("nan") # numpy.NAN
                        y = float("nan") # numpy.NAN
                        z = float("nan") # numpy.NAN
                    else:
                        x = float(line[curr_line_index])*scale
                        y = float(line[curr_line_index+1])*scale
                        z = float(line[curr_line_index+2])*scale
                        
                    # seach for the markername in the markerposiotns map
                    if self.markerNames[i] in self.markerPositions:
                        # append the new markerposition
                        self.markerPositions[self.markerNames[i]].append([x,y,z])
                    else:       
                        # create a new map-entry with the markerposition                 
                        self.markerPositions[self.markerNames[i]] = [[x,y,z]]
                        
            elif line[0] == "NO_OF_FRAMES":
                self.frameCount = int(line[1])
            elif line[0] == "NO_OF_CAMERAS":
                self.cameraCount = int(line[1])
            elif line[0] == "NO_OF_MARKERS":
                self.markerCount = int(line[1])
            elif line[0] == "FREQUENCY":
                self.frequency = float(line[1])
            elif line[0] == "NO_OF_ANALOG":
                self.analogCount = int(line[1])
            elif line[0] == "ANALOG_FREQUENCY":
                self.analogFrequency = float(line[1])
            elif line[0] == "DESCRIPTION":
                for i in range(1,len(line),1):
                    self.description += line[i]+" "
            elif line[0] == "TIME_STAMP":
                for i in range(1,len(line),1):
                    self.timeStamp += line[i]+" "
            elif line[0] == "DATA_INCLUDED":
                for i in range(1,len(line),1):
                    self.dataIncluded += line[i]+" "
            elif line[0] == "MARKER_NAMES":
                for i in range(1,len(line),1):
                    self.markerNames.append(line[i])

#        self.times = numpy.array(tmpListTimes)
        self.times = tmpListTimes

#        for i in self.markerNames:
#            # make the lists to numpy arrays
#            self.markerPositions[i] = numpy.array(self.markerPositions[i])

        # check if there are invalid marker positions
        self.checkInvalidMarkerPos()
        
        if printResults:
            print("NO_OF_FRAMES: "+str(self.frameCount))
            print("NO_OF_CAMERAS: "+str(self.cameraCount))
            print("NO_OF_MARKERS: "+str(self.markerCount))
            print("FREQUENCY: "+str(self.frequency))
            print("NO_OF_ANALOG: "+str(self.analogCount))
            print("ANALOG_FREQUENCY: "+str(self.analogFrequency))
            print("DESCRIPTION: "+str(self.description))
            print("TIME_STAMP: "+str(self.timeStamp))
            print("DATA_INCLUDED: "+str(self.dataIncluded))
            print("MARKER_NAMES: "+str(self.markerNames))
            print("markerPositions: ")
            print(self.markerPositions)

        return True


    # checks if there are invalid positions in the marker map
    def checkInvalidMarkerPos(self):
        if len(self.markerPositions) == 0:
            return False
        # iterate through the marker map
        for i in self.markerPositions:
            start = False;
            startOfFile = False;
            
            startIndex = 0;
            count = 0;
#            startVector = numpy.array([0.0,0.0,0.0])
            startVector = mathutils.Vector()
#            dirVector   = numpy.array([0.0,0.0,0.0])
            dirVector   = mathutils.Vector()
            # iterate through all postions of the marker
            for index in range(len(self.markerPositions[i])):
                # if all entries of the vector are NAN the position is invalid
#                if numpy.isnan(self.markerPositions[i][index][0]) and \
#                   numpy.isnan(self.markerPositions[i][index][1]) and \
#                   numpy.isnan(self.markerPositions[i][index][2]):
                if math.isnan(self.markerPositions[i][index][0]) and \
                   math.isnan(self.markerPositions[i][index][1]) and \
                   math.isnan(self.markerPositions[i][index][2]):
                    # count how many positions are invalid
                    count += 1
                    if not start:
                        start = True
                        # remember where the first invalid position is and what the last valid position is
                        startIndex = index
                        if startIndex != 0:
                              startVector = self.markerPositions[i][index-1]
                        else:
                              startOfFile = True;
                else:
                    if start:
                        # first valid postion after serveral invalid posiotions
                        start = False
                        if not startOfFile:
                            # compute the direction vector for the interpolation
                            dirVector = (self.markerPositions[i][index]-startVector)/count;

                            for n in range(startIndex,index,1):
                                self.markerPositions[i][n] = startVector+dirVector*(n-startIndex+1)

                        else:
                            # case: the invalid MarkerPos is at the start of the file
                            # set all invalid positions to the first valid
                            for n in range(startIndex,index,1):
                                self.markerPositions[i][n] = self.markerPositions[i][index];
                        count = 0
            if start:
                # case: the invalid MarkerPos is at the end of the file
                # set all invalid positions to the last valid
                for n in range(startIndex,len(self.markerPositions[i]),1):
                    self.markerPositions[i][n] = startVector;

        return True


# parser class for parsing yaml-files
class configFileParser():

    # filename of the yaml-file
    m_filename = ""
    # dictionary for the information of the ymal-file
    m_config = {}

    def __init__(self, filename):
        self.loadConfig(filename)


    def loadConfig(self, filename):
        if not os.path.exists(filename):
            print("file not exists: "+filename)
            return False
        currConfig = ""
        currName = ""
        currType = ""
        self.m_filename = filename
        f = open(filename, 'r')
        lines = f.readlines()
        index = 0
        # go through all lines of the yaml-file
        while index < len(lines):
            # split the current line into words
            line=lines[index].split()
            # if the line consists of only 1 word a new group starts
            if len(line) == 1:
                currConfig = (line[0].split(':'))[0]
                # create a new dict on the first level
                self.m_config[currConfig] = {}
            elif line[0] == "-":
                # a new param begins-> get param name
                currName = (line[2].split(':'))[0]
            elif line[0] == "type:":
                # get the type of the param-value
                if len(line) >= 2:
                    currType = line[1]
            elif line[0] == "value:":
                # check what type the value is and cast the word to this type
                value = ""
                if currType == "bool":
                    value = bool(line[1])
                elif currType == "string":
                    if line[1] == "\"\"":
                        value = ""
                    else :
                        value = line[1]
                # split the param Name into the level identifier
                lvl = currName.split('/')
                dict = self.m_config[currConfig]
                i = 0
                for i in range(len(lvl)-1):
                    # go through the level identifier and look if the level already exists
                    if not (lvl[i] in dict):
                        # if not create the level
                        dict[lvl[i]] = {}
                    dict = dict[lvl[i]]
                if len(lvl) > 1:
                    i += 1
                # save the param-value
                dict[lvl[i]] = value
                
            # go to the next line
            index += 1
        f.close()

        return True


    # get the value of the given param name
    def getPropertyValue(self, group, name, value):
        if not (group in self.m_config):
            return False
        lvl = name.split('/')
        value.clear() 
        dict = self.m_config[group]
        i = 0
        # search for the param name
        for i in range(len(lvl)):
            if not (lvl[i] in dict):
                # if the param doesnt exists retrun False
                return False
            dict = dict[lvl[i]]
        value.append(dict)

        # param found return True
        return True


    # set the value of the given param name
    def setPropertyValue(self, group, name, value):
        lvl = name.split('/')
        if not (group in self.m_config):
            return False
        dict = self.m_config[group]
        i = 0
        # search for the param name
        for i in range(len(lvl)-1):
            if not (lvl[i] in dict):
                # if the param doesnt exists retrun False
                return False
            dict = dict[lvl[i]]
        if len(lvl) > 1:
            dict[lvl[i+1]] = value
        else:
            dict[lvl[i]] = value

        # param found return True
        return True


    # get the value of the given param name or create a new param
    def getOrCreateProperty(self, group, name, value):
        if not (group in self.m_config):
            return False
        lvl = name.split('/')
        dict = self.m_config[group]
        i = 0
        for i in range(len(lvl)-1):
            if not (lvl[i] in dict):
                dict[lvl[i]] = {}
            dict = dict[lvl[i]]
        if len(lvl) > 1:
            i += 1
        if not (lvl[i] in dict):
            if len(value) > 0:
                dict[lvl[i]] = value[0]
            else:
                dict[lvl[i]] = ""
        else:
            value.clear() 
            value.append(dict[lvl[i]])

        return True


    def removeAllParams(self, group):
        if not (group in self.m_config):
            return False
        self.m_config[group].clear()

        return True


    def saveParams(self, currdict, paramPath, file):
        if not isinstance(currdict, dict):
            value = currdict
            file.write("  - name: "+paramPath+"\n")
            typeString = ""
            if isinstance(currdict, bool):
                typeString = "bool"
            elif isinstance(currdict, str):
                typeString = "string"
                if value == "" or value == "<none>":
                    value = "\"\""
            file.write("    type: "+typeString+"\n")
            file.write("    value: "+str(value)+"\n")
        else:
            for name in currdict.keys():
                if paramPath != "":
                    self.saveParams(currdict[name],paramPath+"/"+name,file)
                else:
                    self.saveParams(currdict[name],name,file)

        return True


    def save(self):
        f = open(self.m_filename, 'w')

        for group in self.m_config.keys():
            f.write(group+":\n")
            self.saveParams(self.m_config[group],"",f)

        f.close()

        return True


def updateConfigFile(self, context):
    global newConfig
    print("update")
    newConfig = True


def main(filename):
    global newConfig

    if hasattr(bpy.context.scene,"ConfigFile") and not newConfig:
        newConfig = filename!=bpy.context.scene.ConfigFile
    else:
        newConfig = False
        bpy.types.Scene.ConfigFile = bpy.props.StringProperty(name="ConfigFile", default=filename, update=updateConfigFile)
    if newConfig:
        bpy.context.scene.ConfigFile = filename

    # parse the yaml-file
    if loadExperiment(filename):
        # load mars scene
        loadMarsScene(bpy.context.scene.MarsScene)

        # create helperobjects for each marker
        createHelperObjects()

        # animate all helperobjects
        createAnimation()

        cfg.save()


def registerClasses():
    # register the load config file dialog 
    bpy.utils.register_class(LoadYAMLFileDialog)
    # register the nested Propertyclass
    bpy.utils.register_class(NodeProperties)
    # register the Propertiespanel
    bpy.utils.register_class(NodePropertiesPanel)


def unregisterClasses():     
    # register the load config file dialog 
    bpy.utils.unregister_class(LoadYAMLFileDialog)
    # register the nested Propertyclass
    bpy.utils.unregister_class(NodeProperties)
    # register the Propertiespanel
    bpy.utils.unregister_class(NodePropertiesPanel)


if __name__ == '__main__' :
    registerClasses()

    if hasattr(bpy.context.scene,"ConfigFile") and (bpy.context.scene.ConfigFile != ""):
        main(bpy.context.scene.ConfigFile) 
    else:
        # call the newly registered operator
        bpy.ops.load_yaml_file.yaml('INVOKE_DEFAULT')
