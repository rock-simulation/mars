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


#ifndef MARS_SIM_DEV_SHADER_FUNCTION_CALL_H
#define MARS_SIM_DEV_SHADER_FUNCTION_CALL_H

#include <vector>
#include <string>

namespace osg_material_manager {
    class ShaderFunctionCall {
    public:
        ShaderFunctionCall(std::string name, std::vector<std::string> arguments, unsigned int priority=0);
        std::vector<std::string> getArguments();
        void addArgument(std::string arg);
        std::string getName();
        int getPriority();
        bool operator<(const ShaderFunctionCall& other) const;
    protected:
        std::string name;
        unsigned int priority;
        std::vector<std::string> arguments;
    }; // end of class ShaderFunctionCall
} // end of namespace osg_material_manager

#endif //MARS_SIM_DEV_SHADER_FUNCTION_CALL_H
