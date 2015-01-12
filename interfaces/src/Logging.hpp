#ifndef _MARS_LOGGING_
#define _MARS_LOGGING_

#if defined(LOG_FATAL) && defined(CONTROL_CENTER_H) 
    //Everything is fine, the user included us, so we are backward compatible, undefiing the compile-time assertions from ControlCenter.h
    //This prevents a warning, because controlcenter was included before us. which is fine, 
    //but to prevent a redefinition removing the defines first
    #undef LOG_FATAL
    #undef LOG_ERROR
    #undef LOG_WARN
    #undef LOG_INFO
    #undef LOG_DEBUG
#endif

#ifndef ROCK
//Push the logging mechanism to mars if mars is used standalone
#include <mars/data_broker/DataBrokerInterface.h>
// use pushMessage() rather than pushError et al because pushMessage 
// can also take a va_list.
#define LOG_FATAL(...) if(mars::interfaces::ControlCenter::theDataBroker) (mars::interfaces::ControlCenter::theDataBroker->pushMessage(mars::data_broker::DB_MESSAGE_TYPE_FATAL, __VA_ARGS__))
#define LOG_ERROR(...) if(mars::interfaces::ControlCenter::theDataBroker) (mars::interfaces::ControlCenter::theDataBroker->pushMessage(mars::data_broker::DB_MESSAGE_TYPE_ERROR, __VA_ARGS__))
#define LOG_WARN(...) if(mars::interfaces::ControlCenter::theDataBroker) (mars::interfaces::ControlCenter::theDataBroker->pushMessage(mars::data_broker::DB_MESSAGE_TYPE_WARNING, __VA_ARGS__))
#define LOG_INFO(...) if(mars::interfaces::ControlCenter::theDataBroker) (mars::interfaces::ControlCenter::theDataBroker->pushMessage(mars::data_broker::DB_MESSAGE_TYPE_INFO, __VA_ARGS__))
#define LOG_DEBUG(...) if(mars::interfaces::ControlCenter::theDataBroker) (mars::interfaces::ControlCenter::theDataBroker->pushMessage(mars::data_broker::DB_MESSAGE_TYPE_DEBUG, __VA_ARGS__))
#else //ROCK
//Useing the Rock logging system
#include <base/Logging.hpp>
#endif //ROCK

#endif //_MARS_LOGGING_
