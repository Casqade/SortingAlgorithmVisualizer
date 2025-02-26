#include <SortingAlgorithmVisualizer/Backend.hpp>
#include <SortingAlgorithmVisualizer/Atomics.hpp>
#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>
#include <SortingAlgorithmVisualizer/Randomization.hpp>

#include <intrin.h>

#include <cassert>


Backend::Backend(
  IAllocator& allocator )
  : mAllocator{allocator}
{}

Backend::~Backend()
{
  deinit();
}

size_t
Backend::CallbackStackDepth(
  size_t plotCount )
{
  return 6 + 4 * plotCount;
}

size_t
Backend::HeapMemoryBudget(
  size_t plotCount )
{
  return
    sizeof(Backend) +
    CallbackStackDepth(plotCount) +
    sizeof(RandomizeTask*) * plotCount +
    sizeof(PlotData) * plotCount +
    sizeof(ThreadLocalData) * plotCount +
    sizeof(ThreadHandle) * plotCount;
}

void
Backend::init(
  size_t plotCount )
{
  auto callbackStackDepth = CallbackStackDepth(plotCount);

  if ( mDeinitStack.init(callbackStackDepth, mAllocator) == false )
  {
    MessageBox( NULL,
      "Failed to initialize Backend deinit stack",
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return;
  }


  InitializeCriticalSection(&mSharedState.randomizer.tasksAvailableGuard);
  InitializeConditionVariable(&mSharedState.randomizer.tasksAvailable);

  mDeinitStack.push( &mSharedState.randomizer.tasksAvailableGuard,
  [] ( void* data )
  {
    DeleteCriticalSection(static_cast <CRITICAL_SECTION*> (data));
  });


  if ( mSharedState.randomizer.tasks.init(plotCount, mAllocator) == false )
  {
    MessageBox( NULL,
      "Failed to initialize randomizer tasks",
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return;
  }

  mDeinitStack.push( &mSharedState.randomizer.tasks,
  [] ( void* data )
  {
    using Tasks = decltype(mSharedState.randomizer.tasks);
    static_cast <Tasks*> (data)->deinit();
  });


  if ( mPlotData.init(plotCount, mAllocator) == false )
  {
    MessageBox( NULL,
      "Failed to initialize plot data",
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return;
  }

  mDeinitStack.push( &mPlotData,
  [] ( void* data )
  {
    static_cast <decltype(mPlotData)*> (data)->deinit();
  });


  if ( mThreadsData.init(plotCount, mAllocator, {mSharedState}) == false )
  {
    MessageBox( NULL,
      "Failed to initialize thread local data",
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return;
  }

  mDeinitStack.push( &mThreadsData,
  [] ( void* data )
  {
    static_cast <decltype(mThreadsData)*> (data)->deinit();
  });


  if ( mSorterThreads.init(plotCount, mAllocator) == false )
  {
    MessageBox( NULL,
      "Failed to initialize sorter threads",
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return;
  }

  mDeinitStack.push( &mSorterThreads,
  [] ( void* data )
  {
    static_cast <decltype(mSorterThreads)*> (data)->deinit();
  });
}

void
Backend::deinit()
{
  while ( mDeinitStack.popAndCall() == true )
    ;
}

void
Backend::setThreadAffinities()
{
  DWORD_PTR processAffinity {};
  DWORD_PTR systemAffinity {};

  {
    auto result = GetProcessAffinityMask(
      GetCurrentProcess(),
      &processAffinity,
      &systemAffinity );

    if ( result == FALSE )
    {
      MessageBox( NULL,
        "Failed to get process afinity mask. "
        "Thread affinities won't be set",
        NULL, MB_ICONWARNING );

      return;
    }
  }


  auto nextEvenCoreIndex =
  [] ( auto& coreIndex, auto& affinityMask ) -> bool
  {
    while ( coreIndex % 2 != 0 )
    {
      affinityMask &= ~(1 << coreIndex);

      auto hasBit = _BitScanForward64(
        &coreIndex, affinityMask );

      if ( hasBit == FALSE )
        return false;
    }

    return true;
  };


  unsigned long coreIndex {};

  if ( nextEvenCoreIndex(coreIndex, processAffinity) == false )
  {
    MessageBox( NULL,
      "Failed to set threads affinities: "
      "Process affinity mask is too restrictive or this "
      "machine has insufficient number of available CPU cores",
      NULL, MB_ICONWARNING );

    return;
  }


  auto result = SetThreadAffinityMask(
    GetCurrentThread(),
    1 << coreIndex );

  if ( result == 0 )
  {
    MessageBox( NULL,
      "Failed to set affinity for main thread",
      NULL, MB_ICONWARNING );
  }

  processAffinity &= ~(1 << coreIndex);


  if ( nextEvenCoreIndex(coreIndex, processAffinity) == false )
  {
    MessageBox( NULL,
      "Failed to set affinity for randomizer thread: "
      "Process affinity mask is too restrictive or this "
      "machine has insufficient number of available CPU cores",
      NULL, MB_ICONWARNING );

    return;
  }

  result = SetThreadAffinityMask(
    mRandomizerThread,
    1 << coreIndex );

  if ( result == 0 )
  {
    MessageBox( NULL,
      "Failed to set affinity for randomizer thread",
      NULL, MB_ICONWARNING );
  }


  for ( size_t i {}; i < mSorterThreads.size(); ++i )
  {
    processAffinity &= ~(1 << coreIndex);

    if ( nextEvenCoreIndex(coreIndex, processAffinity) == false )
    {
      MessageBox( NULL,
        "Failed to set affinity for sorter thread(s): "
        "Process affinity mask is too restrictive or this "
        "machine has insufficient number of available CPU cores",
        NULL, MB_ICONWARNING );

      return;
    }


    result = SetThreadAffinityMask(
      mSorterThreads[i],
      1 << coreIndex );

    if ( result == 0 )
    {
      MessageBox( NULL,
        "Failed to set affinity for sorter thread",
        NULL, MB_ICONWARNING );
    }
  }
}

void
Backend::start()
{
  if ( ProgramShouldAbort == true )
    return;


  mRandomizerThread = CreateThread(
    NULL, 0,
    RandomizerThreadProc,
    &mSharedState,
    0, 0 );

  if ( mRandomizerThread == NULL )
  {
    MessageBox( NULL,
      "Failed to create randomizer thread",
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return;
  }

  mDeinitStack.push( &mRandomizerThread,
  [] ( void* data )
  {
    CloseHandle(*static_cast <HANDLE*> (data));
  });


  for ( size_t i {}; i < mSorterThreads.size(); ++i )
  {
    auto& thread = mSorterThreads[i];

    thread = CreateThread(
      NULL, 0,
      SorterThreadProc,
      &mThreadsData[i],
      0, 0 );

    if ( thread == NULL )
    {
      MessageBox( NULL,
      "Failed to create sorter thread",
      NULL, MB_ICONERROR );

      ProgramShouldAbort = true;
      return;
    }

    mDeinitStack.push( &thread,
    [] ( void* data )
    {
      CloseHandle(*static_cast <HANDLE*> (data));
    });
  }

  setThreadAffinities();
}

void
Backend::stop()
{
  AtomicStoreRelaxed(
    mSharedState.shutdownRequested, TRUE );

  if ( mSorterThreads.size() > 0 )
  {
    for ( auto&& sorterThread : mSorterThreads )
    {
      if ( sorterThread == NULL )
        continue;


      {
        auto result = WaitForSingleObject(
          sorterThread, INFINITE );

        assert(result != WAIT_FAILED);
      }


      DWORD threadExitCode;

      auto result = GetExitCodeThread(
        sorterThread, &threadExitCode );

      assert(result != FALSE);
      assert(threadExitCode == 0);
    }
  }


  AtomicStoreRelaxed(
    mSharedState.sorterThreadsAreDead, TRUE );


  if ( mRandomizerThread == NULL )
    return;


  EnterCriticalSection(&mSharedState.randomizer.tasksAvailableGuard);
  LeaveCriticalSection(&mSharedState.randomizer.tasksAvailableGuard);
  WakeConditionVariable(&mSharedState.randomizer.tasksAvailable);


  {
    auto result = WaitForSingleObject(
    mRandomizerThread, INFINITE );

    assert(result != WAIT_FAILED);
  }

  DWORD threadExitCode;

  auto result = GetExitCodeThread(
    mRandomizerThread, &threadExitCode );

  assert(result != FALSE);
  assert(threadExitCode == 0);
}

void
Backend::initData(
  size_t plotIndex,
  size_t valueCount )
{
  if ( ProgramShouldAbort == true )
    return;


  assert(plotIndex < mPlotData.size());


  auto& plotData = mPlotData[plotIndex];


  if ( plotData.values.init(valueCount, mAllocator) == false )
  {
    MessageBox( NULL,
      "Failed to initialize plot values",
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return;
  }

  mDeinitStack.push( &plotData.values,
  [] ( void* data )
  {
    using PlotValues = decltype(plotData.values);
    static_cast <PlotValues*> (data)->deinit();
  });


  if ( plotData.colors.init(valueCount, mAllocator) == false )
  {
    MessageBox( NULL,
      "Failed to initialize plot colors",
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return;
  }

  mDeinitStack.push( &plotData.colors,
  [] ( void* data )
  {
    using PlotColors = decltype(plotData.colors);
    static_cast <PlotColors*> (data)->deinit();
  });


  for ( size_t i {}; i < valueCount; ++i )
    plotData.values[i] = i;
}

ISorter*
Backend::sorter(
  size_t plotIndex ) const
{
  assert(plotIndex < mThreadsData.size());

  return mThreadsData[plotIndex].sorter;
}

PlotData*
Backend::plotData(
  size_t plotIndex ) const
{
  assert(plotIndex < mPlotData.size());

  return &mPlotData[plotIndex];
}
