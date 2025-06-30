# SortingAlgorithmVisualizer

[![Windows (MSVC)](https://github.com/Casqade/SortingAlgorithmVisualizer/actions/workflows/windows-build-msvc.yml/badge.svg)](https://github.com/Casqade/SortingAlgorithmVisualizer/actions/workflows/windows-build-msvc.yml)
[![Windows (Clang)](https://github.com/Casqade/SortingAlgorithmVisualizer/actions/workflows/windows-build-clang.yml/badge.svg)](https://github.com/Casqade/SortingAlgorithmVisualizer/actions/workflows/windows-build-clang.yml)

---

This is a take-home interview assignment I did during a job application. 


## Task description:

Write a C++ Win64 application for visualizing various sorting algorithms step-by-step. 
For visualization use any suitable graphics API (OpenGL, Vulkan, D3D12, terminal graphics, etc.). 
[Example video for reference](https://youtu.be/ZZuD6iUe3Pc)


## Requirements:
- Implement at least 2 sorting algorithms
- Use different colors for array element status: Unsorted, Sorting, Sorted
- Introduce perceptible delays between consecutive sorting steps
- Every sorting algorithm instance must reside on a dedicated thread
- Plot data randomisation must be executed on a separate thread
- All threads must be created & initialized during program init stage
- Avoid using STL and modern C++ standards as much as possible (i.e. stick to native WinAPI)
- Prefer lock-free algorithms for thread synchronization
- Avoid big allocations on the stack
- Only one dynamic memory allocation is allowed during runtime
- Write your own data structures, containers, etc.
- All sorting algorithm implementations must derive from base interface
- Minimise CPU -> GPU memory transfers
- No window management libraries are allowed
- If necessary, some third-party libraries can be used (I used `glad` loader)


## Result

It was my first time writing a pure native WinAPI application 
with graphics output without using GLFW and other "shortcuts", 
along with strict "write your own X" and STL avoidance requirements. 
Nevertheless, the experience I gained was worth the effort.

As I wasn't given any strict deadlines, I estimated the task 
doable in about 7-10 days, with 14 days being a personal deadline. 
In the end, I think I spent rougly 9-10 days coding, 
with another 2-3 days being ill somewhere along the way. 

---

https://github.com/user-attachments/assets/0a0a27d4-82e6-402f-bf17-8e6c1ff0903e
