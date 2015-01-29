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
 * \file TextFactory.cpp
 * \author Malte Langosz (malte.langosz@dfki.de)
 * \brief
 *
 * Version 0.1
 */

#include "TextFactory.h"

namespace osg_text {
  
  TextFactory::TextFactory(lib_manager::LibManager *theManager) :
    TextFactoryInterface(theManager) {
  }

  TextFactory::~TextFactory(void) {
  }

  TextInterface* TextFactory::createText(std::string text, double fontSize,
                                         Color textColor,
                                         double posX, double posY,
                                         TextAlign textAlign,
                                         double paddingL, double paddingT,
                                         double paddingR, double paddingB,
                                         Color backgroundColor,
                                         Color borderColor,
                                         double borderWidth) {
    return new Text(text, fontSize, textColor, posX, posY, textAlign,
                    paddingL, paddingT, paddingR, paddingB,
                    backgroundColor, borderColor, borderWidth);
  }

} // end of namespace: osg_text

CLASS_LOADER_REGISTER_CLASS(osg_text::TextFactory, singleton::Interface );
;
