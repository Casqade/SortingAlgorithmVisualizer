#pragma once

#include <SortingAlgorithmVisualizer/CommonTypes.hpp>


class ISorter
{
public:
  ISorter() = default;
  virtual ~ISorter() = default;

  virtual bool step() = 0;
  virtual void reset() = 0;

  RandomizeTask& getRandomizeTask();


protected:
  RandomizeTask mRandomizeTask {};
};
