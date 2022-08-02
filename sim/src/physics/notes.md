**TODOS:**
  * make NodePhysics => ODEObject class interface
  * create child classes for other node types similar to ODEBox
  * check if destructor if ODEObject is called correctly
	  => move around object in scene. This might call setPosition, which destroys and createsObject

  * at the moment, we would still have the switch case in the PhysicsMapper newODEObject method.
    * This should later on be replaced by a call to some Registry to request if a requested type is available and get it
    * Malte has an example for this in some Sensor Class somewhere

  * check if all original NodePhysics functions are integrated into the ODEObject/ODE\* classes. Especially the heightmap callback
  * think about seperating nBody (dynamic ODE) and nGeometry (collision) at some point

**Style:**
  * use smart pointers instead of free etc..
  * check constant correctness
  * sort functions in header files and add doxygen documentation
  * introduce LOG_DEBUG instead of printfs

**QUESTIONS:**
  * Talk about coding style and naming
  * why is SimNode not inheriting from NodeInterface or another similar base class that contains all the get/set Force/Torque/Velocity stuff?

**NOTES:**
  d => ODEMethods
