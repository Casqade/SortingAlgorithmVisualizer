#pragma once

#include <SortingAlgorithmVisualizer/Sorters/ISorterSpecialized.hpp>


template <typename T>
class SelectionSorter : public ISorterSpecialized <T>
{
  size_t mSortedIndex {};
  size_t mCurrentIndex {};
  size_t mMinimumIndex {};


public:
  SelectionSorter() = default;
  ~SelectionSorter() override = default;


  inline void reset() override
  {
    mSortedIndex = 0;
    mCurrentIndex = 0;
    mMinimumIndex = 0;
    resetColors();
  }

  inline bool step() override
  {
    using PlotColor = PlotValueColorIndex::PlotValueColorIndex;

    if ( this->mValues.size() < 2 )
      return true;


    auto& values = this->mValues;
    auto& colors = this->mColors;


    if ( mCurrentIndex != mMinimumIndex && mCurrentIndex != mSortedIndex )
      colors[mCurrentIndex] = PlotColor::Unsorted;

    colors[++mCurrentIndex] = PlotColor::Caret;


    if ( values[mCurrentIndex] < values[mMinimumIndex] )
    {
      colors[mMinimumIndex] = PlotColor::Unsorted;
      mMinimumIndex = mCurrentIndex;
      colors[mMinimumIndex] = PlotColor::SwappedGreater;
    }


    if ( mCurrentIndex == values.size() - 1 )
    {
      std::swap(
        values[mSortedIndex],
        values[mMinimumIndex] );

      colors[mMinimumIndex] = PlotColor::Unsorted;
      colors[mCurrentIndex] = PlotColor::Unsorted;

      mMinimumIndex = ++mSortedIndex;
      mCurrentIndex = mSortedIndex;

      colors[mMinimumIndex] = PlotColor::SwappedGreater;
      colors[mSortedIndex - 1] = PlotColor::Sorted;
    }


    bool isSorted =
      mSortedIndex == values.size() - 1;

    if ( isSorted == true )
      colors[mSortedIndex] = PlotColor::Sorted;


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
