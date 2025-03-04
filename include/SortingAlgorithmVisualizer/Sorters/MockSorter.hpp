#pragma once

#include <SortingAlgorithmVisualizer/Sorters/ISorterSpecialized.hpp>


template <typename T>
class MockSorter : public ISorterSpecialized <T>
{
  size_t mCounter {};
  size_t mSwappedIndices[2] {};


public:
  MockSorter() = default;
  ~MockSorter() override = default;


  inline void reset() override
  {
    mCounter = 0;
    resetColors();
  }

  inline bool step() override
  {
    using PlotValueColorIndex::PlotValueColorIndex;

    if ( this->mValues.size() == 0 )
      return true;


    auto& values = this->mValues;
    auto& colors = this->mColors;

//    simulate sorting operation
    const bool isSorted =
      ++mCounter == values.size();


    if ( isSorted == false )
    {
      colors[mSwappedIndices[0]] = PlotValueColorIndex::Sorted;
      colors[mSwappedIndices[1]] = PlotValueColorIndex::Sorted;

      mSwappedIndices[0] = mCounter - 1;
      mSwappedIndices[1] = mCounter;

      std::swap(
        values[mSwappedIndices[0]],
        values[mSwappedIndices[1]] );

      colors[mSwappedIndices[0]] = PlotValueColorIndex::SwappedLess;
      colors[mSwappedIndices[1]] = PlotValueColorIndex::SwappedGreater;
    }
    else
    {
      std::memset(
        colors.data(),
        PlotValueColorIndex::Sorted,
        sizeof(PlotValueColorIndex) * colors.size() );

      for ( auto&& index : mSwappedIndices )
        index = 0;
    }


    flushValues();
    flushColors();
    swapBuffers();

    return isSorted;
  }

  inline size_t instanceSize() const override
  {
    return sizeof(*this);
  }
};
