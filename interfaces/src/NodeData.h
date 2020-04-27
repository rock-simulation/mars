/*
 *  Copyright 2011, 2012, 2014, DFKI GmbH Robotics Innovation Center
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

#ifndef MARS_CORE_NODE_DATA_H
#define MARS_CORE_NODE_DATA_H

#include "snmesh.h"
#include "MaterialData.h"
#include "contact_params.h"
#include "MARSDefs.h"

#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>

#include <string>
#include <map>


namespace mars {
  namespace interfaces {

    // forward declaration
    class LoadCenter;
    struct terrainStruct;

    enum SimNodeType 
    {
      COLLISION = 0,
      INERTIA = 1,
      FRAME = 2,    // DUMMY SIMNODE 
      VISUAL = 3,
      NONE = 4
    };

    /**
     * NodeData is a struct to exchange node information between the GUI
     * and the simulation
     *
     * \b Example:
     * \n The following example code defines a box with a size of 1x1x1 meter,
     * a density of 1 (default) and a mass of 0.5,
     * and a position at (0.0, 0.0, 1.0).
     */
    struct NodeData {

      /**
       * initialization is not done in the constructor directly to keep backwards
       * compatibility with the NEW_NODE_STRUCT macro.
       */
      void init(const std::string& name = std::string(),
                const utils::Vector& position = utils::Vector::Zero(),
                const utils::Quaternion& orientation = utils::Quaternion::Identity() )
      {
        groupID = 0;
        index = 0;
        filename = "PRIMITIVE";
        physicMode = NODE_TYPE_UNDEFINED;
        pos.setZero();
        pivot.setZero();
        rot.setIdentity();
        movable=false;
        noPhysical=false;
        density=1;
        mass=0;
        ext.setZero();
        mesh.setZero();
        relative_id=0;
        terrain=0;
        visual_offset_pos.setZero();
        visual_offset_rot.setIdentity();
        visual_size.setZero();
        visual_scale = utils::Vector(1.0, 1.0, 1.0);
        c_params.setZero();
        inertia_set=false;
        for(int i=0;i<3;i++){
          for(int j=0;j<3;j++){
            inertia[i][j] = 0;
          }}
        linear_damping = 0;
        angular_damping = 0;
        angular_treshold = 0;
        angular_low  = 0;
        shadow_id = 0;
        isShadowCaster = true;
        isShadowReceiver = true;
        this->name = name;
        pos = position;
        rot = orientation;
        graphicsID1 = graphicsID2 = 0;

        frameID = "";

        simNodeType = SimNodeType::NONE;
      }

      /**
       * @brief default constructor that takes name, position and orientation arguments
       */
      explicit NodeData(const std::string& name = std::string(),
                        const utils::Vector& position = utils::Vector::Zero(),
                        const utils::Quaternion& orientation = utils::Quaternion::Identity())
      {
        init( name, position, orientation );
      }

      static const char* toString(const NodeType &type);

      static NodeType typeFromString(const std::string& str);

      /**
       * @brief initialize the nodestruct for a primitive type
       *
       * only updates the fields relevant to the primitive type, and leaves the rest as it is.
       *
       * @param type the type of node (e.g. box, sphere, etc)
       * @param extents extents of the object
       * @param mass the mass of the object
       */
      void initPrimitive( NodeType type, const utils::Vector& extents, sReal mass )
      {
        physicMode = type;
        ext = extents;
        this->mass = mass;
        filename = "PRIMITIVE";

        origName = toString(type);
      }

      bool fromConfigMap(configmaps::ConfigMap *config, std::string filenamePrefix,
                         LoadCenter *loadCenter = 0);
      void toConfigMap(configmaps::ConfigMap *config,
                       bool skipFilenamePrefix = false,
                       bool exportDefault = false);
      void getFilesToSave(std::vector<std::string> *fileList);

      SimNodeType simNodeType;

      /**
       * The name of the node. \verbatim Default value: "" \endverbatim
       */
      std::string name;

      std::string frameID;

      /** The original object name; needed for importing objects from files (like
       * wavefront object files). \n
       * This string stores also the type of a primitive if the node don't have
       * have a file to load the visual representation from.
       * \verbatim Default value: "" \endverbatim
       */
      std::string origName;

      /**
       * The filename of the node to load the visual representation from.
       * If no file is used the filename have to be set to \c PRIMITIVE and
       * the origName have to be a number indicating the type of primitive to
       * visualize. \verbatim Default value: "PRIMITIVE" \endverbatim
       */
      std::string filename;

      /**
       * Several nodes can be grouped by setting the same \c groupID. If the node
       * is not in a group the id have to be 0.
       * \verbatim Default value: 0 \endverbatim
       */
      int groupID;

      /**
       * The unique index of the node. This value is set by the simulation while
       * adding a new node. \verbatim Default value: 0 \endverbatim
       */
      unsigned long index;

      /**
       * This value holds the physical representation of a node. Valid values are:
       * - NODE_TYPE_MESH
       * - NODE_TYPE_BOX
       * - NODE_TYPE_SPHERE
       * - NODE_TYPE_CAPSULE
       * - NODE_TYPE_CYLINDER
       * - NODE_TYPE_PLANE
       * - NODE_TYPE_TERRAIN
       * .
       * \verbatim Default value: NODE_TYPE_UNDEFINED \endverbatim
       */
      NodeType physicMode;

      /**
       * The position of the node.
       * \verbatim Default value: (0.0, 0.0, 0.0) \endverbatim
       */
      utils::Vector pos;

      /**
       * The pivot point is set automatically by importing objects.
       * \verbatim Default value: (0.0, 0.0, 0.0) \endverbatim
       */
      utils::Vector pivot;

      /**
       * This quaternion describes the orientation of the node.
       * \verbatim Default value: (0.0, 0.0, 0.0, 1.0) \endverbatim
       * \sa Quaternion
       */
      utils::Quaternion rot;

      /**
       * This boolean variable defines if the physical representation is fixed in
       * the world or if it's a movable body.
       * \verbatim Default value: false \endverbatim
       */
      bool movable;

      /**
       * This boolean variable defines if the node have a physical representation
       * or if it's only part of the visualisation. If the value is \c true
       * no physical representation will be created for the node.
       * \verbatim Default value: false \endverbatim
       */
      bool noPhysical;

      /**
       * Physical density of the node. \verbatim Default value: 1.0 \endverbatim
       */
      sReal density;

      /**
       * Physical mass of the node. This value is ignored as long as the above
       * density is not 0.0. \verbatim Default value: 0.0 \endverbatim
       */
      sReal mass;

      /**
       * The size of the node. The ext vector is interpreted depending of the
       * physicMode of the node:
       * - mesh: ext defines the bounding box of the mesh
       * - box: ext defines the size in x, y, and z
       * - sphere: ext.x defines the radius
       * - capsule: ext.x defines the radius and ext.y the length
       * - cylinder: ext.x defines the radius and ext.y the length
       * - plane: the plane is always infinite in x and y (ext is not used)
       * - terrain: ext is not used, the size of the terrain is defined in the
       * terrainStruct
       * .
       * \verbatim Default value: (0.0, 0.0, 0.0) \endverbatim
       */
      utils::Vector ext;

      /**
       * The mesh struct stores the poly information for a physical representation
       * of a mesh. This struct is automatically derived from the visual node
       *  by the simulation if the physicMode is set to NODE_TYPE_MESH.
       * \verbatim Default value: see snmesh \endverbatim \sa snmesh
       */
      snmesh mesh;

      /**
       * The material struct defines the visual material of the node.
       * \verbatim Default value: see maerialStruct \endverbatim
       * \sa MaterialData
       */
      MaterialData material;

      /**
       * Allows to explicitly set shader stages.
       * Shaders are programs running on the GPU, written in GLSL.
       * OpenGL uses a rendering pipeline, some pipeline stages are
       * programmable.
       * OpenGL<3.0 provides a fixed function pipeline with some bloated,
       * all containing, shader stages. With this list you have
       * some influence on the programmable part of the pipeline.
       *
       * If the map is empty, all shaders will be generated.
       * If there is a item with ShaderType=SHADER_TYPE_FFP in the
       * map, fixed function pipeline will be used.
       * Else each item in the list defines a shader stage.
       *
       * \verbatim Default value: empty list \endverbatim
       */
      std::map<ShaderType, std::string> shaderSources;

      /**
       * Currently the position of a node can be set relative to the position
       * of the node with the id equal to the value of relative_id. If the node
       * has its own position the value of relative_id have to be 0.
       * \verbatim Default value: 0 \endverbatim
       */
      unsigned long relative_id;



      /**
       * If the physicMode is set to NODE_TYPE_TERRAIN, this pointer stores
       * all information to create a physical representation of a height map.
       * \verbatim Default value: 0 \endverbatim \sa terrainStruct
       */
      terrainStruct *terrain;

      /**
       * The visual representation of a node can have a different position as the
       * physical representation. This vector defines a relative direction and
       * distance to the physical position of the node.
       * \verbatim Default value: (0.0, 0.0, 0.0) \endverbatim
       */
      utils::Vector visual_offset_pos;

      /**
       * In the same way as the visual_offset_pos, this quaternion can be used
       * to differentiate the orientation of the visual representation from the physical
       * one. \verbatim Default value: (0.0, 0.0, 0.0, 1.0) \endverbatim
       * \sa NodeData::visual_offset_pos
       */
      utils::Quaternion visual_offset_rot;

      /**
       * This vector is used to define the size of the bounding box of the
       * visual representation of a node. The visual object is scaled to fit
       * into it's bounding box and can be set independently of the physical
       * extent. If the vector is zero the original size of the loaded mesh is
       * used. \verbatim Default value: (0.0, 0.0, 0.0) \endverbatim
       */
      utils::Vector visual_size;

      /**
       * This vector is used to scale the bounding box of the
       * visual representation of a node.
       * \verbatim Default value: (1.0, 1.0, 1.0) \endverbatim
       */
      utils::Vector visual_scale;

      /**
       * The contact params define the physical contact properties of the node.
       * The default created contact parameters can be used for a standard stable
       * contact behaviour of the simulation. See contact_params for detailed
       * information. \verbatim Default value: see contact_params \endverbatim
       * \sa contact_params
       */
      contact_params c_params;

      /**
       * Generally the inertia of the physical body is calculated by the physicMode
       * and the mass of a node. In case of a mesh the inertia is calculated for
       * a box with the size given by NodeData::extent. If this boolean is set to
       * \c true, the inertia array (NodeData::inertia) is used instead.
       * \verbatim Default value: false \endverbatim
       */
      bool inertia_set;

      /**
       * If the boolean NodeData::inertia_set is \c true, this array is used
       * to define the inertia of the physical body of this node.
       * \verbatim
       Default value: (0.0, 0.0, 0.0,
       0.0, 0.0, 0.0,
       0.0, 0.0, 0.0)
       \endverbatim
      */
      sReal inertia[3][3];

      /**
       * This value defines a scalar that is multiplied to the linear velocity
       * of the body in every timestep if the value is unequal 0.0. \n Default
       * value: 0.0
       */
      sReal linear_damping;

      /**
       * This value defines a scalar that is multiplied to the angular velocity
       * of the body in every timestep if the value is unequal 0.0. \n Default
       * value: 0.0
       */
      sReal angular_damping;
      sReal angular_treshold;
      sReal angular_low;
      int shadow_id;

      bool isShadowCaster;
      bool isShadowReceiver;
      NodeId graphicsID1, graphicsID2;
      /**
       * If the data is created from a ConfigMap map the original map is
       * stored here.
       */
      configmaps::ConfigMap map;
    }; // end of struct NodeData

    inline void ZERO_NODE_STRUCT(NodeData &a) {
      a.init();
    }

  } // end of namespace interfaces

} // end of namespace mars

#endif // MARS_CORE_NODE_DATA_H
