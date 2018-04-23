/*
 *  Copyright 2011, 2012 DFKI GmbH Robotics Innovation Center
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


#ifndef MARS_UTILS_GEOMETRY_H
#define MARS_UTILS_GEOMETRY_H

#ifdef WIN32
#  ifndef EIGEN_DONT_ALIGN
#    define EIGEN_DONT_ALIGN
#  endif
#endif

#include <Eigen/Core>
#include "Vector.h"
#include "mathUtils.h"

namespace mars {
  namespace utils {

    struct Line;
    struct Plane;

    enum Relation {
      PARALLEL,
      INTERSECT,
      SKEW,
      CONTAINING,
      IDENTICAL_OR_MULTIPLE,
      ORTHOGONAL
    };

    Relation relation(Plane plane1, Plane plane2);
    Relation relation(Plane plane, Line line);
    Relation relation(Plane plane1, Vector point);
    Relation relation(Line line1, Line line2);
    Relation relation(Line line, Vector point);
    Relation relation(Vector vector1, Vector vector2);

    Vector intersect(Plane plane, Line line);
    Line intersect(Plane plane1, Plane plane2);

    Vector elemWiseDivision(Vector vector1, Vector vector2);
    double distance(Plane plane, Vector point, bool absolute=true);

    struct Line {
      Vector point;
      Vector direction;
      bool empty = false;

      enum Method{
        POINT_VECTOR,
        POINT_POINT
      };

      Line(Vector point, Vector vector_type, Method method=Method::POINT_VECTOR) {
        switch (method) {
          case Method::POINT_VECTOR:
            this->point = point;
            this->direction = vector_type.normalized();
          case Method::POINT_POINT:
            assert (relation(point,vector_type) != Relation::IDENTICAL_OR_MULTIPLE);
            this->point = point;
            this->direction = (vector_type-point).normalized();
          default:
            assert(false);
        }

      }

      Line() {
        point = Vector(0,0,0);
        direction = Vector(0,0,0);
        empty = true;
      }

      Vector getPointOnLine(double r) {
        return point + direction * r;
      }
    };

    struct Plane {
      Vector normal;
      Vector point;

      enum Method {
        THREE_POINTS,
        POINT_TWO_VECTORS
      };

      Plane(Vector point, Line line) {
        assert(relation((point-line.point), line.direction) != Relation::IDENTICAL_OR_MULTIPLE);
        this->point = point;
        this->normal = line.direction.cross(point-line.point).normalized();
      }

      Plane(Vector point, Vector vector_type1, Vector vector_type2, Method method=Method::THREE_POINTS) {
        switch (method) {
          case Method::POINT_TWO_VECTORS:
            assert(relation(vector_type1,vector_type2) != Relation::IDENTICAL_OR_MULTIPLE);
            this->point = point;
            this->normal = vector_type1.cross(vector_type2).normalized();
          case Method::THREE_POINTS:
            assert(relation((vector_type1-point),(vector_type2-point)) != Relation::IDENTICAL_OR_MULTIPLE);
            assert(relation(vector_type2,vector_type1) != Relation::IDENTICAL_OR_MULTIPLE);
            this->point = point;
            this->normal = (vector_type1-point).cross(vector_type2-point).normalized();
          default:
            assert(false);
        }
      }

      Plane(Vector point, Vector normal) {
        this->point = point;
        this->normal = normal.normalized();
      }

      void flipNormal() {
        this->normal *= -1;
      }

      void pointNormalTowards(Vector point) {
        double dist = distance(*this, point, false);
        if (dist < 0) {
          this->flipNormal();
        }
      }
    };
  } // end of namespace utils
} // end of namespace mars

#endif /* MARS_UTILS_VECTOR_H */

