#pragma once

#include <SortingAlgorithmVisualizer/Sorters/ISorterSpecialized.hpp>


template <typename T>
class InsertionSorter : public ISorterSpecialized <T>
{
  size_t mSortedIndex {};
  size_t mCurrentIndex {1};
  size_t mUnsortedIndex {1};

  size_t mSwappedIndices[2] {};


public:
  InsertionSorter() = default;
  ~InsertionSorter() override = default;


  inline void reset() override
  {
    mSortedIndex = 0;
    mCurrentIndex = 1;
    mUnsortedIndex = 1;
    mSwappedIndices[0] = 0;
    mSwappedIndices[1] = 0;
    resetColors();
  }

  inline bool step() override
  {
    using PlotColor = PlotValueColorIndex::PlotValueColorIndex;

    if ( this->mValues.size() < 2 )
      return true;


    auto& values = this->mValues;
    auto& colors = this->mColors;

    auto& lhsIndex = mSwappedIndices[0];
    auto& rhsIndex = mSwappedIndices[1];

    colors[lhsIndex] = PlotColor::Sorted;
    colors[rhsIndex] = PlotColor::Sorted;

    lhsIndex = mSortedIndex;
    rhsIndex = mCurrentIndex;

    auto& lhs = values[lhsIndex];
    auto& rhs = values[rhsIndex];


    if ( lhs > rhs )
    {
      std::swap(lhs, rhs);

      colors[lhsIndex] = PlotColor::SwappedLess;
      colors[rhsIndex] = PlotColor::SwappedGreater;

      if ( mSortedIndex > 0 )
      {
        --mSortedIndex;
        --mCurrentIndex;
      }
      else
      {
        mCurrentIndex = ++mUnsortedIndex;
        mSortedIndex = mCurrentIndex - 1;
      }
    }
    else
    {
      if ( mCurrentIndex < mUnsortedIndex )
      {
        colors[lhsIndex] = PlotColor::Sorted;
        colors[rhsIndex] = PlotColor::Sorted;
      }
      else
      {
        colors[lhsIndex] = PlotColor::SwappedLess;
        colors[rhsIndex] = PlotColor::SwappedGreater;
      }

      mCurrentIndex = ++mUnsortedIndex;
      mSortedIndex = mCurrentIndex - 1;
    }


    bool isSorted =
      mUnsortedIndex == this->mValues.size();

    if ( isSorted == true )
    {
      colors[lhsIndex] = PlotColor::Sorted;
      colors[rhsIndex] = PlotColor::Sorted;
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
