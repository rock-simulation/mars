#! /usr/bin/env python

import os
import sys
import subprocess
import yaml
import collections

haveQT5 = True
try:
    from PyQt5.QtWidgets import *
    from PyQt5.QtCore import *
    from PyQt5.QtGui import *
except:
    haveQT5 = False
if not haveQT5:
    from PyQt4.QtCore import *
    from PyQt4.QtGui import *
#import pybob

packages = []
pattern = ""
currentPackage = ""
app = QApplication(sys.argv)

# backup the original config
if not os.path.exists("config_backup.yml"):
    os.system("cp config.yml config_backup.yml")

config = {}
with open("config.yml", "r") as f:
    config = yaml.load(f)
config = collections.OrderedDict(sorted(config.items()))

window = QWidget()
window.setWindowTitle("Plot")
subWindow = QWidget()
vLayout = QVBoxLayout()
subWindow.setLayout(vLayout)
gLayout = QGridLayout()
vLayout.addLayout(gLayout)
gLayout.addWidget(QLabel("file"), 0, 0)
gLayout.addWidget(QLabel("label"), 0, 1)
gLayout.addWidget(QLabel("show"), 0, 2)
gLayout.addWidget(QLabel("rgb"), 0, 3)
gLayout.addWidget(QLabel("a"), 0, 4)
gLayout.addWidget(QLabel("scale"), 0, 5)
gLayout.addWidget(QLabel("offset"), 0, 6)

s1 = ""
s2 = ""
if "_settings" in config:
    if "xlim" in config["_settings"]:
        s1 = config["_settings"]["xlim"]["min"]
        s2 = config["_settings"]["xlim"]["max"]
xmin = QLineEdit(str(s1))
xmax = QLineEdit(str(s2))

labelList = []
scaleList = []
offsetList = []
showList = []
colorRGBList = []
colorAList = []
keyList = []
show = True
i = 1
for key, value in config.iteritems():
    if key == "_settings":
        continue
    keyList.append(key)
    gLayout.addWidget(QLabel(key), i, 0)

    label = ""
    if "label" in value:
        label = value["label"]
    qlabel = QLineEdit(label)
    labelList.append(qlabel)
    gLayout.addWidget(qlabel, i, 1)

    check = QCheckBox("")
    showList.append(check)
    check.setChecked(value["show"])
    gLayout.addWidget(check, i, 2)

    s1 = ""
    if "rgb" in value["color"]:
        s1 = value["color"]["rgb"]
    else:
        s1 = "#"
        s2 = hex(int(value["color"]["r"]*255))[2:]
        if len(s2) == 1:
            s1 += "0"
        s1 += s2
        s2 = hex(int(value["color"]["g"]*255))[2:]
        if len(s2) == 1:
            s1 += "0"
        s1 += s2
        s2 = hex(int(value["color"]["b"]*255))[2:]
        if len(s2) == 1:
            s1 += "0"
        s1 += s2
    colorrgb = QLineEdit(s1)
    colorrgb.setMaxLength(8)
    maxWidth = colorrgb.fontMetrics().maxWidth()
    if maxWidth == 0:
        maxWidth = colorrgb.fontMetrics().width("W")
    if maxWidth > 0:
        colorrgb.setFixedWidth(7*maxWidth)
    colorRGBList.append(colorrgb)
    gLayout.addWidget(colorrgb, i, 3)
    s1 = "255"
    if "a" in value["color"]:
        s1 = str(int(value["color"]["a"]*255))
    colora = QLineEdit(s1)
    colora.setMaxLength(3)
    if maxWidth > 0:
        colora.setFixedWidth(3*maxWidth)
    colorAList.append(colora)
    gLayout.addWidget(colora, i, 4)

    s = ""
    if "scale" in value:
        s = str(value["scale"])
    scale = QLineEdit(s)
    scaleList.append(scale)
    gLayout.addWidget(scale, i, 5)

    s = ""
    if "offset" in value:
        s = str(value["offset"])
    offset = QLineEdit(s)
    offsetList.append(offset)
    gLayout.addWidget(offset, i, 6)

    i += 1


def update():
    global labelList, showList, colorRList, colorGList, colorBList, colorAList
    global keyList, config, xmin, xmax
    for i in xrange(len(keyList)):
        if len(labelList[i].text()) > 0:
            config[keyList[i]]["label"] = str(labelList[i].text())
        elif "label" in config[keyList[i]]:
            config[keyList[i]].pop("label", None)
        config[keyList[i]]["show"] = showList[i]
        config[keyList[i]]["color"]["rgb"] = str(colorRGBList[i].text())
        config[keyList[i]]["color"].pop("r", None)
        config[keyList[i]]["color"].pop("g", None)
        config[keyList[i]]["color"].pop("b", None)
        config[keyList[i]]["color"]["a"] = float(colorAList[i].text())/255.
        config[keyList[i]]["show"] = showList[i].isChecked()
        if len(scaleList[i].text()) > 0:
            config[keyList[i]]["scale"] = float(scaleList[i].text())
        elif "scale" in config[keyList[i]]:
            config[keyList[i]].pop("scale", None)
        if len(offsetList[i].text()) > 0:
            config[keyList[i]]["offset"] = float(offsetList[i].text())
        elif "offset" in config[keyList[i]]:
            config[keyList[i]].pop("offset", None)
    if len(xmin.text()) > 0 and len(xmax.text()) > 0:
        minx = float(xmin.text())
        maxx = float(xmax.text())
        if not "_settings" in config:
            config["_settings"] = {}
            config["_settings"]["file"] = "_settings_dummy_file"
        config["_settings"]["xlim"] = {"min": minx, "max": maxx}
    else:
        if "_settings" in config:
            if "xlim" in config["_settings"]:
                config["_settings"].pop("xlim", None)


    with open("config.yml", "w") as f:
        f.write(yaml.dump(dict(config)))

def plot():
    os.system("python plot.py; open graph.pdf")

def toggle():
    global showList, show
    for t in showList:
        t.setChecked(show)
    show = not show

mainLayout = QVBoxLayout()
area = QScrollArea()
area.setWidget(subWindow)
mainLayout.addWidget(area)
mainLayout.addWidget(QLabel("settings"))

# todo add clear button
hlayout = QHBoxLayout()
hlayout.addWidget(QLabel("x_min"))
hlayout.addWidget(xmin)
hlayout.addWidget(QLabel("x_max"))
hlayout.addWidget(xmax)
mainLayout.addLayout(hlayout)
toggleb = QPushButton("toggle all")
mainLayout.addWidget(toggleb)
push = QPushButton("Update Config")
mainLayout.addWidget(push)
plotb = QPushButton("Plot")
mainLayout.addWidget(plotb)
window.setLayout(mainLayout)

if haveQT5:
    push.clicked.connect(update)
    plotb.clicked.connect(plot)
    toggleb.clicked.connect(toggle)
else:
    push.connect(push, SIGNAL("clicked()"), update)
    plotb.connect(plotb, SIGNAL("clicked()"), plot)
    toggleb.connect(toggleb, SIGNAL("clicked()"), toggle)

window.resize(800, 500)
window.show()

sys.exit(app.exec_())
