#ifndef DATA_BROKER_LOCKABLE_CONTAINER_H
#define DATA_BROKER_LOCKABLE_CONTAINER_H

#include <vector>
#include <mars/utils/Mutex.h>
#include <mars/utils/MutexLocker.h>
#include <cassert>

namespace mars {
  namespace data_broker {

    template <typename T>
    class LockableContainer : public T {
    private:
      mutable mars::utils::Mutex *myMutex;
    public:
      LockableContainer()
        : T(), myMutex(new mars::utils::Mutex)
      {}
      ~LockableContainer()
      { delete myMutex; }
      LockableContainer(const LockableContainer &other)
        : T(), myMutex(new mars::utils::Mutex)
      {
        other.lock();
        T::operator=(other);
        other.unlock();
      }

      LockableContainer& operator=(const LockableContainer &other) {
        if(&other == this)
          return *this;
        this->lock();
        other.lock();
        T::operator=(other);
        other.unlock();
        this->unlock();
        return *this;
      }

      inline void lock() const {
        myMutex->lock();
      }
      inline void unlock() const {
        myMutex->unlock();
      }

      void locked_insert(const typename T::value_type &x) {
        mars::utils::MutexLocker locker(myMutex);
        this->insert(x);
      }

      void locked_push_back(const typename T::value_type &x) {
        mars::utils::MutexLocker locker(myMutex);
        this->push_back(x);
      }

      void locked_clear() {
        mars::utils::MutexLocker locker(myMutex);
        this->clear();
      }

      bool locked_empty() const {
        mars::utils::MutexLocker locker(myMutex);
        return this->empty();
      }

    }; // end of class LockableContainer

  } // end of namespace data_broker
} // end of namespace mars

#endif /* DATA_BROKER_LOCKABLE_CONTAINER_H */
