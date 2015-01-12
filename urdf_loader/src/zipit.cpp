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

#include "zipit.h"

#include <mars/utils/misc.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/Logging.hpp>

#include <cstdlib>
#include <cassert>
#include <iostream>

namespace mars {
  namespace urdf_loader {

    using std::vector;
    using std::string;

    ZipitError Zipit::zipDirectory(const std::string &directory,
                                   const std::string &zipFile,
                                   bool overwrite) {
      return ZIPIT_UNKNOWN_ERROR;
      this->unZipHandle = NULL;
      this->zipHandle = NULL;
    }


    Zipit::Zipit(const string &zipFileName) {
      this->zipFileName = zipFileName;
      this->unZipHandle = NULL;
      this->zipHandle = NULL;
    }
    Zipit::~Zipit() {
      if(zipHandle) closeZipHandle();
      if(unZipHandle) closeUnZipHandle();
    }

    int Zipit::openZipHandle() {
      zipHandle=zipOpen(zipFileName.c_str(), 0); //handle auf das Zipfile erstellen
      if (zipHandle==NULL) {
        zipError(ZIPIT_NO_OPEN_HANDLE);
        return 1;
      }
      return 0;
    }

    int Zipit::closeZipHandle() {
      if (zipClose(zipHandle, "")==0) {
        zipHandle = NULL;
        return 0;
      }
      else {
        zipError(ZIPIT_NO_CLOSE_HANDLE);
        return 1;
      }
    }

    int Zipit::openUnZipHandle() {
      unZipHandle=unzOpen(zipFileName.c_str());
      if (unZipHandle==NULL) {
        zipError(ZIPIT_NO_OPEN_HANDLE);
        return 1;
      }
      return 0;
    }

    int Zipit::closeUnZipHandle() {
      if (unzClose(unZipHandle)==0) {
        unZipHandle = NULL;
        return 0;
      }
      else {
        zipError(ZIPIT_NO_CLOSE_HANDLE);
        return 1;
      }
    }

    /**
     * addToZip(string fileNameToZip, bool zipWithFolders, vector<string> listOfFiles)
     * adds all the files in vector<string> listOfFiles to the zipfile
     * after this, it closes the zipfilehandle
     */
    int Zipit::addToZip(const vector<string> &listOfFiles,
                        const vector<string> &sourceListOfFiles) {
      int zipitError;

      if (openZipHandle()==1) {
        return 1;
      }

      for (unsigned int i=0; i<listOfFiles.size(); i++) {
        zipitError=zipOpenNewFileInZip(zipHandle, listOfFiles[i].c_str(), NULL,
                                       NULL, 0, NULL, 0, NULL, Z_DEFLATED, 0);//Z_DEFLATED,Z_DEFAULT_COMPRESSION);
        if (zipitError!=ZIP_OK) {
          closeZipHandle();
          remove(zipFileName.c_str());
          zipError(ZIPIT_FILE_IN_ZIP_CREATION_ERR);
          return 1;
        }
        std::ifstream inFile;
        inFile.open(sourceListOfFiles[i].c_str(),
                    std::ios::out|std::ios::binary);
        if (!inFile.is_open()) {
          closeZipHandle();
          remove(zipFileName.c_str());
          zipError(ZIPIT_NO_HANDLE_FOR_FILE);
          return 1;
        }
        //bestimmen der groese der Datei
        inFile.seekg(0, std::ios::end);
        int fileLength = inFile.tellg();
        inFile.seekg(0, std::ios::beg);
        //neuen speicher holen
        char *pBuffer = new char[fileLength];//sollte das speicher holen nicht funktionieren, wird abgebrochen
        if (pBuffer==NULL) {
          zipError(ZIPIT_NO_BUFFER);
          zipCloseFileInZip(zipHandle);
          delete[] pBuffer;
          closeZipHandle();
          remove(zipFileName.c_str());
          return 1;
        }
        //wenn es funktioniert haben sollte, wird die datei gelesen
        //hier noch die jeweiligen fehler abfangen
        inFile.read(pBuffer, fileLength); //die gesammte datei wird in den speicher gelesen.
        zipitError=zipWriteInFileInZip(zipHandle, pBuffer, fileLength);//der inhalt des Speichers wird in die Datei im Zip geschrieben
        if (zipitError!=ZIP_OK) {
          zipError(ZIPIT_FILE_NOT_WROTE_IN_ZIP);
          zipCloseFileInZip(zipHandle);
          delete[] pBuffer;
          inFile.close();
          closeZipHandle();
          remove(zipFileName.c_str());
          return 1;
        }
        zipCloseFileInZip(zipHandle);
        delete[] pBuffer;
        inFile.close();
      }
      closeZipHandle();
      return 0;
    }

    /**
     * addToZip(string fileNameToZip, bool zipWithFolders)
     * adds an single file to a zipfile
     * after this, it closes the zipfilehandle
     */
    int Zipit::addToZip(const string &fileNameToZip,
                        const string &sourceFileNameToZip) {
      vector<string> listOfFiles;
      vector<string> sourceListOfFiles;
      listOfFiles.push_back(fileNameToZip);
      sourceListOfFiles.push_back(sourceFileNameToZip);
      return addToZip(listOfFiles, sourceListOfFiles);
    }


    int Zipit::getFromZip(const vector<string> &allFileNamesToRead,
                          const vector<string> &v_whereToStore) {
      int errorNumberZip;
      bool closeAfterwards = false;
      if(unZipHandle == NULL) {
        closeAfterwards = true;
        if (openUnZipHandle()==1) {
          LOG_ERROR("unable to open filehandle");
          return ZIPIT_NO_OPEN_HANDLE;
        }
      }

      for (unsigned int i=0; i<allFileNamesToRead.size(); i++) {
        if (unzLocateFile(unZipHandle, allFileNamesToRead[i].c_str(), 2)!=UNZ_OK) {
          zipError(ZIPIT_FILE_IN_ZIP_NOT_FOUND);
          closeUnZipHandle();
          return ZIPIT_FILE_IN_ZIP_NOT_FOUND;
        } else {
          if (unzOpenCurrentFile(unZipHandle)!=UNZ_OK) {
            zipError(ZIPIT_UNABLE_TO_OPEN_FILE_IN_ZIP);
            closeUnZipHandle();
            return ZIPIT_UNABLE_TO_OPEN_FILE_IN_ZIP;
          }
          FILE * pOutFile;
          int size_buffer = 1024;
          char* fileBuffer[size_buffer]; //a buffer for buffering
          pOutFile = fopen(v_whereToStore[i].c_str(), "wb");
          if (pOutFile!=NULL) { //everythings fine, proceding
            do {
              errorNumberZip = unzReadCurrentFile(unZipHandle, fileBuffer,
                                                  size_buffer);
              if (errorNumberZip<0) {
                zipError(ZIPIT_UNABLE_TO_READ_FILE_IN_ZIP);
                fclose(pOutFile);
                closeUnZipHandle();
                return ZIPIT_UNABLE_TO_READ_FILE_IN_ZIP;
              }
              if (errorNumberZip>0)
                if (fwrite(fileBuffer, 1, errorNumberZip, pOutFile)==0) {
                  errorNumberZip=UNZ_ERRNO;
                  fclose(pOutFile);
                  closeUnZipHandle();
                  zipError(ZIPIT_UNABLE_TO_WRITE_IN_FILE);
                  return ZIPIT_UNABLE_TO_WRITE_IN_FILE;
                }
            } while (errorNumberZip>0);
            if (pOutFile) {
              fclose(pOutFile);
            }
          } else {
            fclose(pOutFile);
            closeUnZipHandle();
            zipError(ZIPIT_NO_BUFFER);
            return ZIPIT_NO_BUFFER;
          }
          if (errorNumberZip==UNZ_OK) {
            //closing actual opened file in the archive
            errorNumberZip=unzCloseCurrentFile(unZipHandle);
            if (errorNumberZip!=UNZ_OK) {
              zipError(ZIPIT_UNABLE_TO_CLOSE_ZIPFILE);
              closeUnZipHandle();
              return ZIPIT_UNABLE_TO_CLOSE_ZIPFILE;
            }
          }
        }
      }
      if(closeAfterwards) closeUnZipHandle();
      return ZIPIT_SUCCESS;
    }

    int Zipit::getFromZip(const string &inZipFileName,
                          const string &whereToStore) {
      vector<string> allFileNamesToRead;
      vector<string> v_whereToStore;
      allFileNamesToRead.push_back(inZipFileName);
      v_whereToStore.push_back(whereToStore);
      return getFromZip(allFileNamesToRead, v_whereToStore);
    }

    void Zipit::zipError(ZipitError errorNumber) {
      if(errorNumber == ZIPIT_SUCCESS)
        return;
      LOG_ERROR("ZIPIT: an error occured");
      switch (errorNumber) {
      case ZIPIT_NO_OPEN_HANDLE:
        LOG_ERROR("unable to open a handle to the zip. maybe the zipfile does not "
                  "exsist or there is no further free memory.");
        break;
      case ZIPIT_NO_CLOSE_HANDLE:
        LOG_ERROR("unable to close a handle to the zip. maybe the zipfile does not "
                  "exsist or it has been closed befor");
        break;
      case ZIPIT_NO_BUFFER:
        LOG_ERROR("unable to allocate memory needed for processing the file");
        break;
      case ZIPIT_FILE_NOT_FOUND:
        LOG_ERROR("unable to open the file. maybe the filename is illegal");
        break;
      case ZIPIT_FILE_IN_ZIP_NOT_FOUND:
        LOG_ERROR("unable to find the spezified file in the zipfile. maybe the file "
                  "does not exsist or the filename is misspelled");
        break;
      case ZIPIT_FILE_IN_ZIP_CREATION_ERR:
        LOG_ERROR("unable to create the file in the zip. maybe there is now more "
                  "space left on the device");
        break;
      case ZIPIT_FILE_NOT_WROTE_IN_ZIP:
        LOG_ERROR("there has no date been written to the zipfile. reasons for "
                  "errors are many. but most possible is, something went wrong");
        break;
      case ZIPIT_UNABLE_TO_OPEN_FILE_IN_ZIP:
        LOG_ERROR("unable to open file in zip");
        break;
      case ZIPIT_UNABLE_TO_READ_FILE_IN_ZIP:
        LOG_ERROR("unable to read file in zip");
        break;
      case ZIPIT_NO_HANDLE_FOR_FILE:
        LOG_ERROR("no handle for file");
        break;
      case ZIPIT_UNABLE_TO_CLOSE_ZIPFILE:
        LOG_ERROR("unable to close zipfile");
        break;
      case ZIPIT_UNABLE_TO_WRITE_IN_FILE:
        LOG_ERROR("unable to write in file");
        break;
      case ZIPIT_UNABLE_TO_WRITE_FILE_IN_ZIP:
        LOG_ERROR("unable to write file in zip");
        break;
      case ZIPIT_UNABLE_TO_CLOSE_FILE_IN_ZIP:
        LOG_ERROR("unable to close file in zip");
        break;
      case ZIPIT_UNKNOWN_ERROR:
        LOG_ERROR("unknown error \"%d\" occurred", errorNumber);
        break;
      case ZIPIT_SUCCESS:
        // We should never get here because we should bail out
        // early on success.  This is here to silence compiler
        // warnings about unhandled switch-cases.
        assert(!"We should never get here");
      }
    }

    int Zipit::unpackWholeZipTo(const string &whereToStore) {
      string filename="";
      int err = 0;

      unsigned long currentFilenameSize=1024;
      if (whereToStore=="") {
        return 0;
      }
      if (openUnZipHandle()==1) {
        std::cout <<"unable to open filehandle" <<std::endl;
        return 1;
      }
      err=unzGoToFirstFile(unZipHandle);
      while (err==UNZ_OK) {
        //std::cout<<"in der schleife"<<std::endl;
        char currentFilename[currentFilenameSize+1];
        err=unzGetCurrentFileInfo(unZipHandle, NULL, currentFilename,
                                  size_t(currentFilenameSize)-1, NULL, 0, NULL, 0);
        if (err==UNZ_OK) {
          filename.assign(currentFilename);
          filename.insert(0, whereToStore);
          getFromZip(currentFilename, filename);
          err=unzGoToNextFile(unZipHandle);
        }
      }
      closeUnZipHandle();
      return 1;
    }

    ZipitError Zipit::unzipAll(const std::string &zipFilename,
                               const std::string &directory) {
      if(!mars::utils::pathExists(zipFilename)) {
        return ZIPIT_FILE_NOT_FOUND;
      }
      unzFile zFile = unzOpen(zipFilename.c_str());
      if(!zFile) {
        return ZIPIT_NO_OPEN_HANDLE;
      }
      size_t filenameBufferSize = 1024;
      char *filenameBuffer = static_cast<char*>(malloc(filenameBufferSize));
      size_t bufferSize = 1024;
      char *buffer = static_cast<char*>(malloc(bufferSize));
      int err = unzGoToFirstFile(zFile);
      while(err == UNZ_OK) {
        // get info about the current file
        unz_file_info info;
        unzGetCurrentFileInfo(zFile, &info, NULL, 0, NULL, 0, NULL, 0);
        // make sure we have enough space for the filename
        if(info.size_filename > filenameBufferSize) {
          filenameBufferSize = info.size_filename;
          filenameBuffer = static_cast<char*>(realloc(filenameBuffer,
                                                      filenameBufferSize));
        }
        // get the filename
        unzGetCurrentFileInfo(zFile, NULL, filenameBuffer, filenameBufferSize,
                              NULL, 0, NULL, 0);
        FILE *fd = fopen(filenameBuffer, "wb");
        unzOpenCurrentFile(zFile);
        size_t bytesRead = 0;
        while(0 < (bytesRead = unzReadCurrentFile(zFile, buffer, bufferSize))) {
          fwrite(buffer, sizeof(char), bytesRead, fd);
          // do nothing and continue reading
        }
        unzCloseCurrentFile(zFile);
        fclose(fd);
        unzGoToNextFile(zFile);
      }
      free(filenameBuffer);
      free(buffer);
      unzClose(zFile);
      return ZIPIT_SUCCESS;
    }

  } // end of namespace urdf_loader
} // end of namespace mars
