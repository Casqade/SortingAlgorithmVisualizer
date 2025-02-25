#pragma once

#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/Randomization.hpp>
#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>
#include <SortingAlgorithmVisualizer/Containers/Array.hpp>


template <typename T>
class ISorterSpecialized : public ISorter
{
public:
  inline ISorterSpecialized( Array <T>& values, Array <PlotValueColorIndex>& colors )
    : mValues{&values}
    , mColors{&colors}
  {
    mRandomizeTask.data = mValues->data();
    mRandomizeTask.elementCount = mValues->size();
  }


protected:
  inline RandomizeFunction* getRandomizeCallback() const override
  {
    return RandomizePlotData <T>;
  }


protected:
  Array <T>* mValues {};
  Array <PlotValueColorIndex>* mColors {};
};
