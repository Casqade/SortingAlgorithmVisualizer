#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>
#include <SortingAlgorithmVisualizer/Allocators/IAllocator.hpp>


void
ISorter::Destroy(
  ISorter* sorter )
{
  auto allocator = ReadObjectAddress <IAllocator> (
    reinterpret_cast <uintptr_t> (sorter) + sorter->instanceSize() );

  sorter->~ISorter();

  allocator->deallocate(sorter);
}

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
