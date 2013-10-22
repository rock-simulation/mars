Mars environment and building {#tutorial_build}
=============================

##Setting up the MARS environment

When working with MARS, you usually make use of its custom environment configuration. Thus when you open a terminal and access the mars development folder (i.e. the folder you installed MARS to, here referenced as $MARS_DEV_ROOT), you want to run the shell script "env.sh", which provides path variables for the binaries and libraries of MARS. Thus you execute

    . env.sh

so your shell has the information needed to work with MARS. You also need to set this environment before you build your own code in the MARS environment.

##Building in MARS

The following part is described for use under Linux only, but will work similar in a Windows or OSX environment.

MARS uses a customly configured build environment, which you'll have to utilize to make your own code work properly with MARS. As decribed in the [Basic plugin tutorial](tutorials/basic/plugin_tutorial.html), you can, after creating your plugin (or whatever other part of MARS you want to build), run the shell script "build.sh":

    ./build.sh
    
to create a "build"-subfolder and compile your code. However, while this is what you want to do most of the time, it is still worthwhile to take a closer look and understand what's actually happening there.

Apart from the output of the build process, the build shell script contains the following lines:

    rm -rf build
    mkdir build
    cd build
    cmake_debug
    make -j4
    cd ..
    
which, in succession, removes any previously existing build folder, (re)creates a new build folder, and compiles the code. "cmake_debug" is part of the MARS environment and refers to the "cmake_debug.sh" shell script in the install/bin/ environment, which in turn executes the command:

    cmake .. -DCMAKE_INSTALL_PREFIX=/$MARS_DEV_ROOT/install -DCMAKE_BUILD_TYPE=DEBUG $@
    
thus running *cmake* with a specific install directory in debug-mode. "mMake -j4"" then allows *make* to compile on up to 4 separate processes, speeding up the compiling process. You can change 4 to any number of CPU cores you would like your system to use for compilation.

So this is what's happening behind the curtains when you run the build.sh script. You can of course always manually execute the same commands.
