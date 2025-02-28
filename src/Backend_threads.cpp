#include <SortingAlgorithmVisualizer/Atomics.hpp>
#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>
#include <SortingAlgorithmVisualizer/MessageFormatting.hpp>

#include <cassert>

/* CVAR_NOTE

  Note on SleepConditionVariableCS:

  MSDN doesn't state whether
  SleepConditionVariableCS failure requires
  leaving critical section or not,
  so we have to assert

*/


const static auto SortingStepPresentTimeMs = 30;
const static auto SortedPlotPresentTimeMs = 3000;


DWORD WINAPI
SorterThreadProc(
  LPVOID data )
{
  auto threadData = static_cast <ThreadLocalData*> (data);

  auto sorter = threadData->sorter;

  auto& sharedState = threadData->sharedState;

  bool isSorted {true};

  while ( AtomicLoadRelaxed(sharedState.shutdownRequested) == FALSE )
  {
    if ( isSorted == true )
    {
      Sleep(SortedPlotPresentTimeMs);


      auto& randomizerData = sharedState.randomizer;

      PVOID currentTask {};

      auto& newTask = sorter->getRandomizeTask();


      auto writeIndex =
        randomizerData.tasksIndices.acquireWriteIndex();

      auto taskIndex = writeIndex % randomizerData.tasks.size();

      randomizerData.tasks[taskIndex] = &newTask;

      randomizerData.tasksIndices.commitWrittenIndex(writeIndex);


      EnterCriticalSection(&randomizerData.tasksAvailableGuard);
      LeaveCriticalSection(&randomizerData.tasksAvailableGuard);
      WakeConditionVariable(&randomizerData.tasksAvailable);

//      TOOD: reset plot colors

      EnterCriticalSection(&newTask.taskFinishedGuard);

      while ( newTask.callback != nullptr )
      {
        auto resultCode = SleepConditionVariableCS(
          &newTask.taskFinished,
          &newTask.taskFinishedGuard,
          INFINITE );

          if ( resultCode == FALSE )
          {
            assert(resultCode != FALSE); // see CVAR_NOTE at the top

            MessageBox( NULL,
              FormatUserMessagePassthrough(
                "SleepConditionVariableCS failure in sorter thread: %1",
                FormatSystemMessage() ),
              NULL, MB_ICONERROR );
          }
      }

      LeaveCriticalSection(&newTask.taskFinishedGuard);

      sorter->reset();
    }
    else
      Sleep(SortingStepPresentTimeMs);

    isSorted = sorter->step();
  }

  return 0;
}


DWORD WINAPI
RandomizerThreadProc(
  LPVOID data )
{
  auto sharedState = static_cast <ThreadSharedData*> (data);

  auto& randomizerData = sharedState->randomizer;

  while ( AtomicLoadRelaxed(sharedState->sorterThreadsAreDead) == FALSE )
  {
    EnterCriticalSection(&randomizerData.tasksAvailableGuard);

    while ( randomizerData.tasksIndices.canAcquireReadIndex() == false &&
            AtomicLoadRelaxed(sharedState->sorterThreadsAreDead) == FALSE )
    {
      auto resultCode = SleepConditionVariableCS(
        &randomizerData.tasksAvailable,
        &randomizerData.tasksAvailableGuard,
        INFINITE );

      if ( resultCode == FALSE )
      {
        assert(resultCode != FALSE); // see CVAR_NOTE at the top

        MessageBox( NULL,
          FormatUserMessagePassthrough(
            "SleepConditionVariableCS failure in randomizer thread: %1",
            FormatSystemMessage() ),
          NULL, MB_ICONERROR );
      }
    }

    LeaveCriticalSection(&randomizerData.tasksAvailableGuard);

    if ( randomizerData.tasksIndices.canAcquireReadIndex() == false )
      continue;


    auto readIndex =
      randomizerData.tasksIndices.acquireReadIndex();

    auto taskIndex = readIndex % randomizerData.tasks.size();

    auto task = randomizerData.tasks[taskIndex];

    if ( task != nullptr )
    {
      task->callback(
        task->data,
        task->elementCount,
        *task->dataGuard );

      EnterCriticalSection(&task->taskFinishedGuard);
        task->callback = nullptr; // mark task as done
      LeaveCriticalSection(&task->taskFinishedGuard);

      WakeConditionVariable(&task->taskFinished);
    }
  }

  return 0;
}
