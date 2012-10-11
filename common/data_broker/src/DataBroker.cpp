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

/**
 * \file DataBroker.h
 * \author Malte Roemmermann, Lorenz Quack
 * \brief DataBroker A small library to manage data communication
 *        within a software framework.
 *
 * Version 0.2
 */

/*
 * FIXME:
 *  - the unregister stuff has potential problems.
 *    consider the AsyncReceivers. in the main loop they are cached for the
 *    deferred callbacks. if they then get unregistered and the object deleted
 *    we will crash when doing the actual deferred call!
 * TODO:
 *  - add buffer struct containing length and void* to DataItem union.
 *    the buffer content should be copied in the pushData() and kept until
 *    a new buffer is pushed to the receivers.
 *  - who should flip the double-buffer?
 */


#include "DataBroker.h"
#include "ProducerInterface.h"
#include "ReceiverInterface.h"

#include <mars/utils/MutexLocker.h>
#include <mars/utils/misc.h>
#include <cerrno>
#include <cstdio>
#include <cstdarg>



namespace mars {
  namespace data_broker {
    
    using namespace mars::utils;

    // Helper struct
    /// \cond HIDDEN_SYMBOLS
    struct DeferredCallback {
      std::list<Receiver> receivers;
      DataInfo info;
      DataPackage package;
      const ReceiverInterface *producer;
    };
    /// \endcond


    // C-function to be called by pthreads to start the thread
    static void* createDataBrokerThread(void *theObject) {
      ((DataBroker*)theObject)->setThreadStopped(false);
      ((DataBroker*)theObject)->run();
      ((DataBroker*)theObject)->setThreadStopped(true);
      pthread_exit(NULL);
      return 0;
    }

    // C-function to be called by pthreads to start the _REALTIME_ thread
    static void* createRealtimeThread(void *theObject) {
      ((DataBroker*)theObject)->lockRealtimeMutex();
      ((DataBroker*)theObject)->setRTThreadStopped(false);
      ((DataBroker*)theObject)->unlockRealtimeMutex();
      ((DataBroker*)theObject)->runRealtime();
      ((DataBroker*)theObject)->setRTThreadStopped(true);
      pthread_exit(NULL);
      return 0;
    }

    DataBroker::DataBroker(mars::lib_manager::LibManager *theManager) :
      mars::lib_manager::LibInterface(theManager),
      DataBrokerInterface(),
      next_id(1), thread_running(false), stop_thread(false),
      realtimeThreadRunning(false) {

      pthread_rwlock_init(&elementsLock, NULL);
      pthread_rwlock_init(&timersLock, NULL);
      pthread_rwlock_init(&triggersLock, NULL);
      pthread_mutex_init(&updatedElementsLock, NULL);
      pthread_mutex_init(&idMutex, NULL);
      pthread_mutex_init(&pendingRegistrationLock, NULL);
      pthread_mutex_init(&wakeupMutex, NULL);
      pthread_mutex_init(&realtimeMutex, NULL);
      pthread_cond_init(&wakeupCondition, NULL);

      updatedElementsBackBuffer = new std::set<DataElement*>;
      updatedElementsFrontBuffer = new std::set<DataElement*>;

      DataElement *e;
      e = createDataElement("data_broker", "newStream", DATA_PACKAGE_READ_FLAG);
      newStreamId = e->info.dataId;

      DataElement *fatalElement, *errorElement, *warningElement;
      DataElement *infoElement, *debugElement;
      
      fatalElement = createDataElement("_MESSAGES_", "fatal", 
                                       DATA_PACKAGE_READ_FLAG);
      pushMessageIds[DB_MESSAGE_TYPE_FATAL] = fatalElement->info.dataId;
      errorElement = createDataElement("_MESSAGES_", "error", 
                                       DATA_PACKAGE_READ_FLAG);
      pushMessageIds[DB_MESSAGE_TYPE_ERROR] = errorElement->info.dataId;
      warningElement = createDataElement("_MESSAGES_", "warning", 
                                         DATA_PACKAGE_READ_FLAG);
      pushMessageIds[DB_MESSAGE_TYPE_WARNING] = warningElement->info.dataId;
      infoElement = createDataElement("_MESSAGES_", "info", 
                                      DATA_PACKAGE_READ_FLAG);
      pushMessageIds[DB_MESSAGE_TYPE_INFO] = infoElement->info.dataId;
      debugElement = createDataElement("_MESSAGES_", "debug", 
                                       DATA_PACKAGE_READ_FLAG);
      pushMessageIds[DB_MESSAGE_TYPE_DEBUG] = debugElement->info.dataId;

      pthread_rwlock_wrlock(&elementsLock);
      publishDataElement(fatalElement);
      publishDataElement(errorElement);
      publishDataElement(warningElement);
      publishDataElement(infoElement);
      publishDataElement(debugElement);
      pthread_rwlock_unlock(&elementsLock);

      createTimer("_REALTIME_");

      pthread_create(&theThread, NULL, createDataBrokerThread, (void*)this);
    }

    DataBroker::~DataBroker() {
      stopRealtimeThread = true;
      stop_thread = true;
      if(pthread_mutex_trylock(&wakeupMutex) == 0) {
        pthread_cond_signal(&wakeupCondition);
        pthread_mutex_unlock(&wakeupMutex);
      }
      while(thread_running || realtimeThreadRunning) {
        msleep(10);
      }
      std::map<unsigned long, DataElement*>::iterator elementIt;
      std::map<std::string, Timer>::iterator timerIt;
      std::map<std::string, Trigger>::iterator triggerIt;
      // TODO: This cleanup code is not really perfect threadingwise
      pthread_rwlock_wrlock(&elementsLock);
      pthread_rwlock_wrlock(&timersLock);
      pthread_rwlock_wrlock(&triggersLock);
      pthread_mutex_lock(&updatedElementsLock);
      updatedElementsBackBuffer->clear();
      updatedElementsFrontBuffer->clear();
      delete updatedElementsBackBuffer;
      delete updatedElementsFrontBuffer;
      for(timerIt = timers.begin(); timerIt != timers.end(); ++timerIt) {
        destroyLock(&timerIt->second.lock);
      }
      timers.clear();
      for(triggerIt = triggers.begin();
          triggerIt != triggers.end(); ++triggerIt) {
        destroyLock(&triggerIt->second.lock);
      }
      triggers.clear();
      for(elementIt = elementsById.begin();
          elementIt != elementsById.end(); ++elementIt) {
        DataElement *element = elementIt->second;
        destroyLock(&element->receiverLock);
        destroyLock(&element->bufferLock);
        delete element->backBuffer;
        delete element->frontBuffer;
        delete element;
      }
      elementsById.clear();
      elementsByName.clear();
      pthread_mutex_unlock(&updatedElementsLock);
      pthread_rwlock_unlock(&triggersLock);
      pthread_rwlock_unlock(&timersLock);
      pthread_rwlock_unlock(&elementsLock);
      destroyLock(&realtimeMutex);
      destroyLock(&triggersLock);
      destroyLock(&timersLock);
      destroyLock(&elementsLock);
      destroyLock(&idMutex);
      destroyLock(&updatedElementsLock);
      destroyLock(&pendingRegistrationLock);
      destroyLock(&wakeupCondition);
      destroyLock(&wakeupMutex);
      fprintf(stderr, "\nDelete data_broker\n");
    }

    void DataBroker::destroyLock(pthread_rwlock_t *rwlock) {
      while((pthread_rwlock_destroy(rwlock) == -1) && (errno == EBUSY));
    }

    void DataBroker::destroyLock(pthread_mutex_t *mutex) {
      while((pthread_mutex_destroy(mutex) == -1) && (errno == EBUSY));
    }

    void DataBroker::destroyLock(pthread_cond_t *cond) {
      while((pthread_cond_destroy(cond) == -1) && (errno == EBUSY));
    }

    bool DataBroker::createTimer(const std::string &timerName) {
      std::map<std::string, Timer>::iterator timerIt;
      bool ok = false;
      bool advanceIterator = true;
      pthread_rwlock_wrlock(&timersLock);
      timerIt = timers.find(timerName);
      if(timerIt == timers.end()) {
        timers[timerName] = Timer();
        timers[timerName].t = 0;
        timers[timerName].receivers.clear();
        pthread_rwlock_init(&timers[timerName.c_str()].lock, NULL);
        ok = true;
        std::map<std::pair<std::string, std::string>, DataElement*>::iterator elementIt;

        DataElement *e = createDataElement("data_broker", "timers/" + timerName, 
                                           DATA_PACKAGE_READ_FLAG);
        timers[timerName].timerElementId = e->info.dataId;
        pthread_rwlock_wrlock(&elementsLock);
        publishDataElement(e);
        pthread_rwlock_unlock(&elementsLock);

        // check for pending timer registrations
        std::list<PendingTimedRegistration>::iterator pendingIt;
        pthread_mutex_lock(&pendingRegistrationLock);
        for(pendingIt = pendingTimedRegistrations.begin();
            pendingIt != pendingTimedRegistrations.end(); /* do nothing */) {
          advanceIterator = true;
          if(pendingIt->timerName == timerName) {
            pthread_rwlock_rdlock(&elementsLock);
            elementIt = elementsByName.find(std::make_pair(pendingIt->groupName,
                                                           pendingIt->dataName));
            if(elementIt == elementsByName.end()) {
              DataElement *element = elementIt->second;
              TimedReceiver timedReceiver = {pendingIt->receiver, element,
                                             pendingIt->updatePeriod,
                                             timerIt->second.t,
                                             pendingIt->callbackParam};
              pthread_rwlock_wrlock(&timerIt->second.lock);
              timerIt->second.receivers.push_back(timedReceiver);
              pthread_rwlock_unlock(&timerIt->second.lock);
              pendingIt = pendingTimedRegistrations.erase(pendingIt);
              advanceIterator = false;
            }
            pthread_rwlock_unlock(&elementsLock);
          }
          if(advanceIterator) {
            ++pendingIt;
          }
        }

        // check for pending timed producers
        std::list<PendingTimedProducer>::iterator pendingProducerIt;
        for(pendingProducerIt = pendingTimedProducers.begin();
            pendingProducerIt != pendingTimedProducers.end(); /* do nothing */) {
          if(pendingProducerIt->timerName == timerName) {
            pthread_rwlock_rdlock(&elementsLock); // ToDo: missing unlock
            elementIt = elementsByName.find(std::make_pair(pendingProducerIt->groupName,
                                                           pendingProducerIt->dataName));
            DataElement *element;
            if(elementIt != elementsByName.end()) {
              element = elementIt->second;
            } else {
              element = createDataElement(pendingProducerIt->groupName,
                                          pendingProducerIt->dataName,
                                          DATA_PACKAGE_NO_FLAG);
            }
            TimedProducer timedProducer = {pendingProducerIt->producer, element,
                                           pendingProducerIt->updatePeriod,
                                           timerIt->second.t,
                                           pendingProducerIt->callbackParam};
            pthread_rwlock_wrlock(&timerIt->second.lock);
            timerIt->second.producers.push_back(timedProducer);
            pthread_rwlock_unlock(&timerIt->second.lock);
            pendingProducerIt = pendingTimedProducers.erase(pendingProducerIt);
          } else {
            ++pendingProducerIt;
          }
        }

        pthread_mutex_unlock(&pendingRegistrationLock);
      }
      
      pthread_rwlock_unlock(&timersLock);
      return ok;
    }
    
    bool DataBroker::stepTimer(const std::string &timerName, long step) {
      std::map<std::string, Timer>::iterator timerIt, endIt;
      std::list<DeferredCallback> deferredCallbacks;
      std::set<DataItemConnection> activeConnections;
      std::set<DataElement*> connectionActivatedElements;
      DataItem currentItem;

      //bool ok = false;
      //fprintf(stderr, "db::stepTimer\n");
      pthread_rwlock_rdlock(&timersLock);
      timerIt = timers.find(timerName);
      // The use of an iterator should be thread-safe.
      // The timers.end() call might not be.
      endIt = timers.end();
      pthread_rwlock_unlock(&timersLock);
      if(timerIt == endIt) {
        return false;
      }
      //ok = true;
      pthread_rwlock_wrlock(&timerIt->second.lock);
      timerIt->second.t += step;
      // call all producers
      /*
        fprintf(stderr, "db::stepTimer: producerSize: %d\n",
        timerIt->second.producers.size());
        fprintf(stderr, "db::stepTimer: receiverSize: %d\n",
        timerIt->second.receivers.size());
      */
      DeferredCallback deferredCallback;
      std::list<TimedProducer>::iterator producerIt;
      for(producerIt = timerIt->second.producers.begin();
          producerIt != timerIt->second.producers.end();
          ++producerIt) {
        if(producerIt->nextTriggerTime <= timerIt->second.t) {
          while(producerIt->updatePeriod > 0 &&
                producerIt->nextTriggerTime <= timerIt->second.t) {
            producerIt->nextTriggerTime += producerIt->updatePeriod;
          }
          DataElement *element = producerIt->element;

          deferredCallback.receivers.clear();

          pthread_rwlock_wrlock(&element->bufferLock);
          producerIt->producer->produceData(element->info,
                                            element->backBuffer,
                                            producerIt->callbackParam);
          std::swap(element->backBuffer, element->frontBuffer);
          pthread_rwlock_rdlock(&element->receiverLock);
          if(!element->syncReceivers.empty()) {
            deferredCallback.package = *element->frontBuffer;
            deferredCallback.info = element->info;
            deferredCallback.producer = NULL;
            deferredCallback.receivers = element->syncReceivers;
          }
          std::list<DataItemConnection>::iterator connectionIt;
          for(connectionIt = element->connections.begin(); 
              connectionIt != element->connections.end(); ++connectionIt) {
            long fromIdx = connectionIt->fromDataItemIndex;
            long toIdx = connectionIt->toDataItemIndex;
            currentItem = (*connectionIt->fromElement->frontBuffer)[fromIdx];
            currentItem.setName((*connectionIt->toElement->backBuffer)[toIdx].getName());
            (*connectionIt->toElement->frontBuffer)[toIdx] = currentItem;
            connectionActivatedElements.insert(connectionIt->toElement);
          }
          pthread_rwlock_unlock(&element->receiverLock);
          pthread_rwlock_unlock(&element->bufferLock);

          pthread_mutex_lock(&updatedElementsLock);
          updatedElementsBackBuffer->insert(element);
          pthread_mutex_unlock(&updatedElementsLock);

          // defer synchronous callbacks until we do not hold any locks anymore
          if(!deferredCallback.receivers.empty())
            deferredCallbacks.push_back(deferredCallback);
        }
      }

      // push time package
      DataPackage p;
      p.add("t", timerIt->second.t);
      pushData(timerIt->second.timerElementId, p);

      // defer receivers
      long time = timerIt->second.t;
      std::list<TimedReceiver> deferredReceivers;
      std::list<TimedReceiver>::iterator timedReceiverIt;

      for(timedReceiverIt = timerIt->second.receivers.begin();
          timedReceiverIt != timerIt->second.receivers.end();
          ++timedReceiverIt) {
        if(timedReceiverIt->nextTriggerTime <= time) {
          while(timedReceiverIt->updatePeriod > 0 &&
                timedReceiverIt->nextTriggerTime <= time) {
            timedReceiverIt->nextTriggerTime += timedReceiverIt->updatePeriod;
          }
          deferredReceivers.push_back(*timedReceiverIt);
        }
      }

      pthread_rwlock_unlock(&timerIt->second.lock);

      // call all deferred receivers
      for(timedReceiverIt = deferredReceivers.begin();
          timedReceiverIt != deferredReceivers.end();
          ++timedReceiverIt) {
        DataElement *element = timedReceiverIt->element;
        pthread_rwlock_rdlock(&element->bufferLock);
        timedReceiverIt->receiver->receiveData(element->info,
                                               *element->frontBuffer,
                                               timedReceiverIt->callbackParam);
        pthread_rwlock_unlock(&element->bufferLock);
      }

      // connections
      std::set<DataElement*>::iterator toElementIt;
      fflush(stderr);
      for(toElementIt = connectionActivatedElements.begin(); 
          toElementIt != connectionActivatedElements.end(); ++toElementIt) {
        DataElement *toElement = *toElementIt;
        //      std::swap(toElement->frontBuffer, toElement->backBuffer);
        pushData(toElement->info.dataId, *toElement->frontBuffer);
      }
      // call deferred sync callbacks
      std::list<DeferredCallback>::iterator callbackIt;
      std::list<Receiver>::iterator receiverIt;
      for(callbackIt = deferredCallbacks.begin();
          callbackIt != deferredCallbacks.end();
          ++callbackIt) {
        for(receiverIt = callbackIt->receivers.begin();
            receiverIt != callbackIt->receivers.end();
            ++receiverIt) {
          receiverIt->receiver->receiveData(callbackIt->info,
                                            callbackIt->package,
                                            receiverIt->callbackParam);
        }
      }

      return true;
    }
    
    bool DataBroker::registerTimedReceiver(ReceiverInterface *receiver,
                                           const std::string &groupName,
                                           const std::string &dataName,
                                           const std::string &timerName,
                                           int updatePeriod,
                                           int callbackParam) {
      std::map<std::string, Timer>::iterator timerIt, endIt;
      std::map<std::pair<std::string, std::string>, DataElement*>::iterator elementIt;
      bool ok = false;
      
      pthread_rwlock_rdlock(&timersLock);
      timerIt = timers.find(timerName);
      endIt = timers.end();
      pthread_rwlock_unlock(&timersLock);
      if(timerIt != endIt) {
        pthread_rwlock_rdlock(&elementsLock);
        elementIt = elementsByName.find(std::make_pair(groupName, dataName));
        if(elementIt != elementsByName.end()) {
          DataElement *element = elementIt->second;
          TimedReceiver timedReceiver = {receiver, element, updatePeriod,
                                         timerIt->second.t, callbackParam};
          pthread_rwlock_wrlock(&timerIt->second.lock);
          timerIt->second.receivers.push_back(timedReceiver);
          pthread_rwlock_unlock(&timerIt->second.lock);
          ok = true;
          if(timerName == "_REALTIME_") {
            lockRealtimeMutex();
            stopRealtimeThread = false;
            if(!startingRealtimeThread) {
              startingRealtimeThread = true;
              if(!realtimeThreadRunning) {
                pthread_create(&realtimeThread, NULL, createRealtimeThread,
                               (void*)this);
              }
            }
            unlockRealtimeMutex();
          }
        }
        pthread_rwlock_unlock(&elementsLock);
      }
      // if there was a problem add to pending receivers
      if(!ok) {
        PendingTimedRegistration tmp;
        tmp.receiver = receiver;
        tmp.groupName = groupName.c_str();
        tmp.dataName = dataName.c_str();
        tmp.timerName = timerName.c_str();
        tmp.updatePeriod = updatePeriod;
        tmp.callbackParam = callbackParam;
        pthread_mutex_lock(&pendingRegistrationLock);
        pendingTimedRegistrations.push_back(tmp);
        pthread_mutex_unlock(&pendingRegistrationLock);
      }
      return ok;
    }

    bool DataBroker::unregisterTimedReceiver(ReceiverInterface *receiver,
                                             const std::string &groupName,
                                             const std::string &dataName,
                                             const std::string &timerName) {
      std::map<std::string, Timer>::iterator timerIt, endIt;
      std::list<TimedReceiver>::iterator receiverIt;
      bool ok = false;
      pthread_rwlock_rdlock(&timersLock);
      timerIt = timers.find(timerName);
      endIt = timers.end();
      pthread_rwlock_unlock(&timersLock);
      if(timerIt != endIt) {
        pthread_rwlock_wrlock(&timerIt->second.lock);
        for(receiverIt = timerIt->second.receivers.begin();
            receiverIt != timerIt->second.receivers.end(); /* do nothing */){
          if(receiverIt->receiver == receiver) {
            receiverIt = timerIt->second.receivers.erase(receiverIt);
            ok = true;
          } else {
            ++receiverIt;
          }
        }
        if(timerName == "_REALTIME_" &&
           timerIt->second.receivers.empty() &&
           timerIt->second.producers.empty()) {
          stopRealtimeThread = true;
        }
        pthread_rwlock_unlock(&timerIt->second.lock);
      }
      // remove receiver from pendingTimedRegistration list
      std::list<PendingTimedRegistration>::iterator pendingIt;
      pthread_mutex_lock(&pendingRegistrationLock);
      for(pendingIt = pendingTimedRegistrations.begin();
          pendingIt != pendingTimedRegistrations.end(); /* do nothing */) {
        if((pendingIt->timerName == timerName) &&
           matchPattern(groupName, pendingIt->groupName) &&
           matchPattern(dataName, pendingIt->dataName)) {
          pendingIt = pendingTimedRegistrations.erase(pendingIt);
        } else {
          ++pendingIt;
        }
      }
      pthread_mutex_unlock(&pendingRegistrationLock);
      return ok;
    }

    bool DataBroker::registerTimedProducer(ProducerInterface *producer,
                                           const std::string &groupName,
                                           const std::string &dataName,
                                           const std::string &timerName,
                                           int updatePeriod,
                                           int callbackParam) {
      std::map<std::string, Timer>::iterator timerIt, endIt;
      std::map<std::pair<std::string, std::string>, DataElement*>::iterator elementIt;
      bool ok = false;
      pthread_rwlock_rdlock(&timersLock);
      timerIt = timers.find(timerName);
      endIt = timers.end();
      pthread_rwlock_unlock(&timersLock);
      if(timerIt != endIt) {
        pthread_rwlock_rdlock(&elementsLock);
        elementIt = elementsByName.find(std::make_pair(groupName, dataName));
        DataElement *element = NULL;
        if(elementIt == elementsByName.end()) {
          element = createDataElement(groupName, dataName, DATA_PACKAGE_NO_FLAG);
        } else {
          element = elementIt->second;
        }
        TimedProducer timedProducer = {producer, element, updatePeriod,
                                       timerIt->second.t, callbackParam};
        pthread_rwlock_wrlock(&timerIt->second.lock);
        timerIt->second.producers.push_back(timedProducer);
        pthread_rwlock_unlock(&timerIt->second.lock);
        pthread_rwlock_unlock(&elementsLock);
        ok = true;
        if(timerName == "_REALTIME_") {
          stopRealtimeThread = false;
          startingRealtimeThread = true;
          if(!realtimeThreadRunning) {
            pthread_create(&realtimeThread, NULL, createRealtimeThread, 
                           (void*)this);
          }
        }
      }
      // if there was a problem add to pending receivers
      if(!ok) {
        PendingTimedProducer tmp;
        tmp.producer = producer;
        tmp.groupName = groupName.c_str();
        tmp.dataName = dataName.c_str();
        tmp.timerName = timerName.c_str();
        tmp.updatePeriod = updatePeriod;
        tmp.callbackParam = callbackParam;
        pthread_mutex_lock(&pendingRegistrationLock);
        pendingTimedProducers.push_back(tmp);
        pthread_mutex_unlock(&pendingRegistrationLock);
      }
      return ok;
    }
    
    bool DataBroker::unregisterTimedProducer(ProducerInterface *producer,
                                             const std::string &groupName,
                                             const std::string &dataName,
                                             const std::string &timerName) {
      std::map<std::string, Timer>::iterator timerIt, endIt;
      std::list<TimedProducer>::iterator producerIt;
      bool ok = false;
      pthread_rwlock_rdlock(&timersLock);
      timerIt = timers.find(timerName);
      endIt = timers.end();
      pthread_rwlock_unlock(&timersLock);
      if(timerIt != endIt) {
        pthread_rwlock_wrlock(&timerIt->second.lock);
        for(producerIt = timerIt->second.producers.begin();
            producerIt != timerIt->second.producers.end(); /* do nothing */) {
          if(producerIt->producer == producer) {
            producerIt = timerIt->second.producers.erase(producerIt);
            ok = true;
          } else {
            ++producerIt;
          }
        }
        if(timerName == "_REALTIME_" &&
           timerIt->second.receivers.empty() &&
           timerIt->second.producers.empty()) {
          stopRealtimeThread = true;
        }
        pthread_rwlock_unlock(&timerIt->second.lock);
      }
      // remove producer from pendingTimedProducers list
      std::list<PendingTimedProducer>::iterator pendingIt;
      pthread_mutex_lock(&pendingRegistrationLock);
      for(pendingIt = pendingTimedProducers.begin();
          pendingIt != pendingTimedProducers.end(); /* do nothing */) {
        if((pendingIt->timerName == timerName) &&
           matchPattern(groupName, pendingIt->groupName) &&
           matchPattern(dataName, pendingIt->dataName)) {
          pendingIt = pendingTimedProducers.erase(pendingIt);
        } else {
          ++pendingIt;
        }
      }
      pthread_mutex_unlock(&pendingRegistrationLock);
      return ok;
    }


    bool DataBroker::createTrigger(const std::string &triggerName) {
      std::map<std::string, Trigger>::iterator triggerIt;
      bool ok = false;
      pthread_rwlock_wrlock(&triggersLock);
      triggerIt = triggers.find(triggerName);
      if(triggerIt == triggers.end()) {
        triggers[triggerName] = Trigger();
        triggers[triggerName].receivers.clear();
        pthread_rwlock_init(&triggers[triggerName].lock, NULL);
        ok = true;
      }
      // check for pending timer registrations
      std::map<std::pair<std::string, std::string>, DataElement*>::iterator elementIt;
      std::list<PendingTriggeredRegistration>::iterator pendingIt;
      pthread_mutex_lock(&pendingRegistrationLock);
      for(pendingIt = pendingTriggeredRegistrations.begin();
          pendingIt != pendingTriggeredRegistrations.end(); /* do nothing */) {
        if(pendingIt->triggerName == triggerName){
          pthread_rwlock_rdlock(&elementsLock);
          elementIt = elementsByName.find(std::make_pair(pendingIt->groupName,
                                                         pendingIt->dataName));
          if(elementIt != elementsByName.end()) {
            TriggeredReceiver triggeredReceiver = { pendingIt->receiver,
                                                    elementIt->second,
                                                    pendingIt->callbackParam };
            pthread_rwlock_wrlock(&triggerIt->second.lock);
            triggerIt->second.receivers.push_back(triggeredReceiver);
            pthread_rwlock_unlock(&triggerIt->second.lock);
            pendingIt = pendingTriggeredRegistrations.erase(pendingIt);
          } else {
            ++pendingIt;
          }
          pthread_rwlock_unlock(&elementsLock);
        }
      }
      pthread_mutex_unlock(&pendingRegistrationLock);
      pthread_rwlock_unlock(&triggersLock);
      return ok;
    }

    bool DataBroker::trigger(const std::string &triggerName) {
      std::map<std::string, Trigger>::iterator triggerIt, endIt;
      std::list<TriggeredReceiver>::iterator receiverIt;
      bool ok = false;
      pthread_rwlock_rdlock(&triggersLock);
      triggerIt = triggers.find(triggerName);
      endIt = triggers.end();
      pthread_rwlock_unlock(&triggersLock);
      if(triggerIt != endIt) {
        pthread_rwlock_rdlock(&triggerIt->second.lock);
        for(receiverIt = triggerIt->second.receivers.begin();
            receiverIt != triggerIt->second.receivers.end();
            ++receiverIt) {
          DataElement *element = receiverIt->element;
          pthread_rwlock_rdlock(&element->bufferLock);
          receiverIt->receiver->receiveData(element->info,
                                            *element->frontBuffer,
                                            receiverIt->callbackParam);
          pthread_rwlock_unlock(&element->bufferLock);
        }
        pthread_rwlock_unlock(&triggerIt->second.lock);
        ok = true;
      }
      return ok;
    }

    bool DataBroker::registerTriggeredReceiver(ReceiverInterface *receiver,
                                               const std::string &groupName,
                                               const std::string &dataName,
                                               const std::string &triggerName,
                                               int callbackParam) {
      std::map<std::string, Trigger>::iterator triggerIt, endIt;
      std::map<std::pair<std::string, std::string>, DataElement*>::iterator elementIt;
      bool ok = false;
      pthread_rwlock_rdlock(&triggersLock);
      triggerIt = triggers.find(triggerName);
      endIt = triggers.end();
      pthread_rwlock_unlock(&triggersLock);
      if(triggerIt != endIt) {
        pthread_rwlock_rdlock(&elementsLock);
        elementIt = elementsByName.find(std::make_pair(groupName, dataName));
        if(elementIt != elementsByName.end()) {
          TriggeredReceiver triggeredReceiver = { receiver, elementIt->second,
                                                  callbackParam };
          pthread_rwlock_wrlock(&triggerIt->second.lock);
          triggerIt->second.receivers.push_back(triggeredReceiver);
          pthread_rwlock_unlock(&triggerIt->second.lock);
          ok = true;
        }
        pthread_rwlock_unlock(&elementsLock);
      }
      return ok;
    }

    bool DataBroker::unregisterTriggeredReceiver(ReceiverInterface *receiver,
                                                 const std::string &groupName,
                                                 const std::string &dataName,
                                                 const std::string &triggerName) {
      std::map<std::string, Trigger>::iterator triggerIt, endIt;
      std::map<std::pair<std::string, std::string>, DataElement*>::iterator elementIt;
      std::list<TriggeredReceiver>::iterator receiverIt;
      bool ok = false;
      pthread_rwlock_rdlock(&triggersLock);
      triggerIt = triggers.find(triggerName);
      endIt = triggers.end();
      pthread_rwlock_unlock(&triggersLock);
      if(triggerIt != endIt) {
        pthread_rwlock_rdlock(&elementsLock);
        elementIt = elementsByName.find(std::make_pair(groupName, dataName));
        if(elementIt != elementsByName.end()) {
          pthread_rwlock_wrlock(&triggerIt->second.lock);
          for(receiverIt = triggerIt->second.receivers.begin();
              receiverIt != triggerIt->second.receivers.end(); /* do nothing */) {
            if((receiverIt->receiver == receiver) &&
               (receiverIt->element == elementIt->second)) {
              receiverIt = triggerIt->second.receivers.erase(receiverIt);
              ok = true;
            } else {
              ++receiverIt;
            }
          }
          pthread_rwlock_unlock(&triggerIt->second.lock);
        }
        pthread_rwlock_unlock(&elementsLock);
      }

      // remove receiver from PendingReceivers list
      std::list<PendingTriggeredRegistration>::iterator pendingIt;
      pthread_mutex_lock(&pendingRegistrationLock);
      for(pendingIt = pendingTriggeredRegistrations.begin();
          pendingIt != pendingTriggeredRegistrations.end(); /* do nothing */) {
        if((pendingIt->triggerName == triggerName) &&
           matchPattern(groupName, pendingIt->groupName) &&
           matchPattern(dataName, pendingIt->dataName)) {
          pendingIt = pendingTriggeredRegistrations.erase(pendingIt);
        } else {
          ++pendingIt;
        }
      }
      pthread_mutex_unlock(&pendingRegistrationLock);

      return ok;
    }

    bool DataBroker::registerSyncReceiver(ReceiverInterface *receiver,
                                          const std::string &groupName,
                                          const std::string &dataName,
                                          int callbackParam) {
      std::vector<DataElement*> elements;
      bool wildcards = hasWildcards(groupName) || hasWildcards(dataName);
      pthread_rwlock_rdlock(&elementsLock);
      getElementsByName(groupName, dataName, &elements);
      for(std::vector<DataElement*>::iterator elementIt = elements.begin();
          elementIt != elements.end(); ++elementIt){
        DataElement *element = *elementIt;
        Receiver r = { receiver, callbackParam };
        pthread_rwlock_wrlock(&element->receiverLock);
        element->syncReceivers.push_back(r);
        pthread_rwlock_unlock(&element->receiverLock);
      }
      if(wildcards || elements.empty()) {
        PendingRegistration tmp = { receiver, groupName.c_str(),
                                    dataName.c_str(), callbackParam };
        pthread_mutex_lock(&pendingRegistrationLock);
        pendingSyncRegistrations.push_back(tmp);
        pthread_mutex_unlock(&pendingRegistrationLock);
      }
      pthread_rwlock_unlock(&elementsLock);
      return (wildcards || !elements.empty());
    }

    bool DataBroker::unregisterSyncReceiver(ReceiverInterface *receiver,
                                            const std::string &groupName,
                                            const std::string &dataName) {
      std::vector<DataElement*> elements;
      std::list<Receiver>::iterator receiverIt;
      std::list<PendingRegistration>::iterator pendingRegistrationIt;
      pthread_rwlock_rdlock(&elementsLock);
      getElementsByName(groupName, dataName, &elements);
      int cnt = 0;
      for(std::vector<DataElement*>::iterator elementIt = elements.begin();
          elementIt != elements.end(); ++elementIt) {
        DataElement *element = *elementIt;
        pthread_rwlock_wrlock(&element->receiverLock);
        for(receiverIt = element->syncReceivers.begin();
            receiverIt != element->syncReceivers.end(); /* do nothing */) {
          if(receiverIt->receiver == receiver) {
            receiverIt = element->syncReceivers.erase(receiverIt);
            ++cnt;
          } else {
            ++receiverIt;
          }
        }
        pthread_rwlock_unlock(&element->receiverLock);
      }
      // remove from pending list
      pthread_mutex_lock(&pendingRegistrationLock);
      for(pendingRegistrationIt = pendingSyncRegistrations.begin();
          pendingRegistrationIt != pendingSyncRegistrations.end(); /*nothing*/) {
        if((pendingRegistrationIt->receiver == receiver) &&
           (matchPattern(groupName, pendingRegistrationIt->groupName)) &&
           (matchPattern(dataName, pendingRegistrationIt->dataName))) {
          pendingRegistrationIt = pendingSyncRegistrations.erase(pendingRegistrationIt);
        } else {
          ++pendingRegistrationIt;
        }
      }
      pthread_mutex_unlock(&pendingRegistrationLock);
      pthread_rwlock_unlock(&elementsLock);
      return cnt;
    }

    bool DataBroker::registerAsyncReceiver(ReceiverInterface *receiver,
                                           const std::string &groupName,
                                           const std::string &dataName,
                                           int callbackParam) {
      std::vector<DataElement*> elements;
      bool wildcards = hasWildcards(groupName) || hasWildcards(dataName);
      pthread_rwlock_rdlock(&elementsLock);
      getElementsByName(groupName, dataName, &elements);
      for(std::vector<DataElement*>::iterator elementIt = elements.begin();
          elementIt != elements.end(); ++elementIt){
        DataElement *element = *elementIt;
        Receiver r = { receiver, callbackParam };
        pthread_rwlock_wrlock(&element->receiverLock);
        element->asyncReceivers.push_back(r);
        pthread_rwlock_unlock(&element->receiverLock);
      }
      if(wildcards || elements.empty()) {
        PendingRegistration tmp = { receiver, groupName.c_str(),
                                    dataName.c_str(), callbackParam };
        pthread_mutex_lock(&pendingRegistrationLock);
        pendingAsyncRegistrations.push_back(tmp);
        pthread_mutex_unlock(&pendingRegistrationLock);
      }
      pthread_rwlock_unlock(&elementsLock);
      return (wildcards || !elements.empty());
    }

    bool DataBroker::unregisterAsyncReceiver(ReceiverInterface *receiver,
                                             const std::string &groupName,
                                             const std::string &dataName) {
      std::vector<DataElement*> elements;
      std::list<Receiver>::iterator receiverIt;
      std::list<PendingRegistration>::iterator pendingRegistrationIt;
      pthread_rwlock_rdlock(&elementsLock);
      getElementsByName(groupName, dataName, &elements);
      int cnt = 0;
      for(std::vector<DataElement*>::iterator elementIt = elements.begin();
          elementIt != elements.end(); ++elementIt) {
        DataElement *element = *elementIt;
        pthread_rwlock_wrlock(&element->receiverLock);
        for(receiverIt = element->asyncReceivers.begin();
            receiverIt != element->asyncReceivers.end(); /* do nothing */) {
          if(receiverIt->receiver == receiver) {
            receiverIt = element->asyncReceivers.erase(receiverIt);
            ++cnt;
          } else {
            ++receiverIt;
          }
        }
        pthread_rwlock_unlock(&element->receiverLock);
      }
      // remove from pending list
      pthread_mutex_lock(&pendingRegistrationLock);
      for(pendingRegistrationIt = pendingAsyncRegistrations.begin();
          pendingRegistrationIt != pendingAsyncRegistrations.end(); /*nothing*/) {
        if((pendingRegistrationIt->receiver == receiver) &&
           (matchPattern(groupName, pendingRegistrationIt->groupName)) &&
           (matchPattern(dataName, pendingRegistrationIt->dataName))) {
          pendingRegistrationIt = pendingAsyncRegistrations.erase(pendingRegistrationIt);
        } else {
          ++pendingRegistrationIt;
        }
      }
      pthread_mutex_unlock(&pendingRegistrationLock);
      pthread_rwlock_unlock(&elementsLock);
      return cnt;
    }

    unsigned long DataBroker::pushData(const std::string &groupName,
                                       const std::string &dataName,
                                       const DataPackage &dataPackage,
                                       const ReceiverInterface *producer,
                                       PackageFlag flags) {
      std::map<std::pair<std::string, std::string>, DataElement*>::iterator elementIt;
      DataElement *element = NULL;
      pthread_rwlock_wrlock(&elementsLock);
      elementIt = elementsByName.find(std::make_pair(groupName, dataName));
      if(elementIt != elementsByName.end()) {
        element = elementIt->second;
      } else {
        element = createDataElement(groupName, dataName, flags);
        publishDataElement(element);
      }
      pthread_rwlock_unlock(&elementsLock);

      pushData(element->info.dataId, dataPackage, producer);
      // hack to solve empty backBuffer problem while using producerCallbacks
      *element->backBuffer = dataPackage;

      return element->info.dataId;
    }

    unsigned long DataBroker::pushData(unsigned long id,
                                       const DataPackage &dataPackage,
                                       const ReceiverInterface *producer) {
      std::list<Receiver>::iterator syncReceiverIt;
      std::map<unsigned long, DataElement*>::iterator elementIt;
      std::set<DataElement*> connectionActivatedElements;
      std::list<Receiver> syncReceivers;
      DataInfo info;
      DataElement *element = NULL;
      pthread_rwlock_rdlock(&elementsLock);
      elementIt = elementsById.find(id);
      if(elementIt == elementsById.end()) {
        // ERROR: id not found!
        pthread_rwlock_unlock(&elementsLock);
        return 0;
      } else {
        element = elementIt->second;
        *element->backBuffer = dataPackage;
        pthread_rwlock_wrlock(&element->bufferLock);
        std::swap(element->backBuffer, element->frontBuffer);
        element->lastProducer = producer;
        pthread_rwlock_unlock(&element->bufferLock);

        pthread_mutex_lock(&updatedElementsLock);
        updatedElementsBackBuffer->insert(element);
        pthread_mutex_unlock(&updatedElementsLock);

        pthread_rwlock_rdlock(&element->receiverLock);
        // defer synchronous callbacks until we do not hold any locks anymore
        syncReceivers = element->syncReceivers;
        info = element->info;
        pthread_rwlock_unlock(&element->receiverLock);
        for(std::list<DataItemConnection>::iterator connectionIt = element->connections.begin(); connectionIt != element->connections.end(); ++connectionIt) {
          long fromIdx = connectionIt->fromDataItemIndex;
          long toIdx = connectionIt->toDataItemIndex;
          DataItem currentItem;
          currentItem = (*connectionIt->fromElement->frontBuffer)[fromIdx];
          currentItem.setName((*connectionIt->toElement->frontBuffer)[toIdx].getName());
          (*connectionIt->toElement->frontBuffer)[toIdx] = currentItem;
          connectionActivatedElements.insert(connectionIt->toElement);
        }
      }
      pthread_rwlock_unlock(&elementsLock);

      // do the synchronous callbacks
      for(syncReceiverIt = syncReceivers.begin();
          syncReceiverIt != syncReceivers.end();
          ++syncReceiverIt) {
        if(syncReceiverIt->receiver != producer)
          syncReceiverIt->receiver->receiveData(info, dataPackage,
                                                syncReceiverIt->callbackParam);
      }

      for(std::set<DataElement*>::iterator toElementIt = connectionActivatedElements.begin(); toElementIt != connectionActivatedElements.end(); ++toElementIt) {
        DataElement *toElement = *toElementIt;
        pushData(toElement->info.dataId, *toElement->frontBuffer);
      }

      // The main thread only releases the wakeupMutex when it goes to sleep.
      // So if we can lock it, we wake up the main thread so it can process the
      // data we just pushed. Otherwise it is still running and will process it.
      if(pthread_mutex_trylock(&wakeupMutex) == 0) {
        pthread_cond_signal(&wakeupCondition);
        pthread_mutex_unlock(&wakeupMutex);
      }
      return id;
    }

    void DataBroker::pushMessage(MessageType messageType,
                                 const std::string &format, va_list args) {
      const int MAX_BUFFER_SIZE = 1024;
      char buffer[MAX_BUFFER_SIZE];
      vsnprintf(buffer, MAX_BUFFER_SIZE-1, format.c_str(), args);
      DataPackage messagePackage;
      messagePackage.add("message", std::string(buffer));
      pushData(pushMessageIds[messageType], messagePackage);
    }

    void DataBroker::pushMessage(MessageType messageType,
                                 const std::string &format, ...) {
      va_list args;
      va_start (args, format);
      pushMessage(messageType, format, args);
      va_end (args);
    }

    void DataBroker::pushFatal(const std::string &format, ...) {
      va_list args;
      va_start(args, format);
      pushMessage(DB_MESSAGE_TYPE_FATAL, format, args);
      va_end(args);
    }

    void DataBroker::pushError(const std::string &format, ...) {
      va_list args;
      va_start(args, format);
      pushMessage(DB_MESSAGE_TYPE_ERROR, format, args);
      va_end(args);
    }

    void DataBroker::pushWarning(const std::string &format, ...) {
      va_list args;
      va_start(args, format);
      pushMessage(DB_MESSAGE_TYPE_WARNING, format, args);
      va_end(args);
    }

    void DataBroker::pushInfo(const std::string &format, ...) {
      va_list args;
      va_start(args, format);
      pushMessage(DB_MESSAGE_TYPE_INFO, format, args);
      va_end(args);
    }

    void DataBroker::pushDebug(const std::string &format, ...) {
      va_list args;
      va_start(args, format);
      pushMessage(DB_MESSAGE_TYPE_DEBUG, format, args);
      va_end(args);
    }

    void DataBroker::runRealtime() {
      long t = getTime();
      long dt;
      while(!stopRealtimeThread) {
        dt = getTimeDiff(t);
        stepTimer("_REALTIME_", dt);
        t += dt;
        msleep(10);
      }
    }

    void DataBroker::run() {
      std::set<DataElement*>::iterator updatedElementsIt;
      std::list<Receiver>::iterator receiverIt;
      std::list<DeferredCallback> deferredCallbacks;
      std::list<DeferredCallback>::iterator callbackIt;

      pthread_mutex_lock(&wakeupMutex);
      while(!stop_thread) {
        pthread_rwlock_rdlock(&elementsLock);
        pthread_mutex_lock(&updatedElementsLock);
        std::swap(updatedElementsBackBuffer, updatedElementsFrontBuffer);
        pthread_mutex_unlock(&updatedElementsLock);

        for(updatedElementsIt = updatedElementsFrontBuffer->begin();
            updatedElementsIt != updatedElementsFrontBuffer->end();
            ++updatedElementsIt) {
          DataElement *element = *updatedElementsIt;

          pthread_rwlock_rdlock(&element->bufferLock);
          pthread_rwlock_rdlock(&element->receiverLock);
          // defer callbacks until we do not hold any lock anymore
          if(!element->asyncReceivers.empty()) {
            DeferredCallback deferred;
            deferred.receivers = element->asyncReceivers;
            deferred.info = element->info;
            deferred.package = *element->frontBuffer;
            deferred.producer = element->lastProducer;
            deferredCallbacks.push_back(deferred);
          }
          pthread_rwlock_unlock(&element->receiverLock);
          pthread_rwlock_unlock(&element->bufferLock);
        }
        updatedElementsFrontBuffer->clear();
        pthread_rwlock_unlock(&elementsLock);

        // make the callbacks
        //pushError("DataBroker::deferredCallbacks %d", deferredCallbacks.size());
        for(callbackIt = deferredCallbacks.begin();
            callbackIt != deferredCallbacks.end(); ++callbackIt) {
          for(receiverIt = callbackIt->receivers.begin();
              receiverIt != callbackIt->receivers.end();
              ++receiverIt) {
            if(receiverIt->receiver != callbackIt->producer)
              receiverIt->receiver->receiveData(callbackIt->info,
                                                callbackIt->package,
                                                receiverIt->callbackParam);
          }
        }
        deferredCallbacks.clear();

        // If there is no data to process go to sleep. pushData() will wake us up.
        pthread_mutex_lock(&updatedElementsLock);
        bool bufferIsEmpty = updatedElementsBackBuffer->empty();
        pthread_mutex_unlock(&updatedElementsLock);
        if(bufferIsEmpty) {
          pthread_cond_wait(&wakeupCondition, &wakeupMutex);
        }
        msleep(10);
      }
      pthread_mutex_unlock(&wakeupMutex);
    }


    const std::vector<DataInfo> DataBroker::getDataList(PackageFlag flags) const {
      std::vector<DataInfo> dataList;
      std::map<unsigned long, DataElement*>::const_iterator it;
      pthread_rwlock_rdlock(&elementsLock);
      for(it = elementsById.begin(); it != elementsById.end(); ++it) {
        if(flags == DATA_PACKAGE_NO_FLAG || flags & it->second->info.flags) {
          dataList.push_back(it->second->info);
        }
      }
      pthread_rwlock_unlock(&elementsLock);
      return dataList;
    }


    const DataPackage DataBroker::getDataPackage(unsigned long id) const {
      DataPackage dataPackage;
      std::map<unsigned long, DataElement*>::const_iterator elementIt;
      pthread_rwlock_rdlock(&elementsLock);
      elementIt = elementsById.find(id);
      if(elementIt != elementsById.end()) {
        DataElement *element = elementIt->second;
        pthread_rwlock_rdlock(&element->bufferLock);
        dataPackage = *elementIt->second->frontBuffer;
        pthread_rwlock_unlock(&element->bufferLock);
      }
      pthread_rwlock_unlock(&elementsLock);
      return dataPackage;
    }

    unsigned long DataBroker::getDataID(const std::string &groupName,
                                        const std::string &dataName) const {
      std::map<std::pair<std::string, std::string>, DataElement*>::const_iterator elementIt;
      unsigned long id = 0;
      pthread_rwlock_rdlock(&elementsLock);
      elementIt = elementsByName.find(std::make_pair(groupName, dataName));
      if(elementIt != elementsByName.end()) {
        id = elementIt->second->info.dataId;
      }
      pthread_rwlock_unlock(&elementsLock);
      return id;
    }
    
    const DataInfo DataBroker::getDataInfo(const std::string &groupName,
                                           const std::string &dataName) const {
      std::map<std::pair<std::string, std::string>, DataElement*>::const_iterator elementIt;
      DataInfo info;
      pthread_rwlock_rdlock(&elementsLock);
      elementIt = elementsByName.find(std::make_pair(groupName, dataName));
      if(elementIt != elementsByName.end()) {
        info = elementIt->second->info;
      }
      pthread_rwlock_unlock(&elementsLock);
      return info;
    }

    unsigned long DataBroker::createId() {
      pthread_mutex_lock(&idMutex);
      unsigned long newId = next_id++;
      pthread_mutex_unlock(&idMutex);
      return newId;
    }


    DataElement *DataBroker::createDataElement(const std::string &groupName,
                                               const std::string &dataName,
                                               PackageFlag flags) {
      DataElement *element = new DataElement;
      element->info.dataId = createId();
      element->info.groupName = groupName.c_str();
      element->info.dataName = dataName.c_str();
      element->info.flags = flags;
      element->backBuffer = new DataPackage;
      element->frontBuffer = new DataPackage;
      pthread_rwlock_init(&element->bufferLock, NULL);
      pthread_rwlock_init(&element->receiverLock, NULL);
      elementsByName[std::make_pair(groupName.c_str(),
                                    dataName.c_str())] = element;
      elementsById[element->info.dataId] = element;
      updatePendingRegistrations(element);
      return element;
    }

    void DataBroker::publishDataElement(const DataElement *element)
    {
      // Inform receivers about new Stream.
      DataPackage package;
      package.add("groupName", element->info.groupName);
      package.add("dataName", element->info.dataName);
      package.add("dataId", (long)element->info.dataId);
      package.add("flags", element->info.flags);
      pthread_rwlock_unlock(&elementsLock);
      pushData(newStreamId, package);
      /*
        pushInfo("DataBroker: new Stream: groupName: \"%s\";"
        " dataName: \"%s\"",
        element->info.groupName.c_str(), element->info.dataName.c_str());
      */
      pthread_rwlock_rdlock(&elementsLock);
    }

    void DataBroker::updatePendingRegistrations(DataElement *newElement) {
      std::list<PendingRegistration>::iterator registrationIt;
      std::list<PendingTimedRegistration>::iterator timedRegistrationIt;
      std::list<PendingTriggeredRegistration>::iterator triggeredRegistrationIt;
      std::map<std::string, Timer>::iterator timerIt;
      std::map<std::string, Trigger>::iterator triggerIt;
      std::string newGroupName = newElement->info.groupName.c_str();
      std::string newDataName = newElement->info.dataName.c_str();

      // pending async receivers
      for(registrationIt = pendingAsyncRegistrations.begin();
          registrationIt != pendingAsyncRegistrations.end(); ) {
        if(matchPattern(registrationIt->groupName, newGroupName) &&
           matchPattern(registrationIt->dataName, newDataName)) {
          Receiver r = {registrationIt->receiver, registrationIt->callbackParam};
          newElement->asyncReceivers.push_back(r);
          // if the registration has wildcards keep it in the pending list...
          if(hasWildcards(registrationIt->groupName) ||
             hasWildcards(registrationIt->dataName)) {
            ++registrationIt;
          } else {
            // ...otherwise remove it.
            registrationIt = pendingAsyncRegistrations.erase(registrationIt);
          }
        } else {
          ++registrationIt;
        }
      }

      // pending sync receivers
      for(registrationIt = pendingSyncRegistrations.begin();
          registrationIt != pendingSyncRegistrations.end(); ) {
        if(matchPattern(registrationIt->groupName, newGroupName) &&
           matchPattern(registrationIt->dataName, newDataName)) {
          Receiver r = {registrationIt->receiver, registrationIt->callbackParam};
          newElement->syncReceivers.push_back(r);
          // if the registration has wildcards keep it in the pending list...
          if(hasWildcards(registrationIt->groupName) ||
             hasWildcards(registrationIt->dataName)) {
            ++registrationIt;
          } else {
            // ...otherwise remove it.
            registrationIt = pendingSyncRegistrations.erase(registrationIt);
          }
        } else {
          ++registrationIt;
        }
      }

      // pending timed receivers
      for(timedRegistrationIt = pendingTimedRegistrations.begin();
          timedRegistrationIt != pendingTimedRegistrations.end();/*do nothing*/){
        bool removeItem = false;
        if(matchPattern(timedRegistrationIt->groupName, newGroupName) &&
           matchPattern(timedRegistrationIt->dataName, newDataName)) {
          timerIt = timers.find(timedRegistrationIt->timerName);
          if(timerIt != timers.end()) {
            TimedReceiver r = { timedRegistrationIt->receiver,
                                newElement,
                                timedRegistrationIt->updatePeriod,
                                timerIt->second.t,
                                timedRegistrationIt->callbackParam };
            timerIt->second.receivers.push_back(r);
            // if the registration has wildcards keep it in the pending list...
            if(!hasWildcards(timedRegistrationIt->groupName) &&
               !hasWildcards(timedRegistrationIt->dataName)) {
              removeItem = true;
            }
          }
        }
        if(removeItem) {
          timedRegistrationIt = pendingTimedRegistrations.erase(timedRegistrationIt);
        } else {
          ++timedRegistrationIt;
        }
      }

      // pending triggered receivers
      for(triggeredRegistrationIt = pendingTriggeredRegistrations.begin();
          triggeredRegistrationIt != pendingTriggeredRegistrations.end();
          /*do nothing*/) {
        bool removeItem = false;
        if(matchPattern(triggeredRegistrationIt->groupName, newGroupName) &&
           matchPattern(triggeredRegistrationIt->dataName, newDataName)) {
          triggerIt = triggers.find(triggeredRegistrationIt->triggerName);
          if(triggerIt != triggers.end()) {
            TriggeredReceiver r = { triggeredRegistrationIt->receiver,
                                    newElement,
                                    triggeredRegistrationIt->callbackParam };
            triggerIt->second.receivers.push_back(r);
            // if the registration has no wildcards remove
            // it from the pending list
            if(!hasWildcards(triggeredRegistrationIt->groupName) &&
               !hasWildcards(triggeredRegistrationIt->dataName)) {
              removeItem = true;
            }
          }
        }
        if(removeItem) {
          triggeredRegistrationIt = pendingTriggeredRegistrations.erase(triggeredRegistrationIt);
        } else {
          ++triggeredRegistrationIt;
        }
      }
    }

    void DataBroker::getElementsByName(const std::string &groupName,
                                       const std::string &dataName,
                                       std::vector<DataElement*> *elements)const {
      std::map<std::pair<std::string, std::string>, DataElement*>::const_iterator elementIt;
      // check if either groupName or dataName has wildcards
      if(!hasWildcards(groupName) && !hasWildcards(dataName)) {
        // no wildcards. do direct lookup.
        elementIt = elementsByName.find(std::make_pair(groupName, dataName));
        if(elementIt != elementsByName.end()) {
          elements->push_back(elementIt->second);
        }
      } else {
        // do wildcard matching on all elements
        for(elementIt = elementsByName.begin();
            elementIt != elementsByName.end(); ++elementIt) {
          if(matchPattern(groupName, elementIt->first.first) &&
             matchPattern(dataName, elementIt->first.second)) {
            elements->push_back(elementIt->second);
          }
        }
      }
    }

    void DataBroker::connectDataItems(const std::string &fromGroupName,
                                      const std::string &fromDataName,
                                      const std::string &fromItemName,
                                      const std::string &toGroupName,
                                      const std::string &toDataName,
                                      const std::string &toItemName) {
      std::map<std::pair<std::string, std::string>, DataElement*>::iterator elementIt;
      DataItemConnection connection;
      fprintf(stderr, "connect DataItems...");
      fflush(stderr);
      // TODO: should we special case wildcards?
      DataElement *element;

      // from element handling
      {
        elementIt = elementsByName.find(std::make_pair(fromGroupName, 
                                                       fromDataName));
        if(elementIt == elementsByName.end()) {
          pushError("could not find from Element: %s, %s\n", 
                    fromGroupName.c_str(),
                    fromDataName.c_str());
          return;
        }
        element = elementIt->second;
        connection.fromElement = element;
        connection.fromDataItemIndex = element->frontBuffer->getIndexByName(fromItemName);
      }
    
      // to element handling
      {
        elementIt = elementsByName.find(std::make_pair(toGroupName, 
                                                       toDataName));
        if(elementIt == elementsByName.end()) {
          pushError("could not find to Element: %s, %s\n", 
                    toGroupName.c_str(),
                    toDataName.c_str());
          return;
        }
        element = elementIt->second;
        connection.toElement = element;
        connection.toDataItemIndex = element->frontBuffer->getIndexByName(toItemName);
      }

      fprintf(stderr, "attaching from %s %s\n", 
              connection.fromElement->info.groupName.c_str(),
              connection.fromElement->info.dataName.c_str());
      fprintf(stderr, "attaching to %s %s\n", 
              connection.toElement->info.groupName.c_str(),
              connection.toElement->info.dataName.c_str());
            
      connection.fromElement->connections.push_back(connection);
      fprintf(stderr, "done\n");
    }

    void DataBroker::disconnectDataItems(const std::string &fromGroupName,
                                         const std::string &fromDataName,
                                         const std::string &fromItemName,
                                         const std::string &toGroupName,
                                         const std::string &toDataName,
                                         const std::string &toItemName) {
      pushError("disconnectDataItems is not implemented, yet!");
    }

    void DataBroker::disconnectDataItems(const std::string &toGroupName,
                                         const std::string &toDataName,
                                         const std::string &toItemName) {
      std::map<unsigned long, DataElement*>::iterator it;
      std::list<DataItemConnection>::iterator jt;

      pthread_rwlock_wrlock(&elementsLock);
      for(it=elementsById.begin(); it!=elementsById.end(); ++it) {
        for(jt=it->second->connections.begin();
            jt!=it->second->connections.end(); ++jt) {
          fprintf(stderr, "da da da \n");
          fprintf(stderr, "searching: form %s %s %ld ---> %s %s %ld\n",
                  jt->fromElement->info.groupName.c_str(),
                  jt->fromElement->info.dataName.c_str(),
                  jt->fromDataItemIndex,
                  jt->toElement->info.groupName.c_str(),
                  jt->toElement->info.dataName.c_str(),
                  jt->toDataItemIndex);

          pthread_rwlock_wrlock(&(jt->toElement->bufferLock));
          if(jt->toDataItemIndex >= (int)jt->toElement->frontBuffer->size()) {
            pushError("DataBroker::disconnectDataItems : connection index does not match!");
          }
          else {
            if(jt->toElement->info.groupName == toGroupName &&
               jt->toElement->info.dataName == toDataName &&
               (*jt->toElement->frontBuffer)[jt->toDataItemIndex].getName() == toItemName) {
              pthread_rwlock_unlock(&jt->toElement->bufferLock);
              it->second->connections.erase(jt);
              //jt = it->second->connections.begin();
              break;
            }
          }
          pthread_rwlock_unlock(&jt->toElement->bufferLock);
        }
      }
      pthread_rwlock_unlock(&elementsLock);
    }

  } // end of namespace data_broker
} // end of namespace mars

DESTROY_LIB(mars::data_broker::DataBroker);
CREATE_LIB(mars::data_broker::DataBroker);
  
