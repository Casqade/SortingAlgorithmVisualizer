#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>


bool
ISorter::tryReading()
{
  return mDataLock.try_lock();
}

void
ISorter::startReading()
{
  mDataLock.lock();
}

void
ISorter::stopReading()
{
  mDataLock.unlock();
}

RandomizeTask&
ISorter::getRandomizeTask()
{
  return mRandomizeTask;
}
