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


#ifndef OSG_MATERIAL_MANAGER_SHADER_TYPES_H
#define OSG_MATERIAL_MANAGER_SHADER_TYPES_H

#include <string>
#include <ostream>

namespace osg_material_manager {

  typedef struct {
    std::string type;
    std::string name;
  } GLSLAttribute;

  typedef GLSLAttribute GLSLVarying;
  typedef GLSLAttribute GLSLUniform;
  typedef GLSLAttribute GLSLSuffix;

  typedef struct {
    std::string name;
    std::string value;
  } GLSLExport;

  typedef struct {
    std::string type;
    std::string name;
    std::string value;
  } GLSLVariable;

  typedef GLSLVariable GLSLConstant;

  std::ostream& operator<<(std::ostream& os, const GLSLAttribute& a);
  std::ostream& operator<<(std::ostream& os, const GLSLExport& a);
  std::ostream& operator<<(std::ostream& os, const GLSLVariable& a);
  bool operator<(const GLSLAttribute& a, const GLSLAttribute& b);
  bool operator<(const GLSLExport& a, const GLSLExport& b);
  bool operator<(const GLSLVariable& a, const GLSLVariable& b);

} // end of namespace osg_material_manager

#endif /* OSG_MATERIAL_MANAGER_SHADER_TYPES_H */

