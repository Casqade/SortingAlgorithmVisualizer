#pragma once

#include <SortingAlgorithmVisualizer/fwd.hpp>
#include <SortingAlgorithmVisualizer/CommonTypes.hpp>


DWORD WINAPI SorterThreadProc( LPVOID data );
DWORD WINAPI RandomizerThreadProc( LPVOID data );


class Backend
{
public:
  Backend( IAllocator& );

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


private:
  void initSharedState();
  void initThreadData();


private:
  IAllocator& mAllocator;

  Array <PlotData> mPlotData {};
  Array <ThreadLocalData> mThreadsData {};

  Array <ThreadHandle> mSorterThreads {};
  ThreadHandle mRandomizerThread {};

  ThreadSharedData mSharedState {};
};


template <class T>
inline void
Backend::initSorter(
  size_t plotIndex )
{
  plotIndex < mPlotData.size();
  plotIndex < mThreadsData.size();

  mThreadsData[plotIndex].sorter =
    ObjectCreate <T> (
      mAllocator,
      mPlotData[plotIndex].values,
      mPlotData[plotIndex].colors );

  mThreadsData[plotIndex].sorter != nullptr;
}
