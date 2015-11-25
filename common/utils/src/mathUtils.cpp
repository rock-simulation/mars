/*
 *  Copyright 2012. 2014, DFKI GmbH Robotics Innovation Center
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

#include "mathUtils.h"
#include "misc.h"

#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <Eigen/Core>
//#include <stdio.h>

namespace mars {
  namespace utils {

    ApproximationFunction getApproximationFunctionFromString(std::string s) {
      if(s == "pipe") return FUNCTION_PIPE;
      if(s == "polynom3") return FUNCTION_POLYNOM3;
      if(s == "polynom5") return FUNCTION_POLYNOM5;
      if(s == "gaussian") return FUNCTION_GAUSSIAN;
      return FUNCTION_UNKNOWN;
    }

    double pipe(double* x, std::vector<double>* c) {
      return (*x);
    }

    double polynom5(double* x, std::vector<double>* c) {
      return (*c)[0]*pow((*x),5) + (*c)[1]*pow((*x),4) + (*c)[2]*pow((*x),3)
        + (*c)[3]*pow((*x),2) + (*c)[4]*((*x)) + (*c)[5];
    }

    double polynom3(double* x, std::vector<double>* c) {
      return (*c)[0]*pow((*x),3) + (*c)[1]*pow((*x),2) + (*c)[2]*((*x)) + (*c)[3];
    }

    /* first parameter is mu, second is sigma
    */
    double gaussian(double* x, std::vector<double>* c) {
      return 1/(SQRT2PI*(*c)[1])*exp(-0.5*pow(((*x)-(*c)[0])/(*c)[1], 2));
    }

    /*double beta_distribution(double* x, std::vector<double>* c) {

    }

    double gamma_distribution(double* x, std::vector<double>* c) {

    }*/

    static const int EULER_AXIS_1 = 2;
    static const int EULER_AXIS_2 = 0;
    static const int EULER_AXIS_3 = 1;

    double getYaw(const Quaternion &q){
        Eigen::Vector3d v = q * Eigen::Vector3d::UnitX();
        return ::atan2(v[1],v[0]);
    }

    double angleBetween(const Vector &v1, const Vector &v2, Vector *axis) {
      double squared_length1 = v1.squaredNorm();
      double squared_length2 = v2.squaredNorm();
      double tmp = sqrt(squared_length1 * squared_length2);
      double angle = acos(v1.dot(v2) / tmp);
      if (axis != NULL) {
        // special case 0 and 180 degrees where the axis is ambiguous
        // Note that acos returns a value in [0, M_PI] so angle is in that range
        if ((angle <= EPSILON) || (M_PI - angle <= EPSILON)) {
          // find an axis that isn't zero and switch it with another one 
          // make sure to keep the length intact
          Vector dummy;
          if (v1(0) > EPSILON) {
            dummy = Vector(v1.y(), -v1.x(), v1.z());
          } else {
            dummy = Vector(v1.x(), v1.z(), -v1.y());
          }
          *axis = dummy.cross(v2) / tmp;
        } else {
          *axis = v1.cross(v2) / tmp;
        }
      }
      return angle;
    }


    Quaternion eulerToQuaternion(const Vector &euler_v) {
#if 1 
      double _heading  = degToRad(euler_v[2]);
      double _attitude = degToRad(euler_v[1]);
      double _bank     = degToRad(euler_v[0]);
      double c1 = cos(_heading/2);
      double s1 = sin(_heading/2);
      double c2 = cos(_attitude/2);
      double s2 = sin(_attitude/2);
      double c3 = cos(_bank/2);
      double s3 = sin(_bank/2);
      double c1c2 = c1*c2;
      double s1s2 = s1*s2;
      Quaternion q;
      q.w() = (double) (c1c2*c3 + s1s2*s3);
      q.x() = (double) (c1c2*s3 - s1s2*c3);
      q.y() = (double) (c1*s2*c3 + s1*c2*s3);
      q.z() = (double) (s1*c2*c3 - c1*s2*s3);
      return q.normalized();
#else
      return
        Eigen::AngleAxisd(euler_v.x()/180.0*M_PI,
                          Eigen::Vector3d::Unit(EULER_AXIS_1)) *
        Eigen::AngleAxisd(euler_v.z()/180.0*M_PI,
                          Eigen::Vector3d::Unit(EULER_AXIS_2)) *
        Eigen::AngleAxisd(euler_v.y()/180.0*M_PI,
                          Eigen::Vector3d::Unit(EULER_AXIS_3));
#endif
    }

    sRotation quaternionTosRotation(const Quaternion &value) {
#if 1 
      sRotation euler;
      double sqw = value.w()*value.w();
      double sqx = value.x()*value.x();
      double sqy = value.y()*value.y();
      double sqz = value.z()*value.z();
      // heading
      euler.gamma = radToDeg(atan2(2.*(value.x()*value.y()+value.z()*value.w()),
                                   (sqx - sqy - sqz + sqw)));
      // bank
      euler.alpha = radToDeg(atan2(2.*(value.y()*value.z()+value.x()*value.w()),
                                   (-sqx - sqy + sqz + sqw)));
      // attitude
      double test = -2. * (value.x()*value.z() - value.y()*value.w());
      test = (test > 1. ? 1. : (test < -1. ? -1. : test));
      euler.beta  = radToDeg(asin(test));
      return euler;
#else
      Vector yaw_roll_pitch = value.toRotationMatrix().eulerAngles(EULER_AXIS_1,
                                                                   EULER_AXIS_2,
                                                                   EULER_AXIS_3);
      sRotation rot;
      rot.alpha = radToDeg(yaw_roll_pitch.x());
      rot.beta = radToDeg(yaw_roll_pitch.y());
      rot.gamma = radToDeg(yaw_roll_pitch.z());

      return rot; 
#endif
    }


    Vector slerp(const Vector &from, const Vector &to, double t) {
      if (fabs(t) > 1) {
        throw std::runtime_error("Argument t to Vector::slerp must be in range [-1, 1].");
      }
    
      double length1 = from.norm();
      double length2 = to.norm();
      double tmp = from.dot(to) / (length1 * length2);
      /* make sure tmp is in the range [-1:1] so acos won't return NaN */
      tmp = (tmp < -1 ? -1 : (tmp > 1 ? 1: tmp));
      double angle = acos(tmp);

      /* if t < 0 we take the long arch of the greate circle to the destiny */
      if (t < 0) {
        angle -= 2 * M_PI;
        t = -t;
      }
      if (from.x() * to.y() < from.y() * to.x())
        angle *= -1;

      Vector ret;
      /* special case angle==0 and angle==360 */
      if ((fabs(angle) <= EPSILON) || 
          (fabs(fabs(angle) - 2 * M_PI) <= EPSILON)) {
        /* approximate with lerp, because slerp diverges with 1/sin(angle) */
        ret = lerp(from, to, t);
      }
      /* special case angle==180 and angle==-180 */
      else if (fabs(fabs(angle) - M_PI) <= EPSILON) {
        throw std::runtime_error("SLERP with 180 degrees is undefined.");
      }
      else {
        double f0 = ((length2 - length1) * t + length1) / ::sin(angle);
        double f1 = ::sin(angle * (1-t)) / length1;
        double f2 = ::sin(angle * t) / length2;
        ret = ((from.array() * f1 + to.array() * f2) * f0).matrix();
      }
      return ret;
    }

    Vector lerp(const Vector &from, const Vector &to, double t) {
      if (t < 0 || t > 1) {
        throw std::runtime_error("Argument t to Vector::lerp must be in range [0, 1]");
      }
      return (from.array() * (1-t) + to.array() * t).matrix();
    }

    void vectorToSpherical(const Vector &v, double *r, double *theta, double *phi) {
      *r = v.norm();
      if (fabs(*r) < EPSILON) {
        *theta = 0;
        *phi = 0;
        return;
      }
      *theta = acos(v.z() / (*r));
      *phi = atan2(v.y(), v.x());
    }

    Vector getProjection(const Vector &v1, const Vector &v2) {
      double a = v1.dot(v2);
      double b = v2.dot(v2);
      Vector ret(v2);
      ret *= a/b;
      return ret;
    }

    Vector vectorFromSpherical(double r, double theta, double phi) {
      return Vector(r * sin(theta) * cos(phi),
                    r * sin(theta) * sin(phi),
                    r * cos(theta));
    }

    bool vectorFromConfigItem(ConfigItem *item, Vector *v) {
      v->x() = (*item)["x"];
      v->y() = (*item)["y"];
      v->z() = (*item)["z"];
      return true;
    }

    void vectorToConfigItem(ConfigItem *item, Vector *v) {
      (*item)["x"] = v->x();
      (*item)["y"] = v->y();
      (*item)["z"] = v->z();
    }

    bool quaternionFromConfigItem(ConfigItem *item, Quaternion *q) {
      q->w() = (*item)["w"];
      q->x() = (*item)["x"];
      q->y() = (*item)["y"];
      q->z() = (*item)["z"];
      return true;
    }

    void quaternionToConfigItem(ConfigItem *item, Quaternion *q) {
      (*item)["w"] = q->w();
      (*item)["x"] = q->x();
      (*item)["y"] = q->y();
      (*item)["z"] = q->z();
    }

    void inertiaTensorToConfigItem(ConfigItem *item, double *inertia) {
      (*item)["i00"] = inertia[0];
      (*item)["i01"] = inertia[1];
      (*item)["i02"] = inertia[2];
      (*item)["i10"] = inertia[3];
      (*item)["i11"] = inertia[4];
      (*item)["i12"] = inertia[5];
      (*item)["i20"] = inertia[6];
      (*item)["i21"] = inertia[7];
      (*item)["i22"] = inertia[8];
    }

    double random_number(double min, double max, int digits) {
      int range = pow(10, digits);
      int r = rand() % range;
      return r / (double)range * (max-min) + min;
    }

    double random_normal_number(double mean, double std, double low, double high) {
      // C++11 - does not work on all compilers, yet
      // std::default_random_engine generator;
      // std::normal_real_distribution<double> normal_dis(mean, std);
      //return normal_dis(generator);

      // return a normally distributed random number, restricted to range [min, max]
      double u1=random_number(0, 1, 3);
      double u2=random_number(0, 1, 3);
      double num = cos(2*M_PI*u2)*sqrt(-2.*log(u1)) * std + mean;
      //fprintf(stderr, "normal num: %f, %f, %f\n", u1, u2, num);
      return std::min(std::max(num, low), high);
    }



  } // end of namespace utils
} /* end of namespace mars */
