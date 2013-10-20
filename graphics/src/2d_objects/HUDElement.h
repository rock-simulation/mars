/*
 *  Copyright 2011, 2012, 2013, DFKI GmbH Robotics Innovation Center
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
 * \file HUDElement.h
 * \author Malte Langosz
 * \brief The "HUDElement" 
 */

#ifndef MARS_GRAPHICS_HUDELEMENT_H
#define MARS_GRAPHICS_HUDELEMENT_H

#include <osg/Group>
#include <string>

namespace mars {
  namespace graphics {

    class HUDElement {

    public:
      HUDElement(void) {}
      virtual ~HUDElement(void) {}
      virtual osg::Group* getNode(void) = 0;
      virtual void switchCullMask() = 0;
      virtual void xorCullMask(unsigned int mask) = 0;
      virtual void setID(unsigned long id) {this->id = id;}
      virtual unsigned long getID(void) {return id;}
      virtual void setConfigPath(std::string path) {config_path = path;}
      virtual void setPos(double x, double y) = 0;

      static int elemCount;

    private:
      unsigned long id;
    protected:
      std::string config_path;
    };

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_HUDELEMENT_H */
