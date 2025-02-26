#pragma once

#include <SortingAlgorithmVisualizer/Sorters/ISorterSpecialized.hpp>


template <typename T>
class MockSorter : public ISorterSpecialized <T>
{
  size_t mCounter {};


public:
  inline MockSorter( Array <T>& values, Array <PlotValueColorIndex>& colors )
    : ISorterSpecialized <T> (values, colors)
  {}

  ~MockSorter() override = default;


  inline void reset() override
  {
    mCounter = 0;
  }

  inline bool step() override
  {
    if ( this->mValues == nullptr )
      return true;


    auto& values = *this->mValues;

//    simulate sorting operation
    const bool isSorted =
      ++mCounter == values.size();


    this->lockData();

//    simulate accumulated write operations
    Sleep(1);
    std::swap( values[0], values[mCounter - 1] );

    this->unlockData();

    return isSorted;
  }

  inline size_t instanceSize() const override
  {
    return sizeof(*this);
  }
};
