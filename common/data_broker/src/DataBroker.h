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

#ifndef DATABROKER_H
#define DATABROKER_H

#ifdef _PRINT_HEADER_
  #warning "DataBroker.h"
#endif

#include "DataBrokerInterface.h"
#include "DataPackage.h"
#include "DataItem.h"
#include "DataInfo.h"

#include <mars/lib_manager/LibManager.h>

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>

#include <pthread.h>

namespace mars {

  namespace data_broker {

    class ReceiverInterface;
    class ProducerInterface;
    struct DataElement;

    inline bool hasWildcards(const std::string &str) {
      return (str.find("*") != str.npos);
    }
    /**
     * \brief basic pattern matching function
     * \param pattern The pattern that should be found in \a str.
     * \param str The string in which to search for the \a pattern.
     * \returns \c true if pattern is found in str. \c false otherwise
     *
     * The pattern may contain the wildcard "*" (asterisk) to mean zero or more
     * of any character. Other wildcards are currently not supported.
     * Examples:
     *   - match("foo", "foo") -> true
     *   - match("foo", "fo") -> false
     *   - match("foo*", "foo") -> true
     *   - match("foo*", "foobar") -> true
     *   - match("foo*", "what is foo") -> false
     *   - match("*wh*is*foo*", "what is foo") -> true
     *
     * \note Escaping wildcards is currently not supported.
     */
    bool match(const std::string &pattern, const std::string &str);

    /// \cond HIDDEN_SYMBOLS  
    struct PendingRegistration {
      ReceiverInterface *receiver;
      std::string groupName;
      std::string dataName;
      int callbackParam;
    };

    struct PendingTimedProducer {
      ProducerInterface *producer;
      std::string groupName;
      std::string dataName;
      std::string timerName;
      int updatePeriod;
      int callbackParam;
    };

    struct PendingTimedRegistration {
      ReceiverInterface *receiver;
      std::string groupName;
      std::string dataName;
      std::string timerName;
      int updatePeriod;
      int callbackParam;
    };

    struct PendingTriggeredRegistration {
      ReceiverInterface *receiver;
      std::string groupName;
      std::string dataName;
      std::string triggerName;
      int callbackParam;
    };

    struct TimedReceiver {
      ReceiverInterface *receiver;
      DataElement *element;
      int updatePeriod;
      long nextTriggerTime;
      int callbackParam;
    };

    struct TimedProducer {
      ProducerInterface *producer;
      DataElement *element;
      int updatePeriod;
      long nextTriggerTime;
      int callbackParam;
    };

    struct Timer {
      long t;
      std::list<TimedProducer> producers;
      std::list<TimedReceiver> receivers;
      pthread_rwlock_t lock;
      unsigned long timerElementId;
    };

    struct TriggeredReceiver {
      ReceiverInterface *receiver;
      DataElement *element;
      int callbackParam;
    };

    struct Trigger {
      std::list<TriggeredReceiver> receivers;
      pthread_rwlock_t lock;
    };

    struct Receiver {
      ReceiverInterface *receiver;
      int callbackParam;
    };

    struct DataElement {
      DataInfo info;
      //    bool updated;
      DataPackage *backBuffer;
      DataPackage *frontBuffer;
      std::list<Receiver> syncReceivers;
      std::list<Receiver> asyncReceivers;
      pthread_rwlock_t bufferLock;
      pthread_rwlock_t receiverLock;
      const ReceiverInterface *lastProducer;
      std::list<DataItemConnection> connections;
    };
    /// \endcond

    class DataBroker : public mars::lib_manager::LibInterface,
                       public DataBrokerInterface {

    public:
      DataBroker(mars::lib_manager::LibManager *theManager);
      virtual ~DataBroker();

      // LibInterface methods
      int getLibVersion() const {return 1;}
      const std::string getLibName() const {return std::string("data_broker");}

      bool createTimer(const std::string &timerName);
      /**
       * @return true if the timer was stepped.
       *         false if no timer with the given name exists.
       */
      bool stepTimer(const std::string &timerName, long step=1);
      bool registerTimedReceiver(ReceiverInterface *receiver,
                                 const std::string &groupName,
                                 const std::string &dataName,
                                 const std::string &timerName,
                                 int updatePeriod,
                                 int callbackParam=0);
      bool unregisterTimedReceiver(ReceiverInterface *receiver,
                                   const std::string &groupName,
                                   const std::string &dataName,
                                   const std::string &timerName);
      bool registerTimedProducer(ProducerInterface *producer,
                                 const std::string &groupName,
                                 const std::string &dataName,
                                 const std::string &timerName,
                                 int updatePeriod,
                                 int callbackParam=0);
      bool unregisterTimedProducer(ProducerInterface *producer,
                                   const std::string &groupName,
                                   const std::string &dataName,
                                   const std::string &timerName);

      bool createTrigger(const std::string &triggerName);
      bool trigger(const std::string &triggerName);
      bool registerTriggeredReceiver(ReceiverInterface *receiver,
                                     const std::string &groupName,
                                     const std::string &dataName,
                                     const std::string &triggerName,
                                     int callbackParam=0);
      bool unregisterTriggeredReceiver(ReceiverInterface *receiver,
                                       const std::string &groupName,
                                       const std::string &dataName,
                                       const std::string &triggerName);

      bool registerSyncReceiver(ReceiverInterface *receiver,
                                const std::string &groupName,
                                const std::string &dataName,
                                int callbackParam=0);
      bool unregisterSyncReceiver(ReceiverInterface *receiver,
                                  const std::string &groupName,
                                  const std::string &dataName);

      bool registerAsyncReceiver(ReceiverInterface *receiver,
                                 const std::string &groupName,
                                 const std::string &dataName,
                                 int callbackParam=0);
      bool unregisterAsyncReceiver(ReceiverInterface *receiver,
                                   const std::string &groupName,
                                   const std::string &dataName);

      unsigned long pushData(const std::string &groupName,
                             const std::string &dataName,
                             const DataPackage &dataPackage,
                             const ReceiverInterface *producer,
                             PackageFlag flags);
      unsigned long pushData(unsigned long id,
                             const DataPackage &dataPackage,
                             const ReceiverInterface *producer=NULL);

      unsigned long getDataID(const std::string &groupName,
                              const std::string &dataName) const;

      const DataInfo getDataInfo(const std::string &groupName,
                                 const std::string &dataName) const;
      const DataPackage getDataPackage(unsigned long id) const;

      const std::vector<DataInfo> getDataList(PackageFlag flag) const;

    
      void connectDataItems(const std::string &fromGroupName,
                            const std::string &fromDataName,
                            const std::string &fromItemName,
                            const std::string &toGroupName,
                            const std::string &toDataName,
                            const std::string &toItemName);
      void disconnectDataItems(const std::string &fromGroupName,
                               const std::string &fromDataName,
                               const std::string &fromItemName,
                               const std::string &toGroupName,
                               const std::string &toDataName,
                               const std::string &toItemName);
      void disconnectDataItems(const std::string &toGroupName,
                               const std::string &toDataName,
                               const std::string &toItemName);


      void run(void);
      void runRealtime(void);
      inline void setThreadStopped(bool val) {thread_running = !val;}
      inline void setRTThreadStopped(bool val) {
        realtimeThreadRunning = !val;
        startingRealtimeThread = false;
      }
      inline void lockRealtimeMutex() {
        pthread_mutex_lock(&realtimeMutex);
      }
      inline void unlockRealtimeMutex() {
        pthread_mutex_unlock(&realtimeMutex);
      }

      virtual void pushMessage(MessageType messageType,
                               const std::string &format, ...);
      virtual void pushFatal(const std::string &format, ...);
      virtual void pushError(const std::string &format, ...);
      virtual void pushWarning(const std::string &format, ...);
      virtual void pushInfo(const std::string &format, ...);
      virtual void pushDebug(const std::string &format, ...);

    private:
      DataElement *createDataElement(const std::string &groupName,
                                     const std::string &dataName,
                                     PackageFlag flags);
      void publishDataElement(const DataElement *element);
      void updatePendingRegistrations(DataElement *newElement);
      unsigned long createId();
      void destroyLock(pthread_rwlock_t *rwlock);
      void destroyLock(pthread_mutex_t *mutex);
      void destroyLock(pthread_cond_t *cond);
      DataElement *getElement(const std::string &groupName,
                              const std::string &dataName) const;
      void pushMessage(MessageType messageType,
                       const std::string &format, va_list args);

      /**
       * Get all DataElements that match groupName and dataName.
       * They may contain wildcards.
       * \sa match
       */
      void getElementsByName(const std::string &groupName,
                             const std::string &dataName,
                             std::vector<DataElement*> *elements) const;

      std::set<DataElement*> *updatedElementsBackBuffer;
      std::set<DataElement*> *updatedElementsFrontBuffer;

      unsigned long next_id;
      pthread_t theThread;
      pthread_t realtimeThread;
      pthread_mutex_t idMutex;
      pthread_mutex_t realtimeMutex;
      bool thread_running, stop_thread;
      bool realtimeThreadRunning, stopRealtimeThread;
      bool startingRealtimeThread;

      std::list<PendingRegistration> pendingAsyncRegistrations;
      std::list<PendingRegistration> pendingSyncRegistrations;
      std::list<PendingTimedProducer> pendingTimedProducers;
      std::list<PendingTimedRegistration> pendingTimedRegistrations;
      std::list<PendingTriggeredRegistration> pendingTriggeredRegistrations;
      std::map<unsigned long, DataElement*> elementsById;
      std::map<std::string, Trigger> triggers;
      std::map<std::pair<std::string, std::string>, DataElement*> elementsByName;
      mutable pthread_rwlock_t elementsLock;
      pthread_rwlock_t timersLock;
      pthread_rwlock_t triggersLock;
      pthread_mutex_t updatedElementsLock;
      pthread_mutex_t pendingRegistrationLock;

      pthread_cond_t wakeupCondition;
      pthread_mutex_t wakeupMutex;
      std::map<std::string, Timer> timers;
      unsigned long newStreamId;
      unsigned long pushMessageIds[__DB_MESSAGE_TYPE_COUNT];
    }; // end of class definition DataBroker

  } // end of namespace data_broker

} // end of namespace mars

#endif // DATABROKER_H
