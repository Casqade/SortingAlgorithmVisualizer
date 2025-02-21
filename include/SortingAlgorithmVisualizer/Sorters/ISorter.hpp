#pragma once

#include <SortingAlgorithmVisualizer/CommonTypes.hpp>

#include <mutex>


class ISorter
{
public:
  ISorter() = default;
  virtual ~ISorter() = default;

  virtual bool step() = 0;
  virtual void reset() = 0;

  bool tryReading();
  void startReading();
  void stopReading();

  RandomizeTask& getRandomizeTask();


protected:
  RandomizeTask mRandomizeTask {};

  std::mutex mDataMutex {};
  std::unique_lock <std::mutex> mDataLock {mDataMutex, std::defer_lock};
};
