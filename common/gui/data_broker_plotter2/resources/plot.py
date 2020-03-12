#! /usr/bin/env python

import yaml
import os



import sys

oldPython = False
if sys.version_info[0] < 3:
    oldPython = True

def getItems(d):
    if oldPython:
        return d.iteritems()
    return d.items()

import matplotlib.pyplot as plt
#from matplotlib.patches import Polygon
import matplotlib as mpl
from matplotlib.ticker import FormatStrFormatter

from matplotlib import rc
#rc('font',**{'family':'sans-serif','sans-serif':['Helvetica']})
## for Palatino and other serif fonts use:
#rc('font',**{'family':'serif','serif':['Palatino']})
#rc('text', usetex=True)
#rc('font',**{'family':'serif','serif':['']})

lineStyles = ['-', 'k:', '-.', '--', '-o', '-^', '-v', '-^', '--v', 'k-', 'k--', 'k-.', 'k:', 'k-v', 'k--v', 'k:v', '-']

mpl.rcParams['lines.linewidth'] = 1.5
mpl.rcParams['axes.linewidth'] = 1
mpl.rcParams['axes.labelsize'] = mpl.rcParams['axes.titlesize']
mpl.rcParams['text.usetex'] = True
#mpl.rcParams['text.latex.preamble'] = [r"\usepackage[varg]{txfonts}"]
mpl.rcParams['text.latex.preamble'] = [r"\usepackage{amsmath}"]
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

# plot function for mars/data_broker_plotter2 export
def doPlot():
    fSize = 1.0
    fHeight = 5
    config = {}
    with open("config.yml", "r") as f:
        config = yaml.load(f)
    if "_settings" in config:
        if "plotHeight" in config["_settings"]:
            fHeight = config["_settings"]["plotHeight"]
        if "plotSize" in config["_settings"]:
            fSize = config["_settings"]["plotSize"]
    plt.clf()
    fig, ax1 = plt.subplots(figsize=(12.5*fSize,fHeight*fSize))
    plt.title("")
    plt.xlabel("simulation time in ms")
    plt.ylabel("depends")
    #plt.xticks([])
    #ax1.xaxis.set_ticklabels([])
    plt.subplots_adjust(left=0.06, right=0.992, top=0.895, bottom=0.099)
    #ax1.yaxis.set_major_formatter(FormatStrFormatter('%.1f'))
    plotLegend = True
    dataList = []
    configList = {}
    if "_settings" in config:
        if "xlim" in config["_settings"]:
            plt.xlim(config["_settings"]["xlim"]["min"],
                     config["_settings"]["xlim"]["max"])
        if "ylim" in config["_settings"]:
            plt.ylim(config["_settings"]["ylim"]["min"],
                     config["_settings"]["ylim"]["max"])
        if "xlabel" in config["_settings"]:
            plt.xlabel(config["_settings"]["xlabel"])
        if "ylabel" in config["_settings"]:
            plt.ylabel(config["_settings"]["ylabel"])
        if "adjust" in config["_settings"]:
            plt.subplots_adjust(left=config["_settings"]["adjust"]["left"],
                                right=config["_settings"]["adjust"]["right"],
                                top=config["_settings"]["adjust"]["top"],
                                bottom=config["_settings"]["adjust"]["bottom"])
        if "useLatexFont" in config["_settings"] and config["_settings"]["useLatexFont"]:
            rc('font',**{'family':'serif','serif':['']})
        if "fontSize" in config["_settings"]:
            mpl.rcParams['font.size'] = str(config["_settings"]["fontSize"])
        if "plotLegend" in config["_settings"]:
            plotLegend = config["_settings"]["plotLegend"]
        if "plotXLabels" in config["_settings"] and not config["_settings"]["plotXLabels"]:
            ax1.xaxis.set_ticklabels([])
        if "plotYLabels" in config["_settings"] and not config["_settings"]["plotYLabels"]:
            ax1.yaxis.set_ticklabels([])

    i = 0
    for d in os.listdir("."):
        if d[-4:] == ".csv":
            label = d[0:-4]
            c = {}
            found = False
            show = False
            for key,value in getItems(config):
                if value["file"] == label:
                    found = True
                    if value["show"]:
                        show = True
                        with open(d, "r") as f:
                            dataList.append(f.readlines())
                    if "label" not in value:
                        s = key
                        if key[:8] == "mars_sim":
                            arrString = key.split("_")
                            s = "_".join(arrString[2:])
                        value["label"] = s.replace("_", "\_")
                    value["key"] = key
                    configList[i] = value
                    break
            if show:
                i += 1
            if not found:
                print "found no config for: " + label

    arrData = []

    i = 0

    for data in dataList:
        scale = 1.0
        offset = 0.0
        shift = 0.0
        xscale = 1.0
        order = 0
        if i in configList:
            c = configList[i]
            if "scale" in c:
                scale = c["scale"]
            if "offset" in c:
                offset = c["offset"]
            if "shift" in c:
                shift = c["shift"]
            if "xscale" in c:
                xscale = c["xscale"]
            if "order" in c:
                order = c["order"]
        x = []
        y = []
        row = 0
        for line in data:
            arrLine = line.split()
            if len(arrLine) == 2:
                x.append(float(arrLine[0])*xscale+shift)
                y.append(float(arrLine[1])*scale+offset)
        if len(x) > 1:
            arrData.append([order, x, y, i])
        i += 1

    ll = []
    arrData.sort(key=lambda x: x[0])
    for data in arrData:
        i = data[3]
        if i in configList:
            c = configList[i]
            cl = "#000000"
            a = 1.0
            if "rgb" in c["color"]:
                cl = c["color"]["rgb"]
            else:
                r = hex(int(float(c["color"]["r"])*255)).split('x')[1]
                g = hex(int(float(c["color"]["g"])*255)).split('x')[1]
                b = hex(int(float(c["color"]["b"])*255)).split('x')[1]
                if len(r) == 1: r = "0" + str(r)
                if len(g) == 1: g = "0" + str(g)
                if len(b) == 1: b = "0" + str(b)
                cl = "#"+r+g+b
            if "a" in c["color"]:
                a = float(c["color"]["a"])
            l, = plt.plot(data[1], data[2], lineStyles[0], label=c["label"],
                          linewidth=1.5, color=cl, alpha=a)
            ll.append(l)
        else:
            l, = plt.plot(data[1], data[2], lineStyles[0], label="plot_"+str(i),
                          linewidth=1.5)
            ll.append(l)

    if plotLegend:
        legend = plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
                            ncol=len(ll), mode="expand", borderaxespad=0.,
                            handletextpad=0.4, markerscale=2.0)

    plt.grid()
    plt.savefig("graph.pdf", facecolor=white, edgecolor=black)
    plt.close('all')


doPlot()
