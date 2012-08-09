/*
 *  Copyright 2011, 2012, DFKI GmbH Robotics Innovation Center
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

#include "Mutex.h"

#include <errno.h>

#include <pthread.h>


namespace mars {
  namespace utils {

    struct PthreadMutexWrapper {
      pthread_mutex_t m;
    };

    Mutex::Mutex(const MutexType &mutexType_) 
      : myMutex(new PthreadMutexWrapper),
        mutexType(mutexType_) {
      pthread_mutexattr_t mutexAttributes;
      pthread_mutexattr_init(&mutexAttributes);
      switch(mutexType) {
      case MUTEX_TYPE_NORMAL:
        pthread_mutexattr_settype(&mutexAttributes, PTHREAD_MUTEX_NORMAL);
        break;
      case MUTEX_TYPE_RECURSIVE:
        pthread_mutexattr_settype(&mutexAttributes, PTHREAD_MUTEX_RECURSIVE);
        break;
      case MUTEX_TYPE_ERRORCHECKED:
      default:
        pthread_mutexattr_settype(&mutexAttributes, PTHREAD_MUTEX_ERRORCHECK);
        break;
      }
      pthread_mutex_init(&myMutex->m, &mutexAttributes);
      pthread_mutexattr_destroy(&mutexAttributes);
    }

    Mutex::~Mutex() {
      pthread_mutex_destroy(&myMutex->m);
      delete myMutex;
    }
  
    MutexError Mutex::lock() {
      int rc = pthread_mutex_lock(&myMutex->m);
      switch(rc) {
      case 0:
        return MUTEX_ERROR_NO_ERROR;
      case EINVAL:
        return MUTEX_ERROR_NO_VALID_MUTEX;
      case EAGAIN:
        return MUTEX_ERROR_RECURSIVE_LIMIT;
      case EDEADLK:
        return MUTEX_ERROR_DEADLOCK;
      default:
        return MUTEX_ERROR_UNKNOWN;
      }
    }

    MutexError Mutex::unlock() {
      int rc = pthread_mutex_unlock(&myMutex->m);
      switch(rc) {
      case 0:
        return MUTEX_ERROR_NO_ERROR;
      case EINVAL:
        return MUTEX_ERROR_NO_VALID_MUTEX;
      case EAGAIN:
        return MUTEX_ERROR_RECURSIVE_LIMIT;
      case EPERM:
        return MUTEX_ERROR_DONT_OWN;
      default:
        return MUTEX_ERROR_UNKNOWN;
      }
    }

    MutexError Mutex::tryLock() {
      int rc = pthread_mutex_trylock(&myMutex->m);
      switch(rc) {
      case 0:
        return MUTEX_ERROR_NO_ERROR;
      case EBUSY:
        return MUTEX_ERROR_BUSY;
      case EINVAL:
        return MUTEX_ERROR_NO_VALID_MUTEX;
      case EAGAIN:
        return MUTEX_ERROR_RECURSIVE_LIMIT;
      default:
        return MUTEX_ERROR_UNKNOWN;
      }
    }

    void* Mutex::getHandle() {
      return &myMutex->m;
    }

  } // end of namespace utils
} // end of namespace mars
