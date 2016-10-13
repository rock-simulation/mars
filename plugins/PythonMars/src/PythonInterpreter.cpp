// Since Python may define some pre-processor definitions which affect the
// standard headers on some systems, you must include Python.h before any
// standard headers are included.
// (See https://docs.python.org/2.7/c-api/intro.html#include-files)

#include "PythonInterpreter.hpp"
#include <Python.h>
#include <numpy/arrayobject.h>

#include <stdexcept>
#include <list>
#include <cstdarg>
#include <sstream>

using namespace configmaps;

namespace mars {

////////////////////////////////////////////////////////////////////////////////
//////////////////////// Memory management /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * A managed pointer to PyObject which takes care about
 * memory management and reference counting.
 *
 * \note Reference counting only works if makePyObjectPtr() is used to create
 *       the pointer. Therefore you should always use makePyObjectPtr() to
 *       create new PyObjectPtrs.
 *
 * \note This type should only be used to encapsulate PyObjects that are
 *       'new references'. Wrapping a 'borrowed reference' will break Python.
 */
typedef shared_ptr<PyObject> PyObjectPtr;

/**
 * This deleter should be used when managing PyObject with shared_ptr
 */
struct PyObjectDeleter
{
    void operator()(PyObject* p) const
    {
        Py_XDECREF(p);
    }
};

/**
 * Make a managed PyObject that will be automatically deleted with the last
 * reference.
 */
PyObjectPtr makePyObjectPtr(PyObject* p)
{
    return PyObjectPtr(p, PyObjectDeleter());
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////// Helper functions //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void throwPythonException()
{
  PyObject* error = PyErr_Occurred(); // Borrowed reference
  if(error != NULL)
  {
    PyObject* ptype, * pvalue, * ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);

    // Exception type
    PyObjectPtr pyexception = makePyObjectPtr(PyObject_GetAttrString(
        ptype, (char*)"__name__"));
    std::string type = PyString_AsString(pyexception.get());

    // Message
    PyObjectPtr pymessage = makePyObjectPtr(PyObject_Str(pvalue));
    std::string message = PyString_AsString(pymessage.get());

    // Traceback
    PyObjectPtr tracebackModule = makePyObjectPtr(PyImport_ImportModule("traceback"));
    std::string traceback;
    if (tracebackModule) {
        PyObjectPtr tbList = makePyObjectPtr(
            PyObject_CallMethod(
                tracebackModule.get(), (char*)"format_exception",
                (char*)"OOO", ptype, pvalue == NULL ? Py_None : pvalue,
                ptraceback == NULL ? Py_None : ptraceback));

        PyObjectPtr emptyString = makePyObjectPtr(PyString_FromString(""));
        PyObjectPtr strRetval = makePyObjectPtr(
            PyObject_CallMethod(emptyString.get(), (char*)"join", (char*)"O",
                                tbList.get()));

        traceback = PyString_AsString(strRetval.get());
    }
    else
    {
        traceback = "Empty traceback";
    }

    Py_XDECREF(ptype);
    Py_XDECREF(pvalue);
    Py_XDECREF(ptraceback);

    std::string error_message = "Python exception (" + type + "): " + message;
    if(traceback != (type + ": " + message + "\n"))
        error_message += "\n" + traceback;
    throw std::runtime_error(error_message);
  }
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////// Type conversions //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct List
{
    PyObjectPtr obj;
    static List make()
    {
        List result = {makePyObjectPtr(PyList_New(0))};
        return result;
    }
    static const bool check(PyObjectPtr obj) { return PyList_Check(obj.get()); }
    const unsigned size() { return PyList_Size(obj.get()); }
    const bool isDouble(unsigned i) { return PyFloat_Check(PyList_GetItem(obj.get(), i)); }
    const double get(unsigned i) { return PyFloat_AsDouble(PyList_GetItem(obj.get(), (Py_ssize_t)i)); }

    bool append(PyObjectPtr item) { return PyList_Append(obj.get(), item.get()) == 0; }
};

struct Tuple
{
    PyObjectPtr obj;
    static const bool check(PyObjectPtr obj) { return PyTuple_Check(obj.get()); }
    const unsigned size() { return PyTuple_Size(obj.get()); }
    const bool isDouble(unsigned i) { return PyFloat_Check(PyTuple_GetItem(obj.get(), i)); }
    const double get(unsigned i) { return PyFloat_AsDouble(PyTuple_GetItem(obj.get(), (Py_ssize_t)i)); }
};

struct Int
{
    PyObjectPtr obj;
    static Int make(int i)
    {
        Int result = {makePyObjectPtr(PyInt_FromLong((long) i))};
        return result;
    }
    const double get() { return (int)PyInt_AsLong(obj.get()); }
};

struct Double
{
    PyObjectPtr obj;
    static Double make(double d)
    {
        Double result = {makePyObjectPtr(PyFloat_FromDouble(d))};
        return result;
    }
    const double get() { return PyFloat_AsDouble(obj.get()); }
};

struct Bool
{
    PyObjectPtr obj;
    static Bool make(bool b)
    {
        Bool result = {makePyObjectPtr(PyBool_FromLong((long)b))};
        return result;
    }
    const bool get() { return (bool)PyObject_IsTrue(obj.get()); }
};

struct String
{
    PyObjectPtr obj;
    static String make(const std::string& s)
    {
        String result = {makePyObjectPtr(PyString_FromString(s.c_str()))};
        return result;
    }
    const std::string get() { return PyString_AsString(obj.get()); }
};


////////////////////////////////////////////////////////////////////////////////
//////////////////////// Helper functions //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

PyObjectPtr importModule(const std::string& module)
{
  PyObjectPtr pyModuleName = String::make(module).obj;
  throwPythonException();
  PyObjectPtr pyModule = makePyObjectPtr(PyImport_Import(pyModuleName.get()));
  throwPythonException();
  return pyModule;
}

PyObjectPtr getAttribute(PyObjectPtr obj, const std::string& attribute)
{
    PyObjectPtr pyAttr = makePyObjectPtr(PyObject_GetAttrString(
        obj.get(), attribute.c_str()));
    throwPythonException();
    return pyAttr;
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////// Implementation details ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct ObjectState
{
    PyObjectPtr objectPtr;
    shared_ptr<Method> currentMethod;
    shared_ptr<Object> currentVariable;
};

struct FunctionState
{
    std::string name;
    PyObjectPtr functionPtr;
    std::list<CppType> args;
    PyObjectPtr result;
};

struct MethodState
{
    PyObjectPtr objectPtr;
    std::string name;
    std::list<CppType> args;
    PyObjectPtr result;
};

struct ModuleState
{
    PyObjectPtr modulePtr;
    shared_ptr<Function> currentFunction;
    shared_ptr<Object> currentVariable;
};

struct ListBuilderState
{
    std::list<CppType> types;
    List list;
};

  void toConfigMap(PyObject *obj, ConfigItem &item) {
    bool knownType = true;

    if(PyBool_Check(obj)) {
      item = (bool)PyObject_IsTrue(obj);      
    }
    else if(PyInt_Check(obj)) {
      item = (int)PyInt_AsLong(obj);
    }
    else if(PyFloat_Check(obj)) {
      item = PyFloat_AsDouble(obj);
    }
    else if(PyString_Check(obj)) {
      item = PyString_AsString(obj);      
    }
    else if(PyList_Check(obj)) {
      const unsigned size = PyList_Size(obj);      
      for(unsigned i = 0; i < size; i++) {
        PyObject *elem = PyList_GetItem(obj, i);
        item[(int)i] = ConfigItem();
        toConfigMap(elem, item[(int)i]);
      }
    }
    else if(PyDict_Check(obj)) {
      PyObject *keyList = PyDict_Keys(obj);
      const unsigned size = PyList_Size(keyList);      
      for(unsigned i = 0; i < size; i++) {
        PyObject *key = PyList_GetItem(keyList, i);
        item[std::string(PyString_AsString(key))];
        item[std::string(PyString_AsString(key))] = ConfigItem();
        PyObject *value = PyDict_GetItem(obj, key);
        toConfigMap(value, item[std::string(PyString_AsString(key))]);
      }
      Py_XDECREF(keyList);
    }
  }

  void toConfigMap(shared_ptr<Object> obj, ConfigItem &item) {
    toConfigMap(obj->state->objectPtr.get(), item);
  }

  PyObject* mapToPyObjectPtr_(ConfigItem *map) {
    if(map->isMap()) {
      PyObject *dict = PyDict_New();
      ConfigMap::iterator it = map->beginMap();
      for(; it!=map->endMap(); ++it) {
        PyObject* obj = mapToPyObjectPtr_(it->second);
        if(obj) {
          PyDict_SetItemString(dict, it->first.c_str(), obj);
          Py_XDECREF(obj);        
        }
      }
      return dict;
    }
    else if(map->isVector()) {
      PyObject *list = PyList_New(0);
      ConfigVector::iterator it = map->begin();
      for(; it!=map->end(); ++it) {
        PyObject* obj = mapToPyObjectPtr_(*it);
        if(obj) {        
          PyList_Append(list, obj);
          Py_XDECREF(obj);          
        }
      }
      return list;
    }
    else if(map->isAtom()) {
      ConfigAtom &atom = *map;
      switch(atom.getType()) {
      case ConfigAtom::DOUBLE_TYPE:
        return PyFloat_FromDouble((double)atom);
        break;
      case ConfigAtom::INT_TYPE:
        return PyFloat_FromDouble((int)atom);
        break;
      case ConfigAtom::BOOL_TYPE:
        return PyFloat_FromDouble((bool)atom);
        break;
      default:
        return PyString_FromString(atom.toString().c_str());
        break;
      }
    }
    return NULL;
  }
  
  PyObjectPtr mapToPyObjectPtr(ConfigMap *map) {
    ConfigItem item(*map);
    return makePyObjectPtr(mapToPyObjectPtr_(&item));
  }

  void toPyObjects(std::va_list& cppArgs, const std::list<CppType>& types, std::vector<PyObjectPtr>& args)
{
    for(std::list<CppType>::const_iterator t = types.begin(); t != types.end();
        t++)
    {
        switch(*t)
        {
        case INT:
        {
            const int i = va_arg(cppArgs, int);
            args.push_back(Int::make(i).obj);
            break;
        }
        case DOUBLE:
        {
            const double d = va_arg(cppArgs, double);
            args.push_back(Double::make(d).obj);
            break;
        }
        case BOOL:
        {
            // bool is promoted to int when passed through "..."
            const int b = va_arg(cppArgs, int);
            args.push_back(Bool::make((bool)b).obj);
            break;
        }
        case STRING:
        {
            std::string* str = va_arg(cppArgs, std::string*);
            args.push_back(String::make(*str).obj);
            break;
        }
        case OBJECT:
        {
            Object* object = va_arg(cppArgs, Object*);
            args.push_back(object->state->objectPtr);
            break;
        }
        case MAP:
        {
            ConfigMap* map = va_arg(cppArgs, ConfigMap*);
            args.push_back(mapToPyObjectPtr(map));
            break;
        }
        case ONEDCARRAY:
        {
            double* array = va_arg(cppArgs, double*);
            int size = va_arg(cppArgs, int);
            npy_intp dims[1] = {(npy_intp) size};
            PyObject *obj = PyArray_SimpleNewFromData(1, dims, NPY_DOUBLE,
                                                      (void*)(array));
            args.push_back(makePyObjectPtr(obj));
            break;
        }
        default:
            throw std::runtime_error("Unknown function argument type");
        }
        throwPythonException();
    }
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////// Public interface //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

PythonInterpreter::PythonInterpreter()
{
    if(!Py_IsInitialized())
        Py_Initialize();
    import_array();
}

PythonInterpreter::~PythonInterpreter()
{
    if(Py_IsInitialized())
        Py_Finalize();
}

const PythonInterpreter& PythonInterpreter::instance()
{
    static PythonInterpreter pythonInterpreter;
    return pythonInterpreter;
}

void PythonInterpreter::addToPythonpath(const std::string& path) const
{
    PyObjectPtr pythonpath = import("sys")->variable("path").state->objectPtr;
    PyObjectPtr entry = String::make(path).obj;
    int res = PyList_Append(pythonpath.get(), entry.get());
    throwPythonException();
    if(res != 0)
        throw std::runtime_error("Could not append " + path + " to sys.path");
}

Object::Object(shared_ptr<ObjectState> state)
    : state(state)
{
}

Method& Object::method(const std::string& name)
{
    state->currentMethod = shared_ptr<Method>(new Method(*state, name));
    return *state->currentMethod;
}

Object& Object::variable(const std::string& name)
{
    ObjectState* objectStatePtr = new ObjectState;
    objectStatePtr->objectPtr = getAttribute(state->objectPtr, name);
    shared_ptr<ObjectState> objectState = shared_ptr<ObjectState>(
        objectStatePtr);
    state->currentVariable = shared_ptr<Object>(new Object(objectState));
    return *state->currentVariable;
}

double Object::asDouble()
{
    Double d = {state->objectPtr};
    const double result = d.get();
    throwPythonException();
    return result;
}

int Object::asInt()
{
    Int i = {state->objectPtr};
    const int result = i.get();
    throwPythonException();
    return result;
}

bool Object::asBool()
{
    Bool b = {state->objectPtr};
    const bool result = b.get();
    throwPythonException();
    return result;
}

std::string Object::asString()
{
    String s = {state->objectPtr};
    const std::string result = s.get();
    throwPythonException();
    return result;
}

Function::Function(ModuleState& module, const std::string& name)
{
    FunctionState* functionState = new FunctionState;
    functionState->name = name;
    functionState->functionPtr = getAttribute(module.modulePtr, name);
    state = shared_ptr<FunctionState>(functionState);
}

Function& Function::pass(CppType type)
{
    state->args.push_back(type);
    return *this;
}

Function& Function::call(...)
{
    const size_t argc = state->args.size();

    std::vector<PyObjectPtr> args;
    args.reserve(argc);
    std::va_list vaList;
    va_start(vaList, this);
    toPyObjects(vaList, state->args, args);
    va_end(vaList);

    switch(argc)
    {
    case 0:
        state->result = makePyObjectPtr(
            PyObject_CallFunctionObjArgs(state->functionPtr.get(), NULL));
        break;
    case 1:
        state->result = makePyObjectPtr(
            PyObject_CallFunctionObjArgs(state->functionPtr.get(),
                                         args[0].get(), NULL));
        break;
    case 2:
        state->result = makePyObjectPtr(
            PyObject_CallFunctionObjArgs(state->functionPtr.get(),
                                         args[0].get(), args[1].get(), NULL));
        break;
    default:
        throw std::runtime_error("Cannot handle more than 2 argument");
    }

    state->args.clear();

    throwPythonException();
    return *this;
}

shared_ptr<Object> Function::returnObject()
{
    ObjectState* objectStatePtr = new ObjectState;
    objectStatePtr->objectPtr = state->result;
    shared_ptr<ObjectState> objectState = shared_ptr<ObjectState>(
        objectStatePtr);
    return shared_ptr<Object>(new Object(objectState));
}

Method::Method(ObjectState& object, const std::string& name)
{
    MethodState* methodState = new MethodState;
    methodState->objectPtr = object.objectPtr;
    methodState->name = name;
    state = shared_ptr<MethodState>(methodState);
}

Method& Method::pass(CppType type)
{
    state->args.push_back(type);
    return *this;
}

Method& Method::call(...)
{
    const size_t argc = state->args.size();

    std::vector<PyObjectPtr> args;
    args.reserve(argc);
    std::va_list vaList;
    va_start(vaList, this);
    toPyObjects(vaList, state->args, args);
    va_end(vaList);

    // For the characters that describe the argument type, see
    // https://docs.python.org/2/c-api/arg.html#c.Py_BuildValue
    // However, we will convert everything to PyObjects before calling the
    // function
    std::string format(argc, 'O');
    char* format_str = const_cast<char*>(format.c_str()); // HACK
    char* method_name_str = const_cast<char*>(state->name.c_str()); // HACK

    switch(argc)
    {
    case 0:
        state->result = makePyObjectPtr(
            PyObject_CallMethod(state->objectPtr.get(), method_name_str,
                                format_str));
        break;
    case 1:
        state->result = makePyObjectPtr(
            PyObject_CallMethod(state->objectPtr.get(), method_name_str,
                                format_str, args[0].get()));
        break;
    case 2:
        state->result = makePyObjectPtr(
            PyObject_CallMethod(state->objectPtr.get(), method_name_str,
                                format_str, args[0].get(), args[1].get()));
        break;
    default:
        throw std::runtime_error("Cannot handle more than 2 argument");
    }

    state->args.clear();

    throwPythonException();
    return *this;
}

shared_ptr<Object> Method::returnObject()
{
    ObjectState* objectStatePtr = new ObjectState;
    objectStatePtr->objectPtr = state->result;
    shared_ptr<ObjectState> objectState = shared_ptr<ObjectState>(
        objectStatePtr);
    return shared_ptr<Object>(new Object(objectState));
}

  Module::Module(const std::string& name)
{
    ModuleState* moduleState = new ModuleState;
    moduleState->modulePtr = importModule(name);
    state = shared_ptr<ModuleState>(moduleState);
}

void Module::reload()
{
  state->modulePtr = makePyObjectPtr(PyImport_ReloadModule(state->modulePtr.get()));
  throwPythonException();
}

Function& Module::function(const std::string& name)
{
    state->currentFunction = shared_ptr<Function>(new Function(*state, name));
    return *state->currentFunction;
}

Object& Module::variable(const std::string& name)
{
    ObjectState* objectStatePtr = new ObjectState;
    objectStatePtr->objectPtr = getAttribute(state->modulePtr, name);
    shared_ptr<ObjectState> objectState = shared_ptr<ObjectState>(
        objectStatePtr);
    state->currentVariable = shared_ptr<Object>(new Object(objectState));
    return *state->currentVariable;
}

shared_ptr<Module> PythonInterpreter::import(const std::string& name) const
{
    return shared_ptr<Module>(new Module(name));
}

shared_ptr<ListBuilder> PythonInterpreter::listBuilder() const
{
    return shared_ptr<ListBuilder>(new ListBuilder);
}

ListBuilder::ListBuilder()
    : state(new ListBuilderState)
{
    state->list = List::make();
}

ListBuilder& ListBuilder::pass(CppType type)
{
    state->types.push_back(type);
    return *this;
}

shared_ptr<Object> ListBuilder::build(...)
{
    const size_t argc = state->types.size();

    std::vector<PyObjectPtr> args;
    args.reserve(argc);
    std::va_list vaList;
    va_start(vaList, this);
    toPyObjects(vaList, state->types, args);
    va_end(vaList);

    for(std::vector<PyObjectPtr>::iterator object = args.begin();
        object != args.end(); object++)
    {
        state->list.append(*object);
        throwPythonException();
    }

    state->types.clear();

    throwPythonException();

    ObjectState* objectStatePtr = new ObjectState;
    objectStatePtr->objectPtr = state->list.obj;
    shared_ptr<ObjectState> objectState = shared_ptr<ObjectState>(
        objectStatePtr);
    return shared_ptr<Object>(new Object(objectState));
}

} // End of namespace mars
