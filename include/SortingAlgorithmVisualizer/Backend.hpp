#pragma once

#include <SortingAlgorithmVisualizer/fwd.hpp>
#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/MessageFormatting.hpp>
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

  template <class T>
  ISorter* addSorter( size_t plotIndex, size_t plotValueCount );

  void start();
  void stop();

  ISorter* sorter( size_t plotIndex ) const;


  static size_t CallbackStackDepth( size_t plotCount );
  static size_t HeapMemoryBudget( size_t plotCount );


private:
  void setThreadAffinities();


private:
  IAllocator& mAllocator;

  Array <ThreadLocalData> mThreadsData {};

  Array <ThreadHandle> mSorterThreads {};
  ThreadHandle mRandomizerThread {};

  ThreadSharedData mSharedState {};

  CallbackStack mDeinitStack {};
};


template <class T>
inline
ISorter*
Backend::addSorter(
  size_t plotIndex,
  size_t plotValueCount )
{
  if ( ProgramShouldAbort == true )
    return nullptr;

  assert(plotValueCount != 0);
  assert(plotIndex < mThreadsData.size());


  auto& sorter = mThreadsData[plotIndex].sorter;
  assert(sorter == nullptr);

  sorter = ObjectCreate <T> (mAllocator);

  if ( sorter == nullptr )
  {
    MessageBox( NULL,
      FormatUserMessagePassthrough(
        "Failed to create sorter %1!u!: "
        "Out of memory budget",
        plotIndex ),
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return nullptr;
  }

  mDeinitStack.push( sorter,
  [] ( void* data )
  {
    ISorter::Destroy(
      static_cast <ISorter*> (data) );
  });


  if ( sorter->init(plotValueCount, mAllocator) == false )
  {
    MessageBox( NULL,
      FormatUserMessagePassthrough(
        "Failed to init sorter %1!u!: "
        "Out of memory budget",
        plotIndex ),
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return nullptr;
  }

  return sorter;
}
