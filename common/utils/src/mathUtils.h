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

#ifndef MARS_UTILS_MATHUTILS_H
#define MARS_UTILS_MATHUTILS_H

#include "Vector.h"
#include "Quaternion.h"
#include "ConfigData.h"

namespace mars {
  namespace utils {

    const double EPSILON = Eigen::NumTraits<double>::epsilon();

    inline bool isNormalized(const Vector &vec)
    { return (fabs(vec.squaredNorm() - 1.) < EPSILON*EPSILON); }

    inline Vector scaleVectorToLength(const Vector &vec, const double &length)
    { return vec * length / vec.norm(); }

    /**
     * Angle between two vectors
     * \param[in] v Vector to which we want to know the angle
     * \param[out] axis If axis != NULL the axis of rotation will be stored here.
     * \return The angle between \c this and v in radian.
     */
    double angleBetween(const Vector &v1, const Vector &v2, Vector *axis=NULL);

    inline Quaternion angleAxisToQuaternion(double angle, const Vector &axis) {
      return Quaternion(Eigen::AngleAxis<double>(angle, axis));
    }

    Quaternion eulerToQuaternion(const Vector &euler_v);
    inline Quaternion eulerToQuaternion(const sRotation &rot)
    { return eulerToQuaternion(Vector(rot.alpha, rot.beta, rot.gamma)); }

    sRotation quaternionTosRotation(const Quaternion &value);

    Vector lerp(const Vector &from, const Vector &to, double t);
    Vector slerp(const Vector &from, const Vector &to, double t);

    inline double distanceSquaredBetween(const Vector &v1, const Vector &v2)
    { return (v2.array() - v1.array()).square().sum(); }
    inline double distanceBetween(const Vector &v1, const Vector &v2)
    { return sqrt(distanceSquaredBetween(v1, v2)); }

    void vectorToSpherical(const Vector &v, double *r, double *theta, double *phi);

    Vector getProjection(const Vector &v1, const Vector &v2);

    Vector vectorFromSpherical(double r, double theta, double phi);

    bool vectorFromConfigItem(ConfigItem *item, Vector *v);
    void vectorToConfigItem(ConfigItem *item, Vector *v);

    bool quaternionFromConfigItem(ConfigItem *item, Quaternion *q);
    void quaternionToConfigItem(ConfigItem *item, Quaternion *q);

  }; // end of namespace utils
}; // end of namespace mars

#endif /* MARS_UTILS_MATHUTILS_H */
