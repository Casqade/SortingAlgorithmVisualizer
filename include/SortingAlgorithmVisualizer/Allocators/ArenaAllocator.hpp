#pragma once

#include <SortingAlgorithmVisualizer/Allocators/IAllocator.hpp>

#include <cstddef>


class ArenaAllocator : public IAllocator
{
public:
  ArenaAllocator() = default;
  ~ArenaAllocator() override;

  void* allocate( size_t bytes, size_t alignment ) override;
  void deallocate( void* block ) override;


protected:
  size_t mBytesAllocated {};
};
