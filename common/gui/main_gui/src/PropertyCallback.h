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
 * \file PropertyCallback.h
 * \author Malte Römmermann
 * \author Vladimir Komsiyski
 */


#ifndef PC_H
#define PC_H

namespace mars {
  namespace main_gui {

    /**
     * \brief Introduced for Windows compatibility.
     *        Calls a PropertyDialog instance associated with it.
     */
    class PropertyCallback {

    public:
      /**
       * \brief Destructor.
       */
      virtual  ~PropertyCallback() {}

      /**
       * \brief Called every time a property has changed its value.
       * \param property The property with a new value.
       * \param value The new value.
       */
      virtual void valueChanged(QtProperty *property, const QVariant &value) {
        (void)property; (void)value;
      }

      /**
       * \brief Called every time another branch
       *        of a QtTreePropertyBrowser has been selected.
       * \param current The current top level item that is selected.
       */
      virtual void topLevelItemChanged(QtProperty *current) {
        (void)current;
      }

      /**
       * \brief Associated with the \a OK button of the Property Dialog.
       */
      virtual void accept() {}

      /**
       * \brief Associated with the \a Cancel button of the Property Dialog.
       */
      virtual void reject() {}

    }; // end class PropertyCallback

  } // end namespace main_gui
} // end namespace mars


#endif /* PC_H */
