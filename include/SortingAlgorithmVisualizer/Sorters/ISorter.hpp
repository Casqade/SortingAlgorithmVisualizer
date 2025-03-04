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
  virtual void resetColors();

  void mapValuesBuffer( void* gpuBuffer );
  void mapColorsBuffer( void* gpuBuffer );

  LONG acquireBuffer( LONG bufferIndex );


  virtual void initValue( void* address, size_t value ) const = 0;

  virtual size_t valueSize() const = 0;
  virtual GLenum valueType() const = 0;

  size_t valueCount() const;

  RandomizeTask& getRandomizeTask();

  static size_t HeapMemoryBudget( size_t valueCount );


protected:
  virtual size_t instanceSize() const = 0;
  virtual RandomizeFunction* getRandomizeCallback() const = 0;

  void swapBuffers();
  void flushValues() const;
  void flushColors() const;


protected:
  CRITICAL_SECTION mDataGuard {};
  RandomizeTask mRandomizeTask {};

  void* mValuesBuffer {};
  void* mColorsBuffer {};

  LONG mFrontBufferIndex {1};
  LONG mBackBufferIndex {2};
  LONG mBufferWasSwapped {};

  PlotColors mColors {};
};
