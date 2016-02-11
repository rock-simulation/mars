/*
 *  Copyright 2016, DFKI GmbH Robotics Innovation Center
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


#include "shader-types.h"

namespace osg_material_manager {

  using namespace std;

  ostream& operator<<(ostream& os, const GLSLExport& a) {
    return os << a.name << " = " << a.value;
  }
  bool operator<(const GLSLExport& a, const GLSLExport& b) {
    return a.name < b.name;
  }

  ostream& operator<<(ostream& os, const GLSLAttribute& a) {
    return os << a.type << " " << a.name;
  }
  bool operator<(const GLSLAttribute& a, const GLSLAttribute& b) {
    return a.name < b.name;
  }

  ostream& operator<<(ostream& os, const GLSLVariable& a) {
    return os << a.type << " " << a.name << " = " << a.value;
  }
  bool operator<(const GLSLVariable& a, const GLSLVariable& b) {
    return a.name < b.name;
  }

} // end of namespace osg_material_manager
