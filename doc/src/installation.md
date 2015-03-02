Installation {#installation}
===============

## Installation

This section describes how to setup a development environment for MARS.
You should create a dedicated development folder on your harddrive.
Something like C:/development/mars_git or /home/my_username/development/mars_git:

    export MARS_DEV_ROOT=/home/my_username/development/mars_git

We will refer to this directory as the development directory or $MARS_DEV_ROOT.
You should clone the git sources into that directory using

    mkdir -p $MARS_DEV_ROOT
    cd $MARS_DEV_ROOT
    git clone git://gitorious.org/rock-simulation/mars.git
    
This will create a new subdirectory "mars" containing the working copy -- note that is not possible to give the directory containing the git-checkout a different name. You then probably have to install some system dependencies. Check the os specific sections below for how to install the dependencies. Afterwards you can continue on all systems in the same way to build and install MARS:

    cd $MARS_DEV_ROOT
    cp mars/scripts/bootstrap/packageList.txt.example mars/scripts/bootstrap/packageList.txt
    ./mars/scripts/bootstrap/mars.sh bootstrap
    
When running it for the first time the script will ask you for the root development directory. Please enter the *absolute* path to your development directory. Next you will be asked how many CPU cores the system may use to build MARS.

### GNU/Linux

You can check the "apt_get_dep.sh" to get a list of the needed dependencies. If you have no conflict with manually installed libraries you can also use the script to install the dependecies:

    cd $MARS_DEV_ROOT
    ./mars/scripts/bootstrap/apt_get_dep.sh*

### OS X:

On "OS X" you have to install QT and OpenSceneGraph manually. Check the project pages for installation help (http://qt-project.org and http://openscenegraph.org).
Other dependencies can be installed by the script "port_get_deb.sh" via macports:

    cd $MARS_DEV_ROOT
    ./mars/scripts/bootstrap/port_get_dep.sh*
    
    
### Windows

#### Prerequisites

* OSG: installed and bin dir in PATH
* SSH: [PuTTY](http://www.chiark.greenend.org.uk/~sgtatham/putty/)
  (pick the "Windows installer for everything except PuTTYtel")
* MinGW / MSYS: [Install MinGW](http://mingw.org/wiki/Getting_Started)
    - download and run the mingw-get-inst graphical user interface installer
    - When asked what components to install make sure the "C Compiler", "C++ Compiler", "MSYS Basic System", and "MinGW Developer ToolKit" options are selected.
    - After installation completed you should install/update some software components. Open a MinGW Shell.
* To get a quick overview over what packages are available type

        mingw-get list | grep Package 
        mingw-get upgrade "gcc=4.6.2-1-mingw32"*
        mingw-get upgrade "g++=4.6.2-1-mingw32"*
        mingw-get upgrade "mingwrt=3.20-mingw32"*
        mingw-get install msys-wget*
        mingw-get install msys-tar*
        mingw-get install msys-zip*
        mingw-get install msys-gzip*
        mingw-get install msys-bzip2*
        mingw-get install msys-sed*
        mingw-get install mingw32-libz*
    
* [Git For Windows](http://msysgit.github.com/)
  (don't take the msysGit version)
* When asked how you would like to adjust your PATH environment select the third option "Run Git and include Unix tools from the Windows Command Promt"

![Run Git and include Unix tools from the Windows Command Prompt](install/git_for_windows_adjust_PATH.PNG)

* When asked how to handle line endings make sure to select the option to "keep the line endings on checkout" and commit as UNIX.
  
### Troubleshooting:

Q: When compiling you get errors like: 

    unknown type uintptr_t*

A: You are probably on windows and git your line endings got messed up. Type

    git config --global core.autocrlf input*
    
in a shell, remove your working directory and checkout mars anew.
