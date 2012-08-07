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

print("new run")

def handleProps(obj):
    obj.select = False
    obj.data.name = obj.name
    if setDefault(obj, "type", "body") == "body":
        obj["id"] = getNextBodyID()

        if obj.parent:
            children = getChildren(obj.parent)
            setGroup = False
            for da in children:
                if da["type"] == "joint":
                    if da["node2"] == obj.name:
                        obj["group"] = getNextGroupID()
                        setGroup = True
                        break
            if not setGroup and obj.parent:
                obj["group"] = obj.parent["group"]
        else:
            obj["group"] = getNextGroupID()

        if "mass" not in obj and "density" not in obj:
            setDefault(obj, "density", 100.0)

    else:
        obj["id"] = getNextJointID()
        setDefault(obj, "node2", "air")
        setDefault(obj, "jointType", "hinge")
        setDefault(obj, "anchor", "node2")
        children = getChildren(obj.parent)
        if len(children) == 2:
            if children[0] != obj:
                obj["node2"] = children[0].name
            else:
                obj["node2"] = children[1].name
        else:
            print(obj.name+": num children: "+str(len(children)))

    children = getChildren(obj)
    for obj in children:
        handleProps(obj)

def getRoot():
    for obj in bpy.data.objects:
        if obj.select and obj.parent == None:
            return obj


root = getRoot()

if root:
    print(root.name)
    handleProps(root)
