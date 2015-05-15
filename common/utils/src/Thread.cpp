/**
 *  Copyright 2011, DFKI GmbH Robotics Innovation Center
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

#include "Thread.h"

#include "misc.h"

#include <pthread.h>

#ifdef WIN32

  #ifdef pthread_cleanup_push
    #undef pthread_cleanup_push
  #endif

  #define pthread_cleanup_push(F, A)\
  {\
      const _pthread_cleanup _pthread_cup = {(F), (A), *pthread_getclean()};\
      __sync_synchronize();\
      *pthread_getclean() = (_pthread_cleanup *) &_pthread_cup;\
      __sync_synchronize()

  #ifdef pthread_cleanup_pop
    #undef pthread_cleanup_pop
  #endif
  
  /* Note that if async cancelling is used, then there is a race here */
  #define pthread_cleanup_pop(E)\
      (*pthread_getclean() = _pthread_cup.next, (E?_pthread_cup.func((pthread_once_t *)_pthread_cup.arg):void()));}

#endif // WIN32
    
namespace mars {
  namespace utils {

    struct PthreadThreadWrapper {
      pthread_t t;
    };

    Mutex Thread::threadListMutex;
    std::list<Thread*> Thread::threads;
  
    Thread::Thread() 
      : myThread(new PthreadThreadWrapper) {

      running = false;
      finished = false;
      // set default stackSize
      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_getstacksize(&attr, &myStackSize);
      pthread_attr_destroy(&attr);
      threadListMutex.lock();
      threads.push_back(this);
      threadListMutex.unlock();
    }

    Thread::~Thread() {
      std::list<Thread*>::iterator it;
      threadListMutex.lock();
      for(it = threads.begin(); it != threads.end(); ++it) {
        if(*it == this) {
          threads.erase(it);
          break;
        }
      }
      threadListMutex.unlock();
      delete myThread;
    }

    void Thread::setStackSize(std::size_t stackSize) {
      // we can only modify the stackSize as long as we're not running yet
      if (!running) {
        if (stackSize) {
          myStackSize = stackSize;
        } else {
          // reset default stackSize
          pthread_attr_t attr;
          pthread_attr_init(&attr);
          pthread_attr_getstacksize(&attr, &myStackSize);
          pthread_attr_destroy(&attr);        
        }
      }
    }

    void Thread::start() {
      running = true;
      pthread_attr_t threadAttributes;
      pthread_attr_init(&threadAttributes);
      pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_attr_setdetachstate(&threadAttributes, PTHREAD_CREATE_JOINABLE);
      pthread_attr_setstacksize(&threadAttributes, myStackSize);
      int rc = pthread_create(&myThread->t, &threadAttributes, 
                              &Thread::runHelper, static_cast<void*>(this));
      pthread_attr_destroy(&threadAttributes);
      if(rc) {
        // TODO: Handle Error!
      }
    }

    void* Thread::runHelper(void *context) {
      // The context is the current thread.
      // If this wasn't a static function the context would be the "this"-pointer.
      Thread *thread = static_cast<Thread*>(context);
      // Setup a cleanupHandler that will be called whether the thread was 
      // canceled, calls exit or terminates normally.
      pthread_cleanup_push(&Thread::cleanupHandler, context);
      thread->run();
      // call pthread_cleanup_pop with non-zero argument to make sure it runs
      // even when we exit the run method normally.
      pthread_cleanup_pop(1);
      return NULL;
    }
  
    void Thread::cleanupHandler(void *context) {
      Thread *thread = static_cast<Thread*>(context);
      thread->running = false;
      thread->finished = true;
    }
  
    void Thread::cancel(bool block) {
      pthread_cancel(myThread->t);
      if(block) {
        while(this->isRunning()) {
          wait(1);
        } // while
      }
    }

    void Thread::setCancellationPoint() {
      pthread_testcancel();
    }

    bool Thread::wait() {
      void *status;
      int rc = pthread_join(myThread->t, &status);
      if(rc) {
        // TODO: Handle Error!
      }
      return true;
    }
    bool Thread::join() {
      return wait();
    }

    bool Thread::wait(unsigned long timeoutMilliseconds) {
      long timeout = getTime() + timeoutMilliseconds;
      while(running && getTime() < timeout) {
        mars::utils::msleep(1);
      }
      return !running;
    }

    bool Thread::isRunning() const {
      return running;
    }

    bool Thread::isFinished() const {
      return finished;
    }

    std::size_t Thread::getStackSize() const {
      return myStackSize;
    }

    bool Thread::isCurrentThread() const {
      pthread_t currentThread = pthread_self();
      return pthread_equal(currentThread, myThread->t);
    }

    Thread* Thread::getCurrentThread() {
      std::list<Thread*>::iterator it;
      pthread_t thisThreadID = pthread_self();

      for(it = threads.begin(); it != threads.end(); ++it) {
        if(pthread_equal(thisThreadID, (*it)->myThread->t)) {
          return *it;
        }
      }
      return NULL;
    }

    void Thread::msleep(unsigned long msec) {
      mars::utils::msleep(msec);
    }

    void Thread::cancelAll(bool block) {
      threadListMutex.lock();
      std::list<Thread*>::iterator threadIt;
      for(threadIt = threads.begin(); threadIt != threads.end(); ++threadIt) {
        (*threadIt)->cancel(block);
      }
      threadListMutex.unlock();
    }

  } // end of namespace utils
} // end of namespace mars
