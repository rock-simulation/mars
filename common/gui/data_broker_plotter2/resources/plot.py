#! /usr/bin/env python

import yaml
import os

import matplotlib.pyplot as plt
#from matplotlib.patches import Polygon
import matplotlib as mpl


#from matplotlib import rc
#rc('font',**{'family':'sans-serif','sans-serif':['Helvetica']})
## for Palatino and other serif fonts use:
#rc('font',**{'family':'serif','serif':['Palatino']})
#rc('text', usetex=True)

lineStyles = ['-', 'k:', '-.', '--', '-o', '-^', '-v', '-^', '--v', 'k-', 'k--', 'k-.', 'k:', 'k-v', 'k--v', 'k:v', '-']

mpl.rcParams['lines.linewidth'] = 1.5
mpl.rcParams['axes.linewidth'] = 1
mpl.rcParams['axes.labelsize'] = mpl.rcParams['axes.titlesize']
mpl.rcParams['text.usetex'] = True
mpl.rcParams['text.latex.preamble'] = [r"\usepackage[varg]{txfonts}"]
black = '#222222'
white = '#ffffff'
mpl.rcParams['lines.color'] = black
mpl.rcParams['text.color'] = black
mpl.rcParams['patch.facecolor'] = white
mpl.rcParams['axes.edgecolor'] = black
mpl.rcParams['axes.labelcolor'] = black
mpl.rcParams['axes.facecolor'] = white
mpl.rcParams['xtick.color'] = black
mpl.rcParams['ytick.color'] = black
mpl.rcParams['font.size'] = '14.0'
mpl.rcParams['lines.markeredgewidth'] = '0.0'
mpl.rcParams['lines.markersize'] = '8'

fSize = 1.0
fHeight = 5

# plot function for mars/data_broker_plotter2 export
def doPlot():
    plt.clf()
    fig, ax1 = plt.subplots(figsize=(12.5*fSize,fHeight*fSize))
    plt.title("")
    plt.xlabel("simulation time in ms")
    plt.ylabel("depends")
    #plt.subplots_adjust(left=0.06, right=0.992, top=0.895, bottom=0.099)

    dataList = []
    config = {}
    with open("config.yml", "r") as f:
        config = yaml.load(f)
    configList = {}
    i = 0
    for d in os.listdir("."):
        if d[-4:] == ".csv":
            with open(d, "r") as f:
                dataList.append(f.readlines())
            label = d[0:-4]
            c = {}
            for key,value in config.iteritems():
                if value["file"] == label:
                    if "label" not in value:
                        s = key
                        if key[:8] == "mars_sim":
                            arrString = key.split("_")
                            s = "_".join(arrString[2:])
                        value["label"] = s
                    value["key"] = key
                    configList[i] = value
                    break
            i += 1

    arrDataX = []
    arrDataY = []

    for data in dataList:
        x = []
        y = []
        row = 0
        for line in data:
            arrLine = line.split()
            if len(arrLine) == 2:
                x.append(float(arrLine[0]))
                y.append(float(arrLine[1]))
        if len(x) > 1:
            arrDataX.append(x)
            arrDataY.append(y)

    ll = []
    for i in range(len(arrDataX)):
        if i in configList:
            c = configList[i]
            r = hex(int(float(c["color"]["r"])*255)).split('x')[1]
            g = hex(int(float(c["color"]["g"])*255)).split('x')[1]            
            b = hex(int(float(c["color"]["b"])*255)).split('x')[1]
            a = float(c["color"]["a"])
            if len(r) == 1: r = "0" + str(r)
            if len(g) == 1: g = "0" + str(g)
            if len(b) == 1: b = "0" + str(b)
            cl = "#"+r+g+b        
            l, = plt.plot(arrDataX[i], arrDataY[i], lineStyles[0], label=c["label"],
                          linewidth=1.5, color=cl, alpha=a)
            ll.append(l)
        else:
            l, = plt.plot(arrDataX[i], arrDataY[i], lineStyles[0], label="plot_"+str(i),
                          linewidth=1.5)
            ll.append(l)

    legend = plt.legend(handles=ll, bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
                        ncol=len(ll), mode="expand", borderaxespad=0.,
                        handletextpad=0.4, markerscale=2.0)

    plt.savefig("graph.pdf", facecolor=white, edgecolor=black)
    plt.close('all')


doPlot()
