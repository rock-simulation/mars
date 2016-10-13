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

#pragma once
#include <string>
#include <vector>
#include <configmaps/ConfigData.h>

#ifdef USE_TR1
    #include <tr1/memory>
    using std::tr1::shared_ptr;
#else
    #include <memory>
    using std::shared_ptr;
#endif

// TODO to allow calling functions/methods with arbitrary arguments we must
// create a callable object and pass a Python argument list


namespace mars {

struct ObjectState;
struct FunctionState;
struct MethodState;
struct ModuleState;
struct ListBuilderState;
class Object;
class Function;
class Method;
class Module;
class ListBuilder;

class PythonInterpreter
{
    shared_ptr<Module> currentModule;

    PythonInterpreter();
    PythonInterpreter(const PythonInterpreter&) {}
public:
    ~PythonInterpreter();
    static const PythonInterpreter& instance();

    void addToPythonpath(const std::string& path) const;
    shared_ptr<Module> import(const std::string& name) const;
    shared_ptr<Module> reload(const std::string& name) const;
    shared_ptr<ListBuilder> listBuilder() const;
};

  void toConfigMap(shared_ptr<Object> obj, configmaps::ConfigItem &item);
  
enum CppType
{
  INT, DOUBLE, BOOL, STRING, ONEDARRAY, ONEDCARRAY, OBJECT, MAP
};

class Object
{
public:
    shared_ptr<ObjectState> state;

    Object(shared_ptr<ObjectState> state);
    Method& method(const std::string& name);
    Object& variable(const std::string& name);
    double asDouble();
    int asInt();
    bool asBool();
    std::string asString();
};

class Function
{
public:
    shared_ptr<FunctionState> state;

    Function(ModuleState& module, const std::string& name);
    Function& pass(CppType type);
    Function& call(...);
    shared_ptr<Object> returnObject();
};

class Method
{
public:
    shared_ptr<MethodState> state;

    Method(ObjectState& object, const std::string& name);
    Method& pass(CppType type);
    Method& call(...);
    shared_ptr<Object> returnObject();
};

class Module
{
public:
    shared_ptr<ModuleState> state;

  Module(const std::string& name);
    Function& function(const std::string& name);
    Object& variable(const std::string& name);
  void reload();
};

class ListBuilder
{
    friend class PythonInterpreter;
    ListBuilder();
public:
    shared_ptr<ListBuilderState> state;

    ListBuilder& pass(CppType type);
    shared_ptr<Object> build(...);
};

} // End of namespace mars
