import math
import numpy

import bpy
import mathutils

# Finds the closest points in "data" to every point in "model".
def getClosestPoints(model, data, bijective=False):
    # init the return value with all zeroes
    closest = numpy.zeros(model.shape)

    # if there are too much vertices the script is very slow
    # version 2 is a try to speed things up
    # TODO: maybe there is a better solution for this case
    if len(model) > len(data):
        bijective = False

    # for all vertices in "model"
    for i in range(len(model)):
        # calculate all the distances between the current point of "model" and "data"
        distances = numpy.sum((model[i] - data)**2,axis=-1)**(1./2)

        # determine the index of the closest point
        i_min = numpy.argmin(distances)

        # set the closest point
        closest[i] = data[i_min]

        # if we want to use each "data" data point only once
        if bijective:
            # delete the used point from data (make sure it is only used once)
            data = numpy.delete(data, i_min, 0)

    # return the array of closest points
    return closest

# find the mapping (translation and orientation) between "A" and "B"
def rigid_transform_3D(A, B):
    # make sure "A" and "B" have the same number of data points
    assert len(A) == len(B)

    # get the total number of data points
    N = A.shape[0]

    # calculate the center of both point clouds
    centroid_A = numpy.mean(numpy.matrix(A), axis=0)
    centroid_B = numpy.mean(numpy.matrix(B), axis=0)

    # center both point clouds
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

    # set the rotation_mode to quaternion else the script doesnt work
    node1.rotation_mode = 'QUATERNION'
    node2.rotation_mode = 'QUATERNION'


#    if node1.scale.x != 1.0 and \
#       node1.scale.y != 1.0 and \
#       node1.scale.z != 1.0:
#        print("WARNING! Not all scaling factors of node <%s> are 1.0! Could be problematic!" % nodeName1)

#    if node2.scale.x != 1.0 and \
#       node2.scale.y != 1.0 and \
#       node2.scale.z != 1.0:
#        print("WARNING! Not all scaling factors of node <%s> are 1.0! Could be problematic!" % nodeName2)

    # get the pointers to both meshes
    mesh1 = node1.data.vertices
    mesh2 = node2.data.vertices

    # initialize the root mean square error and the iteration counter
    rmse = float("inf")

    # variable to check if the rmse has changed
    old_rmse = float("inf")

    # do the iterative closest point (ICP) until rmse is small or for
    # a maximal number of iterations
    i = 0

    while rmse > 0.1 and i < 50:
        # convert the list of vertices from local to global coordinates
        vert1 = numpy.array([node1.rotation_quaternion * \
                             mathutils.Vector(numpy.multiply(v.co , node1.scale)) + \
                             node1.location for v in mesh1])

        vert2 = numpy.array([node2.rotation_quaternion * \
                             mathutils.Vector(numpy.multiply(v.co , node2.scale)) + \
                             node2.location for v in mesh2])

        # calculate the closest point for each point of vert1 from vert2
        # if the rmse hasnt changed since last iteration make sure that
        # every point is only used once
        if old_rmse != rmse:
            vert3 = getClosestPoints(vert1, vert2);
        else:
            vert3 = getClosestPoints(vert1, vert2, True);

        # store the last rmse as the old one
        old_rmse = rmse

        # recover the transformation
        R, t = rigid_transform_3D(vert1, vert3)

        # add the translation t to the current position
        node1.location += mathutils.Vector(t)

        # add the rotation R to the current orientation
        node1.rotation_mode = "QUATERNION"
        node1.rotation_quaternion = mathutils.Matrix(R.tolist()).to_quaternion() * node1.rotation_quaternion

        # calculate the resulting error
        err = numpy.array([node1.rotation_quaternion * v.co + node1.location for v in mesh1])
        err = vert1 - vert3
        err = numpy.multiply(err, err)
        err = numpy.sum(err)
        rmse = math.sqrt(err/len(vert1));
        print("rmse = %f" % rmse)

        # increase the iteration counter
        i += 1
    
    return True


if __name__ == "__main__" :
    nodeName1 = "Cube"
    nodeName2 = "Cube.001"

    main(nodeName1,nodeName2)