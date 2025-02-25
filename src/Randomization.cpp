#include <SortingAlgorithmVisualizer/Randomization.hpp>
#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/Atomics.hpp>


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

      resultCode != 0;
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
