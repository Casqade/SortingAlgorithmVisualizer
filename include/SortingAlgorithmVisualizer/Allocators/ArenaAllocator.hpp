#pragma once

#include <SortingAlgorithmVisualizer/Allocators/IAllocator.hpp>

#include <cstddef>


class ArenaAllocator : public IAllocator
{
public:
  ArenaAllocator() = default;

  void* allocate( size_t bytes, size_t alignment ) override;
  void deallocate( void* block ) override;
};
