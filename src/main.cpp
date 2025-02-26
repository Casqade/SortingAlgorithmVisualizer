#include <SortingAlgorithmVisualizer/Atomics.hpp>
#include <SortingAlgorithmVisualizer/Backend.hpp>
#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/Containers/Array.hpp>
#include <SortingAlgorithmVisualizer/Sorters/MockSorter.hpp>
#include <SortingAlgorithmVisualizer/Allocators/ArenaAllocator.hpp>


bool ProgramShouldAbort {};


int
main()
{
  const size_t plotCount = 2;
  const size_t plotValueCount = 1000;

  const size_t callbackStackDepth = 1;

  const auto heapMemoryBudget =
    sizeof(CallbackTask) * callbackStackDepth +
    Backend::HeapMemoryBudget(plotCount) +
    sizeof(PlotValueType) * plotValueCount * plotCount +
    sizeof(PlotValueColorIndex) * plotValueCount * plotCount +
    sizeof(MockSorter <PlotValueType>) * plotCount +
    sizeof(IAllocator*) * 100; // reserved for alignment padding & allocation bookkeeping


  ArenaAllocator arena {};
  if ( arena.init(heapMemoryBudget) == false )
  {
    MessageBox( NULL,
      "Failed to allocate enough heap memory",
      NULL, MB_ICONERROR );

    return 0;
  }


  CallbackStack deinitStack {};

  if ( deinitStack.init(callbackStackDepth, arena) == false )
  {
    MessageBox( NULL,
      "Failed to initialize WinMain deinit stack",
      NULL, MB_ICONERROR );

    return 0;
  }


  auto backend = ObjectCreate <Backend> (arena, arena);

  if ( backend == nullptr )
  {
    MessageBox( NULL,
      "Failed to allocate memory for Backend",
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


  for ( size_t i {}; i < plotCount; ++i )
  {
    backend->initData(i, plotValueCount);
    backend->initSorter <MockSorter <PlotValueType>> (i);
  }


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
