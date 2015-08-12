cfg_manager {#cfg_manager}
===========

The `cfg_manager` provides a module to load, store, access, and save
configuration properties.


## Using the `cfg_manager`

The `cfg_manager` is generally loaded via the [`lib_manager`](@ref lib_manager) by:

    mars::cfg_manager::CFGManagerInterface *cfg;
    cfg = libManager->getLibraryAs<mars::cfg_manager::CFGManagerInterface>("cfg_manager");
    if(!cfg) { /* error handling */ }

You can access data via the `mars::cfg_manager::CFGManagerInterface`, e.g. by
using the `getOrCreateProperty` method:

    mars::cfg_manager::cfgPropertyStruct data;
    data = cfg->getOrCreateProperty(__groupName__, __paramName__,
                                    __defaultValue__, NULL);
    /* acess data.iValue if value type is int */

You can override the data by setting it's value in the data struct:

    data.sValue = "foo"; // for setting string value
    cfg->setProperty(data);

Another option to set values is to use:

    cfg->setPropertyValue("group", "name", "propName", value);

If you inherit from `mars::cfg_manager::CFGClient` and implement the
`void cfgUpdateProperty(cfgPropertyStruct _propertyS)` method you can change
the last parameter from `NULL` to `this` and the `cfgUpdateProperty` method
will be called if the property is changed. Each property gets an unique id
which you can use to identify the properties if you register to more than
one property:

    void cfgUpdateProperty(cfgPropertyStruct _propertyS) {
      if(data.paramId == _propertyS.paramId) {
        data = _propertyS;
      }
    }

You can load a configuration file via `cfg->loadConfig(filename)` and save
a configuration file for a parameter group via `cfg->writeConfig(filename,
groupName)`.

## File Format

The configuration files are stored in [yaml](http://www.yaml.org) format with
the following syntax:

    __groupName__:
      - { name: __paramName__, type: __type__, value: __value__  }

or:

    __groupName__:
     - name: __paramName__
       type: __type__
       value: __value__

## Example

c++ access of int "myGroup/myParam":

    cfg->getOrCreateProperty("myGroup", "myParam", (int)10, this);

yaml example:

    myGroup:
      - { name: myParam, type: int, value: 10 }
