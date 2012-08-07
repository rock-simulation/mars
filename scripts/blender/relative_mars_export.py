import bpy
import os, glob
import mathutils

#import export_obj
#myobj = __import__(Blender.Get("scriptsdir") + "/export_obj.py")

# ******** TODO:
#               - add jointOffset calculation by using rotation difference
#                 on joint axis of node2

objList = []
jointList = []

def getChildren(parent):
    children = []
    for obj in bpy.data.objects:
        if obj.select:
            if obj.parent == parent:
                children.append(obj)
    return children

def fillList(obj):
    global objList
    global jointList

    if "type" in obj:
        if obj["type"] == "body":
            objList.append(obj)
        else:
            jointList.append(obj)
    obj.select = False
    
    children = getChildren(obj)
    for child in children:
        fillList(child)

#editmode = Blender.Window.EditMode()
#if editmode: Blender.Window.EditMode(0)

filename = "Limes"
scn = bpy.context.scene
path = "/Users/malter/Arbeit/scene/"
out = open(path+filename+".scene", "w")
out.write("""<?xml version="1.0"?>
<!DOCTYPE dfkiMarsSceneFile PUBLIC '-//DFKI/RIC/MARS SceneFile 1.0//EN' ''>
<SceneFile>
  <version>0.1</version>
  <nodelist>
""")

def calcCenter(boundingbox):
    c = [0,0,0]
    for v in boundingbox:
        for i in range(3):
            c[i] += v[i]
    for i in range(3):
        c[i] /= 8.
    return c

def writeNode(obj):
    # get bounding box:
    bBox = obj.bound_box
    center = calcCenter(bBox)
#    center = sum(bBox) * 0.125
    size = [0.0, 0.0, 0.0]
    size[0] = abs(2.0*(bBox[0][0] - center[0]))
    size[1] = abs(2.0*(bBox[0][1] - center[1]))
    size[2] = abs(2.0*(bBox[0][2] - center[2]))

    center = obj.location
#mesh = obj.data
    #for vertex in mesh.vertices:
    #    vertex.co[0] -= center[0]
    #    vertex.co[1] -= center[1]
    #    vertex.co[2] -= center[2]
    
    parentID = 0
    posString = "position"
    rotString = "rotation"
    if obj.parent:
        parentID = obj.parent["id"]
        posString = "relativeposition"
        rotString = "relativerotation"

    obj.rotation_mode = 'QUATERNION'
    q = obj.rotation_quaternion

    out.write('    <node name="'+obj.name+'">\n')
    out.write('      <origname>'+obj.name+'</origname>\n')
    out.write('      <filename>'+obj.name+'.obj</filename>\n')
    out.write('      <index>'+str(obj["id"])+'</index>\n')
    out.write('      <groupid>'+str(obj["group"])+'</groupid>\n')
    out.write('      <physicmode>2</physicmode>\n')
    if parentID:
        out.write('      <relativeid>'+str(parentID)+'</relativeid>\n')
    out.write('      <'+posString+'>\n')
    out.write('        <x>'+str(center[0])+'</x>\n')
    out.write('        <y>'+str(center[1])+'</y>\n')
    out.write('        <z>'+str(center[2])+'</z>\n')
    out.write('      </'+posString+'>\n')
    out.write('      <'+rotString+'>\n')
    out.write('        <x>'+str(q[1])+'</x>\n')
    out.write('        <y>'+str(q[2])+'</y>\n')
    out.write('        <z>'+str(q[3])+'</z>\n')
    out.write('        <w>'+str(q[0])+'</w>\n')
    out.write('      </'+rotString+'>\n')
    out.write('      <movable>true</movable>\n')
    out.write('      <extend>\n')
    out.write('        <x>'+str(size[0])+'</x>\n')
    out.write('        <y>'+str(size[1])+'</y>\n')
    out.write('        <z>'+str(size[2])+'</z>\n')
    out.write('      </extend>\n')
    out.write('      <material_id>1</material_id>\n')
    if "mass" in obj:
        out.write('      <mass>'+str(obj["mass"])+'</mass>\n')
    elif "density" in obj:
        out.write('      <density>'+str(obj["density"])+'</density>\n')
    else:
        out.write('      <density>100</density>\n')
#    out.write('      <pivot>\n')
#    out.write('        <x>'+str(center[0])+'</x>\n')
#    out.write('        <y>'+str(center[1])+'</y>\n')
#    out.write('        <z>'+str(center[2])+'</z>\n')
#    out.write('      </pivot>\n')

    out.write('      <visualsize>\n')
    out.write('        <x>'+str(size[0])+'</x>\n')
    out.write('        <y>'+str(size[1])+'</y>\n')
    out.write('        <z>'+str(size[2])+'</z>\n')
    out.write('      </visualsize>\n')
    out.write('      <coll_bitmask>0</coll_bitmask>\n')

    out.write('    </node>\n')

def writeJoint(joint):
    
    #bBox = joint.bound_box
    pos1 = mathutils.Vector((0.0, 0.0, 0.0))
    pos2 = mathutils.Vector((0.0, 0.0, 1.0))
    center = joint.matrix_world * pos1
    pos2 = joint.matrix_world * pos2
    axis = pos2-center
    node2ID = 0
    node2 = None
    if joint["node2"] in bpy.data.objects:
        node2 = bpy.data.objects[joint["node2"]]
        node2ID = node2["id"]

    offsetAngle = 0.0
    if node2:
        joint.rotation_mode = 'QUATERNION'
        v1 = joint.rotation_quaternion * mathutils.Vector((1.0, 0.0, 0.0))
        v2 = node2.rotation_quaternion * mathutils.Vector((1.0, 0.0, 0.0))
        offsetAngle = v1.angle(v2, 0.0)
        if v1.cross(v2)[2] > 0:
            offsetAngle *= -1

    jointType = {"hinge": 1, "fixed": 6}[joint["jointType"]]
    anchorPos = {"custom": 4, "node1": 1, "node2": 2, "center": 3}[joint["anchor"]]

    #calcCenter(bBox)
    #center = sum(bBox) / 8.
    out.write('    <joint name="'+joint.name+'">\n')
    out.write('      <index>'+str(joint["id"])+'</index>\n')
    out.write('      <type>'+str(jointType)+'</type>\n')
    out.write('      <nodeindex1>'+str(joint.parent["id"])+'</nodeindex1>\n')
    out.write('      <nodeindex2>'+str(node2ID)+'</nodeindex2>\n')
    out.write('      <anchorpos>'+str(anchorPos)+'</anchorpos>\n')
    out.write('      <anchor>\n')
    out.write('        <xpos>'+str(center[0])+'</xpos>\n')
    out.write('        <ypos>'+str(center[1])+'</ypos>\n')
    out.write('        <zpos>'+str(center[2])+'</zpos>\n')
    out.write('      </anchor>\n')
    out.write('      <axis1>\n')
    out.write('        <axis1x>'+str(axis[0])+'</axis1x>\n')
    out.write('        <axis1y>'+str(axis[1])+'</axis1y>\n')
    out.write('        <axis1z>'+str(axis[2])+'</axis1z>\n')
    out.write('      </axis1>\n')
    out.write('      <angle1_offset>'+str(offsetAngle)+'</angle1_offset>\n')
    out.write('    </joint>\n')
    return offsetAngle

def writeMotor(joint, motorValue):
    out.write('    <motor name="'+joint.name+'">\n')
    out.write('      <index>'+str(joint["id"])+'</index>\n')
    out.write('      <jointIndex>'+str(joint["id"])+'</jointIndex>\n')
    out.write('      <axis>1</axis>\n')
    out.write('      <maximumVelocity>6.14</maximumVelocity>\n')
    out.write('      <motorMaxForce>200.0</motorMaxForce>\n')
    out.write('      <type>1</type>\n')
    out.write('      <p>6</p>\n')
    out.write('      <d>3</d>\n')
    out.write('      <max_val>6.28</max_val>\n')
    out.write('      <min_val>-6.28</min_val>\n')
    out.write('      <value>'+str(motorValue)+'</value>\n')
    out.write('    </motor>\n')


root = None

for obj in bpy.data.objects:
    if obj.select:
        if not obj.parent:
            root = obj
            break

if root:
    fillList(root)

for node in objList:
    writeNode(node)

out.write('  </nodelist>\n')
out.write('  <jointlist>\n')

motorValue = []

for joint in jointList:
    motorOffset = writeJoint(joint)
    if joint["jointType"] == "hinge":
        motorValue.append(motorOffset)

out.write('  </jointlist>\n')
out.write('  <motorlist>\n')

i = 0
for joint in jointList:
    if joint["jointType"] == "hinge":
        writeMotor(joint, motorValue[i])
        i += 1

out.write('  </motorlist>\n')

out.write('  <materiallist>\n')
out.write('    <material>\n')
out.write('      <id>1</id>\n')
out.write('      <exists>false</exists>\n')
out.write('    </material>\n')
out.write('  </materiallist>\n')

out.write('  <graphicOptions>\n')
out.write('    <clearColor>\n')
out.write('      <clearColorR>0.550000</clearColorR>\n')
out.write('      <clearColorG>0.670000</clearColorG>\n')
out.write('      <clearColorB>0.880000</clearColorB>\n')
out.write('      <clearColorA>1.000000</clearColorA>\n')
out.write('    </clearColor>\n')
out.write('    <fogEnabled>false</fogEnabled>\n')
out.write('  </graphicOptions>\n')
out.write('</SceneFile>\n')

out.close()

os.chdir(path)
os.system("zip "+filename+".scn "+filename+".scene")


for obj in objList:
    obj.select = True
    #bpy.ops.object.transform_apply(rotation=True)
    location = obj.location.copy()
    rotation = obj.rotation_quaternion.copy()
    parent = obj.parent
    obj.location = [0.0, 0.0, 0.0]
    obj.rotation_quaternion = [1.0, 0.0, 0.0, 0.0]
    obj.parent = None
    out_name = path+obj.name + ".obj"
    bpy.ops.export_scene.obj(filepath=out_name, axis_forward='-Z', axis_up='Y', use_selection=True, use_normals=True)
    #export_obj.write( out_name, [value[1]], EXPORT_NORMALS=True )
    os.system("zip "+filename+".scn "+obj.name+".obj")
    os.system("zip "+filename+".scn "+obj.name+".mtl")
    obj.location = location
    obj.rotation_quaternion = rotation
    obj.parent = parent
    obj.select = False
    
  
# it would be nice to also set the pivot to the object center
#Blender.Redraw()
