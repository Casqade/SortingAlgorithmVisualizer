#pragma once

#include <SortingAlgorithmVisualizer/Containers.hpp>
#include <SortingAlgorithmVisualizer/Randomization.hpp>
#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>


template <typename T>
class ISorterSpecialized : public ISorter
{
public:
  inline ISorterSpecialized( Array <T>& values )
    : mValues{&values}
  {
    mRandomizeTask.callback = RandomizePlotData <T>;
    mRandomizeTask.data = mValues->data();
    mRandomizeTask.elementCount = mValues->size();
    mRandomizeTask.dataMutex = &mDataMutex;
  }


protected:
  Array <T>* mValues {};
  Array <T>* mColors {};
};
