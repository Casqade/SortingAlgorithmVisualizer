#include <SortingAlgorithmVisualizer/Frontend.hpp>
#include <SortingAlgorithmVisualizer/Backend.hpp>
#include <SortingAlgorithmVisualizer/MessageFormatting.hpp>

#include <glad/gl.h>
#include <glad/wgl.h>

#include <cstdio>


static LRESULT CALLBACK
PlotWindowProc( HWND, UINT messageCode, WPARAM, LPARAM );


static const char* VertexShaderSource =
R"(
#version 460

layout(location = 0) uniform float maxValue;

layout(location = 0) in float value;
layout(location = 1) in uint colorIndex;

layout(location = 0) flat out vec3 outColor;


const vec3 ColorPalette[5] =
{

//  unsorted
  {1, 1, 1},

//  sorted
  {0, 1, 0},

//  swapped less
  {0, 0, 1},

//  swapped greater
  {1, 0, 0},

//  bubble sort caret
  {1, 0, 0},
};


void main()
{
  float columnWidthToSpacingRatio = 0.75;
  float columnWidth = 2 * columnWidthToSpacingRatio / maxValue;
  float columnSpacing = 2 * (1 - columnWidthToSpacingRatio) / maxValue;
  float columnHeight = 2 * value / maxValue;

  vec2 vertices[4] = {
    { 0, -1 + columnHeight },
    { 0, -1 },
    { columnWidth, -1 },
    { columnWidth, -1 + columnHeight },
  };


  float columnX =
    -1 + 0.5 * columnSpacing +
    2 * gl_InstanceID / maxValue;

  vec2 vertex =
    vertices[gl_VertexID] +
    vec2(columnX, 0);

  gl_Position = vec4(vertex, 0, 1);

  outColor = ColorPalette[colorIndex];
}
)";

static const char* FragmentShaderSource = R"(
#version 460

layout(location = 0) in vec3 inColor;

layout(location = 0) out vec4 outColor;


void main()
{
  outColor = vec4(inColor, 1.0);
}
)";


static char ShaderLog[256];

static
GLuint
CreateShader(
  GladGLContext& gl,
  GLenum type,
  const char* source )
{
  auto shaderTypeString =
    type == GL_VERTEX_SHADER ?
      "vertex" : "fragment";


  auto vertexShader = gl.CreateShader(type);

  if ( vertexShader == 0 )
  {
    MessageBox( NULL,
      FormatUserMessagePassthrough(
        "Failed to create %1 shader",
        shaderTypeString ),
      NULL, MB_ICONERROR );

    return 0;
  }

  gl.ShaderSource( vertexShader,
    1, &source, nullptr );

  gl.CompileShader(vertexShader);

  GLint status {};
  gl.GetShaderiv( vertexShader,
    GL_COMPILE_STATUS, &status );

  if ( status != GL_TRUE )
  {
    gl.GetShaderInfoLog(
      vertexShader,
      sizeof(ShaderLog),
      nullptr, ShaderLog );

    gl.DeleteShader(vertexShader);

    MessageBox( NULL,
      FormatUserMessagePassthrough(
        "Failed to compile %1 shader: %2",
        shaderTypeString, ShaderLog ),
      NULL, MB_ICONERROR );

    return 0;
  }

  return vertexShader;
}


Frontend::Frontend(
  IAllocator& allocator,
  Backend& backend )
  : mAllocator{allocator}
  , mBackend{backend}
{}

Frontend::~Frontend()
{
  deinit();
}

size_t
Frontend::CallbackStackDepth(
  size_t plotCount )
{
  return 2;
}

size_t
Frontend::PlotCallbackStackDepth()
{
  return 7;
}

size_t
Frontend::HeapMemoryBudget(
  size_t plotCount )
{
  return
    sizeof(CallbackTask) * CallbackStackDepth(plotCount) +
    sizeof(CallbackTask) * PlotCallbackStackDepth() * plotCount +
    sizeof(WindowData) * plotCount;
}

void
Frontend::init(
  size_t plotCount,
  const char** plotTitles,
  HINSTANCE appInstance )
{
  auto callbackStackDepth = CallbackStackDepth(plotCount);

  if ( mDeinitStack.init(callbackStackDepth, mAllocator) == false )
  {
    MessageBox( NULL,
      "Failed to initialize Frontend deinit stack: out of memory budget",
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return;
  }


  if ( mWindows.init(plotCount, mAllocator) == false )
  {
    MessageBox( NULL,
      "Failed to initialize Frontend window data: out of memory budget",
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return;
  }

  mDeinitStack.push( &mWindows,
  [] ( void* data )
  {
    static_cast <decltype(mWindows)*> (data)->deinit();
  });


  static auto windowClassName =
    "SortingAlgorithmVisualizerWindowClass";

  static WNDCLASS wc {};
  wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = PlotWindowProc;
  wc.hInstance = appInstance;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = windowClassName;

  if ( RegisterClass(&wc) == 0 )
  {
    MessageBox( NULL,
      FormatUserMessagePassthrough(
        "Failed to register window class: %1",
        FormatSystemMessage() ),
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return;
  }

  mAppInstance = appInstance;

  mDeinitStack.push( this,
  [] ( void* data )
  {
    UnregisterClass(
      windowClassName,
      static_cast <Frontend*> (data)->mAppInstance );
  });


  auto tmpWindow = CreateWindow(
    windowClassName,
    "SortingAlgorithmVisualizer_tmp",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 1,
    CW_USEDEFAULT, 1,
    NULL, NULL,
    appInstance, this );

  if ( tmpWindow == NULL )
  {
    MessageBox( NULL,
      FormatUserMessagePassthrough(
        "Failed to create temporary window: %1",
        FormatSystemMessage() ),
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return;
  }

  ShowWindow(tmpWindow, SW_HIDE);


  auto tmpDeviceCtx = GetDC(tmpWindow);

  if ( tmpDeviceCtx == NULL )
  {
    MessageBox( NULL,
      FormatUserMessagePassthrough(
        "Failed to get device context for temporary window: %1",
        FormatSystemMessage() ),
      NULL, MB_ICONERROR );

    DestroyWindow(tmpWindow);
    ProgramShouldAbort = true;
    return;
  }


  static PIXELFORMATDESCRIPTOR pfd {};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 0;
  pfd.cStencilBits = 0;
  pfd.cAuxBuffers = 0;
  pfd.iLayerType = PFD_MAIN_PLANE;


  auto pfIndex = ChoosePixelFormat(
    tmpDeviceCtx, &pfd );

  if ( pfIndex == 0 )
  {
    MessageBox( NULL,
      FormatUserMessagePassthrough(
        "Failed to choose suitable pixel format for temporary window: %1",
        FormatSystemMessage() ),
      NULL, MB_ICONERROR );

    DestroyWindow(tmpWindow);
    ProgramShouldAbort = true;
    return;
  }

  if ( SetPixelFormat(tmpDeviceCtx, pfIndex, &pfd) == FALSE )
  {
    MessageBox( NULL,
      FormatUserMessagePassthrough(
        "Failed to set pixel format for temporary window: %1",
        FormatSystemMessage() ),
      NULL, MB_ICONERROR );

    DestroyWindow(tmpWindow);
    ProgramShouldAbort = true;
    return;
  }


  auto tmpRenderCtx = wglCreateContext(tmpDeviceCtx);

  if ( tmpRenderCtx == NULL )
  {
    MessageBox( NULL,
      FormatUserMessagePassthrough(
        "Failed to create temporary OpenGL context: %1",
        FormatSystemMessage() ),
      NULL, MB_ICONERROR );

    DestroyWindow(tmpWindow);
    ProgramShouldAbort = true;
    return;
  }


  auto prevDeviceCtx = wglGetCurrentDC();
  auto prevRenderCtx = wglGetCurrentContext();

  if ( wglMakeCurrent(tmpDeviceCtx, tmpRenderCtx) == FALSE )
  {
    MessageBox( NULL,
      FormatUserMessagePassthrough(
        "Failed to make temporary OpenGL context current: %1",
        FormatSystemMessage() ),
      NULL, MB_ICONERROR );

    wglMakeCurrent(prevDeviceCtx, prevRenderCtx);
    wglDeleteContext(tmpRenderCtx);
    DestroyWindow(tmpWindow);

    ProgramShouldAbort = true;
    return;
  }


  auto wglVersion = gladLoaderLoadWGL(tmpDeviceCtx);

  wglMakeCurrent(prevDeviceCtx, prevRenderCtx);
  wglDeleteContext(tmpRenderCtx);
  DestroyWindow(tmpWindow);

  if ( wglVersion == 0 )
  {
    MessageBox( NULL,
      "Failed to load WGL Extensions",
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return;
  }


  for ( size_t i {}; i < mWindows.size(); ++i )
  {
    auto& windowData = mWindows[i];
    auto& deinitStack = windowData.deinitStack;

    auto& gl = windowData.glContext;
    auto& deviceCtx = windowData.deviceContext;
    auto& renderCtx = windowData.renderContext;
    auto& window = windowData.window;
    auto& program = windowData.shaderProgram;


    auto callbackStackDepth = PlotCallbackStackDepth();

    if ( deinitStack.init(callbackStackDepth, mAllocator) == false )
    {
      MessageBox( NULL,
        FormatUserMessagePassthrough(
          "Failed to initialize deinit stack for window \"%1\": "
          "Out of memory budget",
          plotTitles[i] ),
        NULL, MB_ICONERROR );

      ProgramShouldAbort = true;
      return;
    }


    window = CreateWindow(
      windowClassName,
      plotTitles[i],
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT,
      CW_USEDEFAULT, CW_USEDEFAULT,
      NULL, NULL,
      appInstance, this );

    if ( window == NULL )
    {
      MessageBox( NULL,
        FormatUserMessagePassthrough(
          "Failed to create window \"%1\": %2",
          plotTitles[i],
          FormatSystemMessage() ),
        NULL, MB_ICONERROR );

      ProgramShouldAbort = true;
      return;
    }

    deinitStack.push( &window,
    [] ( void* data )
    {
      auto window = *static_cast <HWND*> (data);
      if ( window != NULL )
        DestroyWindow(window);
    });

    windowData.title = plotTitles[i];


    deviceCtx = GetDC(window);

    if ( deviceCtx == NULL )
    {
      MessageBox( NULL,
        FormatUserMessagePassthrough(
          "Failed to get device context for window \"%1\": %2",
          plotTitles[i],
          FormatSystemMessage() ),
        NULL, MB_ICONERROR );

      ProgramShouldAbort = true;
      return;
    }


    auto pfIndex = ChoosePixelFormat(
      deviceCtx, &pfd );

    if ( pfIndex == 0 )
    {
      MessageBox( NULL,
        FormatUserMessagePassthrough(
          "Failed to choose suitable pixel format for window \"%1\": %2",
          plotTitles[i],
          FormatSystemMessage() ),
        NULL, MB_ICONERROR );

      ProgramShouldAbort = true;
      return;
    }

    if ( DescribePixelFormat(deviceCtx, pfIndex, sizeof(pfd), &pfd) == 0 )
    {
      MessageBox( NULL,
        FormatUserMessagePassthrough(
          "Failed to describe pixel format for window \"%1\": %2",
          plotTitles[i],
          FormatSystemMessage() ),
        NULL, MB_ICONERROR );

      ProgramShouldAbort = true;
      return;
    }

    if ( SetPixelFormat(deviceCtx, pfIndex, &pfd) == FALSE )
    {
      MessageBox( NULL,
        FormatUserMessagePassthrough(
          "Failed to set pixel format for window \"%1\": %2",
          plotTitles[i],
          FormatSystemMessage() ),
        NULL, MB_ICONERROR );

      ProgramShouldAbort = true;
      return;
    }


    static int contextAttributes[]
    {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
      WGL_CONTEXT_MINOR_VERSION_ARB, 6,

      WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,

      WGL_CONTEXT_FLAGS_ARB,
        WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
        ,

      0,
    };


    renderCtx = wglCreateContextAttribsARB(
      deviceCtx, NULL, contextAttributes );

    if ( renderCtx == NULL )
    {
      auto errorCode = GetLastError();
      auto errorMessage = FormatSystemMessage(errorCode);

//      Windows may fail to parse OpenGL errors properly:
//      http://www.gamedev.net/forums/topic/658138-creating-an-opengl-context-on-windows-with-glew/5163081/

      if ( errorMessage == nullptr )
      {
        switch (errorCode & 0xFFFF)
        {
          case ERROR_INVALID_PROFILE_ARB:
          {
            errorMessage = "ERROR_INVALID_PROFILE_ARB";
            break;
          }

          case ERROR_INVALID_VERSION_ARB:
          {
            errorMessage = "ERROR_INVALID_VERSION_ARB";
            break;
          }

          default:
          {
            MessageBox( NULL,
              FormatUserMessagePassthrough(
                "Failed to create OpenGL context for window \"%1\": Unknown error %2!lu!",
                plotTitles[i],
                errorCode ),
              NULL, MB_ICONERROR );

            ProgramShouldAbort = true;
            return;
          }
        }
      }

      MessageBox( NULL,
        FormatUserMessagePassthrough(
          "Failed to create OpenGL context for window \"%1\": %2",
          plotTitles[i],
          errorMessage ),
        NULL, MB_ICONERROR );

      ProgramShouldAbort = true;
      return;
    }

    if ( wglMakeCurrent(deviceCtx, renderCtx) == FALSE )
    {
      MessageBox( NULL,
        FormatUserMessagePassthrough(
          "Failed to make OpenGL context current for window \"%1\": %2",
          plotTitles[i],
          FormatSystemMessage() ),
        NULL, MB_ICONERROR );

      ProgramShouldAbort = true;
      return;
    }

    deinitStack.push( &windowData,
    [] ( void* data )
    {
      auto windowData = static_cast <WindowData*> (data);

      wglMakeCurrent(windowData->deviceContext, NULL);
      wglDeleteContext(windowData->renderContext);
    });


    auto glVersion = gladLoaderLoadGLContext(&gl);

    if ( glVersion == 0 )
    {
      MessageBox( NULL,
        FormatUserMessagePassthrough(
          "Failed to initialize OpenGL context for window \"%1\"",
          plotTitles[i] ),
        NULL, MB_ICONERROR );

      ProgramShouldAbort = true;
      return;
    }

    deinitStack.push( &windowData,
    [] ( void* data )
    {
      auto windowData = static_cast <WindowData*> (data);

      wglMakeCurrent(
        windowData->deviceContext,
        windowData->renderContext );

      gladLoaderUnloadGLContext(&windowData->glContext);
    });


    auto vertexShader = CreateShader(
      gl, GL_VERTEX_SHADER,
      VertexShaderSource );

    if ( vertexShader == 0 )
    {
      ProgramShouldAbort = true;
      return;
    }

    auto fragmentShader = CreateShader(
      gl, GL_FRAGMENT_SHADER,
      FragmentShaderSource );

    if ( fragmentShader == 0 )
    {
      gl.DeleteShader(vertexShader);

      ProgramShouldAbort = true;
      return;
    }


    program = gl.CreateProgram();

    if ( program == 0 )
    {
      gl.DeleteShader(vertexShader);
      gl.DeleteShader(fragmentShader);

      ProgramShouldAbort = true;
      return;
    }

    gl.AttachShader(program, vertexShader);
    gl.AttachShader(program, fragmentShader);

    gl.LinkProgram(program);

    GLint status;
    gl.GetProgramiv( program,
      GL_LINK_STATUS, &status );


    if ( status == 0 )
    {
      gl.GetShaderInfoLog(
        vertexShader,
        sizeof(ShaderLog),
        nullptr, ShaderLog );

      MessageBox( NULL,
        FormatUserMessagePassthrough(
          "Failed to link shader program: %1",
          ShaderLog ),
        NULL, MB_ICONERROR );

      gl.DetachShader(program, fragmentShader);
      gl.DetachShader(program, vertexShader);
      gl.DeleteShader(vertexShader);
      gl.DeleteShader(fragmentShader);
      gl.DeleteProgram(program);

      ProgramShouldAbort = true;
      return;
    }

    gl.DeleteShader(vertexShader);
    gl.DeleteShader(fragmentShader);

    deinitStack.push( &windowData,
    [] ( void* data )
    {
      auto windowData = static_cast <WindowData*> (data);

      auto& gl = windowData->glContext;

      wglMakeCurrent(
        windowData->deviceContext,
        windowData->renderContext );

      gl.UseProgram(0);
      gl.DeleteProgram(windowData->shaderProgram);
    });


    gl.Disable(GL_BLEND);
    gl.Disable(GL_DEPTH_TEST);
    gl.Disable(GL_STENCIL_TEST);
    gl.Disable(GL_MULTISAMPLE);

    gl.StencilMask(0);
    gl.DepthMask(GL_FALSE);

    gl.ClearColor(0, 0, 0, 1);

    gl.UseProgram(program);

    wglSwapIntervalEXT(1);

    wglMakeCurrent(deviceCtx, NULL);

    ShowWindow(window, SW_SHOWDEFAULT);
  }
}

void
Frontend::deinit()
{
  closeAllWindows();

  while ( mDeinitStack.popAndCall() == true )
    ;
}

void
Frontend::addSorter(
  size_t plotIndex,
  ISorter* sorter )
{
  using PlotValueColorIndex::PlotValueColorIndex;

  if ( ProgramShouldAbort == true )
    return;


  auto& windowData = mWindows[plotIndex];
  auto& deinitStack = windowData.deinitStack;

  auto& gl = windowData.glContext;
  auto& shaderProgram = windowData.shaderProgram;
  auto& vertexArray = windowData.vertexArray;
  auto& valuesBufferId = windowData.valuesBufferId;
  auto& colorsBufferId = windowData.colorsBufferId;

  assert(vertexArray == 0);

  wglMakeCurrent(
    windowData.deviceContext,
    windowData.renderContext );


  auto bufferFlags =
    GL_MAP_WRITE_BIT |
    GL_MAP_PERSISTENT_BIT;

  auto valueSize = sorter->valueSize();
  auto valueCount = sorter->valueCount();

  auto colorSize = sizeof(PlotValueColorIndex);

  size_t bufferCount = 3;

  auto valuesBufferSize =
    bufferCount * valueSize * valueCount;

  auto colorsBufferSize =
    bufferCount * colorSize * valueCount;


  gl.CreateBuffers(
    1, &valuesBufferId );

  gl.NamedBufferStorage(
    valuesBufferId,
    valuesBufferSize,
    nullptr, bufferFlags );

  auto valuesBuffer = gl.MapNamedBufferRange(
    valuesBufferId,
    0, valuesBufferSize,
    bufferFlags | GL_MAP_FLUSH_EXPLICIT_BIT );

  if ( valuesBuffer == nullptr )
  {
    MessageBox( NULL,
      FormatUserMessagePassthrough(
        "Failed to map values GPU buffer for sorter \"%1\"",
        windowData.title ),
      NULL, MB_ICONERROR );

    gl.DeleteBuffers(1, &valuesBufferId);
    ProgramShouldAbort = true;
    return;
  }

  deinitStack.push( &windowData,
  [] ( void* data )
  {
    auto windowData = static_cast <WindowData*> (data);
    auto& gl = windowData->glContext;

    wglMakeCurrent(
      windowData->deviceContext,
      windowData->renderContext );

    gl.UnmapNamedBuffer(windowData->valuesBufferId);
    gl.DeleteBuffers(1, &windowData->valuesBufferId);
  });


  gl.CreateBuffers(
    1, &colorsBufferId );

  gl.NamedBufferStorage(
    colorsBufferId,
    colorsBufferSize,
    nullptr, bufferFlags );

  auto colorsBuffer = gl.MapNamedBufferRange(
    colorsBufferId,
    0, colorsBufferSize,
    bufferFlags | GL_MAP_FLUSH_EXPLICIT_BIT );

  if ( colorsBuffer == nullptr )
  {
    MessageBox( NULL,
      FormatUserMessagePassthrough(
        "Failed to map colors GPU buffer for sorter \"%1\"",
        windowData.title ),
      NULL, MB_ICONERROR );

    gl.DeleteBuffers(1, &colorsBufferId);
    ProgramShouldAbort = true;
    return;
  }

  deinitStack.push( &windowData,
  [] ( void* data )
  {
    auto windowData = static_cast <WindowData*> (data);
    auto& gl = windowData->glContext;

    wglMakeCurrent(
      windowData->deviceContext,
      windowData->renderContext );

    gl.UnmapNamedBuffer(windowData->colorsBufferId);
    gl.DeleteBuffers(1, &windowData->colorsBufferId);
  });


  gl.CreateVertexArrays(
    1, &vertexArray );


  gl.EnableVertexArrayAttrib(
    vertexArray, 0 );

  gl.VertexArrayAttribFormat(
    vertexArray,
    0, 1, sorter->valueType(),
    GL_FALSE, 0 );

  gl.VertexArrayAttribBinding(
    vertexArray, 0, 0 );

  gl.VertexArrayVertexBuffer(
    vertexArray,
    0, valuesBufferId,
    0, valueSize );

  gl.VertexArrayBindingDivisor(
    vertexArray, 0, 1 );


  gl.EnableVertexArrayAttrib(
    vertexArray, 1 );

  gl.VertexArrayAttribIFormat(
    vertexArray,
    1, 1, GL_BYTE, 0 );

  gl.VertexArrayAttribBinding(
    vertexArray, 1, 1 );

  gl.VertexArrayVertexBuffer(
    vertexArray,
    1, colorsBufferId,
    0, colorSize );

  gl.VertexArrayBindingDivisor(
    vertexArray, 1, 1 );


  gl.BindVertexArray(vertexArray);

  gl.ProgramUniform1f(
    shaderProgram, 0, valueCount );


  deinitStack.push( &windowData,
  [] ( void* data )
  {
    auto windowData = static_cast <WindowData*> (data);
    auto& gl = windowData->glContext;

    wglMakeCurrent(
      windowData->deviceContext,
      windowData->renderContext );

    for ( auto&& fence : windowData->bufferFences )
    {
      if ( fence == nullptr )
        continue;

      GLenum waitResult {};

      do
      {
        waitResult = gl.ClientWaitSync(fence,
          GL_SYNC_FLUSH_COMMANDS_BIT,
          1000000000 ); // 1 second

        if ( waitResult == GL_WAIT_FAILED )
        {
          MessageBox( NULL,
            FormatUserMessagePassthrough(
              "Failed to wait on fence for sorter \"%1\"",
              windowData->title ),
            NULL, MB_ICONERROR );

          break;
        }

      } while ( waitResult != GL_ALREADY_SIGNALED &&
                waitResult != GL_CONDITION_SATISFIED );

      gl.DeleteSync(fence);
    }

    gl.VertexArrayVertexBuffer(
      windowData->vertexArray,
      0, 0, 0, 0 );

    gl.VertexArrayVertexBuffer(
      windowData->vertexArray,
      1, 0, 0, 0 );

    gl.BindVertexArray(0);
    gl.DeleteVertexArrays(
      1, &windowData->vertexArray );
  });


  for ( size_t bufferIndex {}; bufferIndex < bufferCount; ++bufferIndex )
  {
    auto bufferOffset =
        bufferIndex * valueSize * valueCount;

    for ( size_t i {}; i < valueCount; ++i )
    {
      auto valueOffset = i * valueSize;

      auto valueAddr =
        reinterpret_cast <uintptr_t> (valuesBuffer)
        + bufferOffset + valueOffset;

      sorter->initValue(
        reinterpret_cast <void*> (valueAddr),
        i + 1 );
    }
  }

  std::memset(
    colorsBuffer,
    PlotValueColorIndex::Sorted,
    colorsBufferSize );


  sorter->mapValuesBuffer(valuesBuffer);
  sorter->mapColorsBuffer(colorsBuffer);
}

void
Frontend::draw()
{
  using PlotValueColorIndex::PlotValueColorIndex;

  if (  mWindows.size() == 0 ||
        ProgramShouldAbort == true )
    return;


  for ( size_t i {}; i < mWindows.size(); ++i )
  {
    auto& windowData = mWindows[i];
    auto& gl = windowData.glContext;


    if ( windowData.window == NULL )
      continue;


    RECT windowRect;

    GetClientRect(
      windowData.window, &windowRect );


    wglMakeCurrent(
      windowData.deviceContext,
      windowData.renderContext );

    gl.Viewport( 0, 0,
      windowRect.right, windowRect.bottom );

    gl.ClearColor(0, 0, 0, 1);
    gl.Clear(GL_COLOR_BUFFER_BIT);


    auto sorter = mBackend.sorter(i);

    auto& bufferIndex = windowData.currentBufferIndex;

    auto& fence = windowData.bufferFences[bufferIndex];

    if ( fence != nullptr )
    {
      GLenum waitResult {};
      GLbitfield flags {};
      GLuint64 timeout {};

      do
      {
        waitResult = gl.ClientWaitSync(
          fence, flags, timeout );

        if ( waitResult == GL_ALREADY_SIGNALED ||
             waitResult == GL_CONDITION_SATISFIED )
          break;

        if ( waitResult == GL_WAIT_FAILED )
        {
          MessageBox( NULL,
            FormatUserMessagePassthrough(
              "Failed to wait on fence for sorter \"%1\"",
              windowData.title ),
            NULL, MB_ICONERROR );

          ProgramShouldAbort = true;
          return;
        }

        flags = GL_SYNC_FLUSH_COMMANDS_BIT;
        timeout = 1000000000; // 1 second

      } while ( waitResult == GL_TIMEOUT_EXPIRED );

      gl.DeleteSync(fence);
    }

    bufferIndex = sorter->acquireBuffer(bufferIndex);


    auto valueSize = sorter->valueSize();
    auto valueCount = sorter->valueCount();

    auto valuesBufferSize = valueSize * valueCount;

    auto colorsBufferSize =
      sizeof(PlotValueColorIndex) * valueCount;


    gl.FlushMappedNamedBufferRange(
      windowData.valuesBufferId,
      bufferIndex * valuesBufferSize,
      valuesBufferSize );

    gl.FlushMappedNamedBufferRange(
      windowData.colorsBufferId,
      bufferIndex * colorsBufferSize,
      colorsBufferSize );


    gl.DrawArraysInstancedBaseInstance(
      GL_TRIANGLE_FAN,
      0, 4, valueCount,
      bufferIndex * valueCount );


    fence = gl.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    if ( fence == nullptr )
    {
      MessageBox( NULL,
        FormatUserMessagePassthrough(
          "Failed to create fence for sorter \"%1\"",
          windowData.title ),
        NULL, MB_ICONERROR );

      ProgramShouldAbort = true;
      return;
    }

    SwapBuffers(mWindows[i].deviceContext);
  }
}

void
Frontend::processMessages()
{
  MSG message {};

  while ( PeekMessage( &message, NULL, 0, 0, PM_REMOVE ) != FALSE )
  {
    if ( message.message == WM_QUIT )
    {
      ProgramShouldAbort = true;
      continue;
    }

    TranslateMessage(&message);
    DispatchMessage(&message);
  }
}

void
Frontend::closeAllWindows()
{
  if ( mWindows.size() == 0 )
    return;

  for ( auto&& windowData : mWindows )
  {
    if ( windowData.window == NULL )
      continue;

    while ( windowData.deinitStack.popAndCall() == true )
      ;

    DestroyWindow(windowData.window);
    windowData.window = NULL;
  }
}


LRESULT
CALLBACK
PlotWindowProc(
  HWND window,
  UINT messageCode,
  WPARAM wParam,
  LPARAM lParam )
{
  switch (messageCode)
  {
    case WM_GETMINMAXINFO:
    {
      auto minMaxInfo =
        reinterpret_cast <MINMAXINFO*> (lParam);

      minMaxInfo->ptMinTrackSize.x = 100;
      minMaxInfo->ptMinTrackSize.y = 100;

      return 0;
    }

    case WM_CREATE:
    {
      auto pCreate =
        reinterpret_cast <CREATESTRUCT*> (lParam);

      SetLastError(0);

      auto result = SetWindowLongPtr(
        window, GWLP_USERDATA,
        reinterpret_cast <LONG_PTR> (pCreate->lpCreateParams) );

      if ( result == 0 )
      {
        auto errorCode = GetLastError();

        if ( errorCode == 0 )
          return 0;

        MessageBox( NULL,
          FormatUserMessagePassthrough(
            "Failed to set window Frontend data: %1",
            FormatSystemMessage(errorCode) ),
          NULL, MB_ICONERROR );

        return -1;
      }

      return 0;
    }

    case WM_DESTROY:
    {
      return 0;
    }

    case WM_CLOSE:
    {
      ProgramShouldAbort = true;
      return 0;
    }

    default:
      break;
  }

  return DefWindowProc(
    window, messageCode,
    wParam, lParam );
}
