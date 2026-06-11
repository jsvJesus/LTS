#include "Platform/Window.h"

#include "Core/Logger.h"
#include "Platform/Input.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#include <string>
#include <utility>

#pragma comment(lib, "User32.lib")

namespace Platform
{
    long long PlatformWindowProcedureBridge(
        void* nativeHandle,
        unsigned int message,
        unsigned long long wParam,
        long long lParam
    );

    namespace
    {
        constexpr const wchar_t* WindowClassName = L"GameProject_WindowClass";

        std::wstring Utf8ToWide(const Core::StringView value)
        {
            if (value.empty())
            {
                return {};
            }

            const int requiredSize = ::MultiByteToWideChar(
                CP_UTF8,
                0,
                value.data(),
                static_cast<int>(value.size()),
                nullptr,
                0
            );

            if (requiredSize <= 0)
            {
                std::wstring fallback;
                fallback.reserve(value.size());

                for (const char character : value)
                {
                    fallback.push_back(static_cast<wchar_t>(character));
                }

                return fallback;
            }

            std::wstring result;
            result.resize(static_cast<std::size_t>(requiredSize));

            ::MultiByteToWideChar(
                CP_UTF8,
                0,
                value.data(),
                static_cast<int>(value.size()),
                result.data(),
                requiredSize
            );

            return result;
        }

        Core::String GetLastWin32ErrorMessage(const char* prefix)
        {
            const DWORD errorCode = ::GetLastError();

            Core::String result = prefix;
            result += " ErrorCode=";
            result += std::to_string(errorCode);

            return result;
        }

        bool RegisterPlatformWindowClass(const HINSTANCE instanceHandle)
        {
            WNDCLASSEXW windowClass{};
            windowClass.cbSize = sizeof(WNDCLASSEXW);
            windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            windowClass.lpfnWndProc = [](HWND nativeHandle, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
            {
                return static_cast<LRESULT>(
                    PlatformWindowProcedureBridge(
                        nativeHandle,
                        message,
                        static_cast<unsigned long long>(wParam),
                        static_cast<long long>(lParam)
                    )
                );
            };
            windowClass.cbClsExtra = 0;
            windowClass.cbWndExtra = 0;
            windowClass.hInstance = instanceHandle;
            windowClass.hIcon = ::LoadIconW(nullptr, IDI_APPLICATION);
            windowClass.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
            windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
            windowClass.lpszMenuName = nullptr;
            windowClass.lpszClassName = WindowClassName;
            windowClass.hIconSm = ::LoadIconW(nullptr, IDI_APPLICATION);

            if (::RegisterClassExW(&windowClass) != 0)
            {
                return true;
            }

            const DWORD errorCode = ::GetLastError();

            if (errorCode == ERROR_CLASS_ALREADY_EXISTS)
            {
                return true;
            }

            Core::Logger::Error("Window", GetLastWin32ErrorMessage("Failed to register window class."));
            return false;
        }

        DWORD BuildWindowStyle(const WindowCreateInfo& createInfo)
        {
            DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

            if (createInfo.Resizable)
            {
                style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
            }

            return style;
        }

        RECT BuildWindowRect(const WindowCreateInfo& createInfo, const DWORD style, const DWORD extendedStyle)
        {
            RECT rect{};
            rect.left = 0;
            rect.top = 0;
            rect.right = createInfo.Width;
            rect.bottom = createInfo.Height;

            ::AdjustWindowRectEx(&rect, style, FALSE, extendedStyle);

            return rect;
        }

        int GetCenteredX(const RECT& windowRect)
        {
            const int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
            const int windowWidth = windowRect.right - windowRect.left;

            return (screenWidth - windowWidth) / 2;
        }

        int GetCenteredY(const RECT& windowRect)
        {
            const int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);
            const int windowHeight = windowRect.bottom - windowRect.top;

            return (screenHeight - windowHeight) / 2;
        }
    }

    Window::~Window()
    {
        Destroy();
    }

    bool Window::Create(const WindowCreateInfo& createInfo)
    {
        if (mNativeHandle != nullptr)
        {
            Core::Logger::Warning("Window", "Create called, but window already exists.");
            return true;
        }

        if (createInfo.Width <= 0 || createInfo.Height <= 0)
        {
            Core::Logger::Error("Window", "Invalid window size.");
            return false;
        }

        HINSTANCE instanceHandle = ::GetModuleHandleW(nullptr);

        if (instanceHandle == nullptr)
        {
            Core::Logger::Error("Window", GetLastWin32ErrorMessage("Failed to get module handle."));
            return false;
        }

        if (!RegisterPlatformWindowClass(instanceHandle))
        {
            return false;
        }

        const DWORD windowStyle = BuildWindowStyle(createInfo);
        const DWORD extendedStyle = WS_EX_APPWINDOW;

        const RECT windowRect = BuildWindowRect(createInfo, windowStyle, extendedStyle);

        const int windowWidth = windowRect.right - windowRect.left;
        const int windowHeight = windowRect.bottom - windowRect.top;

        const int windowX = createInfo.StartCentered ? GetCenteredX(windowRect) : CW_USEDEFAULT;
        const int windowY = createInfo.StartCentered ? GetCenteredY(windowRect) : CW_USEDEFAULT;

        const std::wstring wideTitle = Utf8ToWide(createInfo.Title);

        HWND nativeHandle = ::CreateWindowExW(
            extendedStyle,
            WindowClassName,
            wideTitle.c_str(),
            windowStyle,
            windowX,
            windowY,
            windowWidth,
            windowHeight,
            nullptr,
            nullptr,
            instanceHandle,
            this
        );

        if (nativeHandle == nullptr)
        {
            Core::Logger::Error("Window", GetLastWin32ErrorMessage("Failed to create window."));
            return false;
        }

        mNativeHandle = nativeHandle;
        mInstanceHandle = instanceHandle;
        mTitle = createInfo.Title;
        mWidth = createInfo.Width;
        mHeight = createInfo.Height;
        mIsOpen = true;
        mCloseRequested = false;
        mHasFocus = false;

        if (createInfo.VisibleOnCreate)
        {
            Show();
        }

        const Core::String message = "Window created: " + mTitle;
        Core::Logger::Info("Window", message);

        return true;
    }

    void Window::Destroy()
    {
        if (mNativeHandle == nullptr)
        {
            mIsOpen = false;
            return;
        }

        HWND nativeHandle = static_cast<HWND>(mNativeHandle);

        mCloseRequested = true;

        ::DestroyWindow(nativeHandle);

        mNativeHandle = nullptr;
        mInstanceHandle = nullptr;
        mIsOpen = false;
        mHasFocus = false;
    }

    void Window::Show()
    {
        if (mNativeHandle == nullptr)
        {
            return;
        }

        HWND nativeHandle = static_cast<HWND>(mNativeHandle);

        ::ShowWindow(nativeHandle, SW_SHOW);
        ::UpdateWindow(nativeHandle);
    }

    void Window::Hide()
    {
        if (mNativeHandle == nullptr)
        {
            return;
        }

        HWND nativeHandle = static_cast<HWND>(mNativeHandle);

        ::ShowWindow(nativeHandle, SW_HIDE);
    }

    bool Window::PollEvents()
    {
        MSG message{};

        while (::PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                mCloseRequested = true;
                mIsOpen = false;
                return false;
            }

            ::TranslateMessage(&message);
            ::DispatchMessageW(&message);
        }

        return IsOpen();
    }

    bool Window::IsOpen() const
    {
        return mIsOpen && !mCloseRequested && mNativeHandle != nullptr;
    }

    bool Window::IsCloseRequested() const
    {
        return mCloseRequested;
    }

    bool Window::HasFocus() const
    {
        return mHasFocus;
    }

    Core::i32 Window::GetWidth() const
    {
        return mWidth;
    }

    Core::i32 Window::GetHeight() const
    {
        return mHeight;
    }

    void* Window::GetNativeHandle() const
    {
        return mNativeHandle;
    }

    void Window::SetTitle(const Core::StringView title)
    {
        mTitle = Core::String(title);

        if (mNativeHandle == nullptr)
        {
            return;
        }

        const std::wstring wideTitle = Utf8ToWide(mTitle);
        ::SetWindowTextW(static_cast<HWND>(mNativeHandle), wideTitle.c_str());
    }

    long long PlatformWindowProcedureBridge(
        void* nativeHandle,
        const unsigned int message,
        const unsigned long long wParam,
        const long long lParam
    )
    {
        HWND windowHandle = static_cast<HWND>(nativeHandle);

        Window* window = nullptr;

        if (message == WM_NCCREATE)
        {
            const CREATESTRUCTW* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
            window = static_cast<Window*>(createStruct->lpCreateParams);

            ::SetWindowLongPtrW(
                windowHandle,
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(window)
            );
        }
        else
        {
            window = reinterpret_cast<Window*>(
                ::GetWindowLongPtrW(windowHandle, GWLP_USERDATA)
            );
        }

        if (window == nullptr)
        {
            return ::DefWindowProcW(
                windowHandle,
                message,
                static_cast<WPARAM>(wParam),
                static_cast<LPARAM>(lParam)
            );
        }

        switch (message)
        {
        case WM_CLOSE:
            window->mCloseRequested = true;
            ::DestroyWindow(windowHandle);
            return 0;

        case WM_DESTROY:
            if (window->mNativeHandle == windowHandle)
            {
                window->mNativeHandle = nullptr;
            }

            window->mIsOpen = false;
            window->mCloseRequested = true;
            window->mHasFocus = false;

            return 0;

        case WM_SETFOCUS:
            window->mHasFocus = true;
            return 0;

        case WM_KILLFOCUS:
            window->mHasFocus = false;
            return 0;

        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
            {
                window->mWidth = static_cast<Core::i32>(LOWORD(lParam));
                window->mHeight = static_cast<Core::i32>(HIWORD(lParam));
            }
            return 0;

        case WM_INPUT:
            InputSystem::ProcessRawInputMessage(reinterpret_cast<void*>(lParam));
            break;

        default:
            break;
        }

        return ::DefWindowProcW(
            windowHandle,
            message,
            static_cast<WPARAM>(wParam),
            static_cast<LPARAM>(lParam)
        );
    }
}