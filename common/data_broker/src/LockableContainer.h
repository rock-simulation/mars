#ifndef DATA_BROKER_LOCKABLE_CONTAINER_H
#define DATA_BROKER_LOCKABLE_CONTAINER_H

#include <vector>
#include <mars/utils/Mutex.h>
#include <cassert>

namespace mars {
  namespace data_broker {

    template <typename T>
    class LockableContainer : public T {
    private:
      mutable mars::utils::Mutex *myMutex;
      mutable bool isLocked;
    public:
      LockableContainer()
	: T(), myMutex(new mars::utils::Mutex), isLocked(false)
      {}
      ~LockableContainer()
      { delete myMutex; }
      LockableContainer(const LockableContainer &other)
	: T(other), myMutex(new mars::utils::Mutex), isLocked(false)
      {}
      
      LockableContainer& operator=(const LockableContainer &other) {
	if(&other == this)
	  return *this;
	assert(!other.isLocked);
	this->lock();
	other.lock();
	T::operator=(other);
	other.unlock();
	this->unlock();
	return *this;
      }

      inline void lock() const {
	isLocked = true;
	myMutex->lock();
      }
      inline void unlock() const {
	myMutex->unlock();
	isLocked = false;
      }

      void locked_insert(const typename T::value_type &x) {
	lock();
	this->insert(x);
	unlock();
      }

      void locked_push_back(const typename T::value_type &x) {
	lock();
	this->push_back(x);
	unlock();
      }

      void locked_clear() {
	lock();
	this->clear();
	unlock();
      }      

    }; // end of class LockableContainer

  } // end of namespace data_broker
} // end of namespace mars

#endif /* DATA_BROKER_LOCKABLE_CONTAINER_H */
