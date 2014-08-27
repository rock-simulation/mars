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

#ifndef MARS_INTERFACES_SENSOR_BASES_H
#define MARS_INTERFACES_SENSOR_BASES_H

#ifdef _PRINT_HEADER_
  #warning "sensor_bases.h"
#endif

#include "core_objects_exchange.h"

#include <mars/utils/ConfigData.h>
#include <mars/utils/Quaternion.h>
#include <mars/utils/Vector.h>

#include <vector>
#include <limits>


namespace mars {

  namespace interfaces {

    class ControlCenter;

    class BaseConfig {
    public:
      BaseConfig() : updateRate(10) {}
      virtual ~BaseConfig() {}
      std::string name;
      unsigned long id;
      unsigned long updateRate;
    }; // end of class BaseConfig

    class BaseSensor {
    public:
      BaseSensor()
      {
        id = 0;
        name = "UNKNOWN";
        updateRate = 10;
      }
      virtual ~BaseSensor(){}

      BaseSensor(unsigned long id, std::string name):
        id(id),
        name(name)
      {
      }

      unsigned long getID() const{
        return id;
      };

      const std::string getName() const{
        return name;
      }

      virtual int getSensorData(double **data) const{
        return 0;
      };

      virtual int getAsciiData(char *data) const{
        return 0;
      }

      void getCoreExchange(core_objects_exchange* obj) const{
        obj->index = id;
        obj->name = name;
      }

      /*
        static BaseConfig parseConfig(QDomElement *elementNode){
        return BaseConfig();
        }

        virtual QDomElement createConfig() {return QDomElement();}
      */

      static BaseConfig* parseConfig(ControlCenter *control,
                                     utils::ConfigMap *config) {
        return new BaseConfig();
      }

      virtual utils::ConfigMap createConfig() const {
        return utils::ConfigMap();
      }

      //Should be proteted due to compability of old code currently direct accessable
      unsigned long id;
      std::string name; //Todo naming bei mehreren robotern
      unsigned long updateRate;

    protected:

    }; // end of class BaseSensor


    class BaseNodeSensor : public BaseSensor {
    public:
      BaseNodeSensor(unsigned long id, std::string name)
        : BaseSensor(id,name)
      {
        calcAcceletaion = false;
        calcSpeed = false;
        calcPosition = false;
        calcRotationSpeed = false;
        calcOrientation = false;
        acceleration.setZero();
        speed.setZero();
        position.setZero();
        rotationSpeed.setZero();
        orientation.setIdentity();
        attached_node = 0;
      }
      virtual ~BaseNodeSensor(){}

      utils::Quaternion getOrientation() const{
        return orientation;
      }

      unsigned long getAttachedNode(){
        return attached_node;
      }

    protected:
      unsigned long attached_node;
      bool calcAcceletaion, calcSpeed, calcPosition,calcRotationSpeed,calcOrientation;
      utils::Vector acceleration;
      utils::Vector speed;
      utils::Vector position;
      utils::Vector rotationSpeed;
      utils::Quaternion orientation;

    }; // end of class BaseNodeSensor


    template <class T>
    class BaseArraySensor {
    public:
      BaseArraySensor(int cols, int rows, int channels=1)
        : cols(cols),
          rows(rows),
          channels(channels)
      {
        data.resize(rows*cols*channels);
      }

      virtual ~BaseArraySensor(){}

      const std::vector<T> getData() const{
        return data;
      }
      const int& getCols() const{
        return cols;
      }
      const int& getRows() const{
        return rows;
      }
      const int& getChannels() const{
        return channels;
      }

      T &operator[](const int &index){
        assert(index >= 0 && index < (int)data.size());
        return data[index];
      }
      virtual void resize(int cols, int rows, int channels=1){
        this->cols = cols;
        this->rows = rows;
        this->channels = channels;
        data.resize(rows*cols*channels);
      }


    protected:
      int cols;
      int rows;
      int channels;
      std::vector<T> data;

    }; // end of class BaseArraySensor


    template <class T>
    class BaseCameraSensor : public BaseArraySensor<T> {
    public:
      BaseCameraSensor(unsigned long id, std::string name, int cols, int rows, int nChannels, bool depthImage):
        BaseArraySensor<T>(cols,rows,nChannels),
        depthImage(depthImage),
        id(id),
        name(name)
      {
      }

      virtual ~BaseCameraSensor(){}

      bool isDepthImage(){
        return depthImage;
      }


    protected:
      bool depthImage;
      unsigned long id;
      std::string name;

    }; // end of BaseCameraSensor


    class BasePolarIntersectionSensor : public BaseNodeSensor, 
                                        public BaseArraySensor<double> {
    public:
      BasePolarIntersectionSensor(unsigned long id, std::string name, int cols, int rows, double angleX, double angleY, double maxDistance= std::numeric_limits<double>::infinity()):
        BaseNodeSensor(id,name),
        BaseArraySensor<double>(cols,rows),
        maxDistance(maxDistance)
      {
        if(cols == 1) stepX = 0;
        else stepX = angleX / cols;
        if(rows == 1) stepY = 0;
        else stepY = angleY / rows;

        //Must Remain true for correct calculation in Physics
        calcOrientation = true;

      }
      virtual ~BasePolarIntersectionSensor(){}

      virtual int getSensorData(double **data) const{
        (*data) = (new double[this->data.size()]);
        memcpy((*data),&this->data[0],sizeof(double)*this->data.size());
        return this->data.size();
      };



      double stepX;
      double stepY;
      double maxDistance;
    protected:

    }; // end of class BasePolarIntersectionSensor


    class BaseGridIntersectionSensor : public BaseNodeSensor,
                                       public BaseArraySensor<double> {
    public:
      BaseGridIntersectionSensor(unsigned long id, std::string name, int cols, int rows, double stepX, double stepY, double maxDistance= std::numeric_limits<double>::infinity()):
        BaseNodeSensor(id,name),
        BaseArraySensor<double>(cols,rows),
        stepX(stepX),
        stepY(stepY),
        maxDistance(maxDistance)
      {
      }
      virtual ~BaseGridIntersectionSensor(){}


      double stepX;
      double stepY;
      double maxDistance;
    protected:

    }; // end of class BaseGridIntersectionSensor

  } // end of namespace interfaces

} // end of namespace mars

#endif // MARS_INTERFACES_SENSOR_BASES_H
