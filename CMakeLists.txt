cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

cmake_policy(SET CMP0091 NEW)

set(TARGET SortingAlgorithmVisualizer)
project(${TARGET} VERSION 1.0 LANGUAGES C CXX)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

add_executable(${TARGET} WIN32)
target_sources(${TARGET} PRIVATE
  src/main.cpp

  src/Allocators/Alignment.cpp
  src/Allocators/IAllocator.cpp
  src/Allocators/ArenaAllocator.cpp

  src/Containers/CallbackStack.cpp
  src/Containers/RingBuffer.cpp

  src/Backend.cpp
  src/Backend_threads.cpp
  src/Frontend.cpp

  src/ISorter.cpp
  src/TimePoint.cpp
  src/MessageFormatting.cpp

  src/glad/glad.c
  src/glad/wgl.c
)


if(${BUILD_SHARED_LIBS})
  set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
else()
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif()


target_compile_definitions(${TARGET} PUBLIC
  WIN32_LEAN_AND_MEAN
)


if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_link_options(${TARGET} PUBLIC
    /NOIMPLIB
    /NOEXP
  )
else()
  target_compile_options(${TARGET} PUBLIC -m64)
  target_link_options(${TARGET} PUBLIC -m64)

endif()


if(${BUILD_SHARED_LIBS})
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_target_properties(${TARGET} PROPERTIES
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
  endif()

else()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_target_properties(${TARGET} PROPERTIES
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
  else()
    target_link_libraries(${TARGET} PUBLIC
      -static
      -static-libgcc
      -static-libstdc++
    )
  endif()

endif()


find_package(OpenGL REQUIRED)

target_link_libraries(${TARGET} PUBLIC
  OpenGL::GL
)


include(GNUInstallDirs)
target_include_directories(${TARGET} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${CMAKE_INSTALL_INCLUDEDIR}>
)
