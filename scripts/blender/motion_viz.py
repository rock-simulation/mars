import os
import sys
import math
import numpy

import bpy
import mathutils

# global list of nodes
nodeList = []

# global list of parentnodes
parentList = {}

# global list of childrennodes
childrenList = {}

# parser for the yaml-Files
cfg = None

# parser for the motion-Files
mvp = None

# loads an experiment from a yaml-file
def loadExperiment(filename):
    global cfg
    # create the yaml-parser object and parse the file
    cfg = configFileParser(filename)
    
    group = "mviz"
    paramName = "motionfile"
    value = []
    # check if the config-file has an entry for the experiment data file
    if cfg.getPropertyValue(group,paramName,value):
        motionFile = value[0]
        global mvp
        # create the parser for the experimentfiles and parse the experimentfile
        mvp = motion_viz_parser(motionFile)
        # go through all markers and look for parent- and child-nodes
        # if there are such nodes save them in lists
        for marker in mvp.markerNames:
            paramName = marker+"/parent"
            if cfg.getPropertyValue(group,paramName,value) and (value[0] != ""):
                parentList[marker] = value[0]
            paramName = marker+"/child"
            if cfg.getPropertyValue(group,paramName,value) and (value[0] != ""):
                childrenList[marker] = value[0]

# create helper objects for each marker in blender
def createHelperObjects():
    
    for marker in mvp.markerNames:
        # if the marker has a parent- or child-node the helperobject has to be a cylinder 
        if (marker in parentList) or (marker in childrenList):
            ext = mathutils.Vector()
            # the radius the cylinder should have
            ext.x = 0.1
            # the height the cylinder should have
            ext.y = 1
            
            # compute the rotation-quaternion for the cylinder so the cylinder points to
            # the parent- or child-node
            
            # get the startposition of the marker
            currVec = mvp.markerPositions[marker][0]
            # initialize the variables for the rotation and position
            rotation = mathutils.Quaternion()
            pos = numpy.array([0.0,0.0,0.0])
            # if the marker has got a child the cylinder should look to the child-node
            if marker in childrenList:
                childVec = mvp.markerPositions[childrenList[marker]][0]
                # get the vector which points in the direction the cylinder should point to
                dirrVec = currVec-childVec
                # get the rotation-quaternion
                rotation = quatFromTwoVectors(numpy.array([0.0,0.0,1.0]),dirrVec)
                # move the cylinder have the way of the direction-vector so everything sticks together
                pos = mvp.markerPositions[marker][0]*0.01-0.5*dirrVec
            else:
                parentVec = mvp.markerPositions[parentList[marker]][0]
                # get the vector which points in the direction the cylinder should point to
                dirrVec = parentVec-currVec
                # get the rotation-quaternion
                rotation = quatFromTwoVectors(numpy.array([0.0,0.0,1.0]),dirrVec)
                # move the cylinder have the way of the direction-vector so everything sticks together
                pos = mvp.markerPositions[marker][0]*0.01+0.5*dirrVec
            # create the cylinder in blender
            createPrimitive(marker,"cylinder",pos,ext)
            # set the rotation and the height of the new cylinder
            node = nodeList[len(nodeList)-1]
            node.rotation_mode = "QUATERNION"
            node.scale.z = numpy.linalg.norm(dirrVec)
            node.rotation_quaternion = rotation
        else:
            # if the marker has no parents or children the helperobject should be a sphere
            ext = mathutils.Vector()
            ext.x = 0.1
            createPrimitive(marker,"sphere",mvp.markerPositions[marker][0],ext)

# animate the helperobjects with the information of the experimentdata            
def createAnimation():
    # get the fps
    fps = float(bpy.context.scene.render.fps)
    # get the total count of frames
    bpy.context.scene.frame_end = mvp.times[len(mvp.times)-1]*fps
    currIndex = 0
    for frame in range(bpy.context.scene.frame_end):
        # get how much times passed
        currTime = frame/fps
        timeDiff = 1000.0 
        # search for the right time in the experimentdata
        while (math.fabs(mvp.times[currIndex]-currTime)<timeDiff) and (currIndex < len(mvp.times)):
            timeDiff = math.fabs(mvp.times[currIndex]-currTime)
            currIndex += 1
        # set the scene frame to our currentframe
        bpy.context.scene.frame_set(frame)
        # go throug all nodes(marker) and set postion, rotation and height for the new time
        for node in nodeList:
            if (node.name in parentList) or (node.name in childrenList):
                # if the node is has a parent or child node we have to update the rotation and height
                currVec = mvp.markerPositions[node.name][currIndex]
                rotation = mathutils.Quaternion()
                pos = numpy.array([0.0,0.0,0.0])
                # if the marker has got a child the cylinder should look to the child-node
                if node.name in childrenList:
                    childVec = mvp.markerPositions[childrenList[node.name]][currIndex]
                    # get the vector which points in the direction the cylinder should point to
                    dirrVec = currVec-childVec
                    # get the rotation-quaternion
                    rotation = quatFromTwoVectors(numpy.array([0.0,0.0,1.0]),dirrVec)
                    # move the cylinder have the way of the direction-vector so everything sticks together
                    pos = mvp.markerPositions[node.name][currIndex]*0.01-0.5*dirrVec
                else:
                    parentVec = mvp.markerPositions[parentList[node.name]][currIndex]
                    # get the vector which points in the direction the cylinder should point to
                    dirrVec = parentVec-currVec
                    # get the rotation-quaternion
                    rotation = quatFromTwoVectors(numpy.array([0.0,0.0,1.0]),dirrVec)
                    # move the cylinder have the way of the direction-vector so everything sticks together
                    pos = mvp.markerPositions[node.name][currIndex]*0.01+0.5*dirrVec
                
                # update position, rotation and height
                node.scale.z = numpy.linalg.norm(dirrVec)
                node.rotation_quaternion = rotation
                node.location = pos
            else:
                # the node is a sphere -> only update position
                pos = mvp.markerPositions[node.name][currIndex]
                node.location = pos
            # insert the new keyframes in the timeline
            node.keyframe_insert(data_path='location', frame=(frame))
            node.keyframe_insert(data_path='rotation_quaternion', frame=(frame))
            node.keyframe_insert(data_path='scale', frame=(frame))

# create a quaternion which describes the rotation from vec1 to vec2
def quatFromTwoVectors(vec1, vec2):
    # initialize the result quaternion
    q = mathutils.Quaternion()
    # get the vector we rotate around (vector that stands upright on the 2 given vectors)
    a = numpy.cross(vec1, vec2)
    # assign our rotation-vector to ur quaternion
    q.x = a[0]
    q.y = a[1]
    q.z = a[2]
    # compute the angle
    q.w = math.sqrt((numpy.linalg.norm(vec1) ** 2) * (numpy.linalg.norm(vec2) ** 2)) + numpy.dot(vec1, vec2)
    # normalize the quaternion
    q.normalize()
    return q
    
# remove all helperobjects from the blenderscene
def removeAllObjects():
    # select all "MESH" objects
    for object in bpy.data.objects:
        currObj = bpy.data.objects[object.name]
        if currObj.type == "MESH":
            bpy.data.objects[object.name].select = True

    # remove all selected
    bpy.ops.object.delete()     
    # remove the meshes, they have no users anymore
    for item in bpy.data.meshes:
        bpy.data.meshes.remove(item)

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
    times = ""
    
    def __init__(self,filename):
        self.parseFile(filename)
    
    def clearData(self):
        del self.markerNames[:]
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
        self.times = ""

    def parseFile(self, filename):
        if not os.path.exists(filename):
            print("file not exists: "+filename)
            return
        # the positions in experimentfile are to huge so we have to scale them down
        scale = 0.01
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
                tmpListTimes.append([float(line[1])])
                for i in range(len(self.markerNames)):
                    curr_line_index = 2+i*3
                    x = 0.0
                    y = 0.0
                    z = 0.0
                    if line[curr_line_index]=="null" and line[curr_line_index+1]=="null" and line[curr_line_index+2]=="null":
                        # if the current line is "null null null" we have a invalid position
                        # this have to be corrected later. Setting x,y and z to nan so we can find this entry
                        x = numpy.NAN
                        y = numpy.NAN
                        z = numpy.NAN
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
        
        self.times = numpy.array(tmpListTimes)
        for i in self.markerNames:
            # make the lists to numpy arrays
            self.markerPositions[i] = numpy.array(self.markerPositions[i])
        # check if there are invalid marker positions
        self.checkInvalidMarkerPos()
        
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
    # checks if there are invalid positions in the marker map     
    def checkInvalidMarkerPos(self):
        if len(self.markerPositions) == 0:
            return
        # iterate through the marker map
        for i in self.markerPositions:
            start = False;
            startOfFile = False;
            
            startIndex = 0;
            count = 0;
            startVector = numpy.array([0.0,0.0,0.0])
            dirVector   = numpy.array([0.0,0.0,0.0])
            # iterate through all postions of the marker
            for index in range(len(self.markerPositions[i])):
                # if all entries of the vector are NAN the position is invalid
                if numpy.isnan(self.markerPositions[i][index][0]) and numpy.isnan(self.markerPositions[i][index][1]) and numpy.isnan(self.markerPositions[i][index][2]):
                    # count how many positions are invalid
                    count += 1
                    if not start:
                        start = True
                        # remember where the first invalid position is and what the last valid position is
                        startIndex = index
                        if startIndex != 0:
                              startVector = self.markerPositions[i][index-1]
                        else:
                              startOfFile = true;
                else:
                    if start:
                        # first valid postion after serveral invalid posiotions
                        start = False
                        if not startOfFile:
                            # compute the direction vector for the interpolation
                            dirVector =(self.markerPositions[i][index]-startVector)/count;

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
            return
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

    # get the value of the given param name            
    def getPropertyValue(self, group, name, value):
        if group not in self.m_config:
            return False

        del value[:]
        lvl = name.split('/')

        dict = self.m_config[group]
        i = 0
        # search for the param name
        for i in range(len(lvl)):
            if lvl[i] not in dict:
                # if the param doesnt exists return False
                return False
            dict = dict[lvl[i]]
        value.append(dict)

        # param found return True
        return True

    # get the value of the given param name or create a new param
    def getOrCreateProperty(self, group, name, type, value):
        if group not in self.m_config:
            return

        if type == "bool":
            value = bool(value)
        elif type == "string":
            value = str(value)
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
            dict[lvl[i]] = value
        else:
            value = dict[lvl[i]]
 
    def removeParamGroup(self, group):
        if group not in self.m_config:
            return False

        self.m_config[group].clear()
        return True

    def removeAllParams(self):
        self.m_config.clear()
        return True

if __name__ == '__main__' :
    # clear the blender-scene
    removeAllObjects()
    # parse the yaml-file
    loadExperiment("LochExperiment.yaml")
    # create helperobjects for each marker
    createHelperObjects()
    # animate all helperobjects
    createAnimation()


