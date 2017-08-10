from mars_interface import *
from euclid import *

def init():
    clearDict()
    # requestSensor("position")
    # requestSensor("rotation")
    # requestSensor("6dof")
    # setConfig("Graphics", "showCoords", 0)
    # setConfig("Scene", "skydome_enabled", 1)
    # #setConfig("Simulator", "calc_ms", 20)
    # setUpdateTime(1)
    # #setConfig("Robo", "behavior", 0)
    # #requestConfig("Robo", "behavior")

    # clearLines("debug")
    # appendLines("debug", 0., 0., 0.2)
    # appendLines("debug", 0., 0., 0.2)
    # configureLines("debug", 5.0, 0, 1, 0)
    logMessage("setup python interface")
    return sendDict()

def update(marsData):
    clearDict()
    # dof = [0, 0, 0, 0, 0, 0]
    # rotation = [0, 0, 0, 0]
    # x = 0.;
    # y = 0.;
    # if "Sensors" in marsData:
    #     if "6dof" in marsData["Sensors"]:
    #         if len(marsData["Sensors"]["6dof"]) == 6:
    #             for i in range(6):
    #                 dof[i] = marsData["Sensors"]["6dof"][i]
    #     if "position" in marsData["Sensors"]:
    #         x = marsData["Sensors"]["position"][0]
    #         y = marsData["Sensors"]["position"][1]
    #     if "rotation" in marsData["Sensors"]:
    #         #logError("r: "+str(marsData["Sensors"]["rotation"]))
    #         if len(marsData["Sensors"]["rotation"]) == 3:
    #             for i in range(3):
    #                 rotation[i] = marsData["Sensors"]["rotation"][i]*(3.1416/180.)
    # v = Vector3(x, y, 0.3)
    # force = Vector3(dof[0], dof[1], dof[2])*1.
    # logMessage("force: "+str(force))
    # #logMessage("rotation: "+str(rotation))
    # r = Matrix4().new_rotate_euler(rotation[2], 0, 0)
    # force = r*force
    # force += v
    # #force *= 10.
    # clearLines("debug")
    # #appendLines("debug", 0, 0, 0)
    # appendLines("debug", v.x, v.y, 0)
    # appendLines("debug", v.x, v.y, v.z)
    # appendLines("debug", v.x, v.y, v.z)
    # appendLines("debug", force.x, force.y, v.z)
    # #logMessage("force: "+str(force))
    return sendDict()
