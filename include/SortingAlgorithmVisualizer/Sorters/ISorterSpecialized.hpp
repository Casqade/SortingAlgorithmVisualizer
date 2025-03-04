#pragma once

#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/Randomization.hpp>
#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>
#include <SortingAlgorithmVisualizer/Containers/Array.hpp>


template <typename T>
class ISorterSpecialized : public ISorter
{
public:
  ISorterSpecialized() = default;

  inline ~ISorterSpecialized()
  {
    mValues.deinit();
  }


  inline bool init( size_t valueCount, IAllocator& allocator ) override
  {
    if ( mValues.init(valueCount, allocator) == false )
    {
      MessageBox( NULL,
        "Failed to initialize plot values: "
        "Out of memory budget",
        NULL, MB_ICONERROR );

      ProgramShouldAbort = true;
      return false;
    }

    for ( size_t i {}; i < valueCount; ++i )
      mValues[i] = i + 1;

    mRandomizeTask.data = mValues.data();
    mRandomizeTask.elementCount = valueCount;

    return ISorter::init(valueCount, allocator);
  }

  inline void initValue( void* valueAddress, size_t value ) const override
  {
    T convertedValue = value;

    std::memcpy(
      valueAddress,
      &convertedValue,
      sizeof(T) );
  }

  inline size_t valueSize() const override
  {
    return sizeof(T);
  }

  GLenum valueType() const override;


  inline static size_t HeapMemoryBudget( size_t valueCount )
  {
    return
      sizeof(T) * valueCount +
      ISorter::HeapMemoryBudget(valueCount);
  }


protected:
  inline RandomizeFunction* getRandomizeCallback() const override
  {
    return RandomizePlotDataRdtsc <T>;
  }


protected:
  Array <T> mValues {};
};


template <>
inline GLenum
ISorterSpecialized <uint8_t>::valueType() const
{
  return GL_UNSIGNED_BYTE;
}

template <>
inline GLenum
ISorterSpecialized <uint16_t>::valueType() const
{
  return GL_UNSIGNED_SHORT;
}

template <>
inline GLenum
ISorterSpecialized <uint32_t>::valueType() const
{
  return GL_UNSIGNED_INT;
}

template <>
inline GLenum
ISorterSpecialized <int8_t>::valueType() const
{
  return GL_BYTE;
}

template <>
inline GLenum
ISorterSpecialized <int16_t>::valueType() const
{
  return GL_SHORT;
}

template <>
inline GLenum
ISorterSpecialized <int32_t>::valueType() const
{
  return GL_INT;
}

template <>
inline GLenum
ISorterSpecialized <float>::valueType() const
{
  return GL_FLOAT;
}

