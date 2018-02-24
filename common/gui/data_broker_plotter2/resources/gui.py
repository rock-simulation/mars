#! /usr/bin/env python

import os
import sys
import subprocess
import yaml

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
gLayout.addWidget(QLabel("r"), 0, 3)
gLayout.addWidget(QLabel("g"), 0, 4)
gLayout.addWidget(QLabel("b"), 0, 5)
gLayout.addWidget(QLabel("a"), 0, 6)
gLayout.addWidget(QLabel("scale"), 0, 7)
gLayout.addWidget(QLabel("offset"), 0, 8)

labelList = []
scaleList = []
offsetList = []
showList = []
colorRList = []
colorGList = []
colorBList = []
colorAList = []
keyList = []
show = True
i = 1
for key, value in config.iteritems():
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

    colorr = QLineEdit(str(int(value["color"]["r"]*255)))
    maxWidth = QFontMetrics(colorr.font()).maxWidth()
    colorr.setMaxLength(3)
    colorr.setFixedWidth(3*maxWidth)
    colorRList.append(colorr)
    gLayout.addWidget(colorr, i, 3)
    colorg = QLineEdit(str(int(value["color"]["g"]*255)))
    colorg.setMaxLength(3)
    colorg.setFixedWidth(3*maxWidth)
    colorGList.append(colorg)
    gLayout.addWidget(colorg, i, 4)
    colorb = QLineEdit(str(int(value["color"]["b"]*255)))
    colorb.setMaxLength(3)
    colorb.setFixedWidth(3*maxWidth)
    colorBList.append(colorb)
    gLayout.addWidget(colorb, i, 5)
    colora = QLineEdit(str(int(value["color"]["a"]*255)))
    colora.setMaxLength(3)
    colora.setFixedWidth(3*maxWidth)
    colorAList.append(colora)
    gLayout.addWidget(colora, i, 6)

    s = ""
    if "scale" in value:
        s = str(value["scale"])
    scale = QLineEdit(s)
    scaleList.append(scale)
    gLayout.addWidget(scale, i, 7)

    s = ""
    if "offset" in value:
        s = str(value["offset"])
    offset = QLineEdit(s)
    offsetList.append(offset)
    gLayout.addWidget(offset, i, 8)

    i += 1


def update():
    global labelList, showList, colorRList, colorGList, colorBList, colorAList
    global keyList, config
    for i in xrange(len(keyList)):
        if len(labelList[i].text()) > 0:
            config[keyList[i]]["label"] = str(labelList[i].text())
        elif "label" in config[keyList[i]]:
            config[keyList[i]].pop("label", None)
        config[keyList[i]]["show"] = showList[i]
        config[keyList[i]]["color"]["r"] = float(colorRList[i].text())/255.
        config[keyList[i]]["color"]["g"] = float(colorGList[i].text())/255.
        config[keyList[i]]["color"]["b"] = float(colorBList[i].text())/255.
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
    with open("config.yml", "w") as f:
        f.write(yaml.dump(config))

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
