/*
 *  Copyright 2012, DFKI GmbH Robotics Innovation Center
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

#include "WaitCondition.h"
#include "Mutex.h"

#include <pthread.h>
#include <errno.h>

namespace mars {
  namespace utils {

    struct PthreadConditionWrapper {
      pthread_cond_t c;
    };

    WaitCondition::WaitCondition() 
      : myWaitCondition(new PthreadConditionWrapper) {
      pthread_cond_init(&myWaitCondition->c, NULL);
    }

    WaitCondition::~WaitCondition() {
      pthread_cond_destroy(&myWaitCondition->c);
      delete myWaitCondition;
    }
  
    WaitConditionError WaitCondition::wait(Mutex *mutex) {
      pthread_mutex_t *m = static_cast<pthread_mutex_t*>(mutex->getHandle());
      int rc = pthread_cond_wait(&myWaitCondition->c, m);
      switch(rc) {
      case 0:
	return WAITCOND_NO_ERROR;
      case EINVAL:
	return WAITCOND_UNSPECIFIED;
      default:
        return WAITCOND_UNKNOWN;
      }
    }

    WaitConditionError WaitCondition::wait(Mutex *mutex,
					   unsigned long timeoutMilliseconds) {
      struct timespec t;
      t.tv_sec = timeoutMilliseconds / 1000;
      t.tv_nsec = (timeoutMilliseconds % 1000) * 1000000;
      pthread_mutex_t *m = static_cast<pthread_mutex_t*>(mutex->getHandle());
      int rc = pthread_cond_timedwait(&myWaitCondition->c, m, &t);
      switch(rc) {
      case 0:
        return WAITCOND_NO_ERROR;
      case ETIMEDOUT:
        return WAITCOND_TIMEOUT;
      case EINVAL:
	return WAITCOND_UNSPECIFIED;
      default:
        return WAITCOND_UNKNOWN;
      }
    }

    WaitConditionError WaitCondition::wakeOne() {
      int rc = pthread_cond_signal(&myWaitCondition->c);
      switch(rc) {
      case 0:
        return WAITCOND_NO_ERROR;
      case EINVAL:
        return WAITCOND_INTERNAL_ERROR;
      default:
        return WAITCOND_UNKNOWN;
      }
    }

    WaitConditionError WaitCondition::wakeAll() {
      int rc = pthread_cond_broadcast(&myWaitCondition->c);
      switch(rc) {
      case 0:
        return WAITCOND_NO_ERROR;
      case EINVAL:
        return WAITCOND_INTERNAL_ERROR;
      default:
        return WAITCOND_UNKNOWN;
      }
    }

  } // end of namespace utils
} // end of namespace mars
