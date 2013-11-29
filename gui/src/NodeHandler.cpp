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


#include "NodeHandler.h"
#include "Widget_Node_State.h"
#include <mars/utils/mathUtils.h>
#include <mars/interfaces/utils.h>

#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

#include <iostream>

#include <QMessageBox>

namespace mars {
  namespace gui {

    using namespace std;

    NodeHandler::NodeHandler(QtVariantProperty* property, unsigned long index,
                             main_gui::PropertyDialog *pd, 
                             interfaces::ControlCenter *c, NodeTree::Mode m) 
    {
      selected = false;
      mode = m;
      control = c;
      topLevelNode = property;
      state_on = false;
      actualName = topLevelNode->propertyName().toStdString();
      myNodeIndex = (int) index;
      nodeName = QString::number(index).toStdString() + ":" + actualName;
      topLevelNode->setPropertyName(QString::fromStdString(nodeName));
      pDialog = pd;
      filled = false;
      primitive_type = -1;
      editColor = pDialog->getPropertyColor(topLevelNode);
      previewColor = QColor(255, 200, 200);
      //ZERO_NODE_STRUCT(node);
      //ZERO_MATERIAL_STRUCT(material);
      initialized = false;
  
    }


    void NodeHandler::fill() {

      control->nodes->getListNodes(&allNodes);
      utils::Vector pos = node.pos;
      utils::sRotation rot = utils::quaternionTosRotation(node.rot);
      int relative_init = 0;
      interfaces::core_objects_exchange posnode;

      if (!initialized) {
        switch (mode) {
        case NodeTree::PreviewMode:
          pDialog->setPropertyColor(topLevelNode, previewColor);
          if (!initialized) {
            node.ext.x() = node.ext.y() = node.ext.z() = 1;
            node.density = 1;
            primitive_type = interfaces::NODE_TYPE_BOX;
            node.movable = 1;
          }
          break;
        case NodeTree::EditMode:
          for (unsigned int i = 0; i < allNodes.size(); i++)
            if ((long)allNodes[i].index == myNodeIndex) {
              node = control->nodes->getFullNode(allNodes[i].index);
              break;
            }
          control->nodes->getNodeExchange(node.index, &posnode);
          pos = posnode.pos;
          rot = utils::quaternionTosRotation(posnode.rot);
          if (node.name == "") { // terrain
            primitive_type = interfaces::NODE_TYPE_TERRAIN;
            topLevelNode->setPropertyName("Terrain");
            node_name->setValue("Terrain");
          } else if (node.filename == "PRIMITIVE")
            primitive_type = atoi(node.origName.c_str());
          else 
            primitive_type = interfaces::NODE_TYPE_BOX;
          break;
        }
        initialized = true;
  
        if (node.relative_id != 0) {
          interfaces::NodeData parentNode;
          parentNode = control->nodes->getFullNode(node.relative_id);
          relative_init = (int)getIndex(node.relative_id)+1;
          // calculate relative position and rotation
          getRelFromAbs(parentNode, &node);
          pos = node.pos;
          rot = utils::quaternionTosRotation(node.rot);
        }
        else {
          relative_init = 0;
        }

      } else { // already initialized
    
        if (node.relative_id != 0) {
          relative_init = (int)getIndex(node.relative_id)+1;
          pos = node.pos;
        }
        else {
          relative_init = 0;
        }
      }
  

      // create & add the properties
      QStringList enumNames;
      map<QString, QVariant> attr;
      attr.insert(pair<QString, QVariant>(QString("minimum"), 0));
  
      // General
      enumNames << "Mesh" << "Box" << "Sphere" << "Capsule" << "Cylinder"
                << "Infinite Plane" << "Terrain";
      general = 
        pDialog->addGenericProperty("../"+nodeName+"/General",
                                    QtVariantPropertyManager::groupTypeId(),
                                    0);
      node_type = 
        pDialog->addGenericProperty("../"+nodeName+"/General/Type",
                                    QtVariantPropertyManager::enumTypeId(),
                                    primitive_type-1, NULL, &enumNames);
      node_name = 
        pDialog->addGenericProperty("../"+nodeName+"/General/Name",
                                    QVariant::String, 
                                    QString::fromStdString(actualName));
      group_id = 
        pDialog->addGenericProperty("../"+nodeName+"/General/Group",
                                    QVariant::Int, node.groupID, &attr);

      image = pDialog->addGenericProperty("../"+nodeName+"/General/Image", 
                                          VariantManager::filePathTypeId(), "");
      image->setAttribute(QString("directory"), QString("."));
      // Physics
      enumNames.clear();
      enumNames << "Mesh" << "Box" << "Sphere" << "Capsule" << "Cylinder" << "Plane" << "Terrain";
      physics = 
        pDialog->addGenericProperty("../"+nodeName+"/Physics",
                                    QVariant::Bool, !(node.noPhysical));
      physics_model = 
        pDialog->addGenericProperty("../"+nodeName+"/Physics/Model",
                                    QtVariantPropertyManager::enumTypeId(),
                                    node.physicMode-1, NULL, &enumNames);
      physics_props.push_back(physics_model);

      attr.insert(pair<QString, QVariant>(QString("decimals"), 9));
      attr.insert(pair<QString, QVariant>(QString("singleStep"), 0.1));
      density =
        pDialog->addGenericProperty("../"+nodeName+"/Physics/Density",
                                    QVariant::Double, node.density, &attr);
      physics_props.push_back(density);  
      mass =
        pDialog->addGenericProperty("../"+nodeName+"/Physics/Mass",
                                    QVariant::Double, node.mass, &attr);
      physics_props.push_back(mass);
      movable =
        pDialog->addGenericProperty("../"+nodeName+"/Physics/Movable",
                                    QVariant::Bool, node.movable);
      physics_props.push_back(movable);
      // Geometry
      enumNames.clear();
      enumNames << "None";
      for(unsigned int i = 0;i<allNodes.size();i++)
        enumNames << QString::fromStdString(allNodes[i].name);
      attr.clear();
      attr.insert(pair<QString, QVariant>(QString("decimals"), 9));
      attr.insert(pair<QString, QVariant>(QString("singleStep"), 0.1));
      geometry = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry",
                                    QtVariantPropertyManager::groupTypeId(),
                                    0);
      move_all =
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Move connected nodes",
                                    QVariant::Bool, true);
      position = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Position",
                                    QtVariantPropertyManager::groupTypeId(),
                                    0);
      relative_pos = 
        pDialog->addGenericProperty("../" + nodeName +
                                    "/Geometry/Position/Relative To",
                                    QtVariantPropertyManager::enumTypeId(),
                                    relative_init,
                                    NULL, &enumNames);
      pos_x = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Position/x",
                                    QVariant::Double, pos.x(), &attr);
      pos_y = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Position/y",
                                    QVariant::Double, pos.y(), &attr);
      pos_z = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Position/z",
                                    QVariant::Double, pos.z(), &attr);
      rotation = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Rotation",
                                    QtVariantPropertyManager::groupTypeId(),
                                    0);
      rot_alpha = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Rotation/alpha",
                                    QVariant::Double, rot.alpha, &attr);
      rot_beta  = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Rotation/beta",
                                    QVariant::Double, rot.beta, &attr);
      rot_gamma = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Rotation/gamma",
                                    QVariant::Double, rot.gamma, &attr);
      attr.insert(pair<QString, QVariant>(QString("minimum"), 1e-16));
      size = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Size",
                                    QtVariantPropertyManager::groupTypeId(),
                                    0);
      size_x = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Size/x",
                                    QVariant::Double, node.ext.x(), &attr);
      size_y = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Size/y", 
                                    QVariant::Double, node.ext.y(), &attr);
      size_z = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Size/z", 
                                    QVariant::Double, node.ext.z(), &attr);
      size_d = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Size/Diameter", 
                                    QVariant::Double, node.ext.x(), &attr);
      size_r = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Size/Radius", 
                                    QVariant::Double, node.ext.x(), &attr);
      size_h = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Size/Height",
                                    QVariant::Double, node.ext.y(), &attr);
      attr.clear();
      attr.insert(pair<QString, QVariant>(QString("minimum"), 1));
      width = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Width", 
                                    QVariant::Double, 100.0, &attr);
      length = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Length", 
                                    QVariant::Double, 100.0, &attr);
      attr.clear();
      attr.insert(pair<QString, QVariant>(QString("minimum"), 0));
      height = 
        pDialog->addGenericProperty("../"+nodeName+"/Geometry/Maximum Height",
                                    QVariant::Double, 10.0, &attr);
      // ODE Options
      wno = new Widget_Node_Options(pDialog);
      wno->init(node.c_params, nodeName + "/");
      ode = wno->getTopLevelProperty();
  
      // Material
      dcm = new Dialog_Create_Material(pDialog, nodeName + "/");
      if (mode == NodeTree::PreviewMode)
        dcm->setMaterial(&material);
      create_material = dcm->getTopLevelProperty();
  
      dem = new Dialog_Edit_Material(pDialog, nodeName + "/");
      if (mode == NodeTree::EditMode)
        dem->setMaterial(&(node.material));
      edit_material = dem->getTopLevelProperty();


      top_props.push_back(general);
      top_props.push_back(physics);
      top_props.push_back(geometry);
      top_props.push_back(ode);
      top_props.push_back(create_material);
      top_props.push_back(edit_material);
    
      on_type_changed();

      filled = true;

    }


    void NodeHandler::on_type_changed() {
      // adjust the properies in the gui according to the selected type
      if (mode == NodeTree::EditMode) {
        if (node.name == "") { // terrain
          primitive_type = interfaces::NODE_TYPE_TERRAIN;
          topLevelNode->setPropertyName("Terrain");
          node_name->setValue("Terrain");
        } else if (node.filename == "PRIMITIVE")
          primitive_type = atoi(node.origName.c_str());
        else 
          primitive_type = interfaces::NODE_TYPE_BOX;
        node_type->setEnabled(false);
        topLevelNode->removeSubProperty(create_material);
      } else { // preview mode
#warning cleaned up, why is here an +1 ??? this might confuse if someone looks to the enums and enters this value in the config?!?!? (matthias Goldhoorn 15.03.2011)
        primitive_type = node_type->value().toInt() + 1;
        geometry->removeSubProperty(move_all);
        topLevelNode->removeSubProperty(edit_material);
      }
  
      switch (primitive_type) {
      case interfaces::NODE_TYPE_TERRAIN:
        previewOff();
        general->removeSubProperty(group_id);
        general->addSubProperty(image);
        for (unsigned int i = 0; i < (unsigned int)physics_props.size(); ++i)
          physics->removeSubProperty(physics_props[i]);
        physics->setValue(false);
        physics->setEnabled(false);
        pDialog->removeAllSubProperties(geometry);
        geometry->addSubProperty(width);
        geometry->addSubProperty(length);
        geometry->addSubProperty(height);
        topLevelNode->removeSubProperty(ode);
        break;

      case interfaces::NODE_TYPE_PLANE:
        general->removeSubProperty(image);
        general->addSubProperty(group_id);  
        for (unsigned int i = 0; i < (unsigned int)physics_props.size(); ++i)
          physics->removeSubProperty(physics_props[i]);
        physics->setValue(true);
        physics->setEnabled(false);  
        geometry->removeSubProperty(size);
        geometry->removeSubProperty(rotation);
        position->removeSubProperty(relative_pos);
        position->removeSubProperty(pos_x);
        position->removeSubProperty(pos_y);
        geometry->removeSubProperty(width);
        geometry->removeSubProperty(length);
        geometry->removeSubProperty(height);
        topLevelNode->addSubProperty(ode);
        physics_model->setValue(interfaces::NODE_TYPE_PLANE-1);
        break;

      case interfaces::NODE_TYPE_BOX:
        size->removeSubProperty(size_d);
        size->removeSubProperty(size_r);
        size->removeSubProperty(size_h);
        size->addSubProperty(size_x);
        size->addSubProperty(size_y);
        size->addSubProperty(size_z);
        physics_model->setValue(interfaces::NODE_TYPE_BOX-1);
        break;

      case interfaces::NODE_TYPE_SPHERE:
        size->addSubProperty(size_d);
        size->removeSubProperty(size_r);
        size->removeSubProperty(size_h);
        size->removeSubProperty(size_x);
        size->removeSubProperty(size_y);
        size->removeSubProperty(size_z);
        physics_model->setValue(interfaces::NODE_TYPE_SPHERE-1);
        break;

      case interfaces::NODE_TYPE_CYLINDER:
        size->addSubProperty(size_r);
        size->addSubProperty(size_h);
        size->removeSubProperty(size_d);
        size->removeSubProperty(size_x);
        size->removeSubProperty(size_y);
        size->removeSubProperty(size_z);
        physics_model->setValue(interfaces::NODE_TYPE_CYLINDER-1);
        break;

      case interfaces::NODE_TYPE_CAPSULE:
        size->addSubProperty(size_r);
        size->addSubProperty(size_h);
        size->removeSubProperty(size_d);
        size->removeSubProperty(size_x);
        size->removeSubProperty(size_y);
        size->removeSubProperty(size_z);
        physics_model->setValue(interfaces::NODE_TYPE_CAPSULE-1);
        break;
      }

      if (primitive_type != interfaces::NODE_TYPE_TERRAIN && primitive_type != interfaces::NODE_TYPE_PLANE) {
        general->removeSubProperty(image);
        general->addSubProperty(group_id);
        physics->setEnabled(true);
        physics->setValue(!node.noPhysical);
        for (unsigned int i = 0; i < (unsigned int)physics_props.size(); ++i)
          if (node.noPhysical)
            physics->removeSubProperty(physics_props[i]);
          else
            physics->addSubProperty(physics_props[i]);
        geometry->addSubProperty(position);
        geometry->addSubProperty(rotation);
        geometry->addSubProperty(size);
        geometry->removeSubProperty(width);
        geometry->removeSubProperty(length);
        geometry->removeSubProperty(height);    
        position->insertSubProperty(pos_y, 0);
        position->insertSubProperty(pos_x, 0);
        position->insertSubProperty(relative_pos, 0);
      }
  
      if (mode == NodeTree::PreviewMode) 
        update();
  
    }
  


  
    NodeHandler::~NodeHandler() {
      previewOff();
      pDialog->removeGenericProperty(topLevelNode);
      //delete topLevelNode;
    }


    // create the node and transit from Preview to Edit mode
    int NodeHandler::accept() {
      if (mode == NodeTree::EditMode)
        return 1;

      if (node_type->value().toInt() != interfaces::NODE_TYPE_TERRAIN-1) { // not terrain
        if ((control->nodes->addNode(&node)) == 0) {
          QMessageBox::information( pDialog, "Error",
                                    "Primitive Creation Failed!",
                                    "OK", 0); // ok == button 
          return 1;
        } else {
          myNodeIndex = (int) node.index;
          actualName = node.name;
          nodeName = QString::number(myNodeIndex).toStdString() + ":" + actualName;
          topLevelNode->setPropertyName(QString::fromStdString(nodeName));
          dem->setMaterial(&(node.material));
          pDialog->addGenericProperty(topLevelNode, edit_material);
          geometry->insertSubProperty(move_all, 0);
          topLevelNode->removeSubProperty(create_material);
          node_type->setEnabled(false);
        }
      } 
  
      else { // terrain
        // editing terrain is not implemented yet, so just ignore it after creation
        if (image->value().toString() != "") {
          control->nodes->addTerrain(&terrain);
          // a terrain cannot be edited currently
          pDialog->removeAllSubProperties(topLevelNode);  
        } else { 
          QMessageBox::information(pDialog, "Add Terrain",
                                   "No image selected!\nTerrain creation failed",
                                   "OK", 0);
          return 1;
        }
      }
      previewOff(); // close the preview
  
      pDialog->setPropertyColor(topLevelNode, editColor);  
  
      mode = NodeTree::EditMode;
      on_type_changed();
      return 0;
    }



    void NodeHandler::previewOn() {
      if (node_type->value().toInt() == 5 || mode == NodeTree::EditMode || filled == 0) 
        return; // if terrain or not a preview
  
      vector<interfaces::NodeData> tmpNodes;
      tmpNodes.push_back(node);
      if(control->graphics) {
        control->graphics->preview(interfaces::PREVIEW_CREATE,false,tmpNodes);
        control->graphics->showCoords(node.pos,node.rot,node.ext);
      }
    }

    void NodeHandler::previewOff() {
      if (mode == NodeTree::EditMode || filled == false) 
        return; // if not a preview

      if(control->graphics) {
        vector<interfaces::NodeData> tmpNodes;
        tmpNodes.push_back(node);
        control->graphics->preview(interfaces::PREVIEW_CLOSE,false,tmpNodes);
        control->graphics->hideCoords(node.pos);
      }
    }



    void NodeHandler::valueChanged(QtProperty *property, const QVariant &value)
    {
      if (filled == false)
        return;
  
      if (mode == NodeTree::PreviewMode) {
    
        if (property == node_name)
          topLevelNode->setPropertyName(QString::number(myNodeIndex) + ":" + value.toString());
    
        else if (property == node_type)
          on_type_changed();
    
        dcm->accept();
        update();
      }



      else { // mode == EditMode
  
        if (property == node_name) {
          topLevelNode->setPropertyName(QString::number(myNodeIndex) + ":" + value.toString());
          updateNode();
        } else if (property == relative_pos)
          relative_changed();
        else if (property == pos_x || property == pos_y || property == pos_z)
          updatePos();
        else if (property == size_x || property == size_y || property == size_z|| 
                 property == size_d || property == size_r || property == size_h)
          updateSize();
        else if (property == move_all) {
          // todo: switch between relative and absolute positioning of nodes
        } else if (property == rot_alpha || property == rot_beta || 
                   property == rot_gamma)
          updateRot();
        else if (property == physics_model || property == mass || property == physics ||
                 property == density || property == movable ||
                 property == physics || property == group_id)
          updateNode();
        else if (dem->owns(property)) 
          updateMaterial();
        else if (wno->owns(property))
          updateContact();
      }
    }





    void NodeHandler::update() {

      //set mass/density
      if (mass->value().toDouble() != 0) {
        node.mass = mass->value().toDouble();
        node.density = 0;
      }
      else if (density->value().toDouble() != 0) {
        node.density = density->value().toDouble();
        node.mass = 0;
      }
      else if (physics->value().toBool()) {
        fprintf(stderr, "Invalid mass/density values! Setting default values.\n");
        node.density = 1.0;
        node.mass = 0.0;
        density->setValue(1.0);
      }

  
      utils::Vector pos;
      pos.x() = pos_x->value().toDouble();
      pos.y() = pos_y->value().toDouble();
      pos.z() = pos_z->value().toDouble();
      utils::sRotation rot;
      rot.alpha = rot_alpha->value().toDouble();
      rot.beta  = rot_beta->value().toDouble();
      rot.gamma = rot_gamma->value().toDouble();
      utils::Quaternion qrot = utils::eulerToQuaternion(rot);
  
      node.relative_id = relative_pos->value().toInt();
    
      if (node.relative_id == 0){
        node.pos = pos;
        node.rot = qrot;
      }
      else{
        myRelNode = control->nodes->getFullNode(allNodes[relative_pos->
                                                         value().toInt()-1].index);
        node.pos = pos;
        node.rot = qrot;
        getAbsFromRel(myRelNode, &node);
      }
      actualName = node.name = node_name->value().toString().toStdString();
      nodeName = QString::number(myNodeIndex).toStdString() + ":" + actualName;  

      utils::Vector ext;
#warning why +1 again here? *strongly confused*
      switch (node_type->value().toInt()+1) {
      case interfaces::NODE_TYPE_BOX: 
      case interfaces::NODE_TYPE_PLANE:
        ext.x() = size_x->value().toDouble(); 
        ext.y() = size_y->value().toDouble();
        ext.z() = size_z->value().toDouble();
        break;
      case interfaces::NODE_TYPE_SPHERE: 
        ext.x() = size_d->value().toDouble(); 
        ext.y() = size_y->value().toDouble();
        ext.z() = size_z->value().toDouble();
        break;
      case interfaces::NODE_TYPE_CYLINDER:
      case interfaces::NODE_TYPE_CAPSULE:
        ext.x() = size_r->value().toDouble(); 
        ext.y() = size_h->value().toDouble();
        ext.z() = size_z->value().toDouble();
        break;
      case interfaces::NODE_TYPE_TERRAIN: // Terrain
        //    terrain.name = node_name->value().toString().toStdString();
        terrain.srcname = image->value().toString().toStdString();
        terrain.scale = height->value().toDouble();
        terrain.targetWidth = width->value().toDouble();
        terrain.targetHeight = length->value().toDouble();
        dcm->accept();
        terrain.material = material;
        control->loadCenter->loadHeightmap->readPixelData(&terrain);
        break;
      }
      node.ext = ext;
  
      node.physicMode = static_cast<interfaces::NodeType>(physics_model->value().toInt()+1);
      node.groupID = group_id->value().toInt();
      node.movable = movable->value().toBool();
      node.noPhysical = !(physics->value().toBool());
      for (unsigned int i = 0; i < (unsigned int)physics_props.size(); ++i)
        if (node.noPhysical)
          physics->removeSubProperty(physics_props[i]);
        else
          physics->addSubProperty(physics_props[i]);
      node.filename = "PRIMITIVE";
      node.origName = (QString::number(node_type->value().toInt()+1)).toStdString();
  
      //handle infinite plane
#warning Here again, why +1????
      if (node_type->value().toInt()+1 == interfaces::NODE_TYPE_PLANE){
        node.physicMode=interfaces::NODE_TYPE_PLANE;
        node.movable=false;
        //an infinite plane needs no x or y pos ;)
        node.pos.x() = node.pos.y() = 0;
        //set rotation to 0 degrees
        node.rot.x() = node.rot.y() = node.rot.z() = 0;
        node.rot.w() = 1;
      }
      dcm->accept();
      node.material = material;
      node.c_params = wno->get();

      previewOff();
      previewOn();
    }


    void NodeHandler::updateNode() {
      int changes = 0;
  
      if (physics->value().toBool() && mass->value().toDouble() == 0 &&
          density->value().toDouble() == 0) {
        fprintf(stderr, "Invalid mass/density values! Setting default values.\n");
        node.density = 1.0;
        node.mass = 0.0;
        density->setValue(1.0);
      }

      if (node.mass != mass->value().toDouble())
        changes = changes | interfaces::EDIT_NODE_MASS;
      if ( node.density != density->value().toDouble())
        changes = changes | interfaces::EDIT_NODE_MASS;
      if (node.physicMode != physics_model->value().toInt()+1)
        changes = changes | interfaces::EDIT_NODE_TYPE;
      if (node.name != node_name->value().toString().toStdString())
        changes = changes | interfaces::EDIT_NODE_NAME;
      if (node.movable != movable->value().toBool()) 
        changes = changes | interfaces::EDIT_NODE_MOVABLE;
      if (node.groupID != group_id->value().toInt())
        changes = changes | interfaces::EDIT_NODE_GROUP;
      if (node.noPhysical == physics->value().toBool())
        changes = changes | interfaces::EDIT_NODE_PHYSICS;
  
      node.mass = mass->value().toDouble();
      node.density = density->value().toDouble();
      actualName = node.name = node_name->value().toString().toStdString();
      nodeName = QString::number(myNodeIndex).toStdString() + ":" + actualName;
      node.physicMode = static_cast<interfaces::NodeType>(physics_model->value().toInt()+1);
      node.movable = movable->value().toBool();
      node.groupID = group_id->value().toInt();
      node.noPhysical = !(physics->value().toBool());

      if (node.noPhysical) {
        physics->removeSubProperty(physics_model);
        physics->removeSubProperty(density);
        physics->removeSubProperty(mass);
        physics->removeSubProperty(movable);
      } else {
        physics->addSubProperty(physics_model);
        physics->addSubProperty(density);
        physics->addSubProperty(mass);
        physics->addSubProperty(movable);
      }
      //send node info to simulation
      control->nodes->editNode(&node, changes);
    }

    /**
     * positioning mode was changed, so handle it
     */
    void NodeHandler::relative_changed(){
      updatePos();
    }


    void NodeHandler::updatePos(){
      utils::Vector pos;
      int changes = interfaces::EDIT_NODE_POS;

      pos.x() = pos_x->value().toDouble();
      pos.y() = pos_y->value().toDouble();
      pos.z() = pos_z->value().toDouble();
  
      if (relative_pos->value().toInt() == 0) {
        node.pos = pos;
      } else {
        node.relative_id =allNodes[relative_pos->value().toInt()-1].index;
        node.pos = pos;
        myRelNode = control->nodes->getFullNode(allNodes[relative_pos->
                                                         value().toInt()-1].index);
        //getAbsFromRel(myRelNode, &node);
      }
      if (move_all->value().toBool())
        changes = changes | interfaces::EDIT_NODE_MOVE_ALL;
  
      control->nodes->editNode(&node, changes);
    }


    void NodeHandler::updateRot(){
      utils::Vector pos;
      utils::sRotation rot;
      int changes = interfaces::EDIT_NODE_ROT;
  
      pos.x() = pos_x->value().toDouble();
      pos.y() = pos_y->value().toDouble();
      pos.z() = pos_z->value().toDouble();
      rot.alpha = rot_alpha->value().toDouble();
      rot.beta = rot_beta->value().toDouble();
      rot.gamma = rot_gamma->value().toDouble();
      //Quaternion qrot = Quaternion(rot);
      utils::Quaternion qrot = utils::eulerToQuaternion(rot);
      node.pos = pos;
      node.rot = qrot;

      if (relative_pos->value().toInt() != 0) {
        node.relative_id =allNodes[relative_pos->value().toInt()-1].index;
      }

      if (move_all->value().toBool())
        changes = changes | interfaces::EDIT_NODE_MOVE_ALL;
  
      control->nodes->editNode(&node, changes);
    }



    void NodeHandler::updateSize(){
      utils::Vector ext;
    
      switch(primitive_type){
      case interfaces::NODE_TYPE_PLANE: 
      case interfaces::NODE_TYPE_BOX:
        ext.x() = size_x->value().toDouble(); 
        ext.y() = size_y->value().toDouble();
        ext.z() = size_z->value().toDouble();
        break;
      case interfaces::NODE_TYPE_SPHERE: 
        ext.x() = size_d->value().toDouble(); 
        ext.y() = size_y->value().toDouble();
        ext.z() = size_z->value().toDouble();
        break;
      case interfaces::NODE_TYPE_CYLINDER:
      case interfaces::NODE_TYPE_CAPSULE:
        ext.x() = size_r->value().toDouble(); 
        ext.y() = size_h->value().toDouble();
        ext.z() = size_z->value().toDouble();
        break;
      }
    
      if (ext.x()!=0 && ext.y() !=0 && ext.z() !=0)
        node.ext = ext;
      int changes = interfaces::EDIT_NODE_SIZE;
      control->nodes->getListNodes(&allNodes);
      control->nodes->editNode(&node, changes);
    }


    void NodeHandler::updateMaterial(){
      int changes = interfaces::EDIT_NODE_MATERIAL;
      dem->accept();
      control->nodes->editNode(&node, changes);
    }

    void NodeHandler::updateContact(){
      int changes = interfaces::EDIT_NODE_CONTACT;
      node.c_params = wno->get();
      control->nodes->editNode(&node, changes);
    }


    /**
     * returns the combobox index of a node
     */
    unsigned int NodeHandler::getIndex(unsigned long index){
      for(unsigned int i =0;i<allNodes.size();i++){
        if (allNodes[i].index == index){
          return i;
        }
      }
      return 0;
    }


    void NodeHandler::showState() {
      if (mode != NodeTree::EditMode || state_on == true)
        return;
      Widget_Node_State *wns = new Widget_Node_State(pDialog, control, nodeName + " State");
      wns->connect(node.index);
      connect(wns->pDialog, SIGNAL(closeSignal()), this, SLOT(closeState()));
      state_on = true;
    }


    void NodeHandler::closeState() {
      state_on = false;
    }


    void NodeHandler::focusIn() {
      if (filled == false) { 
        fill();
        previewOn();
        on_type_changed();
      }
    }


    void NodeHandler::focusOut() {
      if (filled == true) {
        previewOff();
        pDialog->destroyAllSubProperties(topLevelNode);  
        delete wno; delete dem; delete dcm;
        wno = NULL; dem = NULL; dcm = NULL;
        physics_props.clear(); top_props.clear();
        filled = false;
      }
    }

    QtVariantProperty* NodeHandler::getGeometryProp()
    {
      return geometry;
    }

    QtVariantProperty* NodeHandler::getPositionProp()
    {
      return position;
    }

    QtVariantProperty* NodeHandler::getRotationProp()
    {
      return rotation;
    }

    bool NodeHandler::isSelected() {
      return selected;
    }


    void NodeHandler::setSelected(bool s) {
      selected = s;
    }

  } // end of namespace gui
} // end of namespace mars
