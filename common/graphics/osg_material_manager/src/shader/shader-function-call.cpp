/*
 *  Copyright 2016, DFKI GmbH Robotics Innovation Center
 *
 *  This file is part of the MARS simulation framework.
 *
 *  MARS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3
 *  of the License, or (at your option) any later version.
 *
 *  MARS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with MARS.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "shader-function-call.h"

namespace osg_material_manager {

    using namespace std;

    ShaderFunctionCall::ShaderFunctionCall(std::string name, std::vector<std::string> arguments, unsigned int priority):
            name(name),
            priority(priority),
            arguments(arguments)
    {

    }

    int ShaderFunctionCall::getPriority() {
      return this->priority;
    }

    void ShaderFunctionCall::setPriority(unsigned int newPriority) {
      this->priority = newPriority;
    }

    std::string ShaderFunctionCall::getName() {
      return this->name;
    }

    std::vector<std::string> ShaderFunctionCall::getArguments() {
      return this->arguments;
    }

    void ShaderFunctionCall::addArgument(std::string arg) {
      this->arguments.push_back(arg);
    }

    bool ShaderFunctionCall::operator<(const ShaderFunctionCall& other) const
    {
      return this->priority < other.priority;
    }


} // end of namespace osg_material_manager
