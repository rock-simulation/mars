import bpy
import os, glob
import mathutils


#editmode = Blender.Window.EditMode()
#if editmode: Blender.Window.EditMode(0)

class IDGenerator(object):
    def __init__(self, startID=0):
        self.nextID = startID
    def __call__(self):
        ret, self.nextID = self.nextID, self.nextID + 1
        return ret

getNextBodyID = IDGenerator(1)
getNextJointID = IDGenerator(1)
getNextGroupID = IDGenerator(1)

def setDefault(obj, key, value):
    if key not in obj:
        obj[key] = value
    return obj[key]

def getChildren(parent):
    children = []
    for obj in bpy.data.objects:
        if obj.parent == parent:
            children.append(obj)
    return children

def createWorldProperties():
    setDefault(bpy.data.worlds[0], "path", ".")
    setDefault(bpy.data.worlds[0], "filename", "example")
    setDefault(bpy.data.worlds[0], "exportBobj", False)
    setDefault(bpy.data.worlds[0], "exportMesh", True)

def createBodyProperties(obj):
    obj["id"] = getNextBodyID()
    if obj.parent:
        children = getChildren(obj.parent)
        setGroup = False
        for child in children:
            if "type" in child and child["type"] == "joint":
                if child["node2"] == obj.name:
                    obj["group"] = getNextGroupID()
                    setGroup = True
                    break
        if not setGroup and obj.parent:
            obj["group"] = obj.parent["group"]
    else:
        obj["group"] = getNextGroupID()
    if "mass" not in obj and "density" not in obj:
        setDefault(obj, "mass", 0.1)

def createJointProperties(obj):
    obj["id"] = getNextJointID()
    setDefault(obj, "node2", "air")
    setDefault(obj, "jointType", "hinge")
    setDefault(obj, "anchor", "node2")
    setDefault(obj, "controllerIndex", 0)
    children = getChildren(obj.parent)
    if len(children) == 2:
        if children[0] != obj:
            obj["node2"] = children[0].name
        else:
            obj["node2"] = children[1].name
    else:
        print(obj.name+": num children: "+str(len(children)))

def handleProps(obj):
    print("handle: "+obj.name)
#    obj.select = False
    obj.data.name = obj.name
    defaultType = "body"
    if obj.name.find("joint") > -1:
        defaultType = "joint"

    objType = setDefault(obj, "type", defaultType)
    if objType == "body":
        createBodyProperties(obj)
    elif objType == "joint":
        createJointProperties(obj)

    children = getChildren(obj)
    for obj in children:
        handleProps(obj)


def getRoot():
    for obj in bpy.data.objects:
        if obj.select and obj.parent == None:
            return obj

def main():
    createWorldProperties()
    root = getRoot()

    if root:
        print(root.name)
        handleProps(root)


if __name__ == '__main__':
    main()
