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

### misc functions

mars/utils/misc.h

Some functions are implemented to help working with files, etc..

  - long long getTime()
  - long long getTimeDiff(long long)
  - msleep()
  - bool pathExists(string)
  - bool matchPattern(string, string)
    - matchPattern("foo", "foo") -> true
    - matchPattern("foo", "fo") -> false
    - matchPattern("foo*", "foo") -> true
    - matchPattern("foo*", "foobar") -> true
    - matchPattern("foo*", "what is foo") -> false
    - matchPattern("*wh*is*foo*", "what is foo") -> true
  - string trim(string)
  - removeFilenamePrefix(*string)
  - removeFilenameSuffix(*string)
  - string getFilenameSuffix(string)
  - string getCurrentWorkingDir()
  - string getPathOfFile(string)
  - int createDirectory(string, int mode)
  - vector<string> explodeString(char, string)

\[23.07.2015\]





