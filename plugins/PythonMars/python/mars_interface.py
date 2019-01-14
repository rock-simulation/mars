from time import clock

iDict = {}
startTime = clock()

def timing(s):
    global startTime
    currentTime = clock()
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

def setRunning(value):
    if value:
        iDict["startSim"] = True
        iDict["stopSim"] = False
    else:
        iDict["startSim"] = False
        iDict["stopSim"] = True

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
    iDict["edit"][type_][name].append({"k": key, "v": value})

def editNode(name, key, value):
    edit("nodes", name, key, value)

def editJoint(name, key, value):
    edit("joints", name, key, value)

def editMotor(name, key, value):
    edit("motors", name, key, value)

def editGraphics(key, value):
    edit("graphics", "-1", key, value)

def editGraphicsWindow(winid, key, value):
    edit("graphics", str(winid), key, value)
