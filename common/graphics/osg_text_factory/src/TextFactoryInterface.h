/*
 *  Copyright 2014, DFKI GmbH Robotics Innovation Center
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
 * \file TextFactoryInterface.h
 * \author Malte Langosz
 * \brief 
 **/

#ifndef OSG_TEXT_FACTORY_INTERFACE_H
#define OSG_TEXT_FACTORY_INTERFACE_H

#ifdef _PRINT_HEADER_
#warning "TextFactoryInterface.h"
#endif

#include <lib_manager/LibInterface.hpp>
#include <mars/osg_text/TextInterface.h>
#include <string>

namespace osg_text {

  class TextFactoryInterface : public lib_manager::LibInterface {

  public:
  TextFactoryInterface(lib_manager::LibManager *theManager) :
    lib_manager::LibInterface(theManager) {}

    virtual ~TextFactoryInterface() {}

    CREATE_MODULE_INFO();
    // LibInterface methods
    int getLibVersion() const {return 1;}
    const std::string getLibName() const {return "osg_text_factory";}

    virtual TextInterface* createText(std::string text="", double fontSize=12,
                                      Color textColor=Color(),
                                      double posX=0., double posY=0.,
                                      TextAlign textAlign=ALIGN_LEFT,
                                      double paddingL=0., double paddingT=0.,
                                      double paddingR=0., double paddingB=0.,
                                      Color backgroundColor=Color(),
                                      Color borderColor=Color(),
                                      double borderWidth = 0.0) = 0;
  };

} // end of namespace: osg_text

#endif // OSG_TEXT_FACTORY_INTERFACE_H
