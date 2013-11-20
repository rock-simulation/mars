# -*- coding: utf-8 -*-
"""
Created on Tue Oct 22 10:44:01 2013

@author: Kai von Szadkowski
"""

# This script has to be executed from MARS' /mars/doc/doxygen folder, where it
# should be located. It browses through all directories of mars, detecting
# any folders containing a Markdown file with the same name as the folder, e.g.
# /mars/common/data_broker/data_broker.md
# For any such folder/file pair, a doxygen configuration is created in the
# subfolder "/doxygen/", along with "/css/" and "/images/" subfolders.
# The user can then decide to run all doxygen configurations to create
# documentation for the identified sub-projects, in wich case a softlink
# to the index is additionally created in the /doc/ folder.
#
# NOTE: The script overwrites any locally changed configfiles with the standard
# template that's found in the folder /mars/doc/doxygen/subproject_doxygen_template

from subprocess import call
import os
import shutil

def safe_mkdir(path):
    if not os.path.exists(path):
        os.makedirs(path)

doxygen_list = []
print "Detecting *.md files describing sub-projects..."
for root, dirs, files in os.walk("../../"):
    for d in dirs:
        if d == "doc":
            for f in os.listdir(os.path.join(root, d)):
                if f.endswith(".md") and f[0:-3] == os.path.basename(root):
                    print root
                    subproject = os.path.basename(root)
                    safe_mkdir(root+"/doc/doxygen")
                    safe_mkdir(root+"/doc/src/css")
                    shutil.copy("../src/css/mars_doxygen.css", root+"/doc/src/css")
                    configfile_path = root+"/doc/doxygen/"+subproject+"_doxyconf"
                    shutil.copyfile("../src/subproject_doxygen_template/subproject_doxyconf", configfile_path )
                    with open(configfile_path, 'r') as f:
                        doxy_cfg = f.read()
                    subproject_title = subproject.replace("_", " ").title()
                    doxy_cfg = doxy_cfg.replace("@subproject_title", '"'+subproject_title+'"')                    
                    doxy_cfg = doxy_cfg.replace("@subproject", subproject)
                    with open(configfile_path, 'w') as f:
                        f.write(doxy_cfg)
                    doxygen_list.append(configfile_path)   
print "\n", len(doxygen_list), "doxygen configurations were created:"
for d in doxygen_list:
    print d
resp = raw_input("\nRun Doxygen on these folders? (Y/N) ")
if resp == "Y" or resp == "y":
    iwd = os.getcwd()
    for doxyfile in doxygen_list:
        os.chdir(iwd)
        os.chdir(os.path.dirname(doxyfile))
        call(["doxygen", os.path.basename(doxyfile)])
        call(["ln", "-s", "doxygen/html/index.html", "../"+os.path.basename(doxyfile)[0:-9]+"_doxygen_index.html"])
        
    print "\nDoxygen documentation for sub-projects created."    


