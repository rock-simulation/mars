#include "NodeIMUSensor.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>


//TODO Reduce functionality to single node sensor

namespace mars{
  namespace sim{

    using namespace utils;
    using namespace interfaces;

    BaseSensor* NodeIMUSensor::instanciate(ControlCenter *control,
                                                BaseConfig *config ){

      IDListConfig *cfg = dynamic_cast<IDListConfig*>(config);
      assert(cfg);
      return new NodeIMUSensor(control, *cfg);
    }

    NodeIMUSensor::NodeIMUSensor(ControlCenter *control,
                                           IDListConfig config):
      NodeArraySensor(control, config, false) {

      int i = 0;
      for(i = 0; i<countIDs;i++){
         values_ang.push_back(Vector(0.0, 0.0, 0.0));
         values_lin.push_back(Vector(0.0, 0.0, 0.0));
         a_body.push_back(Vector(0.0, 0.0, 0.0));
         w_body.push_back(Vector(0.0, 0.0, 0.0));
         quaternion.push_back(Quaternion(1,0,0,0));
      }

      for(i=0; i<6; ++i) dataIndices[i] = -1;

      typeName = "NodeIMU";

      pushToDataBroker = 1;

      data_broker::DataPackage dbPackage;

      dbPackage.add("linearAcceleration/x", 0.0);
      dbPackage.add("linearAcceleration/y", 0.0);
      dbPackage.add("linearAcceleration/z", 0.0);

      dbPackage.add("angularVelocity/x", 0.0);
      dbPackage.add("angularVelocity/y", 0.0);
      dbPackage.add("angularVelocity/z", 0.0);

      std::string groupName = "mars_sim";
      std::string dataName = "Sensors/"+name;
      control->dataBroker->pushData(groupName, dataName,
                                    dbPackage, NULL,
                                    data_broker::DATA_PACKAGE_READ_FLAG);
      control->dataBroker->registerTimedProducer(this, groupName, dataName,
                                                 "mars_sim/simTimer",
                                                   updateRate);
    }

    NodeIMUSensor::~NodeIMUSensor(){
      std::string groupName = "mars_sim";
      std::string dataName = "Sensors/"+name;
      control->dataBroker->unregisterTimedProducer(this, groupName,
                                                   dataName,
                                                   "mars_sim/simTimer");
    }


    int NodeIMUSensor::getAsciiData(char* data) const{
      char *p;
      int num_char = 0;
      std::vector<Vector>::const_iterator iter_ang;
      std::vector<Vector>::const_iterator iter_lin;

      p = data;
      for(iter_ang = values_ang.begin(); iter_ang!= values_ang.end(); iter_ang++){
        sprintf(p, "%10.4f %10.4f %10.4f", iter_ang->x(), iter_ang->y(), iter_ang->z());
        num_char += 33;
        p += num_char;
      }

      for(iter_lin = values_lin.begin(); iter_lin!= values_lin.end(); iter_lin++){
        sprintf(p, "%10.4f %10.4f %10.4f", iter_lin->x(), iter_lin->y(), iter_lin->z());
        num_char += 33;
        p += num_char;
      }

      return num_char;
    }

    int NodeIMUSensor::getSensorData(sReal** data) const{
      std::vector<Vector>::const_iterator iter_ang;
      std::vector<Vector>::const_iterator iter_lin;
      int i=0;

      *data = (sReal*)calloc(6*values_ang.size(), sizeof(sReal));
      for(iter_ang= values_ang.begin(); iter_ang!= values_ang.end(); iter_ang++){
        (*data)[i++] = iter_ang->x();
        (*data)[i++] = iter_ang->y();
        (*data)[i++] = iter_ang->z();
      }

      for(iter_lin= values_lin.begin(); iter_lin!= values_lin.end(); iter_lin++){
        (*data)[i++] = iter_lin->x();
        (*data)[i++] = iter_lin->y();
        (*data)[i++] = iter_lin->z();
      }

      return i;
    }

    void NodeIMUSensor::receiveData(const data_broker::DataInfo &info, const data_broker::DataPackage &package, int callbackParam){
      // Get the gravity
      g= control->sim->getGravity();


      if(dataIndices[0] == -1){

        dataIndices[0] = package.getIndexByName("angularVelocity/x");
        dataIndices[1] = package.getIndexByName("angularVelocity/y");
        dataIndices[2] = package.getIndexByName("angularVelocity/z");

        dataIndices[3] = package.getIndexByName("linearAcceleration/x");
        dataIndices[4] = package.getIndexByName("linearAcceleration/y");
        dataIndices[5] = package.getIndexByName("linearAcceleration/z");

        dataIndices[6] = package.getIndexByName("rotation/x");
        dataIndices[7] = package.getIndexByName("rotation/y");
        dataIndices[8] = package.getIndexByName("rotation/z");
        dataIndices[9] = package.getIndexByName("rotation/w");


      }

      //TODO Extend dataIndices for quaternion

      // a_IMU = Frame^R_World * (g - a_Body)
      // Read orientation via package.get
      // control->sim->getGravity for grav
      // add grav to linearAcceleration -> whole measurement
      // transform into BodyCOS with array *= quaternion




      package.get(dataIndices[0], &w_body[callbackParam].x());
      package.get(dataIndices[1], &w_body[callbackParam].y());
      package.get(dataIndices[2], &w_body[callbackParam].z());

      package.get(dataIndices[3], &a_body[callbackParam].x());
      package.get(dataIndices[4], &a_body[callbackParam].y());
      package.get(dataIndices[5], &a_body[callbackParam].z());

      package.get(dataIndices[6], &quaternion[callbackParam].x());
      package.get(dataIndices[7], &quaternion[callbackParam].y());
      package.get(dataIndices[8], &quaternion[callbackParam].z());
      package.get(dataIndices[9], &quaternion[callbackParam].w());


      values_lin[callbackParam] = quaternion[callbackParam]*(g-a_body[callbackParam]);
      values_ang[callbackParam] = quaternion[callbackParam]*w_body[callbackParam];
    }

    void NodeIMUSensor::produceData(const data_broker::DataInfo &info,
                                         data_broker::DataPackage *package,
                                         int callbackParam) {
      if(values_ang.size() == 0 or values_lin.size() == 0) return;

      package->set(0, values_lin[callbackParam].x());
      package->set(1, values_lin[callbackParam].y());
      package->set(2, values_lin[callbackParam].z());

      package->set(3, values_ang[callbackParam].x());
      package->set(4, values_ang[callbackParam].y());
      package->set(5, values_ang[callbackParam].z());
    }

  } // end of namespace sim
} // end of namespace mars
