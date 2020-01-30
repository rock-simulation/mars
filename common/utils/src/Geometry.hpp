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
    /** If the two vectors are colinear this function returns the quotient of the vector's lengths:
    * \f$ f = \frac{\left|\vec{v_1}\right|}{\left|\vec{v_2}\right|} \f$
    * \return the factor \f$ f \f$
    */
    double getFactorFromColinear(Vector vector1, Vector vector2);
    double degToRad(double angle);
    double radToDeg(double angle);

    /** \brief some thing like modulo 2*pi.
    * If the angle is bigger than 2 pi. The angle is reduced respectively.
    * \f$ f(\alpha_{rad}) = (((\alpha_{deg} + 180) \mod 360) - 180)_{rad} \f$
    * \param angle: the angle to clean in rad
    * \return \f$ f(\alpha_{rad})\f$ in rad
    */
    double cleanAngle(double angle);

    bool isnan(Quaternion q);
    bool isnan(Vector v);

    struct Line;
    struct Plane;

    enum Relation {
      ///0
      PARALLEL,
      ///1
      INTERSECT,
      ///2
      SKEW,
      ///3
      CONTAINING,
      ///4
      IDENTICAL_OR_MULTIPLE,
      ///5
      ORTHOGONAL
    };

    /** Returns the relation between two planes
    * \param plane1: a plane
    * \param plane2: another plane
    * \return PARALLEL | INTERSECT | IDENTICAL_OR_MULTIPLE
    */
    Relation relation(Plane plane1, Plane plane2);

    /** Returns the relation between a plane and a line/line segments
    * \param plane: a plane
    * \param line: a line
    * \param check_segment: (default false) if true, the line segement is used
    * \return PARALLEL | INTERSECT | CONTAINING | SKEW (only for segment)
    */
    Relation relation(Plane plane, Line line, bool check_segment=false);

    /** Returns the relation between a plane and a point
    * \param plane: a plane
    * \param point: a point
    * \return CONTAINING | SKEW
    */
    Relation relation(Plane plane1, Vector point);

    /** Returns the relation between two lines
    * \param line1: a line
    * \param line2: another line
    * \return PARALLEL | INTERSECT | IDENTICAL_OR_MULTIPLE | SKEW
    */
    Relation relation(Line line1, Line line2);

    /** Returns the relation between line and point
    * \param line: a line
    * \param point: a point
    * \param check_segment: (default false) if true, the line segement is used
    * \return CONTAINING | SKEW
    */
    Relation relation(Line line, Vector point, bool check_segment=false);

    /** Returns the relation between two vectors
    * \param vector1: a vector
    * \param vector2: another vector
    * \return IDENTICAL_OR_MULTIPLE | ORTHOGONAL | SKEW
    */
    Relation relation(Vector vector1, Vector vector2);

    /** Calculates the intersection point of two lines, if there is one. Otherwise fails assertion/returns NAN-vector
    * \param line1: a line
    * \param line2: another line
    * \return their intersection point or NAN-Vector
    */
    Vector intersect(Line line1, Line line2);

    /** Calculates the intersection point of a plane and line, if there is one. Otherwise fails assertion/returns NAN-vector
    * \param plane: a plane
    * \param line: a line
    * \return their intersection point or NAN-Vector
    */
    Vector intersect(Plane plane, Line line);

    /** Calculates the intersection line of a plane and line, if there is one. Otherwise fails assertion/returns NAN-line
    * \param plane1: a plane
    * \param plane2: another plane
    * \return their intersection line or NAN-line
    */
    Line intersect(Plane plane1, Plane plane2);

    /** \brief Get the distance between a plane and a point.
    * Calculates the distance between the given Plane and the given point.
    * \param plane: the plane
    * \param point: the point
    * \param absolute: (default true) if false, the distance is positive if the plane's normal points towards the point and vice versa
    * \return the distance
    */
    double distance(Plane plane, Vector point, bool absolute=true);

    /** \brief Get the distance between two lines.
    * Calculates the distance between the given lines. Can also calculate the distance of line segments
    * \param line1: a line
    * \param line2: another line
    * \param check_segement: (default false) if true, the distance between line segements is calulated
    * \return the distance
    */
    double distance(Line line1, Line line2, bool check_segment=false);
    /** \brief Get the distance between a line and a point.
    * Calculates the distance between given line and given point. Can also calculate the distance of the line segment to the point
    * \param line: a line
    * \param point: a point
    * \param check_segement: (default false) if true, the distance between the line segement and the point is calulated
    * \return the distance
    */
    double distance(Line line, Vector point, bool check_segment=false);

    /** \brief Get the distance between two points.
    * Calculates the distance between the given lines.
    * \param point1: a point
    * \param point2: another point
    * \return the distance
    */
    double distance(Vector point1, Vector point2);

    struct Line {
      Vector point;
      Vector direction;
      /** \brief Used to define line segements*/
      double r_min=-INFINITY;
      /** \brief Used to define line segements*/
      double r_max=INFINITY;

      bool initialized;
      bool isInitialized() {return !(this->isnan());}

      enum Method{
        ///0
        POINT_VECTOR,
        ///1
        POINT_POINT
      };

      /** \brief Line constructor.
      * Creates a line using the given Method. With the given point as start point. The direction will be normalized.
      * \param point: support point of the line
      * \param vector_type: either the direction or another point depending on the method
      * \param method: POINT_VECTOR | POINT_POINT
      * \param segment_from_vector_length: (default false) whether this will become a line segment
      */
      Line(Vector point, Vector vector_type, Method method=POINT_VECTOR, bool segment_from_vector_length=false);

      /** \brief Dummy line constructor.
      * Creates an uninitialized line. This instance can't be used and needs to be overwritten.
      */
      Line(void);

      Vector getPointOnLine(double r);
      double getFactorForPoint(Vector point);

      /** \brief Get length of the line segment
      * \return the length of the line segement, if it's a regular line: inf
      */
      double getLength() {return r_max - r_min;}

      /** \brief Get vector representation of line segement
      * Returns a vector that points along the positive direction of the line, with the length of the line segement.
      * Use with caution: if the line is not a line segment the Vector is in at least one direction infinite.
      * \return the length of the line segement, if it's a regular line: inf
      */
      Vector getVector() {return (r_max-r_min)*direction;}
      bool isnan();
    };

    struct Plane {
      Vector point;
      Vector normal;

      bool initialized;
      bool isInitialized() {return !(this->isnan());}

      enum Method {
        ///0
        THREE_POINTS,
        ///1
        POINT_TWO_VECTORS
      };

      /** \brief Plane constructor.
      * Creates a plane that contains both the given point and the given line.
      * \param point: support point of the plane
      * \param line: line to be in the plane
      */
      Plane(Vector point, Line line);

      /** \brief Plane constructor.
      * Creates a plane using the given Method. With the given point as support point. The normal is normalized.
      * \param point: support point of the line
      * \param vector_type1: either a second point or a first vector
      * \param vector_type2: either a third point or a second vector
      * \param method: THREE_POINTS | POINT_TWO_VECTORS
      */
      Plane(Vector point, Vector vector_type1, Vector vector_type2, Method method=THREE_POINTS);

      /** \brief Plane constructor.
      * Creates a plane.
      * \param point: support point of the plane
      * \param normal: normal of the plane (will be normalized)
      */
      Plane(Vector point, Vector normal);

      /** \brief Dummy plane constructor.
      * Creates an uninitialized plane. This instance can't be used and needs to be overwritten.
      */
      Plane(void);

      /** \brief Changes the direction of the normal.*/
      void flipNormal();

      /** \brief Points normal towards given point
      * Checks on which side of the plane the point is located. If the normal points to the other side the normal is flipped.
      */
      void pointNormalTowards(Vector point);
      bool isnan();
    };
  } // end of namespace utils
} // end of namespace mars

#endif /* MARS_UTILS_VECTOR_H */

