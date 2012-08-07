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

#include "ReadWriteLocker.h"
#include "ReadWriteLock.h"


namespace mars {
  namespace utils {

    ReadWriteLocker::ReadWriteLocker(ReadWriteLock *readWriteLock, 
                                     ReadWriteLockMode mode)
      : myReadWriteLock(readWriteLock), myMode(mode) {
      isLocked = false;
      relock();
    }
  
    ReadWriteLocker::~ReadWriteLocker() {
      if (isLocked)
        myReadWriteLock->unlock();
    }

    void ReadWriteLocker::unlock() {
      if (isLocked) {
        myReadWriteLock->unlock();
        isLocked = false;
      }
    }
   
    void ReadWriteLocker::relock() {
      if (!isLocked) {
        if (myMode == READWRITELOCK_MODE_READ) {
          myReadWriteLock->lockForRead();
        } else {
          myReadWriteLock->lockForWrite();
        }
        isLocked = true;
      }
    }

  } // end of namepace utils
} // end of namespace mars
