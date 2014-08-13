#ifndef MULTILEVELLASERRANGEFINGER_MARS_H
#define MULTILEVELLASERRANGEFINGER_MARS_H

#ifdef _PRINT_HEADER_
#warning "RttRotatingRaySensor.h"
#endif

#include <mars/interfaces/sim/SensorInterface.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>
#include <mars/interfaces/graphics/draw_structs.h>
#include <mars/interfaces/graphics/GraphicsUpdateInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <base/samples/DistanceImage.hpp>


namespace mars {
  namespace sim {

    class MultiLevelLaserRangeFinderConfig : public interfaces::BaseConfig{
    public:
      MultiLevelLaserRangeFinderConfig(){
        name = "Unknown RttRaySensor";
        numRaysVertical=32;
        numRaysHorizontal=1900;
        rttResolutionX = 640;
        rttResolutionY = 480;
        verticalOpeningAngle= 40 / 180.0 * M_PI;
        horizontalOpeningAngle= 2 * M_PI * (double (numRaysHorizontal - 1)) / numRaysHorizontal;
        attached_node = 0;
        maxDistance = 100.0;
      }

      unsigned long attached_node;
      int numRaysVertical;
      int numRaysHorizontal;
      int rttResolutionX;
      int rttResolutionY;
      double verticalOpeningAngle;
      double horizontalOpeningAngle;
      double maxDistance;
    };

    class MultiLevelLaserRangeFinder : 
      public interfaces::BaseNodeSensor,
      public interfaces::SensorInterface, 
      public data_broker::ReceiverInterface,
      public interfaces::DrawInterface,
      public interfaces::GraphicsUpdateInterface {

    public:
        static interfaces::BaseSensor* instanciate(interfaces::ControlCenter *control,
                                                    interfaces::BaseConfig* config);
        MultiLevelLaserRangeFinder(interfaces::ControlCenter *control, MultiLevelLaserRangeFinderConfig config);
        ~MultiLevelLaserRangeFinder(void);
      
        virtual void preGraphicsUpdate(void);
  
        const std::vector< double >& getSensorData() const; 
        std::vector<double> getPointCloud();
        virtual int getSensorData(double** data) const;
        virtual void receiveData(const data_broker::DataInfo &info,
                                const data_broker::DataPackage &package,
                                int callbackParam);
        virtual void update(std::vector<interfaces::draw_item>* drawItems);
        
        static interfaces::BaseConfig* parseConfig(interfaces::ControlCenter *control,
                                                 utils::ConfigMap *config);
        
        virtual utils::ConfigMap createConfig() const;

        const MultiLevelLaserRangeFinderConfig& getConfig() const;

    private:
        
        void calculateSamplingPixels();
        
        MultiLevelLaserRangeFinderConfig config;
        struct RaySubSensor
        {
            long cam_window_id;
            interfaces::GraphicsWindowInterface *gw;
            interfaces::GraphicsCameraInterface *gc;
            base::samples::DistanceImage distImage;
            std::vector<float> depthBuffer;
            int rttWidth;
            int rttHeight;
            double coveredAngle;
            utils::Quaternion orientation;
        };
        
        struct Lookup
        {
            int x;
            int y;
            struct RaySubSensor *sensor;
            utils::Vector directionVector;
        };
        
        std::vector<Lookup> lookups;
        
        std::vector<RaySubSensor> subSensors;
        utils::Vector position;
        utils::Quaternion orientation;
        
        std::vector<double> rayValues;
        
        std::vector<utils::Vector> directions;
        long positionIndices[3];
        long rotationIndices[4];
    };

  } // end of namespace sim
} // end of namespace mars

#endif
