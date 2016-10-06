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

def requestNode(name):
    global iDict
    iDict["request"].append({"type": "Node", "name": name})

def requestSensor(name):
    global iDict
    iDict["request"].append({"type": "Sensor", "name": name})

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
    iDict["config"][group] = {name: value}

def createPointCloud(name, size):
    global iDict
    iDict["PointCloud"][name] = size
