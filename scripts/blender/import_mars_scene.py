import os

import xml.etree.ElementTree as ET

def main():
    filepath = "/Users/bergatt/Desktop/import_mars_scene"
    filename = "aila_clean.scn"

    # change the current directory
    if os.path.isdir(filepath):
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
    if os.path.isfile(scenepath):
        tree = ET.parse(scenepath)
    else:
        print "ERROR! Couldn't find .scene file (%s)!" % scenepath
        exit()

    # --- DO THE PARSING HERE!!! ---

    # cleaning up afterwards
    os.system("rm -rf tmp")    

if __name__ == '__main__':
    main()