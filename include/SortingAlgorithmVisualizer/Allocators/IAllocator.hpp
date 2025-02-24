#pragma once

#include <cstddef>
#include <cstring>
#include <new>
#include <utility>


class IAllocator
{
public:
  IAllocator() = default;
  IAllocator( const IAllocator& ) = delete;
  virtual ~IAllocator();

  [[nodiscard]] bool init( size_t bytes, IAllocator* parent = {} );
  void deinit();

  virtual void* allocate( size_t bytes, size_t alignment ) = 0;
  virtual void deallocate( void* block ) = 0;


  template <typename T, size_t N, typename... Args>
  T* create( Args&&... );

  template <typename T, typename... Args>
  T* create( size_t count, Args&&... );

  template <typename T>
  void destroy( T* objects, size_t N = 1 );


protected:
  IAllocator* mParent {};

  void* mReservedBlock {};
  size_t mBytesReserved {};
};


template <typename T, typename... Args>
T*
IAllocator::create(
  size_t count,
  Args&&... args )
{
  void* storage = allocate(
    sizeof(T) * count,
    alignof(T) );

  if ( storage == nullptr )
    return nullptr;


  auto objects = static_cast <T*> (storage);

  for ( size_t i {}; i < count; ++i )
    ::new(objects + i) T(std::forward <Args> (args)...);


  return objects;
}

template <typename T>
void
IAllocator::destroy(
  T* objects,
  size_t count )
{
  for ( size_t i {}; i < count; ++i )
    objects[i].~T();

  deallocate(objects);
}


inline void
WriteObjectAddress(
  void* object,
  uintptr_t destination )
{
  auto address =
    reinterpret_cast <uintptr_t> (object);

  std::memcpy(
    reinterpret_cast <void*> (destination),
    &address,
    sizeof(void*) );
}

template <typename T>
inline T*
ReadObjectAddress(
  uintptr_t from )
{
  T* object;

  std::memcpy(
    &object,
    reinterpret_cast <void*> (from),
    sizeof(object) );

  return object;
}


template <typename T, typename... Args>
T*
ObjectsCreate(
  IAllocator& allocator,
  size_t count,
  Args&&... args )
{
  void* storage = allocator.allocate(
    sizeof(T) * count + sizeof(IAllocator*),
    alignof(T) );

  if ( storage == nullptr )
    return nullptr;


  auto objects = static_cast <T*> (storage);

  for ( size_t i {}; i < count; ++i )
    ::new(objects + i) T(std::forward <Args> (args)...);

  WriteObjectAddress(
    &allocator,
    reinterpret_cast <uintptr_t> (objects) + sizeof(T) * count );


  return objects;
}

template <typename T, typename... Args>
T*
ObjectCreate(
  IAllocator& allocator,
  Args&&... args )
{
  return ObjectsCreate <T> (
    allocator, 1,
    std::forward <Args> (args)... );
}

template <typename T, size_t N, typename... Args>
T*
ObjectsCreate(
  IAllocator& allocator,
  Args&&... args )
{
  return ObjectsCreate <T> (
    allocator, N,
    std::forward <Args> (args)... );
}

template <typename T>
void
ObjectsDestroy(
  T* objects,
  size_t count )
{
  auto allocator = ReadObjectAddress <IAllocator> (
    reinterpret_cast <uintptr_t> (objects) + sizeof(T) * count );

  allocator->destroy(objects, count);
}

template <typename T>
void
ObjectDestroy(
  T* objects )
{
  ObjectsDestroy(objects, 1);
}
