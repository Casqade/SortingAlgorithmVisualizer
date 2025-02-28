#include <SortingAlgorithmVisualizer/Atomics.hpp>
#include <SortingAlgorithmVisualizer/Backend.hpp>
#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/MessageFormatting.hpp>
#include <SortingAlgorithmVisualizer/Containers/Array.hpp>
#include <SortingAlgorithmVisualizer/Sorters/MockSorter.hpp>
#include <SortingAlgorithmVisualizer/Allocators/ArenaAllocator.hpp>


bool ProgramShouldAbort {};


int
main()
{
  using PlotValueType = uint32_t;
  using SorterType = MockSorter <PlotValueType>;

  const size_t plotCount = 2;
  const size_t plotValueCount = 1000;

  const size_t callbackStackDepth = 1;

  const auto heapMemoryBudget =
    sizeof(CallbackTask) * callbackStackDepth +
    sizeof(Backend) +
    Backend::HeapMemoryBudget(plotCount) +
    sizeof(SorterType) * plotCount +
    SorterType::HeapMemoryBudget(plotValueCount) * plotCount+
    sizeof(IAllocator*) * 100; // reserved for alignment padding & allocation bookkeeping


  ArenaAllocator arena {};
  if ( arena.init(heapMemoryBudget) == false )
  {
    MessageBox( NULL,
      FormatUserMessagePassthrough(
        "Failed to allocate program memory budget of %1!u! bytes",
        heapMemoryBudget ),
      NULL, MB_ICONERROR );

    return 0;
  }


  CallbackStack deinitStack {};

  if ( deinitStack.init(callbackStackDepth, arena) == false )
  {
    MessageBox( NULL,
      "Failed to initialize WinMain deinit stack: "
      "Out of memory budget",
      NULL, MB_ICONERROR );

    return 0;
  }


  auto backend = ObjectCreate <Backend> (arena, arena);

  if ( backend == nullptr )
  {
    MessageBox( NULL,
      "Failed to create Backend: "
      "Out of memory budget",
      NULL, MB_ICONERROR );

    return 0;
  }

  deinitStack.push( backend,
  [] ( void* data )
  {
    ObjectDestroy(
      static_cast <Backend*> (data) );
  } );


  backend->init(plotCount);

  backend->addSorter <MockSorter <int>> (0, 1000);
  backend->addSorter <MockSorter <int>> (1, 1000);

  backend->start();


  for ( size_t frame {}; frame < 1000; ++frame )
  {
    if ( ProgramShouldAbort == true )
      break;


    for ( size_t i {}; i < plotCount; ++i )
    {
      auto sorter = backend->sorter(i);

      if ( sorter->tryLockData() == FALSE )
        continue;

//      simulate copying to back buffer
      Sleep(5);

      sorter->unlockData();
    }

//    write to vertex buffer
//    glVertexAttribDivisor + glDrawArraysInstanced
//    swap
  }

  backend->stop();


  return 0;
}
