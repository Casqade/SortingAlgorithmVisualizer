#include <SortingAlgorithmVisualizer/Containers/CallbackStack.hpp>

#include <cassert>


CallbackStack::~CallbackStack()
{
  deinit();
}

bool
CallbackStack::init(
  size_t depth,
  IAllocator& allocator )
{
  return mTasks.init(depth, allocator);
}

void
CallbackStack::deinit()
{
  while ( popAndCall() == true )
    ;

  mTasks.deinit();
}

void
CallbackStack::push(
  void* callbackData,
  Callback* callback )
{
  if ( mTop >= mTasks.size() )
  {
    assert(false && "CallbackStack overflow. Try increasing depth");
    return;
  }

  mTasks[mTop++] =
  {
    callbackData, callback,
  };
}

bool
CallbackStack::popAndCall()
{
  if ( mTop == 0 )
    return false;

  auto& task = mTasks[--mTop];

  task.callback(task.callbackData);

  return true;
}

size_t
CallbackStack::depth() const
{
  return mTasks.size();
}
