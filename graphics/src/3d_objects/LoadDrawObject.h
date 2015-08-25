/*
 *  Copyright 2011, 2012, DFKI GmbH Robotics Innovation Center
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

/*
 *  LoadDrawObject.h
 *  General LoadDrawObject to inherit from.
 *
 *  Created by Roemmermann on 21.10.09.
 */

#ifndef MARS_GRAPHICS_LOAD_DRAW_OBJECT_H
#define MARS_GRAPHICS_LOAD_DRAW_OBJECT_H

#include "DrawObject.h"

#include <mars/utils/Vector.h>
#include <configmaps/ConfigData.h>

#include <string>
#include <list>

namespace mars {
  namespace graphics {
    struct LoadDrawObjectPSetBox {
      osg::PrimitiveSet* primitiveSet;
      mars::utils::Vector min, max;
    };

    class LoadDrawObject : public DrawObject {

    public:
      LoadDrawObject(GraphicsManager *g, configmaps::ConfigMap &map,
                     const mars::utils::Vector &ext);

    protected:
      configmaps::ConfigMap info_;
      std::vector<std::vector<LoadDrawObjectPSetBox*>*> gridPSets_;

      virtual std::list< osg::ref_ptr< osg::Geode > > createGeometry();

    private:
      std::list< osg::ref_ptr< osg::Geode > > loadGeodes(std::string filename,
                                                         std::string objname);
    };

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_LOAD_DRAW_OBJECT_H */
