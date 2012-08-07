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

#ifndef MARS_INTERFACES_MARSDEFS_H
#define MARS_INTERFACES_MARSDEFS_H

#ifdef _PRINT_HEADER_
  #warning "MARSDefs.h"
#endif

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
  #define CPP_DEPRECATED  __attribute__((__deprecated__))
#else
  #define CPP_DEPRECATED
#endif /* __GNUC__ */

#define CPP_UNUSED(a) (void)a

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif

#define INVALID_ID    0

namespace mars {
  namespace interfaces {

    typedef unsigned long NodeId;
    typedef unsigned long JointId;
    typedef unsigned long MotorId;

    typedef double sReal;
    typedef sReal mydVector3[4];
    typedef sReal mydVector2[4];

    const sReal MARS_PI  = 3.1415926535897932384626433832795;
    const sReal MARS_PI2 = 6.283185307179586476925286766559;

    const sReal DTR =  0.017453292519943295769236907685;     // Radian To Degree
    const sReal RTD = 57.295779513082320876798154814105;     // Degree To Radian

    // Definition of the node types
    enum NodeType {
      NODE_TYPE_UNDEFINED=0,
      NODE_TYPE_MESH,
      NODE_TYPE_BOX,
      NODE_TYPE_SPHERE,
      NODE_TYPE_CAPSULE,
      NODE_TYPE_CYLINDER,
      NODE_TYPE_PLANE,
      NODE_TYPE_TERRAIN,
      NODE_TYPE_REFERENCE,
      NUMBER_OF_NODE_TYPES
    };

    // Definition of Joint Types
    enum JointType {
      JOINT_TYPE_UNDEFINED=0,
      JOINT_TYPE_HINGE,
      JOINT_TYPE_HINGE2,
      JOINT_TYPE_SLIDER,
      JOINT_TYPE_BALL,
      JOINT_TYPE_UNIVERSAL,
      JOINT_TYPE_FIXED,
      JOINT_TYPE_ISTRUCT_SPINE,
      NUMBER_OF_JOINT_TYPES
    };

    // Definition of Motor Types
    enum MotorType {
      MOTOR_TYPE_UNDEFINED=0,
      MOTOR_TYPE_PID,
      MOTOR_TYPE_DC,
      MOTOR_TYPE_DX117,
      MOTOR_TYPE_DC_MODEL,
      MOTOR_TYPE_PID_MODEL,
      MOTOR_TYPE_PID_FORCE,
    };

    //Definition of Sensor Types
    enum SensorType {
      SENSOR_TYPE_UNDEFINED=0,
      SENSOR_TYPE_TORQUE_AVG,
      SENSOR_TYPE_LOAD,
      SENSOR_TYPE_POSITION,
      SENSOR_TYPE_ROTATION,
      SENSOR_TYPE_GROUND_CONTACT,
      SENSOR_TYPE_GROUND_CONTACT_FORCE,
      SENSOR_TYPE_GROUND_CENTER_OF_GRAVITY,
      SENSOR_TYPE_JOINT_POSITION,
      SENSOR_TYPE_CAMERA,
      SENSOR_TYPE_VELOCITY,
      SENSOR_TYPE_RAY,
      SENSOR_TYPE_RAY_GRID,
      SENSOR_TYPE_ANGULAR_VELOCITY,
      SENSOR_TYPE_MOTOR_CURRENT,  // motors
      SENSOR_TYPE_6DOF,
      SENSOR_TYPE_TORQUE,
      SENSOR_TYPE_MOTOR_POSITION, // motors
      SENSOR_TYPE_JOINT_VELOCITY,
      SENSOR_TYPE_RAY2D,
    };

    // Definition of light types
    enum LightType {
      OMNILIGHT=1,
      SPOTLIGHT,
    };

    //Definition of material faces
    enum MaterialFaceType {
      FACE_NONE=0,
      FACE_FRONT,
      FACE_BACK,
      FACE_BOTH,
    };



    /* Nope not used definitly any more
    //Definition of primitive types
    enum PrimitiveType {
    PRIMITIVE_BOX = 1,
    PRIMITIVE_SPHERE,
    PRIMITIVE_CYLINDER,
    PRIMITIVE_CAPSULE,
    PRIMITIVE_PLANE,
    PRIMITIVE_HEIGHT,
    };
    */

    enum Command {
      COMMAND_NODE_POSITION = 1,
      COMMAND_NODE_ROTATION,
      COMMAND_NODE_APPLY_FORCE,
      COMMAND_NODE_APPLY_FORCE_AT,
      COMMAND_NODE_CONTACT_MOTION1,
      COMMAND_PARAM_ADD,
      COMMAND_MOTOR_POSITION,
      COMMAND_NODE_RELOAD_EXTENT,
      COMMAND_NODE_RELOAD_POSITION,
      COMMAND_NODE_RELOAD_ANGLE,
      COMMAND_JOINT_RELOAD_OFFSET,
      COMMAND_JOINT_RELOAD_AXIS,
      COMMAND_PHYSICS_PARAMETER,
      COMMAND_SIM_QUIT,
      COMMAND_CONTACT_NODE_PARAMETER,
      COMMAND_NODE_VELOCITY,
      COMMAND_NODES_CONNECT,
      COMMAND_NODES_DISCONNECT,
      COMMAND_MOTOR_PID,
      COMMAND_NODE_ANGULAR_VELOCITY,
      COMMAND_NODE_RELOAD_FRICTION,
      COMMAND_NODE_ANGULAR_DAMPING,
      COMMAND_NODE_RELOAD_QUATERNION,
      COMMAND_JOINT_RELOAD_ANCHOR,
      COMMAND_PHYSICS_GRAVITY,
      COMMAND_MOTOR_MAX_VELOCITY,
    };

    enum ParameterType {
      WS_HSLIDER=1, //horizontal slider
      WS_VSLIDER,   //vertical slider
      WS_EDIT,      //lineedit
      WS_CBUTTON,   //colored button
    };

    enum PluginMode {
      PLUGIN_NO_MODE=0,
      PLUGIN_SIM_MODE,
      PLUGIN_GUI_MODE,
    };

    /* the 16. bit is reserved for sensors */
    const unsigned int COLLIDE_MASK_SENSOR = 32768;

    enum IDMapType {
      MAP_TYPE_UNDEFINED=0,
      MAP_TYPE_NODE,
      MAP_TYPE_JOINT,
      MAP_TYPE_MOTOR,
      MAP_TYPE_SENSOR,
      MAP_TYPE_CONTROLLER
    };

    enum EditNodeMask {
      EDIT_NODE_POS      =    1,
      EDIT_NODE_ROT      =    2,
      EDIT_NODE_SIZE     =    4,
      EDIT_NODE_TYPE     =    8,
      EDIT_NODE_MASS     =   16,
      EDIT_NODE_NAME     =   32,
      EDIT_NODE_MOVABLE  =   64,
      EDIT_NODE_MOVE_ALL =  128,
      EDIT_NODE_MATERIAL =  256,
      EDIT_NODE_CONTACT  =  512,
      EDIT_NODE_GROUP    = 1024,
      EDIT_NODE_PHYSICS  = 2048,
    };

    enum PreviewType {
      PREVIEW_CREATE = 1,
      PREVIEW_MOVE,
      PREVIEW_CLOSE,
      PREVIEW_EDIT,
      PREVIEW_COLOR,
    };

    enum OpenMode {
      OPEN_INITIAL = 1,
      OPEN_ACTUAL,
      OPEN_RELATIVE,
    };

    enum AnchorMode {
      ANCHOR_NODE1 = 1,
      ANCHOR_NODE2,
      ANCHOR_CENTER,
      ANCHOR_CUSTOM,
    };

    enum ShaderType {
      SHADER_TYPE_FRAGMENT,
      SHADER_TYPE_GEOMETRY,
      SHADER_TYPE_VERTEX,
      SHADER_TYPE_FFP
    };

  } // end of namespace interfaces
} // end of namespace mars

#endif // MARS_INTERFACES_MARSDEFS_H
