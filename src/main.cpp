#include <SortingAlgorithmVisualizer/Atomics.hpp>
#include <SortingAlgorithmVisualizer/Backend.hpp>
#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/Containers/Array.hpp>
#include <SortingAlgorithmVisualizer/Sorters/MockSorter.hpp>
#include <SortingAlgorithmVisualizer/Allocators/ArenaAllocator.hpp>


int
main()
{
  const size_t plotCount = 2;
  const size_t plotValueCount = 1000;
  using ThreadHandle = HANDLE;

  const auto heapMemoryBudget1 =
    sizeof(Backend) +
    sizeof(RandomizeTask*) * plotCount +
    sizeof(PlotData) * plotCount +
    sizeof(PlotValueType) * plotValueCount * plotCount +
    sizeof(PlotValueColorIndex) * plotValueCount * plotCount +
    sizeof(ThreadLocalData) * plotCount +
    sizeof(ThreadHandle) * plotCount +
    sizeof(MockSorter <PlotValueType>) * plotCount +
    sizeof(IAllocator*) * 100; // reserved for alignment padding & allocation bookkeeping


  ArenaAllocator arena {};
  arena.init(heapMemoryBudget1);

  auto backend = ObjectCreate <Backend> (arena, arena);
  backend != nullptr;

  backend->init(plotCount);


  for ( size_t i {}; i < plotCount; ++i )
  {
    backend->initData(i, plotValueCount);
    backend->initSorter <MockSorter <PlotValueType>> (i);
  }


  backend->start();

  for ( size_t frame {}; frame < 1000; ++frame )
  {
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

  backend->deinit();


  return 0;
}
