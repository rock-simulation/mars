Data Broker {#data_broker}
===========

## Overview

The data\_broker library provides functionality to manage data transfer between the components of the MARS simulation. Its interface is defined in the class [DataBrokerInterface](@ref mars::data_broker::DataBrokerInterface), while the [DataBroker](@ref mars::data_broker::DataBroker) class is the actual implementation.

## Producers and receivers

An instance of DataBroker passes data between registered producers and receivers (see [ProducerInterface](@ref data_broker::ProducerInterface) and [ReceiverInterface](@ref mars::data_broker::ReceiverInterface), respectively) in the MARS framework. For this purpose it holds buffered local copies of the data packages and is thus able to coordinate data transfer between various threads.

Consequently, there are a number of ways in which data can be handled. and for this purpose holds a local copy of the data.  For this purpose, the [DataBroker](@ref mars::data_broker::DataBroker) calls the functions produceData and receiveData of the classes implementing the respective interfaces. When this happens is determined by the way the producer/receiver is registered with the [DataBroker](@ref mars::data_broker::DataBroker). While producers can only be registered as timed producers (using a previously created timer, either manually defined - see [createTimer](@ref mars::data_broker::DataBroker::createTimer) - or using the DataBroker's real time Timer which is automatically created if a producer is registered with a "_realTime_" timer), receivers can be registered in four different ways:

1. timed
2. triggered
3. asynchronous
4. synchronous

### Timed receivers

A timed receiver's receiveData function is called by the stepTimer function of the timer (managed by the DataBroker) with which it was registered. To create a timed receiver one has to do the following:

- call createTimer() with a suitable name
- call registerTimedReceiver() with a desired update period
- call stepTimer() to step the timer and get the updates

The first step can be omitted if an appropriate timer already exists.

NOTE: You will get the latest datum, regardless of whether it has been updated or not since the last timer step.

### Triggered receivers

To create a triggered receiver, which is receiving data as a result of a called trigger, the following steps are necessary:

- call createTrigger() with a suitable triggerName
- call registerTriggered() to register to a certain trigger
- call trigger() to manually trigger a certain trigger and update 
  all receivers registered to it
  
As with timed receivers, one only has to create a new trigger if no appropriate trigger exists already.

NOTE: You will get the latest datum, regardless of whether it has been updated or not since the last trigger call.

### Asynchronous receivers

Asynchronous receivers are updated asynchronously whenever the run() method of the DataBroker is called (which is typically by the MARS simulation), i.e. are not synchronized with the producers which provide the data they receive. This means that if the producer updates with a higher frequency than the receiver, only the last datum is received.

This can be the case if the producer it called from a timer with a higher frequency than whichever thread processes the data of the AsyncReceiver. It can also be the case if data are pushed to the DataBroker via pushData() with a higher frequency.

- call registerAsyncReceiver() with (sensor group and name)
  
### Synchronous receivers

Synchronous receivers are updated synchronously to the thread which calls pushData. This should only be used if it is important that no datum is missed, as it might otherwise slow down whichever process pushes the data.

@Q: not clear in this context if synchronous receivers will receive data if there is noone pushing the data

- call registerSyncReceiver() with (sensor group and name)

### Typical use cases

1) Plotter:
   should run in Asynchronous or Timed mode
2) File Logger:
   should run in Asynchronous or Timed mode
3) Controller:
   should probably run in Asynchronous mode.
4) Command:
   When listening for commands you should run in Synchronous mode.


## Data formats

Data is managed by the DataBroker library in the form of a [DataPackage](@ref mars::data_broker::DataPackage) which itself contains a number of instances of [DataItem](@ref mars::data_broker::DataItem). The latter is a wrapper class for variables of any of the following types:

- int
- long
- float
- double
- string
- bool

contained in an \c std::vector named \c package. The field \c name contains a string describing the [DataItem](@ref mars::data_broker::DataItem).

The class [DataPackage](@ref mars::data_broker::DataPackage) is a container for multiple instances of [DataItem](@ref mars::data_broker::DataItem). These can be added to a package and afterwards accessed either by name ([getItemByName](@ref mars::data_broker::DataItem::getItemByName)) or by index ([getItemByIndex](@ref mars::data_broker::DataItem::getItemByIndex)). Also, the method [getType](@ref mars::data_broker::DataItem::getType) allows to read the type of a [DataItem](@ref mars::data_broker::DataItem) either by index or name as well.


## Important functions

[pushData](@ref mars::data_broker::DataBroker::pushData)

This function allows to easily write data to the DataBroker without the need of implementing the DataProducerInterface first. As long as the used names or consistent (or better, the initially received ID is used), the DataBroker will overwrite the pushed DataPackage. This can be used, e.g., for testing purposes, or in general when handling asynchronous receivers.
Another useful application is communication between libraries: A library can push an initial DataPackage and then register itself as a receiver of that package. If updates are made to the data by any other library, the first one will be notified.


[stepTimer](@ref mars::data_broker::DataBroker::stepTimer)

This function can be called from outside of DataBroker to step any previously created timer. It is also called from within DataBroker to step its own "\_realtime\_" thread which in turn is activated by [runRealtime](@ref mars::data_broker::DataBroker::runRealtime).


[run](@ref mars::data_broker::DataBroker::run)

This function handles asynchronous receivers and has to be called from another thread than the data producers.


[runRealtime](@ref mars::data_broker::DataBroker::runRealtime)

This function activates the "\_realtime\_" timer of a DatBroker, thus handling synchronous receivers as well as timed receivers and producers registered with the timer.


\[27.09.2013\]





