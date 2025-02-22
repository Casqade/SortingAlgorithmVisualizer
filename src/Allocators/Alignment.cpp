#include <SortingAlgorithmVisualizer/Allocators/Alignment.hpp>



bool
IsValidAlignment(
  size_t alignment )
{
  return
    alignment != 0 &&
    !(alignment & alignment - 1);
}

uintptr_t
AlignAddress(
  uintptr_t address,
  size_t alignment )
{
  auto misalignment = address % alignment;

  if ( misalignment != 0 )
    address += (alignment - misalignment);

  return address;
}

void*
AlignAddress(
  void* address,
  size_t alignment )
{
  auto alignedAddress = AlignAddress(
    reinterpret_cast <uintptr_t> (address), alignment );

  return reinterpret_cast <void*> (alignedAddress);
}
