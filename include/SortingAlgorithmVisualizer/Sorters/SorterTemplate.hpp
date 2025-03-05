#pragma once

#include <SortingAlgorithmVisualizer/Sorters/ISorterSpecialized.hpp>


template <typename T>
class SorterTemplate : public ISorterSpecialized <T>
{
public:
  SorterTemplate() = default;
  ~SorterTemplate() override = default;


  inline void reset() override
  {
    resetColors();
  }

  inline bool step() override
  {
    using PlotColor = PlotValueColorIndex::PlotValueColorIndex;

    if ( this->mValues.size() < 2 )
      return true;


    auto& values = this->mValues;
    auto& colors = this->mColors;

    bool isSorted {};



    if ( isSorted == true )
    {
      std::memset(
        colors.data(),
        PlotColor::Sorted,
        colors.size() );
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
