#pragma once

#include <SortingAlgorithmVisualizer/Containers.hpp>
#include <SortingAlgorithmVisualizer/Randomization.hpp>
#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>


template <typename T>
class ISorterSpecialized : public ISorter
{
public:
  inline ISorterSpecialized( PlotData <T>& data )
    : mData{&data}
  {
    mRandomizeTask.callback = RandomizePlotData <T>;
    mRandomizeTask.data = mData->data();
    mRandomizeTask.elementCount = mData->size();
  }


protected:
  PlotData <T>* mData {};
};
