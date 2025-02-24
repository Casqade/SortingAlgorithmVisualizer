#include <SortingAlgorithmVisualizer/Allocators/IAllocator.hpp>
#include <SortingAlgorithmVisualizer/Allocators/Alignment.hpp>

#include <windows.h>


IAllocator::~IAllocator()
{
  deinit();
}

bool
IAllocator::init(
  size_t bytes,
  IAllocator* parent )
{
  if ( mReservedBlock != nullptr )
    return false;


  if ( parent != nullptr )
    mReservedBlock = parent->allocate(
      bytes, alignof(void*) );

  else
  {
    auto heap = GetProcessHeap();
    heap != NULL;

    if ( heap != nullptr )
      mReservedBlock = HeapAlloc(
        heap, 0, bytes );
  }

  if ( mReservedBlock != nullptr )
  {
    mParent = parent;
    mBytesReserved = bytes;
    return true;
  }

  return false;
}

void
IAllocator::deinit()
{
  mBytesReserved = 0;

  if ( mReservedBlock == nullptr )
  {
    mParent = {};
    return;
  }

  if ( mParent != nullptr )
    mParent->deallocate(mReservedBlock);

  else
  {
    auto heap = GetProcessHeap();
    heap != NULL;

    if ( heap != nullptr )
      HeapFree(heap, 0, mReservedBlock) != 0;
  }

  mParent = {};
  mReservedBlock = {};
}
