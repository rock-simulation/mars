MARS Utils {#mars_utils}
===========

## Overview

The mars\_utils library provides some helpful general c++ functionality. It contains a system independet thread and mutex implementation with a similar api like the Qt4 libraries. Some static math and system functions are defined in the misc.h and mathUtils.h. The ConfigData.h includes the ConfigMap wich is a nested std::map/std::vector implementation to store configuration information in a similar way like a python dictionary. 
 

## Threads and Mutex functionality

To use the thread implementation (a wrapping of pthread) just inherit from mars::utils::Thread and implement the protected run() method. To start the thread just call the start() methon on your instance (see [Thread](@ref mars::utils::Thread) for more details). 
Thread example:

    #include <mars/utils/Thread.h>
    #include <iostream>
    #include <string>
    #include <sstream>

    class MyThread : public mars::utils::Thread {

    public:
      MyThread() : done(false) {}

      void stop() {
        done=true;
      }

    protected:
      void run() {
        while(!done) {
          fprintf(stderr, ".");
          msleep(10);
        }
      }

    private:
      bool done;
    };

    int main(int argc, char *argv[]) {
      std::string input;

      MyThread *myThread = new MyThread();
      myThread->start();

      while(input.size() < 1 || input[0] != 'q') {
        getline(std::cin, input);
      }      
      myThread->stop();
      myThread->wait();
      printf("\n");

      return 0;
    }

To use a mutex simply create an instance of mars::utils::Mutex and use the lock() and unlock() method of the mutex. The mars::mutex::MutexLocker class gets a pointer to a mutex in the contructor and locks it there. In the destructor the mutex is unlocked. In this way the MutexLocker can be created as member variable at the begining of a method to lock the method call. While returning from the method call the MutexLocker is deleted and thus the mutex is unlocked.

### ConfigMap
The ConfigMap is a nested implementation of std::maps and std::vectors to provide a python dictanary like data storage. Datatypes that can be stored are:

 - std::string
 - int
 - unsigned int
 - unsinged long
 - double
 - bool

An important feature is the loading and storing of ConfigMaps in yaml files. The example shows the usage of ConfigMaps for further details check [ConfigMap](@ref mars::utils::ConfigData.h).

Notice: While reading and writing yaml files the order of dictanaries is not necessarily kept.

Yaml example file:

    list: [1, 2, a, b]
    name: foo
    value1: 1.3
    value2: 1
    foo:
        foo2: Hello world!
        foo3: blub

Example program:

    #include <mars/utils/ConfigData.h>

    int main(int argc, char *argv[]) {

      // some ConfigMap test
      mars::utils::ConfigMap map;
      map = mars::utils::ConfigMap::fromYamlFile("test.yml");
      mars::utils::ConfigVector::iterator it;
      for(it=map["list"].begin(); it!=map["list"].end(); ++it) {
        printf("list entry: %s\n", it->getString().c_str());
      }
      double value1 = map["value1"];
      printf("value1: %g\n", value1);
      int value2 =  map["value2"];
      printf("value2: %d\n", value2);
      map["value2"] = 3;
      printf("value2 modified: %d\n", value2);

      std::string foo2 = map["foo"]["foo2"];
      std::string foo3 = map["foo"][0]["foo3"];

      printf("%s\n", foo2.c_str());
      printf("%s\n", foo3.c_str());

      // add some keys:
      map["value3"] = 3.14;
      printf("value3: %g\n", (double)map["value3"]);

      map["blub"]["foo"] = "3.14";
      printf("blub/foo: %g\n", (double)map["blub"]["foo"]);

      map.toYamlFile("result.yml");

      return 0;
    }

result.yml:

    foo:
      foo2: Hello world!
      foo3: blub
    list:
      - 1
      - 2
      - a
      - b
    name: foo
    value1: 1.3
    value2: 3
    value3: 3.14
    blub:
      foo: 3.14


\[26.08.2014\]





