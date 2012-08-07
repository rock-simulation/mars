import sys
import os
import re
import tempfile
    
def depsFromCMakeLists(rootDir):
    for root, dirs, files in os.walk(rootDir):
        if "CMakeLists.txt" not in files:
            continue
        fileContent = open(os.path.join(root, "CMakeLists.txt"), "r").read()
        pattern = r'project\((\w+)\)'
        results = re.search(pattern, fileContent)
        if results is None:
            print "could not find project name in CMakeLists.txt in dir: \"%s\"" % root
            continue
        libName = results.group(1)
        pattern = r'pkg_check_modules\s*\(\s*\w+\s*((?:[-"a-zA-Z0-9_]+(?:\s|;)*)+)\)'
        cnt = 0
        for results in re.finditer(pattern, fileContent):
            cnt += 1
            for p in (p.strip() for p in results.group(1).split()):
                if p == "REQUIRED" or p == "QUIETLY":
                    continue
                yield (root, libName, p.strip('"'))

        if cnt == 0:
            print 'INFO: package "%s" seems to have no dependencies' % libName
            
def depsFromPkgConfig(rootDir):
    for root, dirs, files in os.walk(rootDir):
        potentialFiles = [f for f in files if f.endswith(".pc.in")]
        if len(potentialFiles) == 0:
            continue
        if len(potentialFiles) > 1:
            print 'WARNING: more than one pkg_config file in dir "%s"' % root
            continue
        fileContent = open(os.path.join(root, potentialFiles[0]), "r").read()
        libName = potentialFiles[0][:-6]
        pattern = r'Requires.*:\s*((?:[-"a-zA-Z0-9_]+(?:\s|;)*)+?)[\n]'
        cnt = 0
        for results in re.finditer(pattern, fileContent):
            cnt += 1
            for p in (p.strip() for p in results.group(1).split()):
                if p == "REQUIRED" or p == "QUIETLY":
                    continue
                yield (root, libName, p.strip('"'))
        if cnt == 0:
            print 'INFO: package "%s" seems to have no dependencies' % libName

def depsFromManifest(rootDir):
    for root, dirs, files in os.walk(rootDir):
        if "manifest.xml" not in files:
            continue
        fileContent = open(os.path.join(root, "manifest.xml"), "r").read()
        pattern = r'<description brief="(\w+)"'
        results = re.search(pattern, fileContent)
        if results is None:
            print "WARNING: %s doesn't contain a brief description" % os.path.join(root, "manifest.xml")
            continue
        libName = results.group(1)
        pattern = r'(?:(?:depend package=")|(?:rosdep name="))(?:\w+/)*([-/a-zA-Z0-9_]+)"'
        cnt = 0
        for results in re.finditer(pattern, fileContent):
            cnt += 1
            for p in (p.strip() for p in results.group(1).split()):
                if p == "REQUIRED" or p == "QUIETLY":
                    continue
                yield (root, libName, p.strip('"'))
        if cnt == 0:
            print 'INFO: package "%s" seems to have no dependencies' % libName


def _makePdf(s, filename):
    f = tempfile.NamedTemporaryFile("w", delete=False)
    f.write(s)
    f.close()
    os.system('dot -Tpdf %s -o %s' % (f.name, filename))
    os.remove(f.name)


def createDependencyGraph(filename, rootDir, depGenerator):
    s = "digraph G{\n"
    s += "    rankdir=TL;\n"
    for directory, package, dependency in depGenerator(rootDir):
        s += '    "%s" -> "%s";\n' % (package, dependency)
        if "mars" in package:
            s += '    "%s" [group=mars];\n' % package
        elif "common" in directory:
            s += '    "%s" [group=common];\n' % package
    s += "}"
    _makePdf(s, filename)
        



if __name__ == '__main__':
    rootDir = sys.argv[1]
    if not rootDir:
        rootDir = "."
    print "creating dependency graph from CMakeLists"
    createDependencyGraph("depGraph_CMakeLists.pdf", rootDir, depsFromCMakeLists)
    print
    print "creating dependency graph from pkg_config"
    createDependencyGraph("depGraph_pkgconfig.pdf", rootDir, depsFromPkgConfig)
    print
    print "creating dependency graph from manifest"
    createDependencyGraph("depGraph_manifest.pdf", rootDir, depsFromManifest)
    print
