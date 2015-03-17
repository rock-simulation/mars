cfg_manager {#cfg_manager}
===========

The `cfg_manager` provides a module to load, store, access, and save
configuration properties.


## Files

The configuration file format is [yaml](http://www.yaml.org).


## Using the `cfg_manager`

The `cfg_manager` is generally loaded via the [`lib_manager`](@ref lib_manager) by:

    CFGManagerInterface *cfg;
    cfg = libManager->getLibraryAs<CFGManagerInterface>("cfg_manager");
    if(!cfg) { /* error handling */ }

