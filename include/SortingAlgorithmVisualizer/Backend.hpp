#pragma once

#include <SortingAlgorithmVisualizer/fwd.hpp>
#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>
#include <SortingAlgorithmVisualizer/Containers/CallbackStack.hpp>

#include <cassert>


DWORD WINAPI SorterThreadProc( LPVOID data );
DWORD WINAPI RandomizerThreadProc( LPVOID data );


class Backend
{
public:
  Backend( IAllocator& );
  ~Backend();

  Backend( Backend&& ) = delete;
  Backend( const Backend& ) = delete;


  void init( size_t plotCount );
  void deinit();

  void initData( size_t plotIndex, size_t valueCount );

  template <class T>
  void initSorter( size_t plotIndex );

  void start();
  void stop();

  ISorter* sorter( size_t plotIndex ) const;
  PlotData* plotData( size_t plotIndex ) const;


  static size_t CallbackStackDepth( size_t plotCount );
  static size_t HeapMemoryBudget( size_t plotCount );


private:
  void setThreadAffinities();


private:
  IAllocator& mAllocator;

  Array <PlotData> mPlotData {};
  Array <ThreadLocalData> mThreadsData {};

  Array <ThreadHandle> mSorterThreads {};
  ThreadHandle mRandomizerThread {};

  ThreadSharedData mSharedState {};

  CallbackStack mDeinitStack {};
};


template <class T>
inline void
Backend::initSorter(
  size_t plotIndex )
{
  if ( ProgramShouldAbort == true )
    return;


  assert(plotIndex < mPlotData.size());
  assert(plotIndex < mThreadsData.size());


  auto& sorter = mThreadsData[plotIndex].sorter;

  sorter =
    ObjectCreate <T> (
      mAllocator,
      mPlotData[plotIndex].values,
      mPlotData[plotIndex].colors );

  if ( sorter == nullptr )
  {
    MessageBox( NULL,
      "Failed to create sorter",
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return;
  }

  mDeinitStack.push( sorter,
  [] ( void* data )
  {
    ISorter::Destroy(
      static_cast <ISorter*> (data) );
  });
}
