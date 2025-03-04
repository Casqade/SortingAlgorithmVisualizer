#pragma once

#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/Containers/CallbackStack.hpp>


class Frontend
{
public:
  Frontend( IAllocator&, Backend& );
  ~Frontend();

  Frontend( Frontend&& ) = delete;
  Frontend( const Frontend& ) = delete;

  void init(
    size_t plotCount,
    const char** plotTitles,
    HINSTANCE appInstance );

  void addSorter( size_t plotIndex, ISorter* );

  void deinit();

  void draw();
  void processMessages();
  void closeAllWindows();

  static size_t CallbackStackDepth( size_t plotCount );
  static size_t HeapMemoryBudget( size_t plotCount );


private:
  static size_t PlotCallbackStackDepth();


private:
  IAllocator& mAllocator;
  Backend& mBackend;
  CallbackStack mDeinitStack {};

  Array <WindowData> mWindows {};

  HINSTANCE mAppInstance {};
};
