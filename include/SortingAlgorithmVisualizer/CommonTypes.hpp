#pragma once

#include <SortingAlgorithmVisualizer/fwd.hpp>
#include <SortingAlgorithmVisualizer/Containers/Array.hpp>
#include <SortingAlgorithmVisualizer/Containers/RingBuffer.hpp>
#include <SortingAlgorithmVisualizer/Containers/CallbackStack.hpp>

#include <glad/gl.h>

#include <windows.h>

#include <cstdint>


extern bool ProgramShouldAbort;


using ThreadHandle = HANDLE;


using RandomizeFunction =
  void( void* data, size_t elementCount );


struct RandomizeTask
{
  CRITICAL_SECTION taskFinishedGuard {};
  CONDITION_VARIABLE taskFinished {};

  RandomizeFunction* callback {};

  void* data {};
  size_t elementCount {};
};


struct ThreadSharedData
{
  struct
  {
    RingBuffer tasksIndices {};
    Array <RandomizeTask*> tasks {};

    CRITICAL_SECTION tasksAvailableGuard {};
    CONDITION_VARIABLE tasksAvailable {};

  } randomizer {};

  alignas(CacheLineSize)
  LONG shutdownRequested {};
  LONG sorterThreadsAreDead {};
};


struct ThreadLocalData
{
  ThreadSharedData& sharedState;
  ISorter* sorter {};
};


struct WindowData
{
  GladGLContext glContext {};

  HGLRC renderContext {};
  HDC deviceContext {};

  HWND window {};
  const char* title {};

  GLuint shaderProgram {};
  GLuint vertexArray {};

  GLuint valuesBufferId {};
  GLuint colorsBufferId {};

  GLsync bufferFences[3] {};
  LONG currentBufferIndex {0};

  CallbackStack deinitStack {};
};


namespace PlotValueColorIndex
{
enum PlotValueColorIndex : uint8_t
{
  Unsorted,
  Sorted,

  SwappedLess,
  SwappedGreater,

  MaxColors,
};
} // namespace PlotValueColorIndex

using PlotColors = Array <decltype(PlotValueColorIndex::Sorted)>;
