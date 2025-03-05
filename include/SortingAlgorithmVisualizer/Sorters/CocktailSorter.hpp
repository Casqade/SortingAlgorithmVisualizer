#pragma once

#include <SortingAlgorithmVisualizer/Sorters/ISorterSpecialized.hpp>


template <typename T>
class CocktailSorter : public ISorterSpecialized <T>
{
  int64_t mIndex {};
  int64_t mSortedIndexRight {};
  int64_t mSortedIndexLeft {};
  int64_t mSwappedIndices[2] {};
  int8_t mDirection {1};
  bool mSwapped {};


public:
  CocktailSorter() = default;
  ~CocktailSorter() override = default;


  inline void reset() override
  {
    mIndex = 0;
    mSortedIndexLeft = -1;
    mSortedIndexRight = valueCount();
    mSwappedIndices[0] = 0;
    mSwappedIndices[1] = 0;
    mDirection = 1;
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

    if ( lhsIndex > mSortedIndexLeft )
      colors[lhsIndex] = PlotColor::Unsorted;

    if ( rhsIndex < mSortedIndexRight )
      colors[rhsIndex] = PlotColor::Unsorted;

    lhsIndex = mIndex;
    rhsIndex = mIndex + mDirection;

    auto& lhs = values[lhsIndex];
    auto& rhs = values[rhsIndex];


    if ( mDirection > 0 )
    {
      if ( lhs > rhs )
      {
        mSwapped = true;
        std::swap(lhs, rhs);

        colors[lhsIndex] = PlotColor::SwappedLess;
        colors[rhsIndex] = PlotColor::SwappedGreater;
      }
      else
        colors[rhsIndex] = PlotColor::Caret;
    }
    else
    {
      if ( lhs < rhs )
      {
        mSwapped = true;
        std::swap(lhs, rhs);

        colors[lhsIndex] = PlotColor::SwappedGreater;
        colors[rhsIndex] = PlotColor::SwappedLess;
      }
      else
        colors[lhsIndex] = PlotColor::Caret;
    }


    mIndex += mDirection;

    bool exitEarly {};

    if ( mIndex + 1 >= mSortedIndexRight )
    {
      if ( mSwapped == false )
        exitEarly = true;

      mSwapped = false;
      mDirection = -1;

      --mSortedIndexRight;
      mIndex = mSortedIndexRight - 1;

      lhsIndex = mIndex;
      rhsIndex = mIndex;

      colors[mSortedIndexRight] = PlotColor::Sorted;
    }
    else if ( mIndex - 1 <= mSortedIndexLeft )
    {
      if ( mSwapped == false )
        exitEarly = true;

      mSwapped = false;
      mDirection = +1;

      ++mSortedIndexLeft;
      mIndex = mSortedIndexLeft + 1;

      lhsIndex = mIndex;
      rhsIndex = mIndex;

      colors[mSortedIndexLeft] = PlotColor::Sorted;
    }

    bool isSorted =
      mSortedIndexLeft + 1 >= mSortedIndexRight - 1;


    if ( exitEarly == true )
    {
      isSorted = true;

      if ( mSortedIndexLeft >= 0 )
        std::memset(
          &colors[mSortedIndexLeft],
          PlotColor::Sorted,
          mSortedIndexRight - mSortedIndexLeft );
    }
    else if ( isSorted == true )
      colors[mIndex] = PlotColor::Sorted;


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
