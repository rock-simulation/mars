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

#ifndef MARS_UTILS_MISC_H
#define MARS_UTILS_MISC_H

#ifdef WIN32
  #include <sys/timeb.h>
  #include <windows.h>
  #include <io.h>
#else
  #include <sys/time.h>
  #include <unistd.h>
#endif

#include <string>

namespace mars {
  namespace utils {

    /**
     * converts an angle given in radians to degrees.
     * @param r angle in radians
     * @return angle in degrees
     */
    inline double radToDeg(const double &r)
    { return r * 57.295779513082320876798154814105; }
    inline float radToDeg(const float &r)
    { return r * 57.295779513082320876798154814105f; }

    /**
     * converts an angle given in degrees to radians.
     * @param d angle in degrees
     * @return angle in radians
     */
    inline double degToRad(const double &d)
    { return d * 0.017453292519943295769236907685; }
    inline float degToRad(const float &d)
    { return d * 0.017453292519943295769236907685f; }


    /**
     * @return current time in milliseconds
     */
    inline long getTime() {
#ifdef WIN32
      struct timeb timer;
      ftime(&timer);
      return (long)(timer.time*1000 + timer.millitm);
#else
      struct timeval timer;
      gettimeofday(&timer, NULL);
      return ((long)(timer.tv_sec))*1000 + ((long)(timer.tv_usec))/1000;
#endif
    }

    /**
     * @brief returns the time difference between now and a given reference.
     * @param start reference time
     * @return time difference between now and start in milliseconds
     */
    inline long getTimeDiff(long start) {
      return getTime() - start;
    }

    /**
     * sleeps for at least the specified time.
     * @param milliseconds time to sleep in milliseconds
     */
    inline void msleep(unsigned int milliseconds) {
#ifdef WIN32
      ::Sleep(milliseconds);
#else
      static const unsigned int milliToMicro = 1000;
      ::usleep(milliseconds * milliToMicro);
#endif
    }

    /**
     * check whether a file or directory exists.
     * @param path The filesystem path to check.
     * @returns true if path is a valid file or directory. false otherwise.
     * \note This function does not check for neither read nor write access.
     *       Merely for existance.
     */
    inline bool pathExists(const std::string &path) {
#ifdef WIN32
      return (_access(path.c_str(), 0) == 0);
#else
      return (access(path.c_str(), F_OK) == 0);
#endif
    }

    void handleFilenamePrefix(std::string *file, const std::string &prefix);

    void removeFilenamePrefix(std::string *file);
    void removeFilenameSuffix(std::string *file);
    std::string getFilenameSuffix(const std::string &file);
    std::string getPathOfFile(const std::string &filename);

    std::string getCurrentWorkingDir();
    unsigned int createDirectory(const std::string &dir);

  } // end of namespace utils
} // namespace mars

#endif /* MARS_UTILS_MISC_H */
