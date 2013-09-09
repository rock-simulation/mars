import math
import numpy

import bpy
import mathutils

# Finds the closest points in "data" to every point in "model".
def getClosestPoints(model, data):
    closest = numpy.zeros(model.shape)

    for i in range(len(model)):

        distances = numpy.sum((model[i] - data)**2,axis=-1)**(1./2)

        i_min = numpy.argmin(distances)

        closest[i] = data[i_min]

    return closest


def rigid_transform_3D(A, B):
    assert len(A) == len(B)

    N = A.shape[0]; # total points

    centroid_A = numpy.mean(numpy.matrix(A), axis=0)
    centroid_B = numpy.mean(numpy.matrix(B), axis=0)

    # centre the points
    AA = A - numpy.tile(centroid_A, (N, 1))
    BB = B - numpy.tile(centroid_B, (N, 1))

    # dot is matrix multiplication for array
    H = numpy.transpose(AA) * BB

    # singular value decomposition
    U, S, Vt = numpy.linalg.svd(H)

    # calculate the rotation matrix R
    R = Vt.T * U.T

    # special reflection case
    if numpy.linalg.det(R) < 0:
       print("Reflection detected")
       Vt[2,:] *= -1
       R = Vt.T * U.T

    # calculate the translation t
    t = -R*centroid_A.T + centroid_B.T

    return R, t


def main(nodeName1, nodeName2):
    # check if the first node does exist
    if nodeName1 not in bpy.data.objects:
        print("WARNING! Unable to find first node <%s>! Node does not exist!" % nodeName1)
        return False

    # check if the second node does exist
    if nodeName2 not in bpy.data.objects:
        print("WARNING! Unable to find second node <%s>! Node does not exist!" % nodeName2)
        return False

    # get the pointers to both nodes
    node1 = bpy.data.objects[nodeName1]
    node2 = bpy.data.objects[nodeName2]

    mesh1 = node1.data.vertices
    mesh2 = node2.data.vertices

    rmse = float("inf")
    i = 0

    while rmse > 0.1 and i < 50:

        # TODO: incorporate "scale"
        vert1 = numpy.array([node1.rotation_quaternion * v.co + node1.location for v in mesh1])
        vert2 = numpy.array([node2.rotation_quaternion * v.co + node2.location for v in mesh2])

        vert3 = getClosestPoints(vert1, vert2);

        # recover the transformation
        R, t = rigid_transform_3D(vert1, vert3)

        node1.location += mathutils.Vector(t)

        node1.rotation_quaternion = mathutils.Matrix(R.tolist()).to_quaternion() * node1.rotation_quaternion

        err = numpy.array([node1.rotation_quaternion * v.co + node1.location for v in mesh1])
        err = vert1 - vert3
        err = numpy.multiply(err, err)
        err = numpy.sum(err)
        rmse = math.sqrt(err/len(vert1));
        print("rmse = %f" % rmse)

        i += 1

    return True


if __name__ == "__main__" :
    nodeName1 = "Cube"
    nodeName2 = "Cube.001"

    main(nodeName1,nodeName2)