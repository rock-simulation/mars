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

/**
 * \file DataBroker.h
 * \author Malte Roemmermann, Lorenz Quack
 * \brief DataBroker A small library to manage data communication within a
 *        software framework.
 *
 * Version 0.2
 */

#ifndef PRODUCERINTERFACE_H
#define PRODUCERINTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "ProducerInterface.h"
#endif


namespace mars {

  namespace data_broker {

    // forward declarations
    class DataPackage;
    class DataInfo;

    class ProducerInterface {
    public:
      ProducerInterface() {}
      virtual ~ProducerInterface() {}
      
      virtual void produceData(const DataInfo &info,
                               DataPackage *package,
                               int callbackParam) = 0;

    }; // end of class ProducerInterface

  } // end of namespace data_broker

} // end of namespace mars

#endif /* PRODUCERINTERFACE_H */
