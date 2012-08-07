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

namespace mars {
  namespace utils {

    Mutex::Mutex(const MutexType &mutexType_) 
      : mutexType(mutexType_){
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
      pthread_mutex_init(&myMutex, &mutexAttributes);
      pthread_mutexattr_destroy(&mutexAttributes);
    }

    Mutex::~Mutex() {
      pthread_mutex_destroy(&myMutex);
    }
  
    MutexError Mutex::lock() {
      int rc = pthread_mutex_lock(&myMutex);
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
      int rc = pthread_mutex_unlock(&myMutex);
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
      int rc = pthread_mutex_trylock(&myMutex);
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

  } // end of namespace utils
} // end of namespace mars
