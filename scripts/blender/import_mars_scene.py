import os
import sys
import shutil
import ZipFile
import textwrap

import xml.dom.minidom

def getGenericConfig(domElement):

    config = {}

    if domElement.hasAttributes():
        for key, value in domElement.attributes.items():
#            print "attrib [%s : %s]" % (key, value)
            if key not in config:
                config[key] = value
            else:
                print("Warning! Key '%s' already exists!" % key)

    if domElement.hasChildNodes():
        for child in domElement.childNodes:
            key   = child.nodeName
            value = child.nodeValue
            if child.hasChildNodes():
                value = getGenericConfig(child)
#                print "element [%s : %s]" % (key, value)
                if key not in config:
                    config[key] = value
                else:
                    print("Warning! Key '%s' already exists!" % key)
            else:
                # remove indentation and leading and trailing whitespaces
                value = textwrap.dedent(value).strip()
#                value = ''.join(value.split())
                ## FIXME: This seems wrong! what about the other cildren?
                if value != '':
                    return value

    return config


def parseNode(domElement):
    config = getGenericConfig(domElement)
#    print config["name"], ":", config
    return True


def main(fileDir, filename):
    tmpDir = os.path.join(fileDir, "tmp")
    
    # change the current directory
    if os.path.isdir(fileDir) :
        os.chdir(fileDir)
    else:
        print("ERROR! File path (%s) does not exist!" % fileDir)
        sys.exit(1)

    # if there is already a "tmp" directory delete it
    shutil.rmtree(tmpDir, ignore_errors=True)
    
    # extract the .scn file to the "tmp" directory
    ZipFile(os.path.join(fileDir, filename), "r").extractall(tmpDir)

    # path to the .scene file
    scenepath = os.path.join(tmpDir, filename.replace('.scn', '.scene'))

    # if the .scene file exists, read all of its content
    if os.path.isfile(scenepath) :
        dom = xml.dom.minidom.parse(scenepath)
    else:
        print("ERROR! Couldn't find .scene file (%s)!" % scenepath)
        sys.exit(1)

    # --- DO THE PARSING HERE!!! ---
    
    # parsing all nodes
    nodes = dom.getElementsByTagName("node")
    for node in nodes :
        if not parseNode(node):
            print("Error while parsing parsing node!")
            sys.exit(1)

    #cleaning up afterwards
    shutil.rmtree(tmpDir)


if __name__ == '__main__' :
    if len(sys.argv) != 2:
        print("USAGE: %s <fullpath_to_scn_file>" % sys.argv[0])
        sys.exit(1)
    if not os.path.isfile(sys.argv[1]):
        print('USAGE: %s <fullpath_to_scn_file>' % sys.argv[0])
        print('  Error: "%s" is not an existing file!' % sys.argv[1])
        sys.exit(1)
    fileDirectory, filename = os.path.split(sys.argv[1])
    main(fileDirectory, filename)
