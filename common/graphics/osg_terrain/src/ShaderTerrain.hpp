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

/**
 * \file ShaderTerrain.hpp
 * \author Malte Langosz
 * \brief 
 **/

#ifndef OSG_SHADER_TERRAIN_HPP
#define OSG_SHADER_TERRAIN_HPP

#include <configmaps/ConfigData.h>

#include <osg/Group>
#include <osg/Geometry>
#include <osgUtil/CullVisitor>
#include <osg/Transform>

namespace osg_terrain {
  class PosTransform : public osg::Transform {
  public:
    double zOffset;
    double x, y, z;
    bool overridePos;
    // todo: add x and y offsets
    // todo: add fixe position
    virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix,
                                           osg::NodeVisitor* nv) const;
    virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix,
                                           osg::NodeVisitor* nv) const;
  };

  class ShaderTerrain : public osg::Group {

    class BoundCallback: public osg::Drawable::ComputeBoundingBoxCallback {
    public:
      double size[3];

      BoundCallback(){}
      ~BoundCallback(){}
      osg::BoundingBox computeBound(const osg::Drawable &) const;

    };

    class BoundVisitor: public osg::NodeVisitor{
    public:
      BoundVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {}
      virtual void apply(osg::Node &searchNode);
      osg::ref_ptr<osg::Drawable::ComputeBoundingBoxCallback> s;
    };

  public:
    ShaderTerrain(configmaps::ConfigMap map);
    ~ShaderTerrain();

    /*
     * Override position transform
     */
    void setPos(double x, double y, double z);
    osg::ref_ptr<osg::Node> getNode();

  private:
    osg::ref_ptr<BoundCallback> boundCallback;
    osg::ref_ptr<PosTransform> posTransform;
  };

} // end of namespace: osg_terrain

#endif // OSG_SHADER_TERRAIN_H
