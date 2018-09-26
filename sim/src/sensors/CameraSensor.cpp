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

#include "CameraSensor.h"

#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/utils/mathUtils.h>
#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>
#include <mars/utils/Geometry.hpp>
#include <mars/interfaces/sim/LoadCenter.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/EntityManagerInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/interfaces/Logging.hpp>
#include "SimEntity.h"

#include <stdint.h>
#include <cstring>
#include <cstdlib>


namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* CameraSensor::instanciate(ControlCenter *control, BaseConfig *config ){
      CameraConfigStruct *cfg = dynamic_cast<CameraConfigStruct*>(config);
      assert(cfg);
      return new CameraSensor(control,*cfg);
    }


    CameraSensor::CameraSensor(ControlCenter *control, const CameraConfigStruct config) :
      BaseNodeSensor(config.id,config.name),
      SensorInterface(control),
      config(config),
      depthCamera(id,name,config.width,config.height,1,true, false),
      imageCamera(id,name,config.width,config.height,4,false, false),
      logicalCamera(id,name,config.width,config.height,1,false, true)
    {
      renderCam = 2;
      this->attached_node = config.attached_node;
      draw_id = control->nodes->getDrawID(attached_node);
      std::vector<unsigned long>::iterator iter;
      dbPosIndices[0] = -1;

      control->nodes->addNodeSensor(this);
      std::string groupName, dataName;
      this->config.ori_offset = this->config.ori_offset * eulerToQuaternion(Vector(90,0,-90)); //All elements should be X Forwart looging to meet rock-convention, so i add this offset for all setting

      bool found = control->nodes->getDataBrokerNames(attached_node, &groupName, &dataName);
      assert(found);
      if(control->dataBroker->registerTimedReceiver(this, groupName, dataName,"mars_sim/simTimer", config.updateRate)) {
      }

      cam_id=0;
      if(control->graphics) {

        //New
        interfaces::hudElementStruct hudCam;
        hudCam.type            = HUD_ELEMENT_TEXTURE;
        hudCam.width           = config.hud_width;
        hudCam.height          = config.hud_height;
        hudCam.texture_width   = hudCam.width;
        hudCam.texture_height  = hudCam.height;
        hudCam.posx            = 40 + (hudCam.width * config.hud_pos); // aligned in a row
        hudCam.posy            = 30;
        hudCam.border_color[0] = 0.0;
        hudCam.border_color[1] = 0.58824;
        hudCam.border_color[2] = 0.0;
        hudCam.border_color[3] = 1.0;
        hudCam.border_width    = 5.0;

        if(config.show_cam)
          cam_id = control->graphics->addHUDElement(&hudCam);


        cam_window_id = control->graphics->new3DWindow(0, true, config.width,
                                                       config.height, name);
        if(config.show_cam)
          control->graphics->setHUDElementTextureRTT(cam_id, cam_window_id,
                                                     false);

        gw = control->graphics->get3DWindow(cam_window_id);
        gw->setGrabFrames(false);
        if(gw) {
          gc = gw->getCameraInterface();
          control->graphics->addGraphicsUpdateInterface(this);
          gc->setFrustumFromRad(config.opening_width/180.0*M_PI, config.opening_height/180.0*M_PI, 0.5, 100);
          ConfigMap map = config.map;
          if(map.hasKey("distortion_factor")) {
            gw->setupDistortion(map["distortion_factor"]);
          }
        }
      }

      if(!this->config.enabled){
        control->graphics->deactivate3DWindow(cam_window_id);
      }

    }

    CameraSensor::~CameraSensor(void){
      control->dataBroker->unregisterTimedReceiver(this, "*", "*",
                                                   "mars_sim/simTimer");

      if(control->graphics) {
        if(cam_id) {
          control->graphics->removeHUDElement(cam_id);
        }
        if(cam_window_id) {
          control->graphics->remove3DWindow(cam_window_id);
        }
        control->graphics->removeGraphicsUpdateInterface(this);
      }
    }

    void CameraSensor::getCameraInfo(cameraStruct* cs)
    {
      if( gc )
        gc->getCameraInfo( cs );
      else
        std::cerr << "could not get camera info." << std::endl;
    }

    void CameraSensor::getImage(std::vector< Pixel >& buffer)
    {
        assert(buffer.size() == (config.width * config.height));
        int width;
        int height;
        gw->getImageData(reinterpret_cast<char *>(buffer.data()), width, height);

        assert(config.width == width);
        assert(config.height == height);
    }

    void CameraSensor::getDepthImage(std::vector< mars::sim::DistanceMeasurement >& buffer)
    {
        assert(buffer.size() == (config.width * config.height));
        int width;
        int height;
        gw->getRTTDepthData(reinterpret_cast<float *>(buffer.data()), width, height);

        assert(config.width == width);
        assert(config.height == height);
    }

    /** \brief returns all entities in the view of the camera.
    * \param enum ViewMode:
    * CENTER          The center of the bounding box has to be visible to list it
    * VERTEX_OF_BBOX  At least one vertex of the bounding box has to be visible to list it
    * EVERYTHING      Everxthing of the entity has to be visible to list it
    * NOTHING         Nothing of the entity has to be visible to list it
    *  Defines what has to be visible to the camera to get the object
    * \return list of the detected objects
    */
    /* strategy: iterates through all objects. The viewing frustum is represented as the bounding planes.
    * checks for the relevant points if they lie on the positive side of the plane normal.
    */
    void CameraSensor::getEntitiesInView(std::map<unsigned long, SimEntity*> &buffer, unsigned int visVert_threshold) {
      buffer.clear();
      const std::map<unsigned long, SimEntity*>* all_entities = control->entities->subscribeToEntityCreation(nullptr); //get all entities
      //get Camera Info
      cameraStruct cs;
      getCameraInfo(&cs);
      //rotate camera frame to world frame
      Quaternion q = control->graphics->getDrawObjectQuaternion(draw_id);
      Vector view_x = (q * Vector(0, 0, -1)).normalized();
      Vector view_y = (q * Vector(0, -1, 0)).normalized();
      Vector view_z = (q * Vector(1, 0, 0)).normalized();
      Vector viewcenter = view_z.normalized();
      //get planes of viewing frustum, normals pointing inwards
      std::vector<double> f; //frustum
      Plane p[6];
      enum {L, R, B, T, N, F};
      gc->getFrustum(f);
      Vector frustum_center = cs.pos + viewcenter * ((f[N]+f[F])/2);
      //near and far plane Plane(point on the plane, normal)
      p[N] = Plane(viewcenter * f[N] + cs.pos, viewcenter);//normal points away from camera, therefore inward
      p[F] = Plane(viewcenter * f[F] + cs.pos, -viewcenter);//normal points to from camera, therefore inward
      //Plane(point of camera, Line(points at the edges of the frustum on this intersection)
      Vector temp; //Vector to the center point of the intersection
      //left plane
      temp = view_x * f[L] + viewcenter * f[N] + cs.pos;
      p[L] = Plane(cs.pos, view_y * f[B] + temp, view_y * f[T] + temp, Plane::Method::THREE_POINTS);
      p[L].pointNormalTowards(frustum_center);
      //right plane
      temp = view_x * f[R] + viewcenter * f[N] + cs.pos;
      p[R] = Plane(cs.pos, view_y * f[B] + temp, view_y * f[T] + temp, Plane::Method::THREE_POINTS);
      p[R].pointNormalTowards(frustum_center);
      //top plane
      temp = view_y * -f[T] + viewcenter * f[N] + cs.pos;
      p[T] = Plane(cs.pos, view_x * f[L] + temp, view_x * f[R] + temp, Plane::Method::THREE_POINTS);
      p[T].pointNormalTowards(frustum_center);
      //bottom plane
      temp = view_y * -f[B] + viewcenter * f[N] + cs.pos;
      p[B] = Plane(cs.pos, view_x * f[L] + temp, view_x * f[R] + temp, Plane::Method::THREE_POINTS);
      p[B].pointNormalTowards(frustum_center);

      //declare the boundingbox for the entity
      Vector center, extent;
      Quaternion rotation;
      std::vector<utils::Vector> vertices;
      //check for all entities if they are in the view
      for (std::map<unsigned long, SimEntity*>::const_iterator iter = all_entities->begin();
          iter != all_entities->end(); ++iter) {
        iter->second->getBoundingBox(vertices, center);
        vertices.push_back(center);
        unsigned int visible_vertices = 0;
        for (unsigned int v = 0; v<vertices.size() && visible_vertices < visVert_threshold; v++) {
          bool vertex_in_frustum = true;
          //check for each plane of the frustum if the vertex lies on the inner side
          for (int i = L; i<=F; i++) {
            if (distance(p[i], vertices[v], false) < 0) {
              vertex_in_frustum = false;
              break;
            }
          }
          if (vertex_in_frustum == true) {
            visible_vertices++;
          }
        }
        if (visible_vertices >= visVert_threshold) {
          buffer.emplace(iter->first,iter->second);
        }
      }
    }


    // this function is a hack currently, it uses sReal* as byte buffer
    // NOTE: never use the cameraSensor in a controller list!!!!
    // TODO handle depth image and logicalImage
    int CameraSensor::getSensorData(sReal** data) const {
      if(gw) {
        // get image
        int width, height;
        /* todo: handle depth image
        if(false && s_cfg.depthImage) {
          // if there is a depth image, we would like to
          // to also get the normal image data.
          //
          // what is a problem, is that in order to have it
          // in the same block of memory, we need to copy
          // the individual results

          // get the image and depth data
          void *t1 = 0;
          gw->getImageData(&t1, width, height);
          size_t image_size = width * height * sizeof( uint8_t ) * 4;

          float *t2 = 0;
          gw->getRTTDepthData(&t2, width, height);
          size_t depth_size = width * height * sizeof( float );

          // get a new block of memory which will contain
          // both data
          size_t data_size = image_size + depth_size;
          uint8_t *res_mem = (uint8_t*)malloc( data_size );
          *data = (sReal*)res_mem;

          memcpy( res_mem, t1, image_size );
          memcpy( res_mem + image_size, t2, depth_size );

          free( t1 );
          free( t2 );

          return data_size;
        }
        else*/
        {
          unsigned char *buffer;
          gw->getImageData((void**)&buffer, width, height);
          unsigned int size = width*height;
          if(size == 0) return 0;
          *data = (sReal*)calloc(size*4, sizeof(sReal));
          double s = 1./255;
          for(unsigned int i=0; i<size; ++i) {
            (*data)[i*4] = buffer[i*4]*s;
            (*data)[i*4+1] = buffer[i*4+1]*s;
            (*data)[i*4+2] = buffer[i*4+2]*s;
            (*data)[i*4+3] = buffer[i*4+3]*s;
          }
          free(buffer);
          return size*4;
        }
      }
      return 0;
    }

    void CameraSensor::deactivateRendering() {
      if(config.enabled){
        control->graphics->deactivate3DWindow(cam_window_id);
        config.enabled = false;
      }

    }

    void CameraSensor::activateRendering() {
      if(!config.enabled){
        control->graphics->activate3DWindow(cam_window_id);
        config.enabled = true;
      }
    }

    void CameraSensor::preGraphicsUpdate(void) {
      mutex.lock();
      if(gc) {
        Vector p = control->graphics->getDrawObjectPosition(draw_id);
        Quaternion qcorrect = Quaternion(0.5, 0.5, -0.5, -0.5);
        Quaternion q = control->graphics->getDrawObjectQuaternion(draw_id) * qcorrect;
        gc->updateViewportQuat(p.x(), p.y(), p.z(),
                               q.x(), q.y(),
                               q.z(), q.w());
        if(config.enabled) {
          if(renderCam > 2) --renderCam;
          else if(renderCam == 2) {
            control->graphics->activate3DWindow(cam_window_id);
            renderCam = 1;
          }
          else if(renderCam == 1) {
            control->graphics->deactivate3DWindow(cam_window_id);
            renderCam = 0;
          }
        }
      }
      mutex.unlock();
    }

    void CameraSensor::receiveData(const data_broker::DataInfo &info,
                                   const data_broker::DataPackage &package,
                                   int callbackParam) {
      CPP_UNUSED(info);
      mutex.lock();
      renderCam = 2+config.frameOffset;
      if(dbPosIndices[0] == -1) {
        dbPosIndices[0] = package.getIndexByName("position/x");
        dbPosIndices[1] = package.getIndexByName("position/y");
        dbPosIndices[2] = package.getIndexByName("position/z");
        dbRotIndices[0] = package.getIndexByName("rotation/x");
        dbRotIndices[1] = package.getIndexByName("rotation/y");
        dbRotIndices[2] = package.getIndexByName("rotation/z");
        dbRotIndices[3] = package.getIndexByName("rotation/w");
      }
      package.get(dbPosIndices[0], &position.x());
      package.get(dbPosIndices[1], &position.y());
      package.get(dbPosIndices[2], &position.z());
      package.get(dbRotIndices[0], &orientation.x());
      package.get(dbRotIndices[1], &orientation.y());
      package.get(dbRotIndices[2], &orientation.z());
      package.get(dbRotIndices[3], &orientation.w());
      position += (orientation * config.pos_offset);
      orientation= orientation * config.ori_offset;
      mutex.unlock();
    }


    BaseConfig* CameraSensor::parseConfig(ControlCenter *control,
                                          ConfigMap *config) {

      //ConfigMap *config = new ConfigMap();
      //(*config) = (*config_);

      CameraConfigStruct *cfg = new CameraConfigStruct();
      cfg->map = *config;
      unsigned int mapIndex = (*config)["mapIndex"];
      unsigned long attachedNodeID = (*config)["attached_node"];
      if(mapIndex) {
        attachedNodeID = control->loadCenter->getMappedID(attachedNodeID,
                                                          interfaces::MAP_TYPE_NODE,
                                                          mapIndex);
      }
      cfg->attached_node = attachedNodeID;

      ConfigMap::iterator it;
      ConfigMap::iterator it2;

      if((it = config->find("rate")) != config->end())
        cfg->updateRate = it->second;
      else cfg->updateRate = 100;

      if((it = config->find("frame_offset")) != config->end())
        cfg->frameOffset = it->second;
      else cfg->frameOffset = 1;

      if((it = config->find("width")) != config->end())
        cfg->width = it->second;

      if((it = config->find("height")) != config->end())
        cfg->height = it->second;

      if((it = config->find("opening_width")) != config->end()) // deprecated
        cfg->opening_width = it->second;

      if((it = config->find("opening_angle")) != config->end())
        cfg->opening_width = it->second;

      if((it = config->find("opening_height")) != config->end()) // deprecated
        cfg->opening_height = it->second;

      if((it = config->find("opening_angle2")) != config->end())
        cfg->opening_height = it->second;

      // correct opening_height
      if (cfg->opening_height < 0)
        cfg->opening_height = cfg->opening_width * ((double)cfg->height / (double)cfg->width);

      if((it = config->find("show_cam")) != config->end()){
        cfg->show_cam =  it->second;
        if(cfg->show_cam) {
          if((it = config->find("hud_idx")) != config->end())
            cfg->hud_pos = it->second;
        }
      }else{
        cfg->show_cam = false;
      }

      if((it = config->find("enabled")) != config->end()){
        cfg->enabled =  it->second;
      }else{
        cfg->enabled = true;
      }

      if((it = config->find("hud_size")) != config->end()) {
        cfg->hud_width = it->second["x"];
        cfg->hud_height = it->second["y"];
      }

      // if hud_height and hud_width are present, overwrite parameters
      if((it = config->find("hud_width")) != config->end())
        cfg->hud_width = it->second;
      if((it = config->find("hud_height")) != config->end())
        cfg->hud_height = it->second;
      else
        cfg->hud_height = cfg->hud_width * ((double)cfg->height / (double)cfg->width);

      if((it = config->find("position_offset")) != config->end()) {
        cfg->pos_offset[0] = it->second["x"];
        cfg->pos_offset[1] = it->second["y"];
        cfg->pos_offset[2] = it->second["z"];
        LOG_DEBUG("camera position_offset: %g %g %g", cfg->pos_offset[0],
                  cfg->pos_offset[1], cfg->pos_offset[2]);
      }
      if((it = config->find("orientation_offset")) != config->end()) {
        if(it->second.hasKey("yaw")) {
          Vector euler;
          euler.x() = it->second["roll"];
          euler.y() = it->second["pitch"];
          euler.z() = it->second["yaw"];
          cfg->ori_offset = eulerToQuaternion(euler);
        }
        else {
          cfg->ori_offset.x() = it->second["x"];
          cfg->ori_offset.y() = it->second["y"];
          cfg->ori_offset.z() = it->second["z"];
          cfg->ori_offset.w() = it->second["w"];
        }
      }

      return cfg;
    }

    ConfigMap CameraSensor::createConfig() const {

      std::vector<unsigned long>::const_iterator it;
      ConfigMap cfg;
      ConfigMap *tmpCfg;

      cfg["name"] = config.name;
      cfg["id"] = config.id;
      cfg["type"] = std::string("CameraSensor");

      cfg["attached_node"] = config.attached_node;
      cfg["frame_offset"] = config.frameOffset;

      cfg["width"] = config.width;
      cfg["height"] = config.height;

//      cfg["enabled"] = config.enabled;


      if(config.show_cam) {
        cfg["show_cam"] = true;
        cfg["show_cam"]["hud_idx"] = config.hud_pos;
      }

      tmpCfg = cfg["position_offset"];

      (*tmpCfg)["x"] = config.pos_offset[0];
      (*tmpCfg)["y"] = config.pos_offset[1];
      (*tmpCfg)["z"] = config.pos_offset[2];


      Quaternion q = eulerToQuaternion(Vector(90,0,-90));
      q.x() *= -1;
      q.y() *= -1;
      q.z() *= -1;
      q = config.ori_offset * q;

      tmpCfg = cfg["orientation_offset"];
      (*tmpCfg)["x"] = q.x();
      (*tmpCfg)["y"] = q.y();
      (*tmpCfg)["z"] = q.z();
      (*tmpCfg)["w"] = q.w();

      tmpCfg = cfg["hud_size"];
      (*tmpCfg)["x"] = config.hud_width;
      (*tmpCfg)["y"] = config.hud_height;

      return cfg;
    }

  } // end of namespace sim
} // end of namespace mars
