#include "Platform/Input.h"

#include "Core/Logger.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

namespace Platform
{
    namespace
    {
        InputSystem* GActiveInputSystem = nullptr;

        [[nodiscard]] int ToVirtualKey(const KeyCode key)
        {
            return static_cast<int>(key);
        }

        [[nodiscard]] int ToVirtualMouseButton(const MouseButton button)
        {
            switch (button)
            {
            case MouseButton::Left:
                return VK_LBUTTON;

            case MouseButton::Right:
                return VK_RBUTTON;

            case MouseButton::Middle:
                return VK_MBUTTON;

            case MouseButton::X1:
                return VK_XBUTTON1;

            case MouseButton::X2:
                return VK_XBUTTON2;

            default:
                return 0;
            }
        }

        [[nodiscard]] bool IsVirtualKeyDown(const int virtualKey)
        {
            if (virtualKey <= 0)
                return false;

            return (::GetAsyncKeyState(virtualKey) & 0x8000) != 0;
        }

        [[nodiscard]] bool BuildClientRectInScreenSpace(HWND windowHandle, RECT& outRect)
        {
            if (windowHandle == nullptr)
                return false;

            RECT clientRect {};

            if (!::GetClientRect(windowHandle, &clientRect))
                return false;

            POINT topLeft {};
            topLeft.x = clientRect.left;
            topLeft.y = clientRect.top;

            POINT bottomRight {};
            bottomRight.x = clientRect.right;
            bottomRight.y = clientRect.bottom;

            if (!::ClientToScreen(windowHandle, &topLeft))
                return false;

            if (!::ClientToScreen(windowHandle, &bottomRight))
                return false;

            outRect.left = topLeft.x;
            outRect.top = topLeft.y;
            outRect.right = bottomRight.x;
            outRect.bottom = bottomRight.y;

            return true;
        }
    }

    InputSystem::~InputSystem()
    {
        Shutdown();
    }

    bool InputSystem::Initialize(void* nativeWindowHandle)
    {
        Shutdown();

        if (nativeWindowHandle == nullptr)
        {
            Core::Logger::Error("Input", "Failed to initialize input system. Native window handle is null.");
            return false;
        }

        mNativeWindowHandle = nativeWindowHandle;

        ClearAllState();

        GActiveInputSystem = this;

        mRawMouseInputAvailable = RegisterRawMouseInput();

        mInitialized = true;

        Core::Logger::Info("Input", "Input system initialized.");

        if (mRawMouseInputAvailable)
        {
            Core::Logger::Info("Input", "Raw mouse input registered.");
        }
        else
        {
            Core::Logger::Warning("Input", "Raw mouse input is not available.");
        }

        return true;
    }

    void InputSystem::Shutdown()
    {
        if (!mInitialized && mNativeWindowHandle == nullptr)
            return;

        if (GActiveInputSystem == this)
        {
            GActiveInputSystem = nullptr;
        }

        UnregisterRawMouseInput();

        ReleaseCursorClip();
        ApplyCursorVisibility(true);

        ClearAllState();

        mNativeWindowHandle = nullptr;
        mInitialized = false;

        Core::Logger::Info("Input", "Input system shutdown.");
    }

    void InputSystem::Update()
    {
        if (!mInitialized)
            return;

        mPreviousKeys = mCurrentKeys;
        mPreviousMouseButtons = mCurrentMouseButtons;

        mMouseDeltaX = 0;
        mMouseDeltaY = 0;

        mRawMouseDeltaX = 0;
        mRawMouseDeltaY = 0;

        if (!IsWindowFocused())
        {
            ClearCurrentState();

            mPendingRawMouseDeltaX = 0;
            mPendingRawMouseDeltaY = 0;

            mHasMousePosition = false;

            ReleaseCursorClip();
            ApplyCursorVisibility(true);

            return;
        }

        UpdateKeyboard();
        UpdateMouseButtons();
        UpdateMousePosition();
        UpdateRawMouseDelta();

        ApplyCursorMode();
    }

    bool InputSystem::IsKeyDown(const KeyCode key) const
    {
        const int virtualKey = ToVirtualKey(key);

        if (virtualKey < 0 || virtualKey >= static_cast<int>(KeyCount))
            return false;

        return mCurrentKeys[static_cast<std::size_t>(virtualKey)];
    }

    bool InputSystem::IsKeyPressed(const KeyCode key) const
    {
        const int virtualKey = ToVirtualKey(key);

        if (virtualKey < 0 || virtualKey >= static_cast<int>(KeyCount))
            return false;

        const std::size_t index = static_cast<std::size_t>(virtualKey);

        return mCurrentKeys[index] && !mPreviousKeys[index];
    }

    bool InputSystem::IsKeyReleased(const KeyCode key) const
    {
        const int virtualKey = ToVirtualKey(key);

        if (virtualKey < 0 || virtualKey >= static_cast<int>(KeyCount))
            return false;

        const std::size_t index = static_cast<std::size_t>(virtualKey);

        return !mCurrentKeys[index] && mPreviousKeys[index];
    }

    bool InputSystem::IsMouseButtonDown(const MouseButton button) const
    {
        const std::size_t index = static_cast<std::size_t>(button);

        if (index >= MouseButtonCount)
            return false;

        return mCurrentMouseButtons[index];
    }

    bool InputSystem::IsMouseButtonPressed(const MouseButton button) const
    {
        const std::size_t index = static_cast<std::size_t>(button);

        if (index >= MouseButtonCount)
            return false;

        return mCurrentMouseButtons[index] && !mPreviousMouseButtons[index];
    }

    bool InputSystem::IsMouseButtonReleased(const MouseButton button) const
    {
        const std::size_t index = static_cast<std::size_t>(button);

        if (index >= MouseButtonCount)
            return false;

        return !mCurrentMouseButtons[index] && mPreviousMouseButtons[index];
    }

    void InputSystem::SetCursorMode(const CursorMode mode)
    {
        if (mCursorMode == mode)
            return;

        mCursorMode = mode;

        if (mInitialized)
        {
            ApplyCursorMode();
        }
    }

    void InputSystem::ToggleCursorLock()
    {
        if (mCursorMode == CursorMode::Locked)
        {
            SetCursorMode(CursorMode::Normal);
        }
        else
        {
            SetCursorMode(CursorMode::Locked);
        }
    }

    void InputSystem::ProcessRawInputMessage(void* rawInputHandle)
    {
        if (GActiveInputSystem == nullptr)
            return;

        GActiveInputSystem->HandleRawInputMessage(rawInputHandle);
    }

    void InputSystem::UpdateKeyboard()
    {
        for (std::size_t keyIndex = 0; keyIndex < KeyCount; ++keyIndex)
        {
            mCurrentKeys[keyIndex] = IsVirtualKeyDown(static_cast<int>(keyIndex));
        }
    }

    void InputSystem::UpdateMouseButtons()
    {
        for (std::size_t buttonIndex = 0; buttonIndex < MouseButtonCount; ++buttonIndex)
        {
            const MouseButton button = static_cast<MouseButton>(buttonIndex);
            const int virtualButton = ToVirtualMouseButton(button);

            mCurrentMouseButtons[buttonIndex] = IsVirtualKeyDown(virtualButton);
        }
    }

    void InputSystem::UpdateMousePosition()
    {
        HWND windowHandle = static_cast<HWND>(mNativeWindowHandle);

        if (windowHandle == nullptr)
        {
            mHasMousePosition = false;
            return;
        }

        POINT cursorPosition {};

        if (!::GetCursorPos(&cursorPosition))
        {
            mHasMousePosition = false;
            return;
        }

        if (!::ScreenToClient(windowHandle, &cursorPosition))
        {
            mHasMousePosition = false;
            return;
        }

        const Core::i32 newMouseX = static_cast<Core::i32>(cursorPosition.x);
        const Core::i32 newMouseY = static_cast<Core::i32>(cursorPosition.y);

        if (mHasMousePosition)
        {
            mMouseDeltaX = newMouseX - mMouseX;
            mMouseDeltaY = newMouseY - mMouseY;
        }
        else
        {
            mMouseDeltaX = 0;
            mMouseDeltaY = 0;
        }

        mMouseX = newMouseX;
        mMouseY = newMouseY;
        mHasMousePosition = true;
    }

    void InputSystem::UpdateRawMouseDelta()
    {
        mRawMouseDeltaX = mPendingRawMouseDeltaX;
        mRawMouseDeltaY = mPendingRawMouseDeltaY;

        mPendingRawMouseDeltaX = 0;
        mPendingRawMouseDeltaY = 0;
    }

    bool InputSystem::RegisterRawMouseInput()
    {
        HWND windowHandle = static_cast<HWND>(mNativeWindowHandle);

        if (windowHandle == nullptr)
            return false;

        RAWINPUTDEVICE rawInputDevice {};
        rawInputDevice.usUsagePage = 0x01;
        rawInputDevice.usUsage = 0x02;
        rawInputDevice.dwFlags = 0;
        rawInputDevice.hwndTarget = windowHandle;

        const BOOL result = ::RegisterRawInputDevices(
            &rawInputDevice,
            1,
            sizeof(RAWINPUTDEVICE)
        );

        return result == TRUE;
    }

    void InputSystem::UnregisterRawMouseInput()
    {
        RAWINPUTDEVICE rawInputDevice {};
        rawInputDevice.usUsagePage = 0x01;
        rawInputDevice.usUsage = 0x02;
        rawInputDevice.dwFlags = RIDEV_REMOVE;
        rawInputDevice.hwndTarget = nullptr;

        ::RegisterRawInputDevices(
            &rawInputDevice,
            1,
            sizeof(RAWINPUTDEVICE)
        );

        mRawMouseInputAvailable = false;
    }

    void InputSystem::HandleRawInputMessage(void* rawInputHandle)
    {
        if (!mInitialized || rawInputHandle == nullptr)
            return;

        HRAWINPUT inputHandle = static_cast<HRAWINPUT>(rawInputHandle);

        RAWINPUT rawInput {};
        UINT rawInputSize = sizeof(RAWINPUT);

        const UINT bytesRead = ::GetRawInputData(
            inputHandle,
            RID_INPUT,
            &rawInput,
            &rawInputSize,
            sizeof(RAWINPUTHEADER)
        );

        if (bytesRead == static_cast<UINT>(-1))
            return;

        if (rawInput.header.dwType != RIM_TYPEMOUSE)
            return;

        mPendingRawMouseDeltaX += static_cast<Core::i32>(rawInput.data.mouse.lLastX);
        mPendingRawMouseDeltaY += static_cast<Core::i32>(rawInput.data.mouse.lLastY);
    }

    void InputSystem::ApplyCursorMode()
    {
        switch (mCursorMode)
        {
        case CursorMode::Normal:
            ReleaseCursorClip();
            ApplyCursorVisibility(true);
            break;

        case CursorMode::Hidden:
            ReleaseCursorClip();
            ApplyCursorVisibility(false);
            break;

        case CursorMode::Locked:
            ApplyCursorVisibility(false);
            UpdateCursorClip();
            break;

        default:
            ReleaseCursorClip();
            ApplyCursorVisibility(true);
            break;
        }
    }

    void InputSystem::ApplyCursorVisibility(const bool visible)
    {
        if (mCursorCurrentlyVisible == visible)
            return;

        if (visible)
        {
            for (int attemptIndex = 0; attemptIndex < 16; ++attemptIndex)
            {
                const int result = ::ShowCursor(TRUE);

                if (result >= 0)
                    break;
            }
        }
        else
        {
            for (int attemptIndex = 0; attemptIndex < 16; ++attemptIndex)
            {
                const int result = ::ShowCursor(FALSE);

                if (result < 0)
                    break;
            }
        }

        mCursorCurrentlyVisible = visible;
    }

    void InputSystem::UpdateCursorClip()
    {
        HWND windowHandle = static_cast<HWND>(mNativeWindowHandle);

        if (windowHandle == nullptr)
            return;

        RECT clipRect {};

        if (!BuildClientRectInScreenSpace(windowHandle, clipRect))
            return;

        ::ClipCursor(&clipRect);

        mCursorCurrentlyClipped = true;
    }

    void InputSystem::ReleaseCursorClip()
    {
        if (!mCursorCurrentlyClipped)
            return;

        ::ClipCursor(nullptr);

        mCursorCurrentlyClipped = false;
    }

    void InputSystem::ClearCurrentState()
    {
        mCurrentKeys.fill(false);
        mCurrentMouseButtons.fill(false);

        mMouseDeltaX = 0;
        mMouseDeltaY = 0;

        mRawMouseDeltaX = 0;
        mRawMouseDeltaY = 0;
    }

    void InputSystem::ClearAllState()
    {
        mCurrentKeys.fill(false);
        mPreviousKeys.fill(false);

        mCurrentMouseButtons.fill(false);
        mPreviousMouseButtons.fill(false);

        mMouseX = 0;
        mMouseY = 0;

        mMouseDeltaX = 0;
        mMouseDeltaY = 0;

        mRawMouseDeltaX = 0;
        mRawMouseDeltaY = 0;

        mPendingRawMouseDeltaX = 0;
        mPendingRawMouseDeltaY = 0;

        mHasMousePosition = false;
    }

    bool InputSystem::IsWindowFocused() const
    {
        HWND windowHandle = static_cast<HWND>(mNativeWindowHandle);

        if (windowHandle == nullptr)
            return false;

        return ::GetForegroundWindow() == windowHandle;
    }
}