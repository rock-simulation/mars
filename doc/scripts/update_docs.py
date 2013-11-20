# -*- coding: utf-8 -*-
"""
Created on Tue Oct 22 10:44:33 2013

@author: Kai von Szadkowski
"""

# This script executes three other python scripts and runs doxygen
# in the correct order for documentation to be nicely updated.

from subprocess import call
import os

# call(["python", "markdown2rst.py"])
# call(["python", "rst2marshtml.py"])
call(["python", "markdown2marshtml.py"])
call(["python", "create_subproject_doxygens.py"])
resp = raw_input("\nRun Doxygen on main folder? (Y/N) ")
if resp == "y" or resp == "Y":
    os.chdir("../doxygen")
    call(["doxygen", "mars_doxyconf"])
    call(["ln", "-s", "doxygen/html/index.html", "../mars_doxygen_index.html"])
print "\n Mars documentation up to date."
