#pragma once

#include <SortingAlgorithmVisualizer/Sorters/ISorterSpecialized.hpp>


template <typename T>
class BubbleSorter : public ISorterSpecialized <T>
{
  size_t mIndex {};
  size_t mSortedIndex {};
  size_t mSwappedIndices[2] {};


public:
  BubbleSorter() = default;
  ~BubbleSorter() override = default;


  inline void reset() override
  {
    mIndex = 0;
    mSortedIndex = valueCount();
    mSwappedIndices[0] = 0;
    mSwappedIndices[1] = 0;
    resetColors();
  }

  inline bool step() override
  {
    using PlotColor = PlotValueColorIndex::PlotValueColorIndex;

    if ( this->mValues.size() <= 1 )
      return true;


    auto& values = this->mValues;
    auto& colors = this->mColors;

    auto& lhsIndex = mSwappedIndices[0];
    auto& rhsIndex = mSwappedIndices[1];

    colors[lhsIndex] = PlotColor::Unsorted;
    colors[rhsIndex] = PlotColor::Unsorted;

    lhsIndex = mIndex;
    rhsIndex = mIndex + 1;

    auto& lhs = values[lhsIndex];
    auto& rhs = values[rhsIndex];


    if ( lhs > rhs )
    {
      std::swap(lhs, rhs);

      colors[lhsIndex] = PlotColor::SwappedLess;
      colors[rhsIndex] = PlotColor::SwappedGreater;
    }
    else
      colors[rhsIndex] = PlotColor::BubbleSortCaret;


    if ( ++mIndex + 1 == mSortedIndex )
    {
      colors[lhsIndex] = PlotColor::Unsorted;

      mIndex = 0;
      lhsIndex = 0;
      rhsIndex = 0;
      --mSortedIndex;

      colors[mSortedIndex] = PlotColor::Sorted;
    }

    bool isSorted = mSortedIndex == 1;

    if ( isSorted == true )
      colors[0] = PlotColor::Sorted;


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
