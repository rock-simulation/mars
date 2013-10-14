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

/*
 *  RaySensor.cpp
 *  QTVersion
 *
 *  Created by Malte Römmerann
 *
 */

#include "RaySensor.h"

#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/LoadSceneInterface.h>

#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/utils/mathUtils.h>
#include <mars/sim/RaySensor.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

namespace mars {
namespace sim {

using namespace utils;
using namespace interfaces;

BaseSensor* RaySensor::instanciate(ControlCenter *control, BaseConfig *config ){
    RayConfig *cfg = dynamic_cast<RayConfig*>(config);
    assert(cfg);
    return new RaySensor(control,*cfg);
}

RaySensor::RaySensor(ControlCenter *control, RayConfig config): 
    BaseNodeSensor(config.id, config.name),
    SensorInterface(control), config(config) {

    updateRate = 0;
    long attached_node = config.attached_node;

    std::string groupName, dataName;
    drawStruct draw;
    draw_item item;
    int i;
    Vector tmp;
    for(int i = 0; i < 3; ++i)
	positionIndices[i] = -1;
    for(int i = 0; i < 4; ++i)
	rotationIndices[i] = -1;

    control->nodes->addNodeSensor(this);
    bool erg = control->nodes->getDataBrokerNames(attached_node, &groupName, &dataName);
    assert(erg);
    
    if(control->dataBroker->registerTimedReceiver(this, groupName, dataName,"mars_sim/simTimer",updateRate)) {
    }    

    if(control->graphics) {
	/**
	* We need to calculate the width and height in a way
	* that the sensor error is not to big. This means we
	* try not to cover at more than 1x1cm at the maximum distance
	* with one pixel
	* */
	
	double openingAngleForOneCmAtMaxDist = atan2(0.005, config.maxDistance);
	
	//HACK set a 'good' height
	config.opening_height = openingAngleForOneCmAtMaxDist;
	
	std::cout << "Max dist " << config.maxDistance << " openingAngleForOneCmAtMaxDist " << openingAngleForOneCmAtMaxDist << std::endl;
	
	double anglePerCamera = M_PI /2.0;
	
	int numCameras = config.opening_width / anglePerCamera + 1; 
	
	subSensors.resize(numCameras);
	
	double coveredWidth = 0;
	overallRttWidth = 0;
	
	for(int i = 0; i < numCameras; i++)
	{
	    std::cout << "Subcamera " << i << std::endl;
	    
	    //create multiple cameras to cover the width of the sensor
	    double curWidth = std::min(anglePerCamera, config.opening_width - coveredWidth);

	    //compute the configured min amount of pixels
	    int minPixels = config.width * curWidth / config.opening_width;
	    //amout of pixels to get a 'good' sensor resolution at max distance
	    int neededPixels = curWidth / openingAngleForOneCmAtMaxDist;
	    
	    int rttWidth = std::max(minPixels, neededPixels);
	    int rttHeight = std::max<int>(config.height, config.opening_height / openingAngleForOneCmAtMaxDist);

	    std::cout << "Computing width an height for laser depth image to " << rttWidth << " " << rttHeight << std::endl; 

	    long cam_window_id = control->graphics->new3DWindow(0, true, rttWidth, rttHeight, name);

// 	    interfaces::hudElementStruct hudCam;
// 	    hudCam.type            = HUD_ELEMENT_TEXTURE;
// 	    hudCam.width           = 600;
// 	    hudCam.height          = 300;
// 	    hudCam.texture_width   = rttWidth;
// 	    hudCam.texture_height  = rttHeight;
// 	    hudCam.view_width      = hudCam.width;
// 	    hudCam.view_height     = hudCam.height;
// 	    hudCam.posx            = 40 + (hudCam.width * i); // aligned in a row
// 	    hudCam.posy            = 300;
// 	    hudCam.border_color[0] = 0.0;
// 	    hudCam.border_color[1] = 0.58824;
// 	    hudCam.border_color[2] = 0.0;
// 	    hudCam.border_color[3] = 1.0;
// 	    hudCam.border_width    = 5.0;
// 
// 	    long cam_id = control->graphics->addHUDElement(&hudCam);
// 	    control->graphics->setHUDElementTextureRTT(cam_id, cam_window_id,false);

	    
	    interfaces::GraphicsWindowInterface *gw = control->graphics->get3DWindow(cam_window_id);
	    assert(gw);
	    interfaces::GraphicsCameraInterface *gc = NULL;
	    gw->setGrabFrames(false);
	    if(gw) {
		gc = gw->getCameraInterface();
		assert(gc);
		control->graphics->addGraphicsUpdateInterface(this);
		
		std::cout << "Creating camera with opening width " << curWidth << " opening_height " << config.opening_height << std::endl;
		
		gc->setFrustumFromRad(curWidth, config.opening_height, 0.5, 100);
	    }
	    
	    RaySubSensor *rs = &(subSensors[i]);
	    rs->cam_window_id = cam_window_id;
	    rs->gw = gw;
	    rs->gc = gc;
	    rs->rttWidth = rttWidth;
	    rs->rttHeight = rttHeight;
	    rs->depthBuffer.resize(rttHeight * rttWidth);
	    rs->coveredAngle = curWidth;
	    
	    std::cout << " config.opening_width / 2. " << config.opening_width / 2.0 << " coveredWidth " << coveredWidth << " curWidth / 2.0 " << curWidth / 2.0 << std::endl; 
	    
	    double curAngle = config.opening_width / 2.0 - coveredWidth - curWidth / 2.0;
	    
	    std::cout << "Turnin subsensor to Angle " << curAngle << " / " << curAngle/M_PI * 180 << std::endl;
	    
	    //as for the * eulerToQuaternion(Vector(90,0,-90)) part
	    //I have no idea why it is needed, but it fixes the orientation
	    //of the camera.
	    rs->orientation =  Eigen::AngleAxisd(curAngle, Eigen::Vector3d::UnitZ()) * eulerToQuaternion(Vector(90,0,-90));

	    coveredWidth += curWidth;
	    overallRttWidth += rttWidth;
	}
    }
    
    rayValues.resize(config.width, 0);

    position = control->nodes->getPosition(attached_node);
    orientation = control->nodes->getRotation(attached_node);

//     //Drawing Stuff
    draw.ptr_draw = (DrawInterface*)this;
//     item.id = 0;
//     item.type = DRAW_LINE;
//     item.draw_state = DRAW_STATE_CREATE;
//     item.point_size = 1;
//     item.myColor.r = 1;
//     item.myColor.g = 0;
//     item.myColor.b = 0;
//     item.myColor.a = 1;
//     item.texture = "";
//     item.t_width = item.t_height = 0;
//     item.get_light = 0.0;
//     
// 
//     double rad_step = -config.opening_width / config.width;
//     double rad_start = config.opening_width / 2.0; //Starting to Left, because 0 is in front and rock convention posive CCW //(M_PI-rad_angle)/2;
// 
//     //printf("Rad Start: %f, rad_steps: %f, stepX: %f\n",rad_start,rad_steps,stepX);
// 
//     for(i=0; i< config.width; i++){
// 	Eigen::Vector3d curDir = Eigen::AngleAxisd(rad_start + i * rad_step, Eigen::Vector3d::UnitZ()) * Eigen::Vector3d::UnitX();
// 	directions.push_back(curDir);
// 	
// // 	std::cout << "Dir " << i << " " << curDir.transpose() << std::endl;
// 	
// 	item.start = position;
// 	item.end = position;
// 	draw.drawItems.push_back(item);
//     }
// 
    if(control->graphics)
	control->graphics->addDrawItems(&draw);
    
}

RaySensor::~RaySensor(void) {
    control->graphics->removeDrawItems((DrawInterface*)this);
    control->dataBroker->unregisterTimedReceiver(this, "*", "*", 
						"mars_sim/simTimer");
}

void RaySensor::preGraphicsUpdate(void )
{
    for(std::vector<RaySubSensor>::iterator it = subSensors.begin(); it != subSensors.end(); it++)
    {
	if(it->gc) {
	    Eigen::Quaterniond subSensorOrientation = orientation * it->orientation;
	    it->gc->updateViewportQuat(position.x(), position.y(), position.z(),
				subSensorOrientation.x(), subSensorOrientation.y(), subSensorOrientation.z(), subSensorOrientation.w());
	}
    }
}

std::vector<double> RaySensor::getSensorData() const {
    std::vector<double> ret;
    ret.resize(rayValues.size());
    
    std::copy(rayValues.rbegin(), rayValues.rend(), ret.begin());
    
    return ret;
}

void RaySensor::receiveData(const data_broker::DataInfo &info,
			    const data_broker::DataPackage &package,
			    int callbackParam) {
    CPP_UNUSED(info);
    CPP_UNUSED(callbackParam);
    long id;
    package.get(0, &id);

    if(positionIndices[0] == -1) {
	positionIndices[0] = package.getIndexByName("position/x");
	positionIndices[1] = package.getIndexByName("position/y");
	positionIndices[2] = package.getIndexByName("position/z");
	rotationIndices[0] = package.getIndexByName("rotation/x");
	rotationIndices[1] = package.getIndexByName("rotation/y");
	rotationIndices[2] = package.getIndexByName("rotation/z");
	rotationIndices[3] = package.getIndexByName("rotation/w");
    }
    for(int i = 0; i < 3; ++i)
	package.get(positionIndices[i], &position[i]);
    
    package.get(rotationIndices[0], &orientation.x());
    package.get(rotationIndices[1], &orientation.y());
    package.get(rotationIndices[2], &orientation.z());
    package.get(rotationIndices[3], &orientation.w());

}

void RaySensor::update(std::vector<draw_item>* drawItems) {
    
//     std::cout << "Update Called " << std::endl;
    
    
    int width;
    int height;
    
    double widthStep = 1.0;
    
    if(overallRttWidth != config.width)
    {
	widthStep = ((double)overallRttWidth) / config.width; 
    }
    
//     std::cout << "RttWidht " << overallRttWidth << " widthStep " << widthStep << std::endl;
    
    double curX = 0;
    
    std::vector<RaySubSensor>::iterator subSensor = subSensors.begin();
    assert(subSensor != subSensors.end());
    
    subSensor->gw->getRTTDepthData(reinterpret_cast<float *>(subSensor->depthBuffer.data()), width, height);
    
    int curSubRayStart = 0;
    int curSubRayEnd = subSensor->rttWidth;
    
//     std::cout << "Reading ";
    
    for(std::vector<double>::iterator it = rayValues.begin(); it != rayValues.end(); it++)
    {
	int x = curX;
	
	while(x > curSubRayEnd)
	{
// 	    std::cout << "X is " << x << " moving on to next SubSensor " << std::endl;

	    //move on to next camera
	    subSensor++;

	    assert(subSensor != subSensors.end());

	    curSubRayStart = curSubRayEnd;
	    
	    curSubRayEnd += subSensor->rttWidth;
	    
	    subSensor->gw->getRTTDepthData(reinterpret_cast<float *>(subSensor->depthBuffer.data()), width, height);
	    
	    assert(width == subSensor->rttWidth);
	}
	
	double distance = subSensor->depthBuffer[x - curSubRayStart];
	
	*it = distance;
	
// 	std::cout << distance << " ";
	
	curX += widthStep;
	
    }
    
//     std::cout << std::endl;
//     if(!(*drawItems)[0].draw_state) {
// 	for(size_t i=0; i<rayValues.size(); i++) {
// 	    (*drawItems)[i].draw_state = DRAW_STATE_UPDATE;
// 	    (*drawItems)[i].start = position;
// 	    (*drawItems)[i].end = (orientation * directions[i]);
// 	    (*drawItems)[i].end *= rayValues[i];
// 	    (*drawItems)[i].end += (*drawItems)[i].start;
// 	}
//     }
  
}

BaseConfig* RaySensor::parseConfig(ControlCenter *control,
                                       ConfigMap *config) {
      RayConfig *cfg = new RayConfig;
      unsigned int mapIndex = (*config)["mapIndex"][0].getUInt();
      unsigned long attachedNodeID = (*config)["attached_node"][0].getULong();
      if(mapIndex) {
        attachedNodeID = control->loadCenter->loadScene->getMappedID(attachedNodeID,
                                                                     interfaces::MAP_TYPE_NODE,
                                                                     mapIndex);
      }

      ConfigMap::iterator it;
      if((it = config->find("width")) != config->end())
        cfg->width = it->second[0].getInt();
      if((it = config->find("opening_width")) != config->end())
        cfg->opening_width = it->second[0].getDouble();
      if((it = config->find("max_distance")) != config->end())
        cfg->maxDistance = it->second[0].getDouble();
      if((it = config->find("draw_rays")) != config->end())
        cfg->draw_rays = it->second[0].getBool();

      cfg->attached_node = attachedNodeID;
#warning Parse stepX stepY cols and rows
      /*
        ConfigMap::iterator it;

        if((it = config->find("stepX")) != config->end())
        cfg->stepX = it->second[0].getDouble();

        if((it = config->find("stepY")) != config->end())
        cfg->stepY = it->second[0].getDouble();

        if((it = config->find("cols")) != config->end())
        cfg->cols = it->second[0].getUInt();

        if((it = config->find("rows")) != config->end())
        cfg->rows = it->second[0].getUInt();
      */
      return cfg;
    }

    ConfigMap RaySensor::createConfig() const {
      ConfigMap cfg;
      cfg["name"][0] = ConfigItem(config.name);
      cfg["id"][0] = ConfigItem(config.id);
      cfg["type"][0] = ConfigItem("RaySensor");
      cfg["attached_node"][0] = ConfigItem(config.attached_node);
      cfg["width"][0] = ConfigItem(config.width);
      cfg["opening_width"][0] = ConfigItem(config.opening_width);
      cfg["max_distance"][0] = ConfigItem(config.maxDistance);
      cfg["draw_rays"][0] = ConfigItem(config.draw_rays);
      /*
        cfg["stepX"][0] = ConfigItem(config.stepX);
        cfg["stepY"][0] = ConfigItem(config.stepY);
        cfg["cols"][0] = ConfigItem(config.cols);
        cfg["rows"][0] = ConfigItem(config.rows);
      */
      return cfg;
    }

    const RayConfig& RaySensor::getConfig() const {
      return config;
    }

  } // end of namespace sim
} // end of namespace mars
