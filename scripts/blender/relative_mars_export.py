import bpy
import os, glob
import mathutils
import struct

#import export_obj
#myobj = __import__(Blender.Get("scriptsdir") + "/export_obj.py")

nextMaterialID = 1

# ******** TODO:
#               - add jointOffset calculation by using rotation difference
#                 on joint axis of node2

def veckey3d(v):
    return round(v.x, 6), round(v.y, 6), round(v.z, 6)


def exportBobj(outname, obj):
    totverts = totuvco = totno = 1

    face_vert_index = 1

    globalNormals = {}

    if obj.select:
        #obj.select = False
        mesh = obj.data
        write_uv = False
        faceuv =len(mesh.uv_textures)
        if faceuv:
            uv_layer = mesh.uv_textures.active.data[:]
            write_uv = True

        face_index_pairs = [(face, index) for index, face in enumerate(mesh.faces)]

        mesh.calc_normals()

        me_verts = mesh.vertices[:]

        out = open(outname, "wb")

        for v in mesh.vertices:
            da = struct.pack('ifff', 1, v.co[0], v.co[1], v.co[2])
            out.write(da)

        if faceuv:
            uv = uvkey = uv_dict = f_index = uv_index = None

            uv_face_mapping = [[0, 0, 0, 0] for i in range(len(face_index_pairs))]  # a bit of a waste for tri's :/

            uv_dict = {}  # could use a set() here
            uv_layer = mesh.uv_textures.active.data
            for f, f_index in face_index_pairs:
                for uv_index, uv in enumerate(uv_layer[f_index].uv):
                    uvkey = round(uv[0], 6), round(uv[1], 6)
                    try:
                        uv_face_mapping[f_index][uv_index] = uv_dict[uvkey]
                    except:
                        uv_face_mapping[f_index][uv_index] = uv_dict[uvkey] = len(uv_dict)
                        da = struct.pack('iff', 2, uv[0], uv[1])
                        out.write(da)

            uv_unique_count = len(uv_dict)

            del uv, uvkey, uv_dict, f_index, uv_index

        for f, f_index in face_index_pairs:
            if f.use_smooth:
                for v_idx in f.vertices:
                    v = me_verts[v_idx]
                    noKey = veckey3d(v.normal)
                    if noKey not in globalNormals:
                        globalNormals[noKey] = totno
                        totno += 1
                        da = struct.pack('ifff', 3, noKey[0], noKey[1], noKey[2])
                        out.write(da)
            else:
                # Hard, 1 normal from the face.
                noKey = veckey3d(f.normal)
                if noKey not in globalNormals:
                    globalNormals[noKey] = totno
                    totno += 1
                    da = struct.pack('ifff', 3, noKey[0], noKey[1], noKey[2])
                    out.write(da)

        for f, f_index in face_index_pairs:
            f_smooth = f.use_smooth
            if faceuv:
                tface = uv_layer[f_index]
            # wrtie smooth info for face?

            f_v_orig = [(vi, me_verts[v_idx]) for vi, v_idx in enumerate(f.vertices)]

            if len(f_v_orig) == 3:
                f_v_iter = (f_v_orig, )
            else:
                f_v_iter = (f_v_orig[0], f_v_orig[1], f_v_orig[2]), (f_v_orig[0], f_v_orig[2], f_v_orig[3])

            for f_v in f_v_iter:
                da = struct.pack('i', 4)
                out.write(da)

                if faceuv:
                    if f_smooth:  # Smoothed, use vertex normals
                        for vi, v in f_v:
                            da = struct.pack('iii', v.index + totverts, totuvco + uv_face_mapping[f_index][vi], globalNormals[veckey3d(v.normal)])
                            out.write(da)  # vert, uv, normal

                    else:  # No smoothing, face normals
                        no = globalNormals[veckey3d(f.normal)]
                        for vi, v in f_v:
                            da = struct.pack('iii', v.index + totverts, totuvco + uv_face_mapping[f_index][vi], no)
                            out.write(da)  # vert, uv, normal
                else:  # No UV's
                    if f_smooth:  # Smoothed, use vertex normals
                        for vi, v in f_v:
                            da = struct.pack('iii', v.index + totverts, 0, globalNormals[veckey3d(v.normal)])
                            out.write(da)  # vert, uv, normal
                    else:  # No smoothing, face normals
                        no = globalNormals[veckey3d(f.normal)]
                        for vi, v in f_v:
                            da = struct.pack('iii', v.index + totverts, 0, no)
                            out.write(da)  # vert, uv, normal


        out.close()


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

filename = "crex"
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
    global nextMaterialID

    if "marsID" in obj.active_material and obj.active_material["marsID"] != 0:
        matID = obj.active_material["marsID"]
    else:
        obj.active_material["marsID"] = nextMaterialID
        matID = nextMaterialID
        nextMaterialID = nextMaterialID + 1

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

    obj.rotation_mode = 'QUATERNION'
    q = obj.rotation_quaternion

    if obj.parent:
        parentID = obj.parent["id"]
        parent = obj.parent
        parentIQ = parent.matrix_world.to_quaternion()
        parentIQ.invert()
        parentPos = parent.matrix_world * mathutils.Vector((0.0, 0.0, 0.0))
        childPos = obj.matrix_world * mathutils.Vector((0.0, 0.0, 0.0))
        childPos = childPos - parentPos
        center = parentIQ * childPos
        parentRot = parent.matrix_world.to_quaternion()
        childRot = obj.matrix_world.to_quaternion()
        childRot = parentRot.rotation_difference(childRot)
        q = childRot

    out.write('    <node name="'+obj.name+'">\n')
    obj_name = obj.name
    if "use" in obj:
        obj_name = obj["use"]

    out.write('      <origname>'+obj_name+'</origname>\n')
    out.write('      <filename>'+obj_name+'.bobj</filename>\n')
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
    out.write('        <x>'+str(0.9*size[0])+'</x>\n')
    out.write('        <y>'+str(0.9*size[1])+'</y>\n')
    out.write('        <z>'+str(0.9*size[2])+'</z>\n')
    out.write('      </extend>\n')
    out.write('      <material_id>'+str(matID)+'</material_id>\n')
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
    #out.write('      <coll_bitmask>0</coll_bitmask>\n')

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
    out.write('        <x>'+str(center[0])+'</x>\n')
    out.write('        <y>'+str(center[1])+'</y>\n')
    out.write('        <z>'+str(center[2])+'</z>\n')
    out.write('      </anchor>\n')
    out.write('      <axis1>\n')
    out.write('        <x>'+str(axis[0])+'</x>\n')
    out.write('        <y>'+str(axis[1])+'</y>\n')
    out.write('        <z>'+str(axis[2])+'</z>\n')
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

def writeMaterial(material):
    out.write('    <material>\n')
    out.write('      <id>'+str(material["marsID"])+'</id>\n')
    out.write('      <diffuseFront>\n');
    out.write('        <a>1.0</a>\n');
    out.write('        <r>'+str(material.diffuse_color[0])+'</r>\n');
    out.write('        <g>'+str(material.diffuse_color[1])+'</g>\n');
    out.write('        <b>'+str(material.diffuse_color[2])+'</b>\n');
    out.write('      </diffuseFront>\n');
    out.write('      <specularFront>\n');
    out.write('        <a>1.0</a>\n');
    out.write('        <r>'+str(material.specular_color[0])+'</r>\n');
    out.write('        <g>'+str(material.specular_color[1])+'</g>\n');
    out.write('        <b>'+str(material.specular_color[2])+'</b>\n');
    out.write('      </specularFront>\n');
    out.write('      <shininess>'+str(material.specular_hardness/2)+'</shininess>\n');
    out.write('    </material>\n')

root = None

for obj in bpy.data.objects:
    if obj.select:
        if not obj.parent:
            root = obj
            break

if root:
    fillList(root)

for material in bpy.data.materials:
    if "marsID" in material:
        material["marsID"] = 0


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

for material in bpy.data.materials:
    if "marsID" in material and material["marsID"] != 0:
        writeMaterial(material)

out.write('  </materiallist>\n')

out.write('  <graphicOptions>\n')
out.write('    <clearColor>\n')
out.write('      <r>0.550000</r>\n')
out.write('      <g>0.670000</g>\n')
out.write('      <b>0.880000</b>\n')
out.write('      <a>1.000000</a>\n')
out.write('    </clearColor>\n')
out.write('    <fogEnabled>false</fogEnabled>\n')
out.write('  </graphicOptions>\n')
out.write('</SceneFile>\n')

out.close()

os.chdir(path)
os.system("rm "+filename+".scn")
os.system("zip "+filename+".scn "+filename+".scene")


for obj in objList:
    if "use" in obj:
        continue
    obj.select = True
    bpy.context.scene.objects.active = obj
    bpy.ops.object.modifier_apply(modifier='EdgeSplit')
    #bpy.ops.object.transform_apply(rotation=True)
    location = obj.location.copy()
    rotation = obj.rotation_quaternion.copy()
    parent = obj.parent
    obj.location = [0.0, 0.0, 0.0]
    obj.rotation_quaternion = [1.0, 0.0, 0.0, 0.0]
    obj.parent = None
    out_name = path+obj.name + ".bobj"
    exportBobj(out_name, obj)
    #bpy.ops.export_scene.obj(filepath=out_name, axis_forward='-Z', axis_up='Y', use_selection=True, use_normals=True)
    #export_obj.write( out_name, [value[1]], EXPORT_NORMALS=True )
    os.system("zip "+filename+".scn "+obj.name+".bobj")
    #os.system("zip "+filename+".scn "+obj.name+".mtl")
    obj.location = location
    obj.rotation_quaternion = rotation
    obj.parent = parent
    obj.select = False


# it would be nice to also set the pivot to the object center
#Blender.Redraw()
