#pragma once

#include <ODEObjectFactory.h>

namespace mars{
namespace sim{

using namespace ::mars::interfaces;

class ODEObjectCreator : public ODEObjectFactory{
public:
  static ODEObjectCreator* getInstance();

protected:
  ODEObjectCreator();
  virtual ~ODEObjectCreator();    
  virtual std::shared_ptr<NodeInterface> createObject(std::shared_ptr<PhysicsInterface> worldPhysics, NodeData * nodeData) override;

private:
  static ODEObjectCreator* instance;  
};

}
}
