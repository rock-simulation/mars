# -*- coding: utf-8 -*-
"""
Created on Tue Oct 22 10:44:01 2013

@author: Kai von Szadkowski
"""

# This script detects all *.md files in MARS' doc folders and converts them
# to *.rst files using pandoc (http://johnmacfarlane.net/pandoc/)
# TODO:
#    There are still some mistakes when processing the .rst files to html, which can be
#    avoided by introducing changes directly here:
#        - insert { style="width: 80%;" } to image tags to avoid oversized images
#        - getting rid of doxygen-specific links

from subprocess import call
import os

print "Converting markdown files to rst format..."
for root, dirs, files in os.walk("../../"):
    for d in dirs:
        for f in os.listdir(os.path.join(root, d)):
            if f.endswith(".md") and f != "index.md": #don't create index.rst overwriting index.html!
                print "    Converting", f, "in", os.path.join(root, d)
                #print "pandoc", "-i", os.path.join(root, d, f), "-o", os.path.join(root, d, f[0:-3] + ".rst")
                call(["pandoc", "-i", os.path.join(root, d, f), "-o", os.path.join(root, d, f[0:-3] + ".rst")])