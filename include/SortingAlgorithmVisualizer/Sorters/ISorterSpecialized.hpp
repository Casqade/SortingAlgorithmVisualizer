#pragma once

#include <SortingAlgorithmVisualizer/Containers.hpp>
#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/Randomization.hpp>
#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>


template <typename T>
class ISorterSpecialized : public ISorter
{
public:
  inline ISorterSpecialized( Array <T>& values, Array <PlotValueColorIndex>& colors )
    : mValues{&values}
    , mColors{&colors}
  {
    mRandomizeTask.callback = RandomizePlotData <T>;
    mRandomizeTask.data = mValues->data();
    mRandomizeTask.elementCount = mValues->size();
    mRandomizeTask.dataGuard = &mDataGuard;
  }


protected:
  Array <T>* mValues {};
  Array <PlotValueColorIndex>* mColors {};
};
