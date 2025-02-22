#pragma once

#include <cstddef>
#include <cstring>
#include <new>
#include <type_traits>


class IAllocator
{
public:
  IAllocator() = default;
  IAllocator( const IAllocator& ) = delete;
  virtual ~IAllocator();

  bool init( size_t bytes, IAllocator* parent = {} );
  void deinit();

  virtual void* allocate( size_t bytes, size_t alignment ) = 0;
  virtual void deallocate( void* block ) = 0;


  template <typename T, typename... Args>
  T* create( Args&&... );

  template <typename T>
  void destroy( T* object );


protected:
  IAllocator* mParent {};

  void* mReservedBlock {};
  size_t mBytesReserved {};
  size_t mBytesAllocated {};
};


template <typename T, typename... Args>
T*
IAllocator::create(
  Args&&... args )
{
  void* storage = allocate(
    sizeof(T) + sizeof(IAllocator*),
    alignof(T) );

  if ( storage == nullptr )
    return nullptr;


  return new(storage) T(std::forward <Args> (args)...);
}

template <typename T>
void
IAllocator::destroy(
  T* object )
{
  object->~T();
  deallocate(object);
}


template <typename T, typename... Args>
T*
ObjectCreate(
  IAllocator& allocator,
  Args&&... args )
{
  void* storage = allocator.allocate(
    sizeof(T) + sizeof(IAllocator*),
    alignof(T) );

  if ( storage == nullptr )
    return nullptr;

  auto allocatorRefLocation =
    reinterpret_cast <uintptr_t> (storage) + sizeof(T);

  auto allocatorAddress = &allocator;

  std::memcpy(
    reinterpret_cast <void*> (allocatorRefLocation),
    &allocatorAddress,
    sizeof(void*) );


  return new(storage) T(std::forward <Args> (args)...);
}

template <typename T>
void
ObjectDestroy(
  T* object )
{
  object->~T();

  auto allocatorAddress =
    reinterpret_cast <uintptr_t> (object) + sizeof(T);

  IAllocator* allocator;

  std::memcpy(
    &allocator,
    reinterpret_cast <void*> (allocatorAddress),
    sizeof(allocator) );

  allocator->deallocate(object);
}
