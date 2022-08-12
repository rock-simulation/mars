#pragma once

#include <ObjectFactoryInterface.h>

namespace mars{
namespace sim{

using namespace ::mars::interfaces;

class ODEObjectFactory : public ObjectFactoryInterface{
public:
  static ODEObjectFactory* getInstance();

protected:
  ODEObjectFactory();
  virtual ~ODEObjectFactory();    
  virtual std::shared_ptr<NodeInterface> createObject(std::shared_ptr<PhysicsInterface> worldPhysics, NodeData * nodeData) override;

private:
  static ODEObjectFactory* instance;  
};

}
}
