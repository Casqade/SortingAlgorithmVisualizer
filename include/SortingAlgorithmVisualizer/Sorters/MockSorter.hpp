#pragma once

#include <SortingAlgorithmVisualizer/Sorters/ISorterSpecialized.hpp>

#include <thread>


template <typename T>
class MockSorter : public ISorterSpecialized <T>
{
  size_t mCounter {};


public:
  inline MockSorter( PlotData <T>& data )
    : ISorterSpecialized <T>(data)
  {}

  ~MockSorter() override = default;


  inline void reset() override
  {
    mCounter = 0;
  }

  inline bool step() override
  {
    if ( this->mData == nullptr )
      return true;


//    simulate sorting operation
    const bool isSorted =
      ++mCounter == this->mData->size();


    std::lock_guard <std::mutex> lock (this->mDataMutex);

//    simulate accumulated write operations
    std::this_thread::sleep_for(std::chrono::milliseconds{1});

    return isSorted;
  }

  inline size_t instanceSize() const override
  {
    return sizeof(*this);
  }
};
