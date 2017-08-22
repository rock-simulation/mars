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

  DRockGraphSP::DRockGraphSP(string res_path, ConfigMap graph, ConfigMap options) : IShaderProvider(res_path) {
    this->graph = graph;
    this->options = options;
  }

  int DRockGraphSP::getMinVersion() {
    return 0;
  }

  const set<GLSLUniform>& DRockGraphSP::getUniforms() const {
    return uniforms;
  }

  const set<GLSLAttribute>& DRockGraphSP::getVaryings() const {
    return varyings;
  }

  string DRockGraphSP::generateMainSource() {
    return "not implemented yet!";
  }

  const set<string>& DRockGraphSP::getEnabledExtensions() const {
    return enabledExtensions;
  }

  const set<string>& DRockGraphSP::getDisabledExtensions() const {
    return disabledExtensions;
  }

  const std::set<GLSLConstant>& DRockGraphSP::getConstants() const {
    return constants;
  }

  const std::set<GLSLAttribute>& DRockGraphSP::getAttributes() const {
    return attributes;
  }

  const vector<pair<string, string> > DRockGraphSP::getDependencies() const {
    return dependencies;
  }

  string DRockGraphSP::generateDefinitions() {
    return "";
  }
}