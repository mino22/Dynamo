
#include "stdafx.h"
#include "OpenInterfaces.h"

using namespace System;
using namespace Dynamo::Bloodstone;
using namespace Dynamo::Bloodstone::OpenGL;

INITGLPROC(PFNWGLCREATECONTEXTATTRIBSARBPROC,   wglCreateContextAttribsARB);
INITGLPROC(PFNWGLCHOOSEPIXELFORMATARBPROC,      wglChoosePixelFormatARB);
INITGLPROC(PFNGLGETSTRINGPROC,                  glGetString);
INITGLPROC(PFNGLGETINTEGERVPROC,                glGetIntegerv);
INITGLPROC(PFNGLENABLEPROC,                     glEnable);
INITGLPROC(PFNGLDISABLEPROC,                    glDisable);
INITGLPROC(PFNGLPOLYGONMODEPROC,                glPolygonMode);
INITGLPROC(PFNGLCREATESHADERPROC,               glCreateShader);
INITGLPROC(PFNGLSHADERSOURCEPROC,               glShaderSource);
INITGLPROC(PFNGLCOMPILESHADERPROC,              glCompileShader);
INITGLPROC(PFNGLGETSHADERIVPROC,                glGetShaderiv);
INITGLPROC(PFNGLGETSHADERINFOLOGPROC,           glGetShaderInfoLog);
INITGLPROC(PFNGLCREATEPROGRAMPROC,              glCreateProgram);
INITGLPROC(PFNGLDELETEPROGRAMPROC,              glDeleteProgram);
INITGLPROC(PFNGLATTACHSHADERPROC,               glAttachShader);
INITGLPROC(PFNGLDETACHSHADERPROC,               glDetachShader);
INITGLPROC(PFNGLBINDATTRIBLOCATIONPROC,         glBindAttribLocation);
INITGLPROC(PFNGLLINKPROGRAMPROC,                glLinkProgram);
INITGLPROC(PFNGLUSEPROGRAMPROC,                 glUseProgram);
INITGLPROC(PFNGLGETPROGRAMIVPROC,               glGetProgramiv);
INITGLPROC(PFNGLGETPROGRAMINFOLOGPROC,          glGetProgramInfoLog);
INITGLPROC(PFNGLDELETESHADERPROC,               glDeleteShader);
INITGLPROC(PFNGLGENBUFFERSPROC,                 glGenBuffers);
INITGLPROC(PFNGLDELETEBUFFERSPROC,              glDeleteBuffers);
INITGLPROC(PFNGLBUFFERDATAPROC,                 glBufferData);
INITGLPROC(PFNGLGENVERTEXARRAYSPROC,            glGenVertexArrays);
INITGLPROC(PFNGLDELETEVERTEXARRAYSPROC,         glDeleteVertexArrays);
INITGLPROC(PFNGLBINDVERTEXARRAYPROC,            glBindVertexArray);
INITGLPROC(PFNGLGETATTRIBLOCATIONPROC,          glGetAttribLocation);
INITGLPROC(PFNGLGETUNIFORMLOCATIONPROC,         glGetUniformLocation);
INITGLPROC(PFNGLUNIFORM1FPROC,                  glUniform1f);
INITGLPROC(PFNGLUNIFORM2FPROC,                  glUniform2f);
INITGLPROC(PFNGLUNIFORM3FPROC,                  glUniform3f);
INITGLPROC(PFNGLUNIFORM4FPROC,                  glUniform4f);
INITGLPROC(PFNGLUNIFORMMATRIX4FVPROC,           glUniformMatrix4fv);
INITGLPROC(PFNGLENABLEVERTEXATTRIBARRAYPROC,    glEnableVertexAttribArray);
INITGLPROC(PFNGLDISABLEVERTEXATTRIBARRAYPROC,   glDisableVertexAttribArray);
INITGLPROC(PFNGLBINDBUFFERPROC,                 glBindBuffer);
INITGLPROC(PFNGLVERTEXATTRIBPOINTERPROC,        glVertexAttribPointer);
INITGLPROC(PFNGLVIEWPORTPROC,                   glViewport);
INITGLPROC(PFNGLDRAWARRAYSPROC,                 glDrawArrays);
INITGLPROC(PFNGLCLEARPROC,                      glClear);
INITGLPROC(PFNGLCLEARCOLORPROC,                 glClearColor);
INITGLPROC(PFNGLBLENDEQUATIONSEPARATEPROC,      glBlendEquationSeparate);
INITGLPROC(PFNGLBLENDFUNCSEPARATEPROC,          glBlendFuncSeparate);
INITGLPROC(PFNGLPOINTSIZEPROC,                  glPointSize);

GraphicsContext::GraphicsContext() : 
    mMajorVersion(0),
    mMinorVersion(0),
    mRenderWindow(nullptr),
    mhRenderContext(nullptr),
    mpDefaultCamera(nullptr)
{
}

bool GraphicsContext::InitializeCore(HWND hWndOwner)
{
    if (mhRenderContext != nullptr) {
        auto message = L"'GraphicsContext::InitializeCore' called twice";
        throw gcnew InvalidOperationException(gcnew String(message));
    }

    if (InitializeWithDummyContext(hWndOwner) == false)
        return false; // Context creation failed.

    const int deviceAttributes[] =
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,     32,
        WGL_DEPTH_BITS_ARB,     24,
        WGL_STENCIL_BITS_ARB,   8,
        WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
        WGL_SAMPLES_ARB,        8,
        0, // End
    };

    HDC hDeviceContext = ::GetDC(hWndOwner);

    int pixelFormat = 0;
    unsigned int formatCount = 0;
    GL::wglChoosePixelFormatARB(hDeviceContext, deviceAttributes,
        nullptr, 1, &pixelFormat, &formatCount);

    // Get the best available match of pixel format for the device context
    PIXELFORMATDESCRIPTOR descriptor = { 0 };
    ::DescribePixelFormat(hDeviceContext, pixelFormat,
        sizeof(PIXELFORMATDESCRIPTOR), &descriptor);

    if (::SetPixelFormat(hDeviceContext, pixelFormat, &descriptor) == false) {
        ::ReleaseDC(hWndOwner, hDeviceContext);
        return false;
    }

    int attributes[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, mMajorVersion,
        WGL_CONTEXT_MINOR_VERSION_ARB, mMinorVersion, 
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    mhRenderContext = GL::wglCreateContextAttribsARB(hDeviceContext, 0, attributes);
    wglMakeCurrent(hDeviceContext, mhRenderContext);

    ::ReleaseDC(hWndOwner, hDeviceContext); // Done with device context.
    mRenderWindow = hWndOwner;

    // Create the default camera.
    mpDefaultCamera = new Camera(this);

    // Default states of our renderer.
    GL::glEnable(GL_DEPTH_TEST);
    GL::glPointSize(4.0f);
    return true;
}

void GraphicsContext::UninitializeCore(void)
{
    if (mhRenderContext == nullptr)
        return;

    HDC hDeviceContext = ::GetDC(mRenderWindow);
    ::wglMakeCurrent(hDeviceContext, nullptr);
    ::ReleaseDC(mRenderWindow, hDeviceContext); // Done with device context.

    // Now that the default context is reset, destroy the render context.
    ::wglDeleteContext(mhRenderContext);
    mhRenderContext = nullptr;

    if (mpDefaultCamera != nullptr) {
        delete mpDefaultCamera;
        mpDefaultCamera = nullptr;
    }
}

ICamera* GraphicsContext::GetDefaultCameraCore(void) const
{
    return mpDefaultCamera;
}

IVertexShader* GraphicsContext::CreateVertexShaderCore(const std::string& content) const
{
    VertexShader* pVertexShader = new VertexShader(this);
    pVertexShader->LoadFromContent(content);
    return pVertexShader;
}

IFragmentShader* GraphicsContext::CreateFragmentShaderCore(const std::string& content) const
{
    FragmentShader* pFragmentShader = new FragmentShader(this);
    pFragmentShader->LoadFromContent(content);
    return pFragmentShader;
}

IShaderProgram* GraphicsContext::CreateShaderProgramCore(
    IVertexShader* pVertexShader, IFragmentShader* pFragmentShader)
{
    auto pvs = dynamic_cast<VertexShader *>(pVertexShader);
    auto pfs = dynamic_cast<FragmentShader *>(pFragmentShader);
    ShaderProgram* pShaderProgram = new ShaderProgram(pvs, pfs);
    return pShaderProgram;
}

IVertexBuffer* GraphicsContext::CreateVertexBufferCore(void) const
{
    return new VertexBuffer();
}

void GraphicsContext::BeginRenderFrameCore(HDC deviceContext) const
{
    RECT rcClient;
    ::GetClientRect(this->mRenderWindow, &rcClient);

    GL::glViewport(0, 0, rcClient.right, rcClient.bottom);
    GL::glClearColor(0.941176f, 0.941176f, 0.941176f, 1.0f); // #F0F0F0
    GL::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // If the camera is animating, this is the right time to update it.
    if (mpDefaultCamera->IsInTransition())
        mpDefaultCamera->UpdateFrame();
}

void GraphicsContext::ActivateShaderProgramCore(IShaderProgram* pShaderProgram) const
{
    ShaderProgram* pProgram = dynamic_cast<ShaderProgram *>(pShaderProgram);
    if (pProgram == nullptr)
        return;

    pProgram->Activate();
}

void GraphicsContext::RenderVertexBufferCore(IVertexBuffer* pVertexBuffer) const
{
    auto pBuffer = dynamic_cast<VertexBuffer *>(pVertexBuffer);
    if (pBuffer != nullptr)
        pBuffer->Render();
}

bool GraphicsContext::EndRenderFrameCore(HDC deviceContext) const
{
    ::SwapBuffers(deviceContext);
    return mpDefaultCamera->IsInTransition(); // Request frame update if needed.
}

void GraphicsContext::EnableAlphaBlendCore(void) const
{
    GL::glEnable(GL_BLEND);
    GL::glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    GL::glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
}

void GraphicsContext::ClearDepthBufferCore(void) const
{
    GL::glClear(GL_DEPTH_BUFFER_BIT);
}

bool GraphicsContext::InitializeWithDummyContext(HWND hWndOwner)
{
    wchar_t wndClassName[128] = { 0 };
    ::GetClassName(hWndOwner, wndClassName, _countof(wndClassName));
    HWND hWndParent = ::GetParent(hWndOwner);

    HWND hWndTemporary = CreateWindowEx(0, wndClassName, nullptr,
        WS_CHILD, 0, 0, 100, 100, hWndParent, nullptr, nullptr, 0);

    if (nullptr == hWndTemporary)
        return false;

    PIXELFORMATDESCRIPTOR descriptor = { 0 };
    descriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    descriptor.nVersion = 1;
    descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    descriptor.iPixelType = PFD_TYPE_RGBA;
    descriptor.cColorBits = 32;
    descriptor.cDepthBits = 24;
    descriptor.cStencilBits = 8;
    descriptor.iLayerType = PFD_MAIN_PLANE;

    bool contextCreated = false;

    // Get the best available match of pixel format for the device context
    HDC hDeviceContext = ::GetDC(hWndTemporary);
    int format = ::ChoosePixelFormat(hDeviceContext, &descriptor);
    if (::SetPixelFormat(hDeviceContext, format, &descriptor))
    {
        HGLRC tempContext = ::wglCreateContext(hDeviceContext);
        if (tempContext != nullptr)
        {
            ::wglMakeCurrent(hDeviceContext, tempContext);

            if (GL::Initialize()) // Initialize OpenGL extension.
            {
                // Determine the current OpenGL version numbers.
                GL::glGetIntegerv(GL_MAJOR_VERSION, &mMajorVersion);
                GL::glGetIntegerv(GL_MINOR_VERSION, &mMinorVersion);
                contextCreated = true;
            }

            wglMakeCurrent(hDeviceContext, nullptr);
            wglDeleteContext(tempContext); // Discard temporary context.
        }
    }

    ::ReleaseDC(hWndTemporary, hDeviceContext); // Done with device context.
    ::DestroyWindow(hWndTemporary);
    return contextCreated;
}