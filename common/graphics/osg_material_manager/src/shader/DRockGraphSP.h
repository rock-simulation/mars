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

#ifndef MARS_DEV_DROCKGRAPHSP_H
#define MARS_DEV_DROCKGRAPHSP_H

#include "IShaderProvider.h"
#include "configmaps/ConfigData.h"

namespace osg_material_manager {

  using namespace std;
  using namespace configmaps;

  class DRockGraphSP : public IShaderProvider {
  public:
    DRockGraphSP(string res_path, ConfigMap graph, ConfigMap options);

    int getMinVersion();

    const set<GLSLUniform>& getUniforms() const;

    const set<GLSLAttribute>& getVaryings() const;

    const set<string>& getEnabledExtensions() const;

    const set<string>& getDisabledExtensions() const;

    const vector<pair<string, string> > getDependencies() const;

    const set<GLSLConstant>& getConstants() const;

    const set<GLSLAttribute>& getAttributes() const;

    string generateMainSource();

    string generateDefinitions();

  private:
    ConfigMap model;
    /**
     * Containing options needed for shader generation.
     * Fields:
     * num_lights: needed
     */
    ConfigMap options;

    set<GLSLUniform> uniforms;
    set<GLSLAttribute> varyings;
    set<string> enabledExtensions;
    set<string> disabledExtensions;
    vector<pair<string, string> > dependencies;
    set<GLSLConstant> constants;
    set<GLSLAttribute> attributes;

    /**
     * Parses the graph file to fill the fields with the correct content
     */
    void parseGraph();
  };
}


#endif //MARS_DEV_DROCKGRAPHSP_H
