#pragma once

#include <mars/interfaces/sim/NodeInterface.h>

namespace mars{
namespace sim{

using namespace ::mars::interfaces;

class ObjectFactoryInterface{
public:
  virtual std::shared_ptr<NodeInterface> createObject(std::shared_ptr<PhysicsInterface> worldPhysics, NodeData * nodeData) = 0;
};
}
}