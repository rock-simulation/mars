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
 
#ifndef ZIPIT_H_INCLUDED
#define ZIPIT_H_INCLUDED

 
#include <string>
#include <fstream>
#include <vector>

#include <minizip/unzip.h>
#include <minizip/zip.h>


/**
 * up to now, this class just adds and reads files form a zip achive
 * there is no ability for overwriting a Zip or a File in a Zip or 
 * things like that.
 * it just adds the files at the end of the zipfile
 * 
 * things to do:
 * - many
 * - 
 */

namespace mars {
  namespace scene_loader {

    enum ZipitError {
      ZIPIT_SUCCESS = 0,
      ZIPIT_NO_OPEN_HANDLE,
      ZIPIT_NO_CLOSE_HANDLE,
      ZIPIT_NO_BUFFER,
      ZIPIT_FILE_NOT_FOUND,
      ZIPIT_FILE_IN_ZIP_NOT_FOUND,
      ZIPIT_FILE_IN_ZIP_CREATION_ERR,
      ZIPIT_FILE_NOT_WROTE_IN_ZIP,
      ZIPIT_UNABLE_TO_OPEN_FILE_IN_ZIP,
      ZIPIT_UNABLE_TO_READ_FILE_IN_ZIP,
      ZIPIT_UNABLE_TO_WRITE_FILE_IN_ZIP,
      ZIPIT_UNABLE_TO_CLOSE_FILE_IN_ZIP,
      ZIPIT_NO_HANDLE_FOR_FILE,
      ZIPIT_UNABLE_TO_CLOSE_ZIPFILE,
      ZIPIT_UNABLE_TO_WRITE_IN_FILE,
      ZIPIT_UNKNOWN_ERROR,
    };

    class Zipit
    {
    public:
      /**
       * int zipit(string zipFileName);
       * the constructor just initialises some internal values, i.e.
       * the name of the zipfile.
       * it does not open an handle to the zipfile. there are spezial methods
       * doing this.
       */
      Zipit(const std::string &zipFileName);
      ~Zipit();
    

      ZipitError unzipAll(const std::string &zipFilename,
                          const std::string &directory);
      ZipitError zipDirectory(const std::string &directory,
                              const std::string &zipFile,
                              bool overwrite);
      /**
       * int addToZip(string fileNameToZip, bool zipWithFolders); and
       * int addToZip(string fileNameToZip, bool zipWithFolders, vector<string> listOfFiles);
       * both adding file(s) to a zipfile.
       */
    
      int addToZip(const std::string &fileNameToZip ,
                   const std::string &sourceFileNameToZip);
      int addToZip(const std::vector<std::string> &listOfFiles,
                   const std::vector<std::string> &sourceListOfFiles);
      /**
       * 
       * 
       * an important fearture to add:
       * an ability to spezify an folder, where the files should be stored
       */
      int getFromZip(const std::string &inZipFileName,
                     const std::string &whereToStore);
      int getFromZip(const std::vector<std::string> &allFileNamesToRead,
                     const std::vector<std::string> &v_whereToStore);
    
      int unpackWholeZipTo(const std::string &whereToStore); 
      int closeZipHandle();
      int closeUnZipHandle();

    private:
   
      /**
       * int openZipHandle();
       * int closeZipHandle();
       * int openUnZipHandle();
       * int closeUnZipHandle();
       * 
       * they just open or close an handle to zipfile spezified above
       */
      int openZipHandle();
   
      int openUnZipHandle();
    
    
    
      /**
       * until now, this function does nothing, but in future it should be just to generate
       * uniformed errorMessages
       */
      void zipError(ZipitError errorNumber);
    
      /**
       * just a few internal variables.
       */
      zipFile zipHandle;
      unzFile unZipHandle;
      std::string zipFileName;
    };

  } // end of namespace scene_loader
} // end of namespace mars

#endif
