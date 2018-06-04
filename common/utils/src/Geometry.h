/*
 *  Copyright 2018 DFKI GmbH Robotics Innovation Center
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

    Vector intersect(Line line1, Line line2);
    Vector intersect(Plane plane, Line line);
    Line intersect(Plane plane1, Plane plane2);

    Vector elemWiseDivision(Vector vector1, Vector vector2);
    double distance(Plane plane, Vector point, bool absolute=true);
    double distance(Line line1, Line line2);

    struct Line {
      Vector point;
      Vector direction;
      double r_min=-INFINITY;
      double r_max=INFINITY;

      bool initialized;
      bool isInitialized() {
        if (initialized) {
          initialized = (point != Vector(NAN, NAN, NAN));
        }
        return initialized;
      }

      enum Method{
        POINT_VECTOR,
        POINT_POINT
      };

      Line(Vector point, Vector vector_type, Method method=Method::POINT_VECTOR);
      Line(void);

      Vector getPointOnLine(double r);
      double getFactorForPoint(Vector point);
    };

    struct Plane {
      Vector point;
      Vector normal;
      bool initialized;
      bool isInitialized() {
        if (initialized) {
          initialized = (point != Vector(NAN, NAN, NAN));
        }
        return initialized;
      }

      enum Method {
        THREE_POINTS,
        POINT_TWO_VECTORS
      };

      Plane(Vector point, Line line);
      Plane(Vector point, Vector vector_type1, Vector vector_type2, Method method=Method::THREE_POINTS);
      Plane(Vector point, Vector normal);
      Plane(void);

      void flipNormal();
      void pointNormalTowards(Vector point);
    };
  } // end of namespace utils
} // end of namespace mars

#endif /* MARS_UTILS_VECTOR_H */

