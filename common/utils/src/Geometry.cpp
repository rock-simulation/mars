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

#include <Eigen/Core>
#include "Vector.h"
#include "Geometry.h"
#include <cmath>

namespace mars {
  namespace utils {
    Line::Line(Vector point, Vector vector_type, Method method/*=Method::POINT_VECTOR*/) : point(point) {
      switch (method) {
        case Method::POINT_VECTOR:
          direction = vector_type.normalized();
          break;
        case Method::POINT_POINT:
          assert (relation(point,vector_type) != Relation::IDENTICAL_OR_MULTIPLE);
          direction = (vector_type-point).normalized();
          break;
        default:
          assert(method == Method::POINT_VECTOR || method == Method::POINT_POINT);
          break;
      }
      initialized = true;
    }

    Line::Line() : point(Vector(NAN, NAN, NAN)), direction(Vector(0,0,0)), initialized(false) {
    }

    Vector Line::getPointOnLine(double r) {
      assert(isInitialized() == true);
      return point + direction * r;
    }

    double Line::getFactorForPoint(Vector point) {
      assert(isInitialized() == true);
      Vector factor = elemWiseDivision(point, direction);
      assert(factor.x() == factor.y() and factor.y() == factor.z());
      return factor.x();
    }

    Plane::Plane(Vector point, Line line) : point(point) {
      assert(relation((point-line.point), line.direction) != Relation::IDENTICAL_OR_MULTIPLE);
      normal = line.direction.cross(point-line.point).normalized();
      initialized = true;
    }

    Plane::Plane(Vector point, Vector vector_type1, Vector vector_type2, Method method/*=Method::THREE_POINTS*/) : point(point) {
      switch (method) {
        case Method::POINT_TWO_VECTORS:
          assert(relation(vector_type1,vector_type2) != Relation::IDENTICAL_OR_MULTIPLE);
          normal = vector_type1.cross(vector_type2).normalized();
          break;
        case Method::THREE_POINTS:
          assert(relation((vector_type1-point),(vector_type2-point)) != Relation::IDENTICAL_OR_MULTIPLE);
          assert(relation(vector_type2,vector_type1) != Relation::IDENTICAL_OR_MULTIPLE);
          normal = (vector_type1-point).cross(vector_type2-point).normalized();
          break;
        default:
          assert(method == Method::POINT_TWO_VECTORS || method == Method::THREE_POINTS);
          break;
      }
      initialized = true;
    }

    Plane::Plane(Vector point, Vector normal) :
      point(point), normal(normal.normalized()), initialized(true)
    {
    }

    Plane::Plane(void) : point(Vector(NAN, NAN, NAN)), normal(Vector(0,0,0)), initialized(false) {
    }

    void Plane::flipNormal() {
      assert(isInitialized() == true);
      normal *= -1;
    }

    void Plane::pointNormalTowards(Vector point) {
      assert(isInitialized() == true);
      double dist = distance(*this, point, false);
      if (dist < 0) {
        flipNormal();
      }
    }

    Relation relation(Vector vector1, Vector vector2) {
      double s, c;
      s = vector1.dot(vector2);
      c = (vector1.cross(vector2)).dot(Vector(1,1,1));
      if (c <= 3*EPSILON){
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

    /**
    * \brief returns the relation of a line and a point. r_min and r_max give the value range of the line factor, thus the length of the line
    * \return Relation
    */
    Relation relation(Line line, Vector point) {
      Vector vec = point - line.point;
      if (relation(line.direction, vec) == Relation::IDENTICAL_OR_MULTIPLE) {
        double r = line.getFactorForPoint(point);
        if (r<line.r_min) return Relation::SKEW;
        if (r>line.r_max) return Relation::SKEW;
        return Relation::CONTAINING;
      }
      return Relation::SKEW;
    }

    /**
    * \brief calculates the intersection point of two lines if they intersect
    * \return intersection point, otherwise NAN-vector
    */
    Vector intersect(Line line1, Line line2) {
      if (relation(line1, line2) == Relation::INTERSECT) {
        Vector vert = line1.direction.cross(line2.direction);
        Plane pln(line1.point, line1.direction, vert, Plane::Method::POINT_TWO_VECTORS);
        return intersect(pln, line2);
      }
      return Vector(NAN,NAN,NAN);
    }

    /**
    * \brief calculates the intersection point of plane and line if they intersect
    * \return intersection point, otherwise NAN-vector
    */
    Vector intersect(Plane plane, Line line) {
      if (relation(plane, line) == Relation::INTERSECT) {
        double r = (plane.normal.dot(plane.point) - plane.normal.dot(line.point)) /
                              (plane.normal.dot(line.direction));
        return line.getPointOnLine(r);
      }
      return Vector(NAN,NAN,NAN);
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

    /**
    * \brief calculates the distance between plane and point. if absolute is set false it returns the distance positive,
    *  if it's on the side of the plane to which the plane's normals points
    * \return intersection line, otherwise 0-vector line
    */
    double distance(Plane plane, Vector point, bool absolute/*=true*/) {
      Line perpendicular(point, plane.normal,Line::Method::POINT_VECTOR);
      Vector projected = intersect(plane, perpendicular);
      Vector distance = (point - projected);
      assert(relation(distance, plane.normal) == Relation::IDENTICAL_OR_MULTIPLE);
      int sign=1;
      if (absolute == false && distance.dot(plane.normal) < 0) {
        sign=-1;
      }
      return sign*distance.norm();
    }

    /**
    * \brief calculates the distance between plane and point. r limits define the range of the line.
    * \return intersection line, otherwise 0-vector line
    */
    double distance(Line line1, Line line2) {
      Relation rel = relation(line1, line2);
      if (rel == Relation::IDENTICAL_OR_MULTIPLE) {
        return 0.0;
      } else if (rel == Relation::PARALLEL) {
        return (line1.point - line2.point).norm();
      } else if (rel == Relation::INTERSECT || rel == Relation::SKEW) {
        double r1, r2;
        Vector intersection;
        if (rel == Relation::INTERSECT) {
          intersection = intersect(line1, line2);
          r1 = line1.getFactorForPoint(intersection);
          r2 = line2.getFactorForPoint(intersection);
        } else {
          //make plane parallel to line2 containing line1
          Plane pln(line1.point, line1.direction, line2.direction, Plane::Method::POINT_TWO_VECTORS);
          double dist = distance(pln, line2.point, false);// we can't simply take this distance as we have to check the value range
          line1.point += dist * pln.normal;
          assert(relation(line1, line2) == Relation::INTERSECT);
          intersection = intersect(line1, line2); //point where the lines are the closest
          r1 = line1.getFactorForPoint(intersection - dist * pln.normal);
          r2 = line2.getFactorForPoint(intersection);
        }
        r1 = fmin(line1.r_max, fmax(r1, line1.r_min));
        r2 = fmin(line2.r_max, fmax(r2, line2.r_min));
        Relation rel1 = relation(line1, intersection);
        Relation rel2 = relation(line2, intersection);
        if (rel1 == Relation::CONTAINING && rel2 == Relation::CONTAINING) {
          // intersection point in range
          return 0.0;
        }
        //distance between the points closest to the intersection
        return (line1.getPointOnLine(r1) - line2.getPointOnLine(r2)).norm();
      } else {
        assert(false);
      }

    }

  } // end of namespace utils
} // end of namespace mars

