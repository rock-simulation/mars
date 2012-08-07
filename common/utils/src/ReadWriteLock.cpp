/*
 *  Copyright 2011, 2012 DFKI GmbH Robotics Innovation Center
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

#include "ReadWriteLock.h"

namespace mars {
  namespace utils {

    ReadWriteLock::ReadWriteLock() {
      // TODO error checking?
      pthread_rwlock_init(&myReadWriteLock, NULL);
    }

    ReadWriteLock::~ReadWriteLock() {
      // TODO error checking?
      pthread_rwlock_destroy(&myReadWriteLock);
    }

    void ReadWriteLock::lockForRead() {
      // TODO error checking?
      pthread_rwlock_rdlock(&myReadWriteLock);
    }
    void ReadWriteLock::lockForWrite() {
      // TODO error checking?
      pthread_rwlock_wrlock(&myReadWriteLock);
    }
    bool ReadWriteLock::tryLockForRead() {
      return (pthread_rwlock_tryrdlock(&myReadWriteLock) == 0);
    }
    bool ReadWriteLock::tryLockForWrite() {
      return (pthread_rwlock_trywrlock(&myReadWriteLock) == 0);
    }
    void ReadWriteLock::unlock() {
      pthread_rwlock_unlock(&myReadWriteLock);
    }

  } // end of namespace utils
} // end of namespace mars
