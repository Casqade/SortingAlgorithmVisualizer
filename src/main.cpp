#include <SortingAlgorithmVisualizer/Atomics.hpp>
#include <SortingAlgorithmVisualizer/Backend.hpp>
#include <SortingAlgorithmVisualizer/Frontend.hpp>
#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/MessageFormatting.hpp>
#include <SortingAlgorithmVisualizer/Containers/Array.hpp>
#include <SortingAlgorithmVisualizer/Sorters/MockSorter.hpp>
#include <SortingAlgorithmVisualizer/Allocators/ArenaAllocator.hpp>


bool ProgramShouldAbort {};


int
WINAPI
WinMain(
  HINSTANCE appInstance,
  HINSTANCE,
  LPSTR cmdArgs,
  int windowShowState )
{
  const size_t plotCount = 2;

  using Sorter1Type = MockSorter <float>;
  using Sorter2Type = MockSorter <int>;

  const size_t plot1ValueCount = 10;
  const size_t plot2ValueCount = 100;

  const size_t callbackStackDepth = 2;

  const auto heapMemoryBudget =
    sizeof(CallbackTask) * callbackStackDepth +
    sizeof(Backend) +
    Backend::HeapMemoryBudget(plotCount) +
    sizeof(Frontend) +
    Frontend::HeapMemoryBudget(plotCount) +
    sizeof(Sorter1Type) +
    sizeof(Sorter2Type) +
    Sorter1Type::HeapMemoryBudget(plot1ValueCount) +
    Sorter2Type::HeapMemoryBudget(plot2ValueCount) +

//    allocation bookkeeping
    sizeof(IAllocator*) * 9 +
    sizeof(IAllocator*) * 4 * plotCount +

//    alignment padding
    200;


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


  const char* plotTitles[] =
  {
    "Plot 1",
    "Plot 2",
  };

  backend->addSorter <Sorter1Type> (0, plot1ValueCount);
  backend->addSorter <Sorter2Type> (1, plot2ValueCount);


  auto frontend = ObjectCreate <Frontend> (arena, arena, *backend);

  if ( frontend == nullptr )
  {
    MessageBox( NULL,
      "Failed to create Frontend: "
      "Out of memory budget",
      NULL, MB_ICONERROR );

    return 0;
  }

  deinitStack.push( frontend,
  [] ( void* data )
  {
    ObjectDestroy(
      static_cast <Frontend*> (data) );
  } );


  frontend->init(plotCount, plotTitles, appInstance);

  for ( size_t i {}; i < plotCount; ++i )
    frontend->addSorter( i, backend->sorter(i) );


  if ( ProgramShouldAbort == false )
    backend->start();


  do
  {
    frontend->processMessages();
    frontend->draw();
  }
  while ( ProgramShouldAbort == false );

  backend->stop();

  frontend->closeAllWindows();


  return 0;
}
