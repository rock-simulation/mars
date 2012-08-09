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

#ifndef MARS_UTILS_WAIT_CONDITION_H
#define MARS_UTILS_WAIT_CONDITION_H


namespace mars {
  namespace utils {

    struct PthreadConditionWrapper;
    class Mutex;

    enum WaitConditionError {
      WAITCOND_NO_ERROR=0,
      WAITCOND_TIMEOUT,
      WAITCOND_UNSPECIFIED,
      WAITCOND_INTERNAL_ERROR,
      WAITCOND_UNKNOWN,
    };
  
    class WaitCondition {
    public:
      explicit WaitCondition();
      ~WaitCondition();

      /**
       */
      WaitConditionError wait(Mutex *mutex);
      WaitConditionError wait(Mutex *mutex, unsigned long timeoutMilliseconds);

      /**
       */
      WaitConditionError wakeOne();

      WaitConditionError wakeAll();

    private:
      // disallow copying
      WaitCondition(const WaitCondition &);
      WaitCondition &operator=(const WaitCondition &);

      PthreadConditionWrapper *myWaitCondition;

    }; // end of class WaitCondition
  
  } // end of namespace utils
} // end of namespace mars


#endif /* MARS_UTILS_WAIT_CONDITION_H */

