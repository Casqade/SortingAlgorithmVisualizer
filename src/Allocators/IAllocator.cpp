#include <SortingAlgorithmVisualizer/Allocators/IAllocator.hpp>
#include <SortingAlgorithmVisualizer/Allocators/Alignment.hpp>
#include <SortingAlgorithmVisualizer/MessageFormatting.hpp>

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

    if ( heap != nullptr )
    {
      mReservedBlock = HeapAlloc(
        heap, 0, bytes );
    }
    else
    {
      MessageBox( NULL,
        FormatUserMessagePassthrough(
          "Failed get process heap for allocation: %1",
          FormatSystemMessage() ),
        NULL, MB_ICONERROR );
    }
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

    if ( heap == nullptr )
    {
      MessageBox( NULL,
        FormatUserMessagePassthrough(
          "Failed get process heap for deallocation: %1",
          FormatSystemMessage() ),
        NULL, MB_ICONERROR );
    }
    else if ( HeapFree(heap, 0, mReservedBlock) == FALSE )
    {
      MessageBox( NULL,
        FormatUserMessagePassthrough(
          "Failed to free heap memory block: %1",
          FormatSystemMessage() ),
        NULL, MB_ICONERROR );
    }
  }

  mParent = {};
  mReservedBlock = {};
}
