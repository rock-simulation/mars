# -*- coding: utf-8 -*-
import os
import textwrap
#from docutils import core
import subprocess
#import re #regular expressions
from PIL import Image    

def compareLists(a, b):
    lena = len(a)
    lenb = len(b)
    different = 0
    for i in range(min(lena, lenb)):
        if a[i] != b[i]:
            different += 1
    return different + abs(lena-lenb)
    

def createHeader(relPath, linklist, fileName, local_links):
    # create linklist
    li_list = ""
    indent = 0
    prevdirs = []
    for f in linklist:
        if f["fileName"] == "index.md" or f["outPath"].find("/groups/") >= 0:
            continue
        
        dirs = f["outPath"][3:].split("/")[:-1]
        n_subfolders = len(dirs)
        print prevdirs, dirs, compareLists(dirs, prevdirs)
        diff = compareLists(dirs, prevdirs)
        if diff > 1 and dirs != []:
            li_list += '</ul>\n'
        if n_subfolders > indent:
            if (n_subfolders-indent) == 1:
                li_list += '  '*indent+ '<li>' + dirs[n_subfolders-indent].title() + '</li>\n'                
            if (n_subfolders-indent) > 1:
                li_list += '  '*indent+ '<li>' + dirs[n_subfolders-indent-1].title() + '</li>\n'
            li_list += '  '*indent+'<ul>\n  '#*(n_subfolders-indent)
        elif n_subfolders < indent:
            li_list += '</ul>\n'*(indent-n_subfolders-1)
        li_list += '  '*indent + '<li><a href="' + relPath + os.path.join(f["outPath"][2:], f["fileName"][:-3]+".html") + '">' + f["fileName"][:-3].title().replace("_", " ") + '</a></li>\n'
        indent = n_subfolders
        prevdirs = dirs
        if f["fileName"] == fileName: #if we have to add the local links:
            level = 1
            for ll in local_links:
                name = ll[1]
                if ll[0] > level:
                    li_list += '<ul>\n  ' * (ll[0]-level)
                elif ll[0] < level:
                    li_list += '</ul>\n'*(level-ll[0])
                li_list += '<li><a href=#' + name + '>' + name.title().replace("-", " ") + '</a></li>\n'
                level = ll[0]
            li_list += '</ul>\n'*(level-1)
    if indent > 0:
        li_list += '</ul>\n'*(indent)

    out = '''\
             <!DOCTYPE html>
             <html>
               <head>
                 <title>MARS Simulator</title>
                 <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
                 <meta name="description" content="MARS is a flexible physics simulator.">
                 <meta name="author" content="MARS Project">
                 <meta name="keywords" content="MARS, simulation, physics, robotics">
                 <link rel="stylesheet" type="text/css" href="{relPath}/src/css/mars_default.css" media="all" />
               </head>
             <body>
               <div class="nav-box">
                 <h2>Navigation</h2>
                 <nav>
                   <ol>
                     <li><a href="{relPath}/mars_manual/index.html">Home</a></li>
                     {links}
                   </ol>
                 </nav>
               </div>
               <div id="content">
               <header>
                 <a href="{relPath}/mars_manual/index.html"><img src="{relPath}/src/images/logo_v2_wob.png" alt="MARS" /></a>
               </header>
       '''.format(relPath = relPath, links = li_list)
    return textwrap.dedent(out)

def createFooter():
    out = '''\
          <footer>
            <a href="http://validator.w3.org/check?uri=referer" target="_blank">
              <img src="http://www.w3.org/Icons/valid-html401"
                   alt="Valid HTML 4.01 Transitional"/>
            </a>
            <a href="http://jigsaw.w3.org/css-validator/check/referer" target="_blank">
              <img src="http://jigsaw.w3.org/css-validator/images/vcss"
                   alt="Valid CSS!"/>
            </a>
          </footer>
        </div>
      </body>
    </html>
          '''
    return textwrap.dedent(out)

def create_outpath(filePath):
    # this part is rather ugly, but os.path.join() didn't work, either
    tmpPath = filePath.replace("../", "")
    tmpPath = tmpPath.replace("doc", "")
    tmpPath = tmpPath.replace("/src", "")
    if len(tmpPath) > 0:
        if tmpPath[0] == "/":
            tmpPath = tmpPath[1:]
        if tmpPath[-1] != "/":
            tmpPath = tmpPath+"/"
    outPath = "../mars_manual/" + tmpPath
    return outPath

def constructRelativePath(filePath):
    n = filePath[:-1].count("/")
    if n == 0:
        return ".", 0
    else:
        relPath = ".."
        for i in range(n-1):
            relPath+="/.."
        return relPath, n

def convertMdToHtml(linklist, f):
    filePath = linklist[f]["filePath"]
    fileName = linklist[f]["fileName"]
    outPath = linklist[f]["outPath"]
    
    if outPath.find("/groups/") >= 0:
        print "Skipping group index file:", linklist[f]["fileName"]
        return 0

    # read in and convert markdown file
    inFile = open(os.path.join(filePath, fileName), "r")
    bodyString = inFile.read()
    inFile.close()
    #bodyString = core.publish_parts(bodyString, writer_name='html') ['html_body'] #original code for rst-html-conversion
    pandoc = subprocess.Popen(["pandoc", os.path.join(filePath, fileName), "-f", "markdown+pipe_tables", "-t", "html"], stdout = subprocess.PIPE)
    bodyString = pandoc.communicate()[0]#.replace('''<div class="figure">\n<img''', '''<div class="figure">\n<img width="100%"''')

    # create local links
    local_links = []
    bodyLines = str(bodyString).split("\n")
    for line in bodyLines:
        pos = line.find("<h") # create local link list
        if pos >= 0:
            level = int(line[pos+2])
            id_pos = line.find('id="')
            name = line[id_pos+4:line.find('"', id_pos+4)]
            if level > 1:
                local_links.append([level, name])

    # create header and footer
    relPathInfo = constructRelativePath(outPath)
    relPath = relPathInfo[0]
    n_level = relPathInfo[1]
    headerString = createHeader(relPath, linklist, fileName, local_links)
    footerString = createFooter()                
    
    # edit HTML output of pandoc    
    alt_bodyString = ""
    for line in bodyLines:    
        # line-by-line modifications of pandoc's html output
        altLine = line
        minstart = 0
        # get rid of doxygen class links
        while altLine.find("::") >= 0 and altLine.find("<a") >= 0:
            #print altLine
            pos = altLine.find("::", minstart)
            start = altLine.rfind("<a", minstart, pos)
            if start < 0:
                minstart=pos+1
                continue
            end = altLine.find("/a>", pos)+3
            name_start = altLine.rfind(">", start, end-3)
            #print start, pos, name_start, end
            name = altLine[name_start+1:end-4]
            if start < name_start and name_start < end:
                altLine = altLine.replace(altLine[start:end], "<i>" + name + "</i>")
                #print name
            #print altLine, "\n"
        if altLine.find("<img src=") >= 0:
            url_start = altLine.find('src="') + 5
            url_end = altLine.find('"', url_start)
            url = altLine[url_start:url_end]
            image_filename = url.replace("../", "../src/")
            #print image_filename
            try:
                im = Image.open(image_filename)
                print "Rescaling image:", image_filename
                width, height = im.size
                ratio = height*1.0 / width*1.0
                img_width = 900
                if width > img_width:
                    output_width = img_width
                else:
                    output_width = width
                #print output_width, ratio, int(output_width*ratio)
                altLine = altLine.replace("<img src=", "<img " + 'width="' + str(int(output_width)) + '" height="' +str(int(ratio*output_width)) + '" src=')
            except IOError:
                print "Image could not be loaded:", fileName, "-", image_filename
            altLine = altLine.replace(url, "../" * (n_level) + image_filename.replace("../",""))
        alt_bodyString += altLine+"\n"

    #output
    if not os.path.exists(outPath):
        os.makedirs(outPath)
    outFile = open(os.path.join(outPath, fileName[:-3]+".html"), "w")
    outFile.write("<!-- DO NOT EDIT THIS FILE! IT IS AUTOMATICALLY GENERATED BY markdown2marshtml //-->\n")
    outFile.write(headerString)
    outFile.write(alt_bodyString)
    outFile.write(footerString)
    outFile.close()
    #print "    Converting " + filePath + "/" + fileName + " to " + os.path.join(outPath, fileName[:-3] + ".html")
    return 1

if __name__ == '__main__':

    #the following code browses through all directories and automatically converts all .md files
    print "Converting .md to mars/doc/*.html..."
    linklist = []
    #find all links
    for root, dirs, files in os.walk("../../"):
        for f in files:
            if f.endswith(".md"):
                linklist.append({"filePath": root, "fileName": f, "outPath": create_outpath(root)})
    n_files = 0;
    for f in range(len(linklist)):
        n_files += convertMdToHtml(linklist, f)
        
    print "In total,", str(n_files)+"/"+str(len(linklist)), "files were processed."


