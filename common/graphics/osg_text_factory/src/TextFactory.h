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
 * \file TextFactory.h
 * \author Malte Langosz
 * \brief 
 **/

#ifndef OSG_TEXT_FACTORY_H
#define OSG_TEXT_FACTORY_H

#ifdef _PRINT_HEADER_
#warning "TextFactory.h"
#endif

#include "TextFactoryInterface.h"
#include <mars/osg_text/Text.h>

#include <string>

namespace osg_text {

  class TextFactory : public TextFactoryInterface {

  public:
    TextFactory(mars::lib_manager::LibManager *theManager);

    ~TextFactory();

    TextInterface* createText(std::string text="", double fontSize=12,
                                Color textColor=Color(),
                                double posX=0., double posY=0.,
                                TextAlign textAlign=ALIGN_LEFT,
                                double paddingL=0., double paddingT=0.,
                                double paddingR=0., double paddingB=0.,
                                Color backgroundColor=Color(),
                                Color borderColor=Color(),
                                double borderWidth = 0.0);
  };

} // end of namespace: osg_text

#endif // OSG_TEXT_FACTORY_H
