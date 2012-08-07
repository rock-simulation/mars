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

#ifndef MARS_UTILS_MUTEXLOCKER_H
#define MARS_UTILS_MUTEXLOCKER_H


namespace mars {
  namespace utils {

    class Mutex;
  
    class MutexLocker {
    public:
      explicit MutexLocker(Mutex *mutex);
      ~MutexLocker();
    
      void unlock();
      void relock();
    private:
      // disallow copying
      MutexLocker(const MutexLocker &);
      MutexLocker &operator=(const MutexLocker &);

      Mutex *myMutex;
      bool isLocked;
    }; // end of class MutexLocker

  } // end of namespace utils
} // end of namespace mars


#endif /* MARS_UTILS_MUTEXLOCKER_H */

