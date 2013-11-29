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

#include "Dialog_Import_Mesh.h"
#include <mars/utils/mathUtils.h>
#include <iostream>
#include <stdlib.h>

#include <osg/ComputeBoundsVisitor>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osg/PositionAttitudeTransform>

#include <mars/main_gui/GuiInterface.h>

#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

#include <QMessageBox>

namespace mars {
  namespace gui {

    /**
     * Dialog constructor. Creates a new dialog, reads the given filename and 
     * fills it with the informations of the first node
     *
     * pre:
     *
     * - filename should contain a supported file type
     */
    Dialog_Import_Mesh::Dialog_Import_Mesh(interfaces::ControlCenter* c,
                                           main_gui::GuiInterface *gui) 
      : main_gui::BaseWidget(0, c->cfg, "Dialog_Import_Mesh"),
        pDialog(new main_gui::PropertyDialog(NULL)), mainGui(gui) {
  
      filled = false;
      user_input = false;
      initialized = false;
      control = c;

      pDialog->setAttribute(Qt::WA_DeleteOnClose);
      pDialog->setWindowTitle(tr("Import Mesh"));
      pDialog->setPropCallback(dynamic_cast<main_gui::PropertyCallback*>(this));
      QObject::connect(pDialog, SIGNAL(closeSignal()), this, SLOT(closeDialog()));
      wno = new Widget_Node_Options(pDialog);

      QStringList enumNames;
      std::map<QString, QVariant> attr;
      attr.insert(std::pair<QString, QVariant>(QString("decimals"), 9));
      attr.insert(std::pair<QString, QVariant>(QString("singleStep"), 0.1));
      attr.insert(std::pair<QString, QVariant>(QString::fromStdString("minimum"), 1e-16));
  
      myFileName = pDialog->addGenericProperty("../File", VariantManager::filePathTypeId(), "");
      myFileName->setAttribute("directory", QString("."));
      meshes = pDialog->addGenericProperty("../Object", QtVariantPropertyManager::enumTypeId(),
                                           QVariant(0), NULL, &enumNames);
      scale_factor = pDialog->addGenericProperty("../Scale Factor", QVariant::Double, 1.0, &attr);
      move_all = pDialog->addGenericProperty("../Move All", QVariant::Bool, false);
  
      //  initialize dialog items with information from first node
      general = pDialog->addGenericProperty("../General", QtVariantPropertyManager::groupTypeId(), 0);
      node_name = pDialog->addGenericProperty("../General/Name", QVariant::String, "");
      attr.clear();
      attr.insert(std::pair<QString, QVariant>(QString::fromStdString("minimum"), 0));
      group_id = pDialog->addGenericProperty("../General/Group ID", QVariant::Int, 0, &attr);
  
      // Physics
      physics = pDialog->addGenericProperty("../Physics", QVariant::Bool, true);
      enumNames.clear();
      enumNames << "Mesh" << "Box" << "Sphere" << "Capsule" << "Cylinder";
      physics_model = pDialog->addGenericProperty("../Physics/Model", QtVariantPropertyManager::enumTypeId(),
                                                  QVariant(0), NULL, &enumNames);
      attr.insert(std::pair<QString, QVariant>(QString("decimals"), 9));
      attr.insert(std::pair<QString, QVariant>(QString("singleStep"), 0.1));
      density = pDialog->addGenericProperty("../Physics/Density", QVariant::Double, 1.0, &attr);
      mass = pDialog->addGenericProperty("../Physics/Mass", QVariant::Double, 0.0, &attr);
      unmovable = pDialog->addGenericProperty("../Physics/Unmovable", QVariant::Bool, false);
  
      // Physical Geometry
      attr.clear();
      attr.insert(std::pair<QString, QVariant>(QString("decimals"), 9));
      attr.insert(std::pair<QString, QVariant>(QString("singleStep"), 0.1));
      geometry = pDialog->addGenericProperty("../Physical Geometry", QtVariantPropertyManager::groupTypeId(), 0);
      position = pDialog->addGenericProperty("../Physical Geometry/Position", 
                                             QtVariantPropertyManager::groupTypeId(), 0);
      pos_x = pDialog->addGenericProperty("../Physical Geometry/Position/x", QVariant::Double, 0, &attr);
      pos_y = pDialog->addGenericProperty("../Physical Geometry/Position/y", QVariant::Double, 0, &attr);
      pos_z = pDialog->addGenericProperty("../Physical Geometry/Position/z", QVariant::Double, 0, &attr);
      rotation = pDialog->addGenericProperty("../Physical Geometry/Rotation", 
                                             QtVariantPropertyManager::groupTypeId(), 0);
      rot_alpha = pDialog->addGenericProperty("../Physical Geometry/Rotation/alpha", 
                                              QVariant::Double, 0, &attr);
      rot_beta  = pDialog->addGenericProperty("../Physical Geometry/Rotation/beta",  
                                              QVariant::Double, 0, &attr);
      rot_gamma = pDialog->addGenericProperty("../Physical Geometry/Rotation/gamma", 
                                              QVariant::Double, 0, &attr);  
      size = pDialog->addGenericProperty("../Physical Geometry/Size", 
                                         QtVariantPropertyManager::groupTypeId(), 0);
      attr.insert(std::pair<QString, QVariant>(QString::fromStdString("minimum"), 1e-16));
      size_x = pDialog->addGenericProperty("../Physical Geometry/Size/x", QVariant::Double, 
                                           1.0, &attr);
      size_y = pDialog->addGenericProperty("../Physical Geometry/Size/y", QVariant::Double, 
                                           1.0, &attr);
      size_z = pDialog->addGenericProperty("../Physical Geometry/Size/z", QVariant::Double, 
                                           1.0, &attr);
      size_d = pDialog->addGenericProperty("../Physical Geometry/Size/Diameter", QVariant::Double, 
                                           1.0, &attr);
      size_r = pDialog->addGenericProperty("../Physical Geometry/Size/Radius", QVariant::Double, 
                                           1.0, &attr);
      size_h = pDialog->addGenericProperty("../Physical Geometry/Size/Height", QVariant::Double, 
                                           1.0, &attr);
  
      // Visual Geometry
      attr.clear();
      attr.insert(std::pair<QString, QVariant>(QString("decimals"), 9));
      attr.insert(std::pair<QString, QVariant>(QString("singleStep"), 0.1));
      visual_geometry = pDialog->addGenericProperty("../Visual Geometry", 
                                                    QtVariantPropertyManager::groupTypeId(), 0);
      visual_position = pDialog->addGenericProperty("../Visual Geometry/Position ",
                                                    QtVariantPropertyManager::groupTypeId(), 0);
      visual_pos_x = pDialog->addGenericProperty("../Visual Geometry/Position /x", QVariant::Double, 0.0, &attr);
      visual_pos_y = pDialog->addGenericProperty("../Visual Geometry/Position /y", QVariant::Double, 0.0, &attr);
      visual_pos_z = pDialog->addGenericProperty("../Visual Geometry/Position /z", QVariant::Double, 0.0, &attr);
      visual_rotation = pDialog->addGenericProperty("../Visual Geometry/Rotation ", 
                                                    QtVariantPropertyManager::groupTypeId(), 0);
      visual_rot_alpha = pDialog->addGenericProperty("../Visual Geometry/Rotation /alpha", QVariant::Double, 0.0, &attr);
      visual_rot_beta  = pDialog->addGenericProperty("../Visual Geometry/Rotation /beta",  QVariant::Double, 0.0, &attr);
      visual_rot_gamma = pDialog->addGenericProperty("../Visual Geometry/Rotation /gamma", QVariant::Double, 0.0, &attr);  
      visual_size = pDialog->addGenericProperty("../Visual Geometry/Size ", 
                                                QtVariantPropertyManager::groupTypeId(), 0);
      attr.insert(std::pair<QString, QVariant>(QString::fromStdString("minimum"), 1e-16));
      visual_size_x = pDialog->addGenericProperty("../Visual Geometry/Size /x", QVariant::Double, 1.0, &attr);
      visual_size_y = pDialog->addGenericProperty("../Visual Geometry/Size /y", QVariant::Double, 1.0, &attr);
      visual_size_z = pDialog->addGenericProperty("../Visual Geometry/Size /z", QVariant::Double, 1.0, &attr);
      visual_size_d = pDialog->addGenericProperty("../Visual Geometry/Size /Diameter", 
                                                  QVariant::Double, 1.0, &attr);
      visual_size_r = pDialog->addGenericProperty("../Visual Geometry/Size /Radius", 
                                                  QVariant::Double, 1.0, &attr);
      visual_size_h = pDialog->addGenericProperty("../Visual Geometry/Size /Height", 
                                                  QVariant::Double, 1.0, &attr);
    
      size->removeSubProperty(size_d); 
      size->removeSubProperty(size_h);
      size->removeSubProperty(size_r); 
      visual_size->removeSubProperty(visual_size_d);   
      visual_size->removeSubProperty(visual_size_h); 
      visual_size->removeSubProperty(visual_size_r); 

      dcm = new Dialog_Create_Material(pDialog);

      /*
        scale_all = new QPushButton(tr("Scale"));
        connect(scale_all, SIGNAL(clicked()), this, SLOT(on_scale()));
        pDialog->buttonBox->addButton(scale_all, QDialogButtonBox::ActionRole);

        remove_mesh = new QPushButton(tr("Remove"));
        connect(remove_mesh, SIGNAL(clicked()), this, SLOT(on_remove()));
        pDialog->buttonBox->addButton(remove_mesh, QDialogButtonBox::ActionRole);
      */

      pDialog->addGenericButton("Scale", this, SLOT(on_scale()));
      pDialog->addGenericButton("Remove", this, SLOT(on_remove()));

      filled = true;

    }

    Dialog_Import_Mesh::~Dialog_Import_Mesh() {
    }

    void Dialog_Import_Mesh::fill() {
      std::vector<std::string> nameString;
      QStringList enumNames;
      filled = false;

      QString filename = myFileName->value().toString();
      if (filename.isEmpty()) {
        initialized = false;
        return;
      }
      osg::ref_ptr<osg::Node> osgReadNode;
      osgReadNode = osgDB::readNodeFile(filename.toStdString());
      fprintf(stderr, "import: %s\n", filename.toStdString().c_str());
      // only start if file has been read correctly
      if (osgReadNode == NULL) {
        initialized = false;
        return;
      }

      allNodes.clear();

      // convert node to group for getting the group functions
      // don't use Node if conversion failed
      osg::ref_ptr<osg::Geode> osgGeodeFromRead = NULL;
      osg::ref_ptr<osg::Group> osgGroupFromRead = NULL;

      // when importing .obj files all nodes are imported as osg::Group (with
      // all objects being its children)
      if ((osgGroupFromRead = osgReadNode->asGroup()) != 0) {
        fprintf(stderr, "\n");
        // get childrens names
        for (unsigned int i = 0 ; i < osgGroupFromRead->getNumChildren(); i++){
          fprintf(stderr, "Dialog_Import_Mesh num child: %d\n", i);
          osg::ref_ptr<osg::Node> createdNode = osgGroupFromRead->getChild(i);
          // search for same names
          fprintf(stderr, " createdNode->getName() = %s\n",
                  createdNode->getName().c_str());
          bool found = false;
          for(unsigned int j=0; j<nameString.size();j++){
            if ((nameString[j] == createdNode->getName())) {
              found = true;
            }
          }
          if(!found) 
            nameString.push_back(createdNode->getName());  
        }
      }
      // when importing .stl files the mesh is stored as an osg::Geode
      else if ((osgGeodeFromRead = osgReadNode->asGeode()) != 0) {
        fprintf(stderr, "\n");
        // clean the filename of everything path related and just keep the pure
        // file name
        std::string name = filename.toStdString();
        const size_t last_slash_idx = name.find_last_of("\\/");
        if (std::string::npos != last_slash_idx) {
          name.erase(0, last_slash_idx + 1);
        }
        //the name for the new node is the filename of the STL file
        nameString.push_back(name);

      }

      // combine nodes with same name
      // go through the namelist
      for (unsigned int j=0; j<nameString.size();j++){
        osg::ref_ptr<osg::PositionAttitudeTransform> transform =
          new osg::PositionAttitudeTransform();
        bool found = false;
        // create a groupnode for each name in the list
        osg::ref_ptr<osg::Group>createdGroup = new osg::Group;
        if (osgGroupFromRead != 0) {
          for (unsigned int i = j ; i < osgGroupFromRead->getNumChildren(); i ++){
            // add all nodes with same name to the group
            osg::ref_ptr<osg::Node> createdNode = osgGroupFromRead->getChild(i);
            if (createdNode->getName() == nameString[j]) {
              createdGroup->addChild(createdNode.get());
              found = true;
            }
            else {
              if (found){
                i = osgGroupFromRead->getNumChildren();
                found = false;
              }
            }
          }
        }
        else if (osgGeodeFromRead != 0) {
          //if the read file is a .STL file the node was read as geode
          createdGroup->addChild(osgReadNode);
        }

        // attach the group to a transform ndoe
        transform->addChild(createdGroup.get());
        // create bounding box to get center point
        osg::ComputeBoundsVisitor cbbv;
        createdGroup->accept(cbbv);
        osg::BoundingBox bb = cbbv.getBoundingBox();
        utils::Vector ex;
        // compute bounding box has to be done in this way
        (fabs(bb.xMax()) > fabs(bb.xMin())) ? ex.x() = fabs(bb.xMax() - bb.xMin()) 
          : ex.x() = fabs(bb.xMin() - bb.xMax());
        (fabs(bb.yMax()) > fabs(bb.yMin())) ? ex.y() = fabs(bb.yMax() - bb.yMin()) 
          : ex.y() = fabs(bb.yMin() - bb.yMax());
        (fabs(bb.zMax()) > fabs(bb.zMin())) ? ex.z() = fabs(bb.zMax() - bb.zMin()) 
          : ex.z() = fabs(bb.zMin() - bb.zMax());
    
        // set pivot point to the center of node instead of the file origin
        transform->setPivotPoint(bb.center());
        transform->setPosition(bb.center());
    
        // set nodes position
        osg::Vec3 oPos = transform->getPosition();
        utils::Vector pos;
        pos.x() = oPos[0];
        pos.y() = oPos[1];
        pos.z() = oPos[2];
   
        // construct NodeData for each object and initialize it
        interfaces::NodeData thisNode;
        interfaces::MaterialData thisMaterial;
        //ZERO_NODE_STRUCT(thisNode);
        //ZERO_MATERIAL_STRUCT(thisMaterial);

        dcm->setMaterial(&material);

        thisNode.rot = utils::Quaternion(1,0,0,0);
        thisNode.pos = pos;
        thisNode.name = nameString[j];
        thisNode.origName = nameString[j];
        thisNode.ext = ex;
        thisNode.physicMode = interfaces::NODE_TYPE_BOX;
        thisNode.groupID = 0;
        thisNode.density = 1;
        thisNode.mass = 0;
        thisNode.movable = true;
        thisNode.filename = myFileName->value().toString().toStdString();
        thisNode.material = thisMaterial;
        thisNode.pivot = pos;
        thisNode.visual_offset_pos = utils::Vector(0,0,0);
        thisNode.visual_offset_rot = utils::Quaternion(1,0,0,0);
        thisNode.visual_size = ex;
        thisNode.relative_id = 0;
        // attach node to allNode vector
        allNodes.push_back(thisNode);
        // add node name to dialogs node list
        enumNames << QString::fromStdString(nameString[j]);
      }
  
      meshes->setAttribute("enumNames", enumNames);
      meshes->setValue(0);
      if(allNodes.size() > 0) {
        wno->init(allNodes[0].c_params);
      }
      /*
        if (allNodes.size() == 1)
        remove_mesh->setEnabled(false);
      */
      control->graphics->preview(interfaces::PREVIEW_CREATE,false,allNodes);

      user_input = true;

      filled = true;
      initialized = true;
      item_changed();
    }

    void Dialog_Import_Mesh::closeDialog() {
      if (initialized)
        control->graphics->preview(interfaces::PREVIEW_CLOSE,false,allNodes);
      if (pDialog) {
        mainGui->removeDockWidget((void*)pDialog);
        delete pDialog;
        pDialog = NULL;
      }
      control->graphics->closeAxis();
    }

    void Dialog_Import_Mesh::valueChanged(QtProperty *property, const QVariant &value)
    {
      (void)value;

      if (filled == false) 
        return;
  
      if (property == myFileName) 
        fill();
  
      if (initialized == false)
        return;
  
      else if (property == meshes)
        item_changed();
  
      else if (property == physics_model)
        model_changed();
  
      something_happened();
      dcm->accept();
    }


    void Dialog_Import_Mesh::reject() {
      if (pDialog) pDialog->close();
    }


    /**when pressing the ok button this function saves the last
       edited node and sends a vector with all nodes to the mainwindow */
    void Dialog_Import_Mesh::accept() {
      if (initialized == false) {
        QMessageBox::information( 0, "Simulation", "No mesh selected!", "OK", 0);
        return;
      } else
        control->nodes->addNode(allNodes);

      if (pDialog) pDialog->close();
    }



    /**
     * scale all objects by fixed factor
     */
    void Dialog_Import_Mesh::on_scale(){
      if (initialized == false)
        return;

      // scale all nodes
      double factor = scale_factor->value().toDouble();
      int current = meshes->value().toInt();  
      for (unsigned int i=0;i<allNodes.size();i++){
        allNodes[i].visual_size.x() *= factor;
        allNodes[i].visual_size.y() *= factor; 
        allNodes[i].visual_size.z() *= factor;
        allNodes[i].ext.x() *= factor;
        allNodes[i].ext.y() *= factor;
        allNodes[i].ext.z() *= factor;
        allNodes[i].pos.x() *= factor;
        allNodes[i].pos.y() *= factor;
        allNodes[i].pos.z() *= factor;
      }
      filled = false;
      //set new values in edit fields
      pos_x->setValue(allNodes[current].pos.x());
      pos_y->setValue(allNodes[current].pos.y());
      pos_z->setValue(allNodes[current].pos.z());
      size_x->setValue(allNodes[current].ext.x());
      size_y->setValue(allNodes[current].ext.y());
      size_z->setValue(allNodes[current].ext.z());
      visual_size_x->setValue(allNodes[current].ext.x());
      visual_size_y->setValue(allNodes[current].ext.y());
      visual_size_z->setValue(allNodes[current].ext.z());
      filled = true;
      control->graphics->preview(interfaces::PREVIEW_EDIT,true,allNodes);
      //  control->graphics->preview(PREVIEW_CLOSE,false,allNodes);
      //  control->graphics->preview(PREVIEW_CREATE,false,allNodes);
    }


    /**
     *  remove an item from the list
     */
    void Dialog_Import_Mesh::on_remove(){
      if (initialized == false)
        return;

      if (allNodes.size()>1) {
        // remove the node from the vector
        int current = meshes->value().toInt();  
        allNodes.erase(allNodes.begin() + current);
        control->graphics->removePreviewNode(current);

        //  control->graphics->preview(PREVIEW_CLOSE,false,allNodes);
        //  control->graphics->preview(PREVIEW_CREATE,false,allNodes);

        QStringList enumNames;
        filled = false;
        // update the combo
        for (unsigned int i = 0; i < allNodes.size(); i++)
          enumNames << QString::fromStdString(allNodes[i].name);
        meshes->setAttribute("enumNames", enumNames);
        /*
          if (allNodes.size() == 1)
          remove_mesh->setEnabled(false);
        */
        filled = true;
      }
    }

    void Dialog_Import_Mesh::model_changed(){
      if (initialized == false)
        return;

      if(user_input) {
        switch (physics_model->value().toInt()+1){
        case interfaces::NODE_TYPE_MESH:
        case interfaces::NODE_TYPE_BOX:
          size->addSubProperty(size_x);
          size->addSubProperty(size_y);
          size->addSubProperty(size_z);
          size->removeSubProperty(size_d);
          size->removeSubProperty(size_r);
          size->removeSubProperty(size_h);
          visual_size->addSubProperty(visual_size_x);
          visual_size->addSubProperty(visual_size_y);
          visual_size->addSubProperty(visual_size_z);
          visual_size->removeSubProperty(visual_size_d);
          visual_size->removeSubProperty(visual_size_r);
          visual_size->removeSubProperty(visual_size_h);
          break;
        case interfaces::NODE_TYPE_SPHERE:
          size->addSubProperty(size_d);
          size->removeSubProperty(size_x);
          size->removeSubProperty(size_y);
          size->removeSubProperty(size_z);
          size->removeSubProperty(size_r);
          size->removeSubProperty(size_h);
          visual_size->addSubProperty(visual_size_d);
          visual_size->removeSubProperty(visual_size_x);
          visual_size->removeSubProperty(visual_size_y);
          visual_size->removeSubProperty(visual_size_z);
          visual_size->removeSubProperty(visual_size_r);
          visual_size->removeSubProperty(visual_size_h);
          break;
        case interfaces::NODE_TYPE_CYLINDER:
        case interfaces::NODE_TYPE_CAPSULE:
          size->addSubProperty(size_r);
          size->addSubProperty(size_h);
          size->removeSubProperty(size_x);
          size->removeSubProperty(size_y);
          size->removeSubProperty(size_z);
          size->removeSubProperty(size_d);
          visual_size->addSubProperty(visual_size_r);
          visual_size->addSubProperty(visual_size_h);
          visual_size->removeSubProperty(visual_size_x);
          visual_size->removeSubProperty(visual_size_y);
          visual_size->removeSubProperty(visual_size_z);
          visual_size->removeSubProperty(visual_size_d);
          break;
        }
        something_happened();
      }
    }

    /** the selected item changed so update the lineEdit boxes */
    void Dialog_Import_Mesh::item_changed() {
      QString tempQString;
      utils::sRotation rot, vrot;
      int current = meshes->value().toInt();
      user_input = false;
      filled = false;
      wno->init(allNodes[current].c_params);
      pos_x->setValue(allNodes[current].pos.x());
      pos_y->setValue(allNodes[current].pos.y());
      pos_z->setValue(allNodes[current].pos.z());
      visual_pos_x->setValue(allNodes[current].visual_offset_pos.x());
      visual_pos_y->setValue(allNodes[current].visual_offset_pos.y());
      visual_pos_z->setValue(allNodes[current].visual_offset_pos.z());
  
      size_x->setValue(allNodes[current].ext.x());
      size_y->setValue(allNodes[current].ext.y());
      size_z->setValue(allNodes[current].ext.z());
      visual_size_x->setValue(allNodes[current].visual_size.x());
      visual_size_y->setValue(allNodes[current].visual_size.y());
      visual_size_z->setValue(allNodes[current].visual_size.z());
  
      // convert rotation from Quaternion to euler angles to display them
      rot = utils::quaternionTosRotation(allNodes[current].rot);
      //vrot = allNodes[current].visual_offset_rot.toEuler();
      vrot = utils::quaternionTosRotation(allNodes[current].visual_offset_rot);
      rot_alpha->setValue(rot.alpha);
      rot_beta->setValue(rot.beta);
      rot_gamma->setValue(rot.gamma);
      visual_rot_alpha->setValue(vrot.alpha);
      visual_rot_beta->setValue(vrot.beta);
      visual_rot_gamma->setValue(vrot.gamma);
  
      physics->setValue(allNodes[current].noPhysical);
      group_id->setValue(allNodes[current].groupID);
      physics_model->setValue(allNodes[current].physicMode-1);
      node_name->setValue(QString::fromStdString(allNodes[current].name));
      unmovable->setValue(!allNodes[current].movable);
      user_input = true;

      // set current preview color
    
      //set preview color for selected one
      interfaces::MaterialData mat = allNodes[current].material;
      utils::Color diffuse;
      diffuse.r=0.0;
      diffuse.g=0.0;
      diffuse.b=1.0;
      diffuse.a=1.0;
      mat.diffuseFront = diffuse;
      mat.diffuseBack = diffuse;

      filled = true;

      control->graphics->preview(interfaces::PREVIEW_COLOR, false, allNodes, current, &mat);  
      //  control->graphics->preview(PREVIEW_CLOSE,false,allNodes);
      //  control->graphics->preview(PREVIEW_CREATE,false,allNodes);

    }


    /** this function updates the NodeData structs if anything in
        the dialog has happened */
    void Dialog_Import_Mesh::something_happened() {
      if (initialized == false)
        return;

      bool resize = false;
      int current = meshes->value().toInt();
      //set mass/density
      if (mass->value().toDouble() != 0) {
        allNodes[current].mass = mass->value().toDouble();
        allNodes[current].density = 0;
      }
      else{
        allNodes[current].density = density->value().toDouble();
        allNodes[current].mass = 0;
      }
  
      utils::Vector pos, vpos;
      pos.x()= pos_x->value().toDouble();
      pos.y()= pos_y->value().toDouble();
      pos.z()= pos_z->value().toDouble();
      if (move_all->value() == true){
        utils::Vector offset;
        offset.x() = pos.x()-allNodes[current].pos.x();
        offset.y() = pos.y()-allNodes[current].pos.y();
        offset.z() = pos.z()-allNodes[current].pos.z();
        for(unsigned int i=0;i<allNodes.size();i++){
          allNodes[i].pos.x() += offset.x();
          allNodes[i].pos.y() += offset.y();
          allNodes[i].pos.z() += offset.z();
        }
      }
      else{
        allNodes[current].pos = pos;
      }
      vpos.x()= visual_pos_x->value().toDouble();
      vpos.y()= visual_pos_y->value().toDouble();
      vpos.z()= visual_pos_z->value().toDouble();
      allNodes[current].visual_offset_pos = vpos;
  
      allNodes[current].relative_id = 0;

      // set size
      utils::Vector ext, vext;
      ext.x()= size_x->value().toDouble();
      ext.y()= size_y->value().toDouble();
      ext.z()= size_z->value().toDouble();
      vext.x()= visual_size_x->value().toDouble();
      vext.y()= visual_size_y->value().toDouble();
      vext.z()= visual_size_z->value().toDouble();
      if (vext.x() != allNodes[current].visual_size.x() || 
          vext.y() != allNodes[current].visual_size.y() || 
          vext.z() != allNodes[current].visual_size.z() ){
        resize=true;
      }
      allNodes[current].ext = ext;
      allNodes[current].visual_size = vext;
  
      allNodes[current].name = node_name->value().toString().toStdString();
	
      utils::sRotation rot, vrot;
      rot.alpha = rot_alpha->value().toDouble();
      rot.beta = rot_beta->value().toDouble();
      rot.gamma = rot_gamma->value().toDouble();
      vrot.alpha = visual_rot_alpha->value().toDouble();
      vrot.beta = visual_rot_beta->value().toDouble();
      vrot.gamma = visual_rot_gamma->value().toDouble();
  
      allNodes[current].rot = utils::eulerToQuaternion(rot);
      allNodes[current].visual_offset_rot = utils::eulerToQuaternion(vrot);
  
      allNodes[current].groupID = group_id->value().toInt();
      allNodes[current].noPhysical = physics->value().toBool();
      allNodes[current].physicMode = static_cast<interfaces::NodeType>(physics_model->value().toInt()+1);
      allNodes[current].movable = !(unmovable->value().toBool());
	
      allNodes[current].filename=myFileName->value().toString().toStdString();
      allNodes[current].c_params = wno->get();

      control->graphics->preview(interfaces::PREVIEW_EDIT,true,allNodes);  
      //  control->graphics->preview(PREVIEW_CLOSE,false,allNodes);
      //  control->graphics->preview(PREVIEW_CREATE,false,allNodes);
    }

  } // end of namespace gui
} // end of namespace mars
