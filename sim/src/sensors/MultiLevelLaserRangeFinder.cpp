#include "MultiLevelLaserRangeFinder.h"

#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/LoadSceneInterface.h>

#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/utils/mathUtils.h>
#include <base/Float.hpp>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* MultiLevelLaserRangeFinder::instanciate(ControlCenter *control, BaseConfig *config ){
      MultiLevelLaserRangeFinderConfig *cfg = dynamic_cast<MultiLevelLaserRangeFinderConfig*>(config);
      assert(cfg);
      return new MultiLevelLaserRangeFinder(control,*cfg);
    }

MultiLevelLaserRangeFinder::MultiLevelLaserRangeFinder(ControlCenter *control, MultiLevelLaserRangeFinderConfig config): 
    BaseNodeSensor(config.id, config.name),
    SensorInterface(control), config(config) 
{

    updateRate = config.updateRate;
    long attached_node = config.attached_node;

    std::string groupName, dataName;
    Vector tmp;
    for(int i = 0; i < 3; ++i)
        positionIndices[i] = -1;
    for(int i = 0; i < 4; ++i)
        rotationIndices[i] = -1;

    control->nodes->addNodeSensor(this);
    bool erg = control->nodes->getDataBrokerNames(attached_node, &groupName, &dataName);
    assert(erg);
    
    //register timer for caputuring the data
    control->dataBroker->registerTimedReceiver(this, groupName, dataName,"mars_sim/simTimer",updateRate);

    if(control->graphics) {
        double anglePerCamera = M_PI /2.0;
        
        int numCameras = ceil(config.horizontalOpeningAngle / anglePerCamera); 
                
        subSensors.resize(numCameras);
        
        for(int i = 0; i < numCameras; i++)
        {
            std::cout << "Subcamera " << i << std::endl;
            
            //create multiple cameras to cover the width of the sensor
            double curWidth = anglePerCamera;

            int rttWidth = config.rttResolutionX;
            int rttHeight = config.rttResolutionY;

            std::cout << "Computing width an height for laser depth image to " << rttWidth << " " << rttHeight << std::endl; 

            long cam_window_id = control->graphics->new3DWindow(0, true, rttWidth, rttHeight, name);

//          interfaces::hudElementStruct hudCam;
//          hudCam.type            = HUD_ELEMENT_TEXTURE;
//          hudCam.width           = 600;
//          hudCam.height          = 300;
//          hudCam.texture_width   = rttWidth;
//          hudCam.texture_height  = rttHeight;
//          hudCam.view_width      = hudCam.width;
//          hudCam.view_height     = hudCam.height;
//          hudCam.posx            = 40 + (hudCam.width * i); // aligned in a row
//          hudCam.posy            = 300;
//          hudCam.border_color[0] = 0.0;
//          hudCam.border_color[1] = 0.58824;
//          hudCam.border_color[2] = 0.0;
//          hudCam.border_color[3] = 1.0;
//          hudCam.border_width    = 5.0;
// 
//          long cam_id = control->graphics->addHUDElement(&hudCam);
//          control->graphics->setHUDElementTextureRTT(cam_id, cam_window_id,false);

            
            interfaces::GraphicsWindowInterface *gw = control->graphics->get3DWindow(cam_window_id);
            assert(gw);
            interfaces::GraphicsCameraInterface *gc = NULL;
            gw->setGrabFrames(false);
            if(gw) {
                gc = gw->getCameraInterface();
                assert(gc);
                control->graphics->addGraphicsUpdateInterface(this);
                
                std::cout << "Creating camera with opening width " << curWidth << " opening_height " << config.verticalOpeningAngle << std::endl;
                
                gc->setFrustumFromRad(anglePerCamera, anglePerCamera, 0.5, 100);
            }
            
            RaySubSensor *rs = &(subSensors[i]);
            rs->cam_window_id = cam_window_id;
            rs->gw = gw;
            rs->gc = gc;
            rs->rttWidth = rttWidth;
            rs->rttHeight = rttHeight;
            rs->depthBuffer.resize(rttHeight * rttWidth);
            rs->coveredAngle = curWidth;
            
            rs->distImage.setSize(rttWidth, rttHeight);
            cameraStruct cam_info;
            gc->getCameraInfo(&cam_info);
            rs->distImage.setIntrinsic(cam_info.scale_x, cam_info.scale_y, cam_info.center_x, cam_info.center_y );
            
            std::cout << " config.horizontalOpeningAngle / 2. " << config.horizontalOpeningAngle / 2.0 << " curWidth / 2.0 " << curWidth / 2.0 << std::endl; 
            
            double curAngle = 2*M_PI - i * M_PI / 2.0;
            
            std::cout << "Turning subsensor to Angle " << curAngle << " / " << curAngle/M_PI * 180 << std::endl;
            
            //as for the * eulerToQuaternion(Vector(90,0,-90)) part
            //I have no idea why it is needed, but it fixes the orientation
            //of the camera.
            rs->orientation =  Eigen::AngleAxisd(curAngle, Eigen::Vector3d::UnitZ()) * eulerToQuaternion(Vector(90,0,-90));
        }
    }
    
    rayValues.resize(config.numRaysVertical * config.numRaysHorizontal, 0);

    position = control->nodes->getPosition(attached_node);
    orientation = control->nodes->getRotation(attached_node);

    //even if we don't draw anything, the need to
    //register ourself here, or we won't get RTT images
    drawStruct draw;
    draw.ptr_draw = (DrawInterface*)this;
    if(control->graphics)
        control->graphics->addDrawItems(&draw);
    
    calculateSamplingPixels();
}

MultiLevelLaserRangeFinder::~MultiLevelLaserRangeFinder(void) {
  if(control->graphics)
    control->graphics->removeDrawItems((DrawInterface*)this);
  control->dataBroker->unregisterTimedReceiver(this, "*", "*", "mars_sim/simTimer");
}

void MultiLevelLaserRangeFinder::preGraphicsUpdate(void )
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

const std::vector<double> &MultiLevelLaserRangeFinder::getSensorData() const {
    return rayValues;
}

int MultiLevelLaserRangeFinder::getSensorData(double** data) const
{
    throw std::runtime_error("Awfull interface, is not supported by this sensor");
    
}


void MultiLevelLaserRangeFinder::receiveData(const data_broker::DataInfo &info,
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

void MultiLevelLaserRangeFinder::calculateSamplingPixels()
{
    const double verticalStartAngle = -config.verticalOpeningAngle / 2.0;
    const double scansVertical = config.numRaysVertical;
    const double scansHorizontal = config.numRaysHorizontal;
    
    double stepHorizontal = config.horizontalOpeningAngle / (scansHorizontal - 1);
    double stepVertical = config.verticalOpeningAngle / (scansVertical - 1);
    
    
    double curHorAngle = 0;
    
    std::vector<RaySubSensor>::iterator it = subSensors.begin();
    
    const double b_x = (config.rttResolutionX / 2.0) / tan(M_PI/4.0); 
    const double b_y = (config.rttResolutionY / 2.0) / tan(M_PI/4.0); 
    
    int counter = 0;
    
    lookups.resize(config.numRaysHorizontal * config.numRaysVertical);
    
    //Fill all distance images with 1.0. 
    //This need to be done, so that we can precompute the directionVector
    for(std::vector<RaySubSensor>::iterator it = subSensors.begin(); it != subSensors.end();it++)
    {
        std::fill(it->distImage.data.begin(), it->distImage.data.end(), 1.0);
    }
    
    
    // Run through the image horizontally.  
    for(int h = 0; h < config.numRaysHorizontal; h++)
    {
        double horAngle = h * stepHorizontal;
        assert(fabs(curHorAngle - (horAngle - counter * M_PI / 2)) < 0.001);
        
        int x = tan(curHorAngle) * b_x + (config.rttResolutionX / 2.0);

        for(int v = 0; v < config.numRaysVertical; v++)
        {
            double verAngle = verticalStartAngle + v * stepVertical;
            
            int y = tan(verAngle) / cos(curHorAngle) * b_y + (config.rttResolutionY / 2.0);
            
            Lookup &lookup(lookups[v + (config.numRaysHorizontal - h - 1) * config.numRaysVertical]);
            lookup.x = x;
            lookup.y = y;
            lookup.sensor = &(*it);

            Eigen::Vector3d dirVec;
            bool result = it->distImage.getScenePoint(x, y, dirVec);
            assert(result);
            lookup.directionVector = dirVec;
        }
        
        curHorAngle += stepHorizontal;
        
        if(curHorAngle > M_PI/4.0)
        {
            counter++;
            std::cout << "Camera " << counter << std::endl;
            curHorAngle -= M_PI/2.0;
            it++;
            if(it == subSensors.end())
                it = subSensors.begin();
        }
    }
    
    std::cout << "Got " << lookups.size() << " lookup " << std::endl;
    
}



void MultiLevelLaserRangeFinder::update(std::vector<draw_item>* drawItems) {
    
//     std::cout << "Update Called " << std::endl;
    
    //update distance images
    for(std::vector<RaySubSensor>::iterator it = subSensors.begin(); it != subSensors.end();it++)
    {
        it->gw->getRTTDepthData(it->distImage.data.data(), config.rttResolutionX, config.rttResolutionY);
    }
    
    const int width = config.rttResolutionX;
    
    int validCnt = 0;
    
    for(int h = 0; h < config.numRaysHorizontal; h++)
    {
        for(int v = 0; v < config.numRaysVertical; v++)
        {
            const int curScanPos = h*config.numRaysVertical + v;
            Lookup &lookup(lookups[curScanPos]);
            
            const int &x(lookup.x);
            const int &y(lookup.y);
            
            const int curImagePos = y*width + x;
            
            const float &dist(lookup.sensor->distImage.data[curImagePos]);
            if(boost::math::isnormal( dist ))
            {
                rayValues[curScanPos] = (dist * lookup.directionVector).norm();
                
                Eigen::Vector3d p;
                lookup.sensor->distImage.getScenePoint(x, y, p);
                
                assert(fabs(p.norm() -  rayValues[curScanPos]) < 0.0001);
                validCnt++;
            }
            else
            {
                rayValues[curScanPos] = base::unset<float>();
            }
        }
    }
}

BaseConfig* MultiLevelLaserRangeFinder::parseConfig(ControlCenter *control,
                                    ConfigMap *config) {
    MultiLevelLaserRangeFinderConfig *cfg = new MultiLevelLaserRangeFinderConfig;
    unsigned int mapIndex = (*config)["mapIndex"];
    unsigned long attachedNodeID = (*config)["attached_node"];
    if(mapIndex) {
        attachedNodeID = control->loadCenter->getMappedID(attachedNodeID,
                                                            interfaces::MAP_TYPE_NODE,
                                                            mapIndex);
    }
    cfg->attached_node = attachedNodeID;
 
    ConfigMap::iterator it;
    if((it = config->find("numRaysVertical")) != config->end())
      cfg->numRaysVertical = it->second;
    if((it = config->find("numRaysHorizontal")) != config->end())
      cfg->numRaysHorizontal = it->second;
    if((it = config->find("rttResolutionX")) != config->end())
      cfg->rttResolutionX = it->second;
    if((it = config->find("rttResolutionY")) != config->end())
      cfg->rttResolutionY = it->second;
    
    if((it = config->find("verticalOpeningAngle")) != config->end())
      cfg->verticalOpeningAngle = it->second;
    if((it = config->find("horizontalOpeningAngle")) != config->end())
      cfg->horizontalOpeningAngle = it->second;
    if((it = config->find("maxDistance")) != config->end())
      cfg->maxDistance = it->second;

    return cfg;
}

ConfigMap MultiLevelLaserRangeFinder::createConfig() const {
    ConfigMap cfg;
    cfg["name"] = config.name;
    cfg["id"] = config.id;
    cfg["type"] = "MultiLevelLaserRangeFinder";
    cfg["attached_node"] = config.attached_node;
    cfg["numRaysVertical"] = config.numRaysVertical;
    cfg["numRaysHorizontal"] = config.numRaysHorizontal;
    cfg["rttResolutionX"] = config.rttResolutionX;
    cfg["rttResolutionY"] = config.rttResolutionY;
    cfg["max_distance"] = config.maxDistance;
    cfg["verticalOpeningAngle"] = config.verticalOpeningAngle;
    cfg["horizontalOpeningAngle"] = config.horizontalOpeningAngle;
    cfg["rate"] = config.updateRate;
    cfg["maxDistance"] = config.maxDistance;
    return cfg;
}

const MultiLevelLaserRangeFinderConfig& MultiLevelLaserRangeFinder::getConfig() const {
    return config;
}

} // end of namespace sim
} // end of namespace mars
