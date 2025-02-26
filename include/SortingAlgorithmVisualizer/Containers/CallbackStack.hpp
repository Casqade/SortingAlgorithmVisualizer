#pragma once

#include <SortingAlgorithmVisualizer/Allocators/IAllocator.hpp>
#include <SortingAlgorithmVisualizer/Containers/Array.hpp>

#include <cstddef>


using Callback = void( void* data );

struct CallbackTask
{
  void* callbackData {};
  Callback* callback {};
};

class CallbackStack
{
  Array <CallbackTask> mTasks {};
  size_t mTop {};


public:
  CallbackStack() = default;
  CallbackStack( CallbackStack&& ) = delete;
  CallbackStack( const CallbackStack& ) = delete;
  ~CallbackStack();


  [[nodiscard]] bool init( size_t depth, IAllocator& );
  void deinit();

  void push( void* callbackData, Callback* );
  bool popAndCall();

  size_t depth() const;
};
