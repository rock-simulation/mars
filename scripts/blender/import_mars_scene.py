import os

import xml.dom.minidom

def getGenericConfig(domElement):

    config = {}

    if domElement.hasAttributes():
        attributes = domElement.attributes
        for (key,value) in attributes.items():
#            print "attrib [%s : %s]" % (key, value)
            if key not in config.keys():
                config[key] = value
            else:
                print "Warning! Key \'%s\' already exists!" % key

    if domElement.hasChildNodes():
        children = domElement.childNodes
        for child in children:
            key   = child.nodeName
            value = child.nodeValue
            if child.hasChildNodes():
                value = getGenericConfig(child)
#                print "element [%s : %s]" % (key, value)
                if key not in config.keys():
                    config[key] = value
                else:
                    print "Warning! Key \'%s\' already exists!" % key
            else:
                value = ''.join(value.split())
                if value != '':
                    return value

    return config


def parseNode(domElement):
    config = getGenericConfig(domElement)
#    print config["name"], ":", config
    return True


def main():
    filepath = "/Users/bergatt/Desktop/import_mars_scene"
    filename = "aila_clean.scn"

    # change the current directory
    if os.path.isdir(filepath) :
        os.chdir(filepath)
    else:
        print "ERROR! File path (%s) does not exist!" % filepath
        exit()

    # if there is already a "tmp" directory delete it
    if os.path.isdir(filepath+os.sep+"tmp") :
        os.system("rm -rf tmp")
    
    # extract the .scn file to the "tmp" directory
    os.system("unzip %s -d tmp" % filename)

    # path to the .scene file
    scenepath = filepath+os.sep+"tmp"+os.sep+filename.replace('.scn','.scene')

    # if the .scene file exists, read all of its content
    if os.path.isfile(scenepath) :
        dom = xml.dom.minidom.parse(scenepath)
    else:
        print "ERROR! Couldn't find .scene file (%s)!" % scenepath
        exit()

    # --- DO THE PARSING HERE!!! ---
    
    # parsing all nodes
    nodes = dom.getElementsByTagName("node")
    for node in nodes :
        if not parseNode(node):
            print "Error while parsing parsing node!"
            exit()

    #cleaning up afterwards
    os.system("rm -rf tmp")

if __name__ == '__main__' :
    main()