/*
 *  Copyright 2021 DFKI GmbH Robotics Innovation Center
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
 * \file ContactsPhysics.hpp
 * \author Raul Dominguez
 * \brief "ContactsPhysisc" is a struct to exchange contact information between the core of Mars and Physics Plugins.
 *
 */

#ifndef CONTACTS_PHYSICS_H
#define CONTACTS_PHYSICS_H

#ifdef _PRINT_HEADER_
  #warning "ContactPhysics.h"
#endif

#include <ode/contact.h>
#include <smurf/Collidable.hpp>

#ifndef ODE11
  #define dTriIndex int
#endif

namespace mars {
  namespace sim {

    /*
     */
    struct ContactsPhysics {
        std::shared_ptr<std::vector<dContact>> contactsPtr; 
        int numContacts;
        std::shared_ptr<smurf::Collidable> collidable;
    };

  } // end of namespace sim
} // end of namespace mars

#endif  // CONTACT_PHYSICS_H
