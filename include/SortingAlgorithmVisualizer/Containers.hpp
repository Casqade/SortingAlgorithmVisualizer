#pragma once

#include <SortingAlgorithmVisualizer/Allocators/IAllocator.hpp>

#include <cstddef>
#include <utility>


template <typename T>
class Array
{
  T* mValues {};
  size_t mSize {};


public:
  Array() = default;
  Array( const Array& ) = delete;

  inline Array( Array&& other )
  {
    std::swap(mValues, other.mValues);
    std::swap(mSize, other.mSize);
  }

  inline ~Array()
  {
    deinit();
  }

  inline bool init( size_t elementCount, IAllocator& allocator, const T& defaultValue )
  {
    if ( elementCount == 0 )
      return false;


    mValues = ObjectsCreate <T> (
      allocator, elementCount, defaultValue );

    if ( mValues != nullptr )
    {
      mSize = elementCount;
      return true;
    }

    return false;
  }

  template <typename... Args>
  inline bool init( size_t elementCount, IAllocator& allocator, Args&&... args )
  {
    if ( elementCount == 0 )
      return false;


    mValues = ObjectsCreate <T> (
      allocator, elementCount,
      std::forward <T> (args)... );

    if ( mValues != nullptr )
    {
      mSize = elementCount;
      return true;
    }

    return false;
  }

  inline void deinit()
  {
    if ( mValues == nullptr )
      return;

    ObjectsDestroy(mValues, mSize);

    mSize = 0;
    mValues = {};
  }

  inline size_t size() const
  {
    return mSize;
  }

  inline T* data() const
  {
    return mValues;
  }

  inline T& operator [] ( size_t index )
  {
    return mValues[index];
  }

  inline T& operator [] ( size_t index ) const
  {
    return mValues[index];
  }

  inline T* begin() const
  {
    return mValues;
  }

  inline T* end() const
  {
    return mValues + mSize;
  }
};
