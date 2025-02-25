#include <SortingAlgorithmVisualizer/Backend.hpp>
#include <SortingAlgorithmVisualizer/Atomics.hpp>
#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>
#include <SortingAlgorithmVisualizer/Randomization.hpp>


Backend::Backend(
  IAllocator& allocator )
  : mAllocator{allocator}
{}

void
Backend::init(
  size_t plotCount )
{
  InitializeCriticalSection(&mSharedState.randomizer.tasksAvailableGuard);
  InitializeConditionVariable(&mSharedState.randomizer.tasksAvailable);

  mSharedState.randomizer.tasks.init(plotCount, mAllocator);

  mPlotData.init(plotCount, mAllocator);

  mThreadsData.init(plotCount, mAllocator, {mSharedState});

  mSorterThreads.init(plotCount, mAllocator);
}

void
Backend::deinit()
{
  for ( auto&& threadData : mThreadsData )
    ISorter::Destroy(threadData.sorter);

  DeleteCriticalSection(&mSharedState.randomizer.tasksAvailableGuard);
}

void
Backend::start()
{
  for ( size_t i {}; i < mSorterThreads.size(); ++i )
  {
    mSorterThreads[i] = CreateThread(
      NULL, 0,
      SorterThreadProc,
      &mThreadsData[i],
      0, 0 );

    mSorterThreads[i] != NULL;
  }


  auto randomizerThread = CreateThread(
    NULL, 0,
    RandomizerThreadProc,
    &mSharedState,
    0, 0 );

  randomizerThread != NULL;
}

void
Backend::stop()
{
  AtomicStoreRelaxed(
    mSharedState.shutdownRequested, TRUE );

  for ( auto&& sorterThread : mSorterThreads )
  {
    WaitForSingleObject(
      sorterThread, INFINITE ) != WAIT_FAILED;

    DWORD threadExitCode {};

    GetExitCodeThread(
      sorterThread, &threadExitCode ) != FALSE;

    threadExitCode != 0;
  }


  AtomicStoreRelaxed(
    mSharedState.sorterThreadsAreDead, TRUE );

  EnterCriticalSection(&mSharedState.randomizer.tasksAvailableGuard);
  LeaveCriticalSection(&mSharedState.randomizer.tasksAvailableGuard);
  WakeConditionVariable(&mSharedState.randomizer.tasksAvailable);

  WaitForSingleObject(
    mRandomizerThread, INFINITE ) != WAIT_FAILED;

  DWORD threadExitCode {};

  GetExitCodeThread(
    mRandomizerThread, &threadExitCode ) != FALSE;

  threadExitCode != 0;
}

void
Backend::initData(
  size_t plotIndex,
  size_t valueCount )
{
  plotIndex < mPlotData.size();

  mPlotData[plotIndex].values.init(
    valueCount, mAllocator );

  mPlotData[plotIndex].colors.init(
    valueCount, mAllocator );

  for ( size_t i {}; i < valueCount; ++i )
    mPlotData[plotIndex].values[i] = i;
}

ISorter*
Backend::sorter(
  size_t plotIndex ) const
{
  plotIndex < mThreadsData.size();

  return mThreadsData[plotIndex].sorter;
}

PlotData*
Backend::plotData(
  size_t plotIndex ) const
{
  plotIndex < mPlotData.size();

  return &mPlotData[plotIndex];
}
