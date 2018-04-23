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

#include <Eigen/Core>
#include "Vector.h"
#include "Geometry.h"

namespace mars {
  namespace utils {

    Relation relation(Vector vector1, Vector vector2) {
      Vector quot;
      double s;
      s = vector1.dot(vector2);
      if (quot.x()-quot.y() <= EPSILON && quot.y()-quot.z() <= EPSILON){
        return Relation::IDENTICAL_OR_MULTIPLE;
      } else if (s <= EPSILON) {
        return Relation::ORTHOGONAL;
      }
      return Relation::SKEW;
    }

    Vector elemWiseDivision(Vector vector1, Vector vector2) {
      return Vector(vector1.x() / vector2.x(),
                    vector1.y() / vector2.y(),
                    vector1.z() / vector2.z());
    }

    Relation relation(Plane plane1, Plane plane2) {
      if (relation(plane1.normal,plane2.normal) == Relation::IDENTICAL_OR_MULTIPLE) {
        if (relation(plane1, plane2.point) == Relation::CONTAINING) {
          return Relation::IDENTICAL_OR_MULTIPLE;
        }
        return Relation::PARALLEL;
      }
      return Relation::INTERSECT;

    }

    Relation relation(Plane plane, Line line) {
      if (relation(plane.normal,line.direction) == Relation::ORTHOGONAL) {
        if (relation(plane, line.point) == Relation::CONTAINING) {
          return Relation::CONTAINING;
        }
        return Relation::PARALLEL;
      }
      return Relation::INTERSECT;
    }

    Relation relation(Plane plane, Vector point) {
      if (relation(plane.normal, plane.point-point) == Relation::ORTHOGONAL) {
        return Relation::CONTAINING;
      }
      return Relation::SKEW;
    }

    Relation relation(Line line1, Line line2){
      if (relation(line1.direction, line2.direction) == Relation::IDENTICAL_OR_MULTIPLE) {
        if (relation(line1, line2.point) == Relation::CONTAINING) {
          return Relation::IDENTICAL_OR_MULTIPLE;
        }
        return Relation::PARALLEL;
      }
      if (relation(line1,line2.point) == Relation::CONTAINING) {
        return Relation::INTERSECT;
      }
      return Relation::SKEW;
    }

    Relation relation(Line line, Vector point) {
      Vector vec = point - line.point;
      if (relation(line.direction, vec) == Relation::IDENTICAL_OR_MULTIPLE) {
        return Relation::CONTAINING;
      }
      return Relation::SKEW;
    }

    /**
    * \brief calculates the intersection point of plane and line if they intersect
    * \return intersection point, otherwise 0-vector
    */
    Vector intersect(Plane plane, Line line) {
      if (relation(plane, line) == Relation::INTERSECT) {
        double r = (-plane.normal.dot(line.point - plane.point))/plane.normal.dot(line.direction);
        return line.getPointOnLine(r);
      }
      return Vector(0,0,0);
    }

    /**
    * \brief calculates the intersection line of 2 planes if they intersect
    * \return intersection line, otherwise 0-vector line
    */
    Line intersect(Plane plane1, Plane plane2) {
      if (relation(plane1,plane2) == Relation::INTERSECT) {
        //direction of intersection
        Vector direction = plane1.normal.cross(plane2.normal);
        //create a line that is ORTHOGONAL to the intersection and plane1.normal and lies in plane1
        Line dummy(plane1.point, direction.cross(plane1.normal));
        //the intersection point of this line and plane2 is a point on the intersection line of both planes
        Vector point = intersect(plane2, dummy);
        return Line(point, direction);
      }
      return Line();
    }

    double distance(Plane plane, Vector point, bool absolute/*=true*/) {
      Line perpendicular(point, plane.normal);
      Vector projected = intersect(plane, perpendicular);
      Vector distance = (point - projected);
      assert(relation(distance, plane.normal) == Relation::IDENTICAL_OR_MULTIPLE);
      if (!absolute) {
        return elemWiseDivision(distance, plane.normal).x();
      }
      return distance.norm();
    }

  } // end of namespace utils
} // end of namespace mars

