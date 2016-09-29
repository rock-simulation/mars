/*
 *  Copyright 2016 DFKI GmbH Robotics Innovation Center
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
 * \file EntitySubscriberInterface.h
 * \author Kai von Szadkowski (kai.von-szadkowski@uni-bremen.de)
 * \brief "EntitySubscriber" is an interface to allow components of the simulation
 *        to be notified when an entity is created (or changed).
 */

#ifndef ENTITY_SUBSCRIBER_INTERFACE_H
#define ENTITY_SUBSCRIBER_INTERFACE_H

#ifdef _PRINT_HEADER_
#warning "EntitySubscriberInterface.h"
#endif

namespace mars {

  namespace sim {
    class SimEntity;
  }

  namespace interfaces {

    /**
     * The interface for notifying components of changes in simulation entities.
     *
     */
    class EntitySubscriberInterface {

    public:
      EntitySubscriberInterface(void) {};
      virtual ~EntitySubscriberInterface(void) {};
      virtual void registerEntity(sim::SimEntity *) = 0;

    };

  } // end of namespace interfaces
} // end of namespace mars

#endif  // ENTITY_SUBSCRIBER_INTERFACE_H
