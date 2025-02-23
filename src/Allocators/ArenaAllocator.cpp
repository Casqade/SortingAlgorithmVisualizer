#include <SortingAlgorithmVisualizer/Allocators/ArenaAllocator.hpp>
#include <SortingAlgorithmVisualizer/Allocators/Alignment.hpp>


ArenaAllocator::~ArenaAllocator()
{
  mBytesAllocated = 0;
}

void*
ArenaAllocator::allocate(
  size_t bytes,
  size_t alignment )
{
  if (  mReservedBlock == nullptr ||
        IsValidAlignment(alignment) == false )
    return nullptr;


  auto reservedBlock =
    reinterpret_cast <uintptr_t> (mReservedBlock);

  auto allocatedBlock = AlignAddress(
    reservedBlock + mBytesAllocated,
    alignment );

  auto allocationEnd = allocatedBlock + bytes;

  if ( allocationEnd > reservedBlock + mBytesReserved )
    return nullptr;

  mBytesAllocated = allocationEnd - reservedBlock;

  return reinterpret_cast <void*> (allocatedBlock);
}

void
ArenaAllocator::deallocate(
  void* block )
{
//  NOP
}
