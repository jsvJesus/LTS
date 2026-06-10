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

        mInitialized = true;

        Core::Logger::Info("Input", "Input system initialized.");

        return true;
    }

    void InputSystem::Shutdown()
    {
        if (!mInitialized && mNativeWindowHandle == nullptr)
            return;

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

        if (!IsWindowFocused())
        {
            ClearCurrentState();
            mHasMousePosition = false;
            return;
        }

        UpdateKeyboard();
        UpdateMouseButtons();
        UpdateMousePosition();
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

    void InputSystem::ClearCurrentState()
    {
        mCurrentKeys.fill(false);
        mCurrentMouseButtons.fill(false);

        mMouseDeltaX = 0;
        mMouseDeltaY = 0;
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