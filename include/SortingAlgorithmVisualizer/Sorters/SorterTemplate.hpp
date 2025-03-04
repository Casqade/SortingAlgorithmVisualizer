#pragma once

#include <SortingAlgorithmVisualizer/Sorters/ISorterSpecialized.hpp>


template <typename T>
class BubbleSorter : public ISorterSpecialized <T>
{
public:
  BubbleSorter() = default;
  ~BubbleSorter() override = default;


  inline void reset() override
  {
    resetColors();
  }

  inline bool step() override
  {
    using PlotValueColorIndex::PlotValueColorIndex;

    if ( this->mValues.size() == 0 )
      return true;


    auto& values = this->mValues;
    auto& colors = this->mColors;

    bool isSorted {};



    if ( isSorted == true )
    {
      std::memset(
        colors.data(),
        PlotValueColorIndex::Sorted,
        sizeof(PlotValueColorIndex) * colors.size() );
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
