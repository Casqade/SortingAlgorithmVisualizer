#pragma once

#include <cstddef>


template <typename T>
class PlotData
{
  T* mValues {};
  size_t mSize {};


public:
  PlotData() = default;
  PlotData( const PlotData& ) = delete;
  PlotData( PlotData&& other ) = delete;

  inline ~PlotData()
  {
    delete[] mValues;
  }

  inline void init( size_t elements )
  {
    mValues = new T[elements];
    mSize = elements;
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
};
