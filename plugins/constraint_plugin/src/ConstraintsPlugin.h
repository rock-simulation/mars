/*
 *  Copyright 2012, DFKI GmbH Robotics Innovation Center
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
 * \file ConstraintsPlugin.h
 * \author Lorenz Quack (lorenz.quack@dfki.de)
 * \brief Plugin to create constraints between Nodes.
 *
 * Version 0.1
 */

#ifndef CONSTRAINTS_PLUGIN_H
#define CONSTRAINTS_PLUGIN_H

#ifdef _PRINT_HEADER_
  #warning "ConstraintsPlugin.h"
#endif

#include <map>
#include <vector>

#include <mars/interfaces/sim/MarsPluginTemplate.h>
#include <mars/cfg_manager/CFGDefs.h>
#include <mars/main_gui/MenuInterface.h>
#include <mars/main_gui/GuiInterface.h>

#include "BaseConstraint.h"


namespace mars {
  namespace plugins {

    namespace constraints_plugin {

      enum ParseResult { PARSE_SUCCESS,
                         PARSE_SUCCESS_EOS,
                         PARSE_ERROR_EOS,
                         PARSE_ERROR_NODEID,
                         PARSE_ERROR_NODEATTR };

      class ConstraintsPlugin: public interfaces::MarsPluginTemplate,
                               public cfg_manager::CFGClient,
                               public main_gui::MenuInterface {

      public:
        ConstraintsPlugin(lib_manager::LibManager *theManager);
        ~ConstraintsPlugin();

        // LibInterface methods
        int getLibVersion() const {return 1;}
        const std::string getLibName() const {return std::string("constraints_plugin");}

        void init();
        void reset();
        void update(interfaces::sReal time_ms);
        void loadConstraintDefs();
        void saveConstraintDefs() const;
        void loadConstraints();
        void saveConstraints() const;
        void loadMotors();
        void saveMotors() const;

        void parseNodeConstraints(const std::string &paramName,
                                  const std::string &s);
        AttributeType parseAttribute(const std::string &attributeString);
        double getNodeAttribute(interfaces::NodeId nodeId, AttributeType attr);


        // cfg_dfki::CFGClient
        void cfgUpdateProperty(cfg_manager::cfgPropertyStruct property);
        void cfgParamCreated(cfg_manager::cfgParamId id);
        void cfgParamRemoved(cfg_manager::cfgParamId id);

        // gui_core::MenuInterface
        void menuAction(int action, bool checked = false);
    
      private:
        ParseResult parseIdentifier(const std::string &s, size_t *pos,
                                    interfaces::NodeId *nodeId, AttributeType *attr,
                                    double *factor);
        void parseConstraintFromString(const std::string &name,
                                       const std::string &s);

        typedef std::vector<BaseConstraint*> ConstraintsContainer;
        typedef std::map<cfg_manager::cfgParamId, ConstraintsContainer> ConstraintsLookup;
        ConstraintsLookup constraints;
        main_gui::GuiInterface *gui;

      }; // end of class definition ConstraintsPlugin

    } // end of namespace constraints_plugin
  } // end of namespace plugins
} // end of namespace mars

#endif // CONSTRAINTS_PLUGIN_H
