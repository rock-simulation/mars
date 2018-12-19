
#ifndef NODEIMUSENSOR_H
#define NODEIMUSENSOR_H

#ifdef _PRINT_HEADER_
#warning "NodeIMUSensor.h"
#endif

#include "NodeArraySensor.h"
#include <mars/data_broker/DataPackage.h>
//#include <mars/data_broker/DataPackageMapping.h>
#include <mars/data_broker/ProducerInterface.h>


namespace mars{
  namespace sim{

    class NodeIMUSensor : public NodeArraySensor,
                          public data_broker::ProducerInterface {

    public:
      NodeIMUSensor(interfaces::ControlCenter *control, IDListConfig config);
      ~NodeIMUSensor(void);

      virtual int getAsciiData(char* data) const;
      virtual int getSensorData(interfaces::sReal** data) const;

      virtual void receiveData(const data_broker::DataInfo &info,const data_broker::DataPackage &package, int callbackParam);
      virtual void produceData(const data_broker::DataInfo &info,
                               data_broker::DataPackage *package,
                               int callbackParam);


      static interfaces::BaseSensor* instanciate(interfaces::ControlCenter *control, interfaces::BaseConfig *config);

    private:
      std::vector<utils::Vector> values_ang;
      std::vector<utils::Vector> values_lin;
      utils::Vector g;
      std::vector<utils::Quaternion> quaternion;
      std::vector<utils::Vector> a_body;
      std::vector<utils::Vector> w_body;
      long dataIndices[10];
      int pushToDataBroker;
      // Data broker
      //data_broker::DataPackageMapping packageMapping;
      //void setupDataPackageMapping();
    };
  }
}

#endif
