#pragma once

#include <SortingAlgorithmVisualizer/CommonTypes.hpp>

#include <windows.h>


class ISorter
{
public:
  ISorter();
  virtual ~ISorter();

  static void Destroy( ISorter* );

  virtual bool init( size_t valueCount, IAllocator& );

  virtual bool step() = 0;
  virtual void reset() = 0;

  BOOL tryLockData();
  void lockData();
  void unlockData();

  RandomizeTask& getRandomizeTask();

  static size_t HeapMemoryBudget( size_t valueCount );


protected:
  virtual size_t instanceSize() const = 0;
  virtual RandomizeFunction* getRandomizeCallback() const = 0;


protected:
  CRITICAL_SECTION mDataGuard {};
  RandomizeTask mRandomizeTask {};

  PlotColors mColors {};
};
