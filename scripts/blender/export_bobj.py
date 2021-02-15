# ******** TODO:
#               - add jointOffset calculation by using rotation difference
#                 on joint axis of node2

import bpy
import os, glob
import mathutils
import struct

scn = bpy.context.scene

def roundV(v):
    return round(v.x, 6), round(v.y, 6), round(v.z, 6)

def exportBobj(outname, obj):
    numNormals = 1
    numUVs = 1
    globalNormals = {}

    if obj.select_get():

        # ignore dupli children
        if obj.parent and obj.parent.dupli_type in {'VERTS', 'FACES'}:
            # XXX
            print(obj.name, 'is a dupli child - ignoring')
            return

        #obj.select = False
        mesh = obj.to_mesh()
        mesh.calc_loop_triangles()
        #mesh.transform(obj.matrix_world)

        write_uv = False
        for key,value in mesh.uv_layers.items():
            uv_layer = value
            write_uv = True
            break


        mesh.calc_normals()

        out = open(outname+".bobj", "wb")
        #out2 = open(outname+".obj", "w")
        for v in mesh.vertices:
            out.write(struct.pack('ifff', 1, v.co[0], v.co[1], v.co[2]))
            #out2.write("1 {} {} {}\n".format(v.co[0], v.co[1], v.co[2]))

        uv_face_mapping = {}
        if write_uv:
            for tri in mesh.loop_triangles:
                uv_face_mapping[tri] = {}
                for loop_index in tri.loops:
                    uv_face_mapping[tri][loop_index] = numUVs
                    numUVs += 1
                    out.write(struct.pack('iff', 2, uv_layer.data[loop_index].uv[0], uv_layer.data[loop_index].uv[1]))
                    #out2.write("2 {} {}\n".format(uv_layer.data[loop_index].uv[0], uv_layer.data[loop_index].uv[1]))


        for tri in mesh.loop_triangles:
            for index in tri.vertices:
                v = mesh.vertices[index]
                n = roundV(v.normal)
                if n not in globalNormals:
                    globalNormals[n] = numNormals
                    numNormals += 1
                    out.write(struct.pack('ifff', 3, n[0], n[1], n[2]))
                    #out2.write("3 {} {} {}\n".format(n[0], n[1], n[2]))
            
        for tri in mesh.loop_triangles:
            da = struct.pack('i', 4)
            out.write(da)
            #out2.write(" 4")
            for i in range(len(tri.vertices)):
                vIndex = tri.vertices[i]
                v = mesh.vertices[vIndex]
                if write_uv:
                    uvIndex = tri.loops[i]
                    #print(uv_face_mapping[tri])
                    uvFace = uv_face_mapping[tri][uvIndex]
                    da = struct.pack('iii', vIndex + 1, uvFace, globalNormals[roundV(v.normal)])
                    out.write(da)  # vert, uv, normal
                    #out2.write(" {} {} {}".format(vIndex + 1, uvFace, globalNormals[roundV(v.normal)]))
                else:
                    da = struct.pack('iii', vIndex + 1, 0, globalNormals[roundV(v.normal)])
                    out.write(da)  # vert, uv, normal
                    #out2.write(" {} {} {}".format(vIndex + 1, 0, globalNormals[roundV(v.normal)]))
            #out2.write("\n")

        out.close()
        #out2.close()


exportBobj("ground2", bpy.context.collection.objects["Plane"])
