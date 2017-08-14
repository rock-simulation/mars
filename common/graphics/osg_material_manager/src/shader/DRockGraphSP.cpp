/*
 *  Copyright 2017, DFKI GmbH Robotics Innovation Center
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

#include "DRockGraphSP.h"

namespace osg_material_manager {

  DRockGraphSP::DRockGraphSP(string res_path, ConfigMap graph) : IShaderProvider(res_path) {
    this->graph = graph;
  }

  int DRockGraphSP::getMinVersion() {
    return 0;
  }

  const set<GLSLUniform>& DRockGraphSP::getUniforms() const {
    return set<GLSLUniform>();
  }

  const set<GLSLAttribute>& DRockGraphSP::getVaryings() const {
    return set<GLSLAttribute>();
  }

  string DRockGraphSP::generateMainSource() {
    return "not implemented yet!";
  }

  const set<string>& DRockGraphSP::getEnabledExtensions() const {
    return set<string>();
  }

  const set<string>& DRockGraphSP::getDisabledExtensions() const {
    return set<string>();
  }

  const std::set<GLSLConstant>& DRockGraphSP::getConstants() const {
    return set<GLSLConstant>();
  }

  const std::set<GLSLAttribute>& DRockGraphSP::getAttributes() const {
    return set<GLSLAttribute>();
  }

  const vector<pair<string, string> >& DRockGraphSP::getDependencies() const {
    return vector<pair<string, string> >();
  }

  string DRockGraphSP::generateDefinitions() {
    return "";
  }
}