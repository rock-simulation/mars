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
#include "Geometry.hpp"
#include <cmath>

#ifndef DEBUG_CHECKS
#define DEBUG_CHECKS 0
#endif

namespace mars {
  namespace utils {
    double getFactorFromColinear(Vector vector1, Vector vector2) {
      assert(relation(vector1, vector2) == IDENTICAL_OR_MULTIPLE);
      return vector1.norm()/vector2.norm();
    }

    double degToRad(double angle) {
      return M_PI * angle/180.0;
    }

    double radToDeg(double angle) {
      return 180 * angle/M_PI;
    }

    double cleanAngle(double angle) {
      return degToRad((fmod(((radToDeg(angle))+180.0), 360.0)-180.0));
    }

    bool isnan(Quaternion q) {
      bool out=false;
      out |= std::isnan(q.w());
      out |= std::isnan(q.x());
      out |= std::isnan(q.y());
      out |= std::isnan(q.z());
      return out;
    }

    bool isnan(Vector v) {
      bool out=false;
      out |= std::isnan(v.x());
      out |= std::isnan(v.y());
      out |= std::isnan(v.z());
      return out;
    }


    Line::Line(Vector point, Vector vector_type, Method method/*=POINT_VECTOR*/, bool segment_from_vector_length) : point(point) {
      assert(method == POINT_VECTOR || method == POINT_POINT);
      switch (method) {
        case POINT_VECTOR:
          direction = vector_type;
          break;
        case POINT_POINT:
          assert (relation(point, vector_type) != IDENTICAL_OR_MULTIPLE);
          direction = (vector_type - point);
          break;
      }
      if (segment_from_vector_length) {
        r_min = 0;
        r_max = direction.norm();
      }
      direction.normalize();
      initialized = true;
    }

    Line::Line() : point(Vector(NAN, NAN, NAN)), direction(Vector(0,0,0)), initialized(false) {
    }

    Vector Line::getPointOnLine(double r) {
      assert(isInitialized());
      return point + direction * r;
    }

    double Line::getFactorForPoint(Vector point) {
      assert(isInitialized());
      assert(relation(*this, point, /*check segment*/false) == CONTAINING);
      return getFactorFromColinear((point - this->point), this->direction);
    }

    bool Line::isnan() {
      bool out = !initialized;
      out |= std::isnan(point.norm());
      out |= std::isnan(direction.norm());
      return out;
    }


    Plane::Plane(Vector point, Vector vector_type1, Vector vector_type2, Method method/*=THREE_POINTS*/) : point(point) {
      assert(method == POINT_TWO_VECTORS || method == THREE_POINTS);
      switch (method) {
        case POINT_TWO_VECTORS:
          assert(relation(vector_type1,vector_type2) != IDENTICAL_OR_MULTIPLE);
          normal = vector_type1.cross(vector_type2).normalized();
          break;
        case THREE_POINTS:
          assert(relation((vector_type1-point),(vector_type2-point)) != IDENTICAL_OR_MULTIPLE);
          normal = (vector_type1-point).cross(vector_type2-point).normalized();
          break;
      }
      initialized = true;
    }

    Plane::Plane(Vector point, Vector normal) :  point(point), normal(normal), initialized(true) {
      this->normal.normalize();
    }

    Plane::Plane(void) : point(Vector(NAN, NAN, NAN)), normal(Vector(0,0,0)), initialized(false) {
    }

    void Plane::flipNormal() {
      assert(isInitialized());
      normal *= -1;
    }

    void Plane::pointNormalTowards(Vector point) {
      assert(isInitialized());
      double dist = distance(*this, point, false);
      if (dist < 0) {
        flipNormal();
      }
    }

    bool Plane::isnan() {
      bool out = !initialized;
      out |= std::isnan(point.norm());
      out |= std::isnan(normal.norm());
      return out;
    }


    Relation relation(Vector vector1, Vector vector2) {
      assert(!(std::isnan(vector1.norm()) && std::isnan(vector2.norm())));
      assert(!std::isnan(vector1.norm()));
      assert(!std::isnan(vector2.norm()));
      Vector c;
      double s;
      s = vector1.dot(vector2);
      c = vector1.cross(vector2);
      if (fabs(c.x()) <= EPSILON && fabs(c.y()) <= EPSILON && fabs(c.z()) <= EPSILON){
        return IDENTICAL_OR_MULTIPLE;
      } else if (fabs(s) <= EPSILON) {
        return ORTHOGONAL;
      }
      return SKEW;
    }

    Relation relation(Plane plane1, Plane plane2) {
      if (relation(plane1.normal,plane2.normal) == IDENTICAL_OR_MULTIPLE) {
        if (relation(plane1, plane2.point) == CONTAINING) {
          return IDENTICAL_OR_MULTIPLE;
        }
        return PARALLEL;
      }
      return INTERSECT;
    }

    Relation relation(Plane plane, Line line, bool check_segment/*=false*/) {
      if (relation(plane.normal,line.direction) == ORTHOGONAL) {
        if (relation(plane, line.point) == CONTAINING) {
          return CONTAINING;
        }
        return PARALLEL;
      }
      if (check_segment) {
        if (relation(line,intersect(plane, line)) != CONTAINING) {
          return SKEW;
        }
      }
      return INTERSECT;
    }

    Relation relation(Plane plane, Vector point) {
      if (relation(plane.normal, plane.point-point) == ORTHOGONAL) {
        return CONTAINING;
      }
      return SKEW;
    }

    Relation relation(Line line1, Line line2){
      if (relation(line1.direction, line2.direction) == IDENTICAL_OR_MULTIPLE) {
        if (relation(line1, line2.point) == CONTAINING) {
          return IDENTICAL_OR_MULTIPLE;
        }
        return PARALLEL;
      }
      Plane pln(line1.point, line1.direction, line2.direction, Plane::POINT_TWO_VECTORS);
      if (relation(pln, line2.point) == CONTAINING) {
        return INTERSECT;
      }
      return SKEW;
    }

    Relation relation(Line line, Vector point, bool check_segment/*=false*/) {
      if (relation(line.direction, (point - line.point)) == IDENTICAL_OR_MULTIPLE) {
        if (check_segment) {
          double r = line.getFactorForPoint(point);
          if (r<line.r_min) return SKEW;
          if (r>line.r_max) return SKEW;
        }
        return CONTAINING;
      }
      return SKEW;
    }


    Vector intersect(Line line1, Line line2) {
      if (relation(line1, line2) == INTERSECT) {

        Vector perpendicular(line1.direction.cross(line2.direction));
        Plane pln1(line1.point, line1.direction, perpendicular, Plane::POINT_TWO_VECTORS);
        Vector p2 = intersect(pln1, line2);
      #if DEBUG_CHECKS
        //check
        Plane pln2(line2.point, line2.direction, perpendicular, Plane::POINT_TWO_VECTORS);
        Vector p1 = intersect(pln2, line1);
        if(relation(p1, p2) != IDENTICAL_OR_MULTIPLE) {
          fprintf(stderr, "Assertion failed! p1(%f, %f, %f) != p2(%f, %f, %f)\n",
                  p1.x(), p1.y(), p1.z(), p2.x(), p2.y(), p2.z());
          //assert(false);
        }
      #endif
        return p2;
      }
      fprintf(stderr, "Assertion failed! Relation is not INTERSECT: %d\n", relation(line1, line2));
      assert(false);
      return Vector(NAN, NAN, NAN);
    }

    Vector intersect(Plane plane, Line line) {
      if (relation(plane, line, /*check_segment*/false) == INTERSECT) {
        //using the hesse normal form, setting line in plane
        double r = (plane.normal.dot(plane.point - line.point)) /
                              (plane.normal.dot(line.direction));
        return line.getPointOnLine(r);
      }
      fprintf(stderr, "Assertion failed! Relation is not INTERSECT: %d\n", relation(plane, line));
      assert(false);
      return Vector(NAN, NAN, NAN);
    }

    Line intersect(Plane plane1, Plane plane2) {
      if (relation(plane1,plane2) == INTERSECT) {
        //direction of intersection
        Vector direction = plane1.normal.cross(plane2.normal);
        //create a line that is ORTHOGONAL to the intersection and plane1.normal and lies in plane1
        Line dummy(plane1.point, direction.cross(plane1.normal));
        //the intersection point of this line and plane2 is a point on the intersection line of both planes
        Vector point = intersect(plane2, dummy);
        return Line(point, direction);
      }
      fprintf(stderr, "Assertion failed! Relation is not INTERSECT: %d\n", relation(plane1,plane2));
      assert(false);
      return Line();
    }


    double distance(Plane plane, Vector point, bool absolute/*=true*/) {
      Line perpendicular(point, plane.normal, Line::POINT_VECTOR);
      Vector projected = intersect(plane, perpendicular);
      Vector distance = (point - projected);
      assert(relation(distance, plane.normal) == IDENTICAL_OR_MULTIPLE);
      int sign=1;
      if (!absolute && distance.dot(plane.normal) < 0) {
        sign=-1;
      }
      return sign*distance.norm();
    }

    double distance(Line line1, Line line2, bool check_segment/*=false*/) {
      Relation rel = relation(line1, line2);
      switch (rel) {
        case IDENTICAL_OR_MULTIPLE:
          return 0.0;
          break;
        case PARALLEL:
          return distance(line1, line2.point, check_segment);
          break;
        default: //INTERSECT or SKEW
          double r1, r2;
          Vector intersection;
          if (rel == INTERSECT) {
            //we need to do this to check if the intersection is in the  line segment
            intersection = intersect(line1, line2);
            r1 = line1.getFactorForPoint(intersection);
            r2 = line2.getFactorForPoint(intersection);
          } else {
            /*
            //make a plane which is parallel to line2 containing line1
            Plane pln(line1.point, line1.direction, line2.direction, Plane::POINT_TWO_VECTORS);
            double dist = distance(pln, line2.point, false);// we can't simply take this distance as we have to check the value range
            line1.point += dist * pln.normal;
            assert(relation(line1, line2) == INTERSECT);
            intersection = intersect(line1, line2); //point where the lines are the closest
            r1 = line1.getFactorForPoint(intersection);
            r2 = line2.getFactorForPoint(intersection);
            */
            /*
            //make a plane which contains lin1 and the vector that is orthogonal to both lines
            Plane pln(line1.point, line1.direction, line1.direction.cross(line2.direction), Plane::POINT_TWO_VECTORS);
            //get the intersection of line2 and pln
            intersection = intersect(pln, line2);
            r2 = line2.getFactorForPoint(intersection);
            //and the other way around
            pln = Plane(line2.point, line2.direction, line2.direction.cross(line1.direction), Plane::POINT_TWO_VECTORS);
            intersection = intersect(pln, line1);
            r1 = line1.getFactorForPoint(intersection);
            */

            //from condition that shortest connection is orthogonal to both lines. scalar product zero
            double dp1  = line1.direction.dot(line1.point);
            double dd1  = line1.direction.dot(line1.direction);
            double d1p2 = line1.direction.dot(line2.point);
            double dd12 = line1.direction.dot(line2.direction);
            double d2p1 = line2.direction.dot(line1.point);
            double dp2  = line2.direction.dot(line2.point);
            double dd2  = line2.direction.dot(line2.direction);
            r2 = (d2p1+(dd12*(d1p2-dp1)/dd1)-dp2) / (dd2-dd12*dd12/dd1);
            r1 = (d1p2-dp1+r2*dd12)/dd1;
          }
          if (check_segment) {
            r1 = fmin(line1.r_max, fmax(r1, line1.r_min));
            r2 = fmin(line2.r_max, fmax(r2, line2.r_min));
          }
          //distance between the points closest to the intersection
          return distance(line1.getPointOnLine(r1), line2.getPointOnLine(r2));
          break;
      }
      assert(false);
      return -1;
    }

    double distance(Line line, Vector point, bool check_segment/*=false*/) {
      Relation rel = relation(line, point);
      switch (rel) {
        case CONTAINING:
          return 0.0;
          break;
        default: //SKEW
          Plane pln(point, line.direction);
          Vector point2 = intersect(pln, line);
          if (check_segment) {
            point2 = line.getPointOnLine(fmin(line.r_max, fmax(line.getFactorForPoint(point2), line.r_min)));
          }
          return distance(point, point2);
      }
      assert(false);
      return -1;
    }

    double distance(Vector point1, Vector point2) {
      return (point2-point1).norm();
    }

  } // end of namespace utils
} // end of namespace mars

