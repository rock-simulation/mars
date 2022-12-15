from time import time

iDict = {}
startTime = time()

def timing(s):
    global startTime
    currentTime = time()
    if currentTime > startTime+s:
        startTime = currentTime
        return True
    return False

def clearDict():
    global iDict
    iDict.clear()
    iDict["log"] = {}
    iDict["log"]["debug"] = []
    iDict["log"]["error"] = []
    iDict["commands"] = {}
    iDict["configMotorValues"] = {}
    iDict["applyForce"] = {}
    iDict["applyTorque"] = {}
    iDict["connectNodes"] = []
    iDict["disconnectNodes"] = []
    iDict["request"] = []
    iDict["config"] = {}
    iDict["PointCloud"] = {}
    iDict["ConfigPointCloud"] = {}
    iDict["Lines"] = {}
    iDict["CameraSensor"] = {}

def sendDict():
    global iDict
    return iDict

def setMotor(name, value):
    global iDict
    iDict["commands"][name] = {"value": value}

def setConfigMotor(name, value):
    global iDict
    iDict["configMotorValues"][name] = {"value": value}

def applyForce(name, value):
    global iDict
    iDict["applyForce"][name] = {"value": value}

def applyTorque(name, value):
    global iDict
    iDict["applyTorque"][name] = {"value": value}

def connectNodes(name1, name2):
    global iDict
    iDict["connectNodes"].append([name1, name2])

def disconnectNodes(name1, name2):
    global iDict
    iDict["disconnectNodes"].append([name1, name2])

def setRunning(value):
    if value:
        iDict["startSim"] = True
        iDict["stopSim"] = False
    else:
        iDict["startSim"] = False
        iDict["stopSim"] = True

def resetSim():
    iDict["resetSim"] = True

def quitSim():
    iDict["quitSim"] = True

def setUpdateTime(value):
    iDict["updateTime"] = float(value)

def requestNode(name):
    global iDict
    iDict["request"].append({"type": "Node", "name": name})

def requestSensor(name):
    global iDict
    iDict["request"].append({"type": "Sensor", "name": name})

def requestMotor(name):
    global iDict
    iDict["request"].append({"type": "Motor", "name": name})

def requestConfig(group, name):
    global iDict
    iDict["request"].append({"type": "Config", "group": group, "name": name})

def requestDataBroker(group_name, package_name, data_name):
    global iDict
    iDict["request"].append({"type": "DataBroker", "g": group_name, "name": package_name, "d": data_name})

def logMessage(s):
    global iDict
    iDict["log"]["debug"].append(s)

def logError(s):
    global iDict
    iDict["log"]["error"].append(s)

def setConfig(group, name, value):
    global iDict
    if "config" in iDict and group in iDict["config"]:
        iDict["config"][group][name] = value
    else:
        iDict["config"][group] = {name: value}

def createPointCloud(name, size):
    global iDict
    iDict["PointCloud"][name] = size

def configurePointCloud(name, size, r, g, b):
    global iDict
    iDict["ConfigPointCloud"][name] = [float(size), float(r),
                                       float(g), float(b)]

def handleLines(name, args):
    global iDict
    if not name in iDict["Lines"]:
        iDict["Lines"][name] = []
    iDict["Lines"][name].append(args)

def configureLines(name, size, r, g, b, bezier=0, interpolationPoints=20):
    handleLines(name, {"config": [float(size), float(r), float(g), float(b), int(bezier), int(interpolationPoints)]})

def clearLines(name):
    handleLines(name, "clear")

def removeLines(name):
    handleLines(name, "remove")

def appendLines(name, x, y, z):
    handleLines(name, {"append": [float(x), float(y), float(z)]})

def requestCameraSensor(name):
    global iDict
    if name in iDict["CameraSensor"]:
        iDict["CameraSensor"][name] |= 1
    else:
        iDict["CameraSensor"][name] = 1

def requestDepthCameraSensor(name):
    global iDict
    if name in iDict["CameraSensor"]:
        iDict["CameraSensor"][name] |= 2
    else:
        iDict["CameraSensor"][name] = 2

def pushData(group, name, dataName, value):
    global iDict
    if not "ToDataBroker" in iDict:
        iDict["ToDataBroker"] = []
    iDict["ToDataBroker"].append({"g":group, "n":name, "d":dataName, "v":value})

def edit(type_, name, key, value):
    global iDict
    if not "edit" in iDict:
        iDict["edit"] = {}
    if not type_ in iDict["edit"]:
        iDict["edit"][type_] = {}
    if not name in iDict["edit"][type_]:
        iDict["edit"][type_][name] = []
    iDict["edit"][type_][name].append({"k": key, "v": str(value)})

def editNode(name, key, value):
    edit("nodes", name, key, value)

def editJoint(name, key, value):
    edit("joints", name, key, value)

def editMotor(name, key, value):
    edit("motors", name, key, value)

def editMaterial(name, key, value):
    edit("materials", name, key, value)

def editGraphics(key, value):
    edit("graphics", "-1", key, value)

def editGraphicsWindow(winid, key, value):
    edit("graphics", str(winid), key, value)

def setNodePose(name, x, y, z, qx, qy, qz, qw):
    if not "edit" in iDict:
        iDict["edit"] = {"nodePose": {}}
    elif not "nodePose" in iDict["edit"]:
        iDict["edit"]["nodePose"] = {}
    iDict["edit"]["nodePose"][name] = [x, y, z, qx, qy, qz, qw]

def setSingleNodePose(name, x, y, z, qx, qy, qz, qw):
    if not "edit" in iDict:
        iDict["edit"] = {"nodePoseSingle": {}}
    elif not "nodePoseSingle" in iDict["edit"]:
        iDict["edit"]["nodePoseSingle"] = {}
    iDict["edit"]["nodePoseSingle"][name] = [x, y, z, qx, qy, qz, qw]
