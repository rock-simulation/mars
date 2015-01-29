/*
 * 
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
 */

/*
 * 
 * 
 * HOW TO:
 *  1) producer:
 *     - push an initial data set and get an ID back
 *     - then call pushData(ID, ...)
 *  2) receiver:
 *     a) Asynchronous (this should be the default):
 *        - call registerAsyncReceiver() with (sensor group and name)
 *        - updates the receiver from another thread than the pushing thread.
 *        NOTE: If the producer pushes fast you only receive the last datum.
 *     b) Synchronous
 *        - call registerSyncReceiver() with (sensor group and name)
 *        - updates the receiver from within the pushData-call.
 *        NOTE: only use this if it is important that you don't miss a datum.
 *     c) Timed:
 *        - call createTimer() with a suitable name
 *        - call registerTimedReceiver() with a desired update period
 *        - call stepTimer() to step the timer and get the updates
 *        NOTE: you will get the latest datum. regardless whether it's been 
 *              updated or not since the last update.
 *     d) Triggered:
 *        - call createTrigger() with a suitable triggerName
 *        - call registerTriggered() to register to a certain trigger
 *        - call trigger() to manually trigger a certain trigger and update 
 *          all receivers registered to it.
 *        NOTE: you will get the latest datum. regardless whether it's been 
 *              updated or not since the last update.
 *
 * TYPICAL USECASES:
 *  1) Plotter:
 *     should run in Asynchronous or Timed mode
 *  2) File Logger:
 *     should run in Asynchronous or Timed mode
 *  3) Controller:
 *     should probably run in Asynchronous mode.
 *  4) Command:
 *     When listening for commands you should run in Synchronous mode.
 */

/**
 * \file DataBrokerInterface.h
 * \author Malte Roemmermann, Lorenz Quack
 * \brief DataBroker A small library to manage data communication within a software framework.
 *
 * Version 0.2
 */

#ifndef DATABROKERINTERFACE_H
#define DATABROKERINTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "DataBrokerInterface.h"
#endif

#include "DataPackage.h"
#include "DataInfo.h"

#include <mars/utils/Singleton.hpp>

#include <cstdarg>
#include <string>
#include <vector>

namespace mars {


  namespace data_broker {

    class ReceiverInterface;
    class ProducerInterface;

    enum MessageType {
      DB_MESSAGE_TYPE_FATAL,
      DB_MESSAGE_TYPE_ERROR,
      DB_MESSAGE_TYPE_WARNING,
      DB_MESSAGE_TYPE_INFO,
      DB_MESSAGE_TYPE_DEBUG,

      __DB_MESSAGE_TYPE_COUNT
    };

    /** \brief The interface every DataBroker should implement. */
    class DataBrokerInterface : public singleton::Interface {

    public:
      /**
       * \brief Constructor takes no arguments.
       */
      DataBrokerInterface(){}
      virtual ~DataBrokerInterface() {}

      // LibInterface methods
      virtual int getLibVersion() const
      { return 1; }
      virtual const std::string getLibName() const
      { return "data_broker"; }
//      CREATE_MODULE_INFO();

      /**
       * \brief creates a new timer with the given name
       * \param timerName the name of the new timer
       * \returns \c false if a timer with the given name already exists. 
       *          \c true otherwise.
       * \see stepTimer, registerTimedReceiver, unregisterTimedReceiver, 
       *      ReceiverInterface
       */
      virtual bool createTimer(const std::string &timerName) = 0;

      /**
       * \brief advances the timer timerName by step
       * \param timerName The name of the timer that should be stepped.
       *                  It should have been previously created with
       *                  \ref createTimer.
       * \param step The amount by which the timer should be stepped.
       *             This can be anything from the amount of seconds passed in
       *             real time to an artificial simulation time or something 
       *             entirely different. It's up to you.
       * \return \c true if the timer was stepped. 
       *         \c false if no timer with the name \a timerName exists.
       *
       * Stepping a timer with this function will advance its internal time by 
       * the given step. It will then check all receivers who registered to this
       * timer with \ref registerTimedReceiver if their update time has expired
       * in which case they will be called from within this thread.
       * \see createTimer, registerTimedReceiver, unregisterTimedReceiver, 
       *      ReceiverInterface
       */
      virtual bool stepTimer(const std::string &timerName, long step=1) = 0;

      /**
       * \brief registers a receiver for a group/data with a timer
       * \param receiver The ReceiverInterface that should be called back.
       * \param groupName The groupName of the DataPackage the \a receiver is 
       *                  interested in.
       * \param dataName The dataName of the DataPackage the \a receiver is 
       *                 interested in.
       * \param timerName The name of the timer that should call 
       *                  the \a receiver.
       * \param updatePeriod The number of timer steps that should lie between 
       *                     callbacks. If you pass 0 the \a receiver will be 
       *                     called back every time the timer is 
       *                     \ref stepTimer "stepped".
       * \param callbackParam An optional \c int that will be passed back to 
       *                      the \a receiver in 
       *                      \ref ReceiverInterface::receiveData "receiveData".
       *                      This can be used by the \a receiver to distinguish
       *                      callbacks from different registrations.
       * \return \c false if the timer \a timerName doesn't exist. 
       *         \c true otherwise. If this method returns \c false this doesn't
       *         necessarily indicate an error. The registration will be cached 
       *         in case the timer gets created at a later time. This is only 
       *         intended as feedback in case you *know* the timer should exist.
       * \see createTimer, stepTimer, unregisterTimedReceiver, ReceiverInterface
       */
      virtual bool registerTimedReceiver(ReceiverInterface *receiver,
                                         const std::string &groupName,
                                         const std::string &dataName,
                                         const std::string &timerName,
                                         int updatePeriod,
                                         int callbackParam=0) = 0;

      /**
       * \brief unregister a receiver from receiving callbacks from a timer 
       *        for certain group/data
       * \param receiver The ReceiverInterface that wants to unregister.
       * \param groupName The groupName of the DataPackage for which the
       *                  \a receiver no longer wishes to receive callbacks 
       *                  from the timer.
       * \param dataName The dataName of the DataPackage for which the
       *                 \a receiver no longer wishes to receive callbacks 
       *                 from the timer.
       * \param timerName The name of the timer that should stop notifying the
       *                  \a receiver.
       * \return \c true if the \a receiver successfully unregistered. 
       *         \c false otherwise. Returning \c false could have several
       *         reasons
       *         - No timer called \a timerName was \ref createTimer "created"
       *         - No DataPackage with the given \a groupName and \a dataName 
       *           exist.
       *         - The \a receiver was not registered with this timer for the
       *           DataPackage of the given \a groupName and \a dataName.
       * \see createTimer, stepTimer, registerTimedReceiver, ReceiverInterface
       */
      virtual bool unregisterTimedReceiver(ReceiverInterface *receiver,
                                           const std::string &groupName,
                                           const std::string &dataName,
                                           const std::string &timerName) = 0;

      /**
       * \todo 
       */
      virtual bool registerTimedProducer(ProducerInterface *producer,
                                         const std::string &groupName,
                                         const std::string &dataName,
                                         const std::string &timerName,
                                         int updatePeriod,
                                         int callbackParam=0) = 0;

      /**
       * \todo
       */
      virtual bool unregisterTimedProducer(ProducerInterface *producer,
                                           const std::string &groupName,
                                           const std::string &dataName,
                                           const std::string &timerName) = 0;


      /**
       * \brief create a new trigger with the given name
       * \param triggerName the name of the new trigger
       * \returns \c false if a trigger with the given name already exists. 
       *          \c true otherwise.
       * \see trigger, registerTriggeredReceiver, unregisterTriggeredReceiver, 
       *      ReceiverInterface
       */
      virtual bool createTrigger(const std::string &triggerName) = 0;


      /**
       * \brief triggers the trigger triggerName
       * \param triggerName The name of the trigger that should be triggered.
       *                    It should have been previously created with
       *                    \ref createTrigger.
       * \return \c true if the trigger was triggered. 
       *         \c false if no trigger with the name \a triggerName exists.
       *
       * Triggering a trigger causes it to send the latest DataPackage to all
       * \ref ReceiverInterface "receivers" that registered to this trigger.
       *
       * \see createTrigger, registerTriggeredReceiver, 
       *      unregisterTriggeredReceiver, ReceiverInterface
       */
      virtual bool trigger(const std::string &triggerName) = 0;

      /**
       * \brief registers a receiver for a group/data with a trigger
       * \param receiver The ReceiverInterface that should be called back.
       * \param groupName The groupName of the DataPackage the \a receiver is 
       *                  intersted in.
       * \param dataName The dataName of the DataPackage the \a receiver is 
       *                 intersted in.
       * \param triggerName The name of the trigger that should call 
       *                    the \a receiver.
       * \param callbackParam An optional \c int that will be passed back to 
       *                      the \a receiver in 
       *                      \ref ReceiverInterface::receiveData "receiveData".
       *                      This can be used by the \a receiver to distinguish
       *                      callbacks from different registrations.
       * \return \c false if the trigger \a triggerName doesn't exist. 
       *         \c true otherwise. If this method returns \c false this doesn't
       *         necessarily indicate an error. The registration will be cached 
       *         in case the trigger gets created at a later time. This is only 
       *         intended as feedback in case you *know* the trigger should
       *         exist.
       * \see createTrigger, trigger, unregisterTriggeredReceiver, 
       *      ReceiverInterface
       */
      virtual bool registerTriggeredReceiver(ReceiverInterface *receiver,
                                             const std::string &groupName,
                                             const std::string &dataName,
                                             const std::string &triggerName,
                                             int callbackParam=0) = 0;

      /**
       * \brief register a receiver from receiving callbacks from a trigger 
       *        for certain group/data
       * \param receiver The ReceiverInterface that wants to unregister.
       * \param groupName The groupName of the DataPackage for which the
       *                  \a receiver no longer wishes to receive callbacks 
       *                  from the trigger.
       * \param dataName The dataName of the DataPackage for which the
       *                 \a receiver no longer wishes to receive callbacks 
       *                 from the trigger.
       * \param triggerName The name of the trigger that should stop notifying
       *                    the \a receiver.
       * \return \c true if the \a receiver successfully unregistered. 
       *         \c false otherwise. Returning \c false could have several
       *         reasons
       *         - No trigger called \a timerName was
       *           \ref createTrigger "created"
       *         - No DataPackage with the given \a groupName and \a dataName 
       *           exist.
       *         - The \a receiver was not registered with this trigger for the
       *           DataPackage of the given \a groupName and \a dataName.
       * \see createTrigger, trigger, registerTriggeredReceiver, ReceiverInterface
       */
      virtual bool unregisterTriggeredReceiver(ReceiverInterface *receiver,
                                               const std::string &groupName,
                                               const std::string &dataName,
                                               const std::string &triggerName)=0;

      /**
       * \brief register a receiver to receive a synchronous callback 
       *        for a certain stream
       * \param receiver The ReceiverInterface that should be called whenever
       *                 a certain stream is \ref pushData "pushed" to the 
       *                 DataBroker.
       * \param groupName The groupName of the DataPackage the \a receiver is
       *                  interested in.
       * \param dataName The dataName of the DataPackage the \a receiver is
       *                 interested in.
       * \param callbackParam An optional \c int that will be passed back to 
       *                      the \a receiver in 
       *                      \ref ReceiverInterface::receiveData "receiveData".
       *                      This can be used by the \a receiver to distinguish
       *                      callbacks from different registrations.
       * \return \c true if the registration was successful.
       *         \c false if no DataPackage with the given \a groupName and 
       *         \a dataName exists. This doesn't necessarily indicate an error.
       *         The registration will be cached and may be performed when the
       *         requested DataPackage is pushed at a later time.
       *
       * The \ref ReceiverInterface::receiveData "callbacks" to the 
       * \ref ReceiverInterface "receivers"
       * will be performed synchronous by the DataBroker. 
       * This means that the callback will occur
       * from within the same thread where the \ref pushData happened and the 
       * call to pushData will not return until all 
       * receivers that 
       * \ref registerSyncReceiver "registered" 
       * to this stream were called and returned from their callbacks.
       *
       * \see unregisterSyncReceiver, registerAsyncReceiver, ReceiverInterface, 
       *      pushData
       */
      virtual bool registerSyncReceiver(ReceiverInterface *receiver,
                                        const std::string &groupName,
                                        const std::string &dataName,
                                        int callbackParam=0) = 0;

      /**
       * \brief unregister a receiver from receiving callbacks for certain 
       *        group/data
       * \param receiver The ReceiverInterface that wants to unregister.
       * \param groupName The groupName of the DataPackage for which the
       *                  \a receiver no longer wishes to receive callbacks.
       * \param dataName The dataName of the DataPackage for which the
       *                 \a receiver no longer wishes to receive callbacks.
       * \return \c true if the \a receiver successfully unregistered. 
       *         \c false otherwise. Returning \c false could have several
       *         reasons
       *         - No DataPackage with the given \a groupName and \a dataName 
       *           exist.
       *         - The \a receiver was not registered for the
       *           DataPackage of the given \a groupName and \a dataName.
       * \see registerSyncReceiver, ReceiverInterface
       */
      virtual bool unregisterSyncReceiver(ReceiverInterface *receiver,
                                          const std::string &groupName,
                                          const std::string &dataName) = 0;
    
      /**
       * \brief register a receiver to receive a asynchronous callback 
       *        for a certain stream
       * \param receiver The ReceiverInterface that should be called whenever
       *                 a certain stream is \ref pushData "pushed" to the 
       *                 DataBroker.
       * \param groupName The groupName of the DataPackage the \a receiver is
       *                  interested in.
       * \param dataName The dataName of the DataPackage the \a receiver is
       *                 interested in.
       * \param callbackParam An optional \c int that will be passed back to 
       *                      the \a receiver in 
       *                      \ref ReceiverInterface::receiveData "receiveData".
       *                      This can be used by the \a receiver to distinguish
       *                      callbacks from different registrations.
       * \return \c true if the registration was successful.
       *         \c false if no DataPackage with the given \a groupName and 
       *         \a dataName exists. This doesn't necessarily indicate an error.
       *         The registration will be cached and may be performed when the
       *         requested DataPackage is pushed at a later time.
       *
       * The \ref ReceiverInterface::receiveData "callbacks" to the 
       * \ref ReceiverInterface "receivers"
       * will be performed asynchronous by the DataBroker. 
       * This means that the callback will occur
       * from a different thread as the one from which \ref pushData was called.
       * The receivers will only get a callback for the latest pushData call.
       * This means that asynchronous receivers might miss some DataPackages
       * when they are pushed fast to the DataBroker. If you cannot afford to 
       * miss DataPackages you should use \ref registerSyncReceiver.
       *
       * \see unregisterAsyncReceiver, registerSyncReceiver, ReceiverInterface, 
       *      pushData
       */
      virtual bool registerAsyncReceiver(ReceiverInterface *receiver,
                                         const std::string &groupName,
                                         const std::string &dataName,
                                         int callbackParam=0) = 0;

      /**
       * \brief unregister a receiver from receiving callbacks for certain 
       *        group/data
       * \param receiver The ReceiverInterface that wants to unregister.
       * \param groupName The groupName of the DataPackage for which the
       *                  \a receiver no longer wishes to receive callbacks.
       * \param dataName The dataName of the DataPackage for which the
       *                 \a receiver no longer wishes to receive callbacks.
       * \return \c true if the \a receiver successfully unregistered. 
       *         \c false otherwise. Returning \c false could have several
       *         reasons
       *         - No DataPackage with the given \a groupName and \a dataName 
       *           exist.
       *         - The \a receiver was not registered for the
       *           DataPackage of the given \a groupName and \a dataName.
       * \see registerAsyncReceiver, ReceiverInterface
       */
      virtual bool unregisterAsyncReceiver(ReceiverInterface *receiver,
                                           const std::string &groupName,
                                           const std::string &dataName) = 0;

      /**
       * \brief pushes a DataPackage into the DataBroker
       * \param groupName A string to identify different 
       *                  \ref DataPackage "DataPackages" belonging to the 
       *                  same group or category. The combination of
       *                  \a groupName and \a dataName should be unique.
       * \param dataName A string to to identify this DataPackage.
       *                 The combination of \a groupName and \a dataName 
       *                 should be unique.
       * \param dataPackage The DataPackage that will be distributed to the 
       *                    \ref ReceiverInterface "receivers".
       * \param producer In case a \ref ReceiverInterface "receiver" pushes
       *                 data to the stream it has registered itself to, it
       *                 normally would get a callback. To prevent this
       *                 self-callback it may pass pointer to itself to prevent
       *                 receiving a callback for this single push it did
       *                 itself.
       *                 If \c NULL is passed this has no effect.
       * \param flags This is used to indicate the nature of the data.
       * \return A unique pushId. 
       *
       * An object that regularly pushes data to the DataBroker should only use
       * this method once and record the returned pushId. Subsequently it should
       * use the pushId and call 
       * \ref pushData(unsigned long,const DataPackage&,const ReceiverInterface*) "pushData(unsigned long,...)"
       * for better performance.
       *
       * \see pushData(unsigned long, const DataPackage&, const ReceiverInterface*)
       */
      virtual unsigned long pushData(const std::string &groupName,
                                     const std::string &dataName,
                                     const DataPackage &dataPackage,
                                     const ReceiverInterface *producer,
                                     PackageFlag flags) = 0;

      /**
       * \brief pushes a DataPackage into the DataBroker
       * \param id The pushId previously returned by \ref pushData(const std::string&,const std::string&,const DataPackage&,const ReceiverInterface*,PackageFlag) "this method".
       * \param dataPackage The DataPackage that will be distributed to the 
       *                    \ref ReceiverInterface "receivers".
       * \param producer In case a \ref ReceiverInterface "receiver" pushes 
       *                 data to the stream it has registered itself to, it
       *                 normally would get a callback. To prevent this
       *                 self-callback it may pass pointer to itself to prevent
       *                 receiving a callback for this single push it did 
       *                 itself.
       *                 If \c NULL is passed this has no effect.
       * \return The pushId \a id.
       *
       * An object that regularly pushes data to the DataBroker should use the
       * \ref pushData(const std::string&,const std::string&,const DataPackage&,const ReceiverInterface*,PackageFlag) "named version" 
       * of this method once and record the returned pushId. 
       * Subsequently it should use the pushId and call this method for 
       * better performance.
       *
       * \see pushData(const std::string& , const std::string&, const DataPackage&, const ReceiverInterface*, PackageFlag)
       */
      virtual unsigned long pushData(unsigned long id,
                                     const DataPackage &dataPackage,
                                     const ReceiverInterface *producer=NULL) =0;

      /**
       * \brief get the unique dataId assosiated with a given groupName and 
       *        dataName
       * \param groupName The \ref DataInfo::groupName of the DataPackage.
       * \param dataName The \ref DataInfo::dataName of the DataPackage.
       * \return The \ref DataInfo::dataId of the \ref DataPackage with the 
       *         given \a groupName and \a dataName. If no such package exists
       *         this method returns 0.
       */
      virtual unsigned long getDataID(const std::string &groupName,
                                      const std::string &dataName) const = 0;
    
      /**
       * \brief get the DataInfo assosiated with a certain DataPackage
       * \param groupName The \ref DataInfo::groupName of the DataPackage
       * \param dataName The \ref DataInfo::dataName of the DataPackage
       * \return A copy of the \ref DataInfo of the \ref DataPackage with the 
       *         given \a groupName and \a dataName. If no such package exists
       *         this method returns and DataInfo object with dataId set to 0 
       *         and empty strings as groupName and dataName.
       */
      virtual const DataInfo getDataInfo(const std::string &groupName,
                                         const std::string &dataName) const = 0;

      /**
       * \brief get the DataPackage with a given dataId
       * \param dataId The unique DataInfo::dataId of the DataPackage to return.
       * \return A copy of the DataPackage with the given \a dataId.
       */
      virtual const DataPackage getDataPackage(unsigned long dataId) const = 0;
    
      /**
       * \brief get a list of all DataInfo items currently in the DataBroker
       * \param flag A bitmask to filter out what kind of DataPackages we are 
       *             interested in. 
       */
      virtual const std::vector<DataInfo> getDataList(PackageFlag flag=DATA_PACKAGE_NO_FLAG) const = 0;

      virtual void connectDataItems(const std::string &fromGroupName,
                                    const std::string &fromDataName,
                                    const std::string &fromItemName,
                                    const std::string &toGroupName,
                                    const std::string &toDataName,
                                    const std::string &toItemName) = 0;
      virtual void disconnectDataItems(const std::string &fromGroupName,
                                       const std::string &fromDataName,
                                       const std::string &fromItemName,
                                       const std::string &toGroupName,
                                       const std::string &toDataName,
                                       const std::string &toItemName) = 0;
      virtual void disconnectDataItems(const std::string &toGroupName,
                                       const std::string &toDataName,
                                       const std::string &toItemName) = 0;

      virtual void pushMessage(MessageType messageType, 
                               const std::string &format, va_list args) = 0;
      virtual void pushMessage(MessageType messageType,
                               const std::string &format, ...) = 0;
      virtual void pushFatal(const std::string &format, ...) = 0;
      virtual void pushError(const std::string &format, ...) = 0;
      virtual void pushWarning(const std::string &format, ...) = 0;
      virtual void pushInfo(const std::string &format, ...) = 0;
      virtual void pushDebug(const std::string &format, ...) = 0;

    }; // end of class definition DataBrokerInterface


  } // end of namespace data_broker

} // end of namespace mars

#endif // DATABROKERINTERFACE_H
