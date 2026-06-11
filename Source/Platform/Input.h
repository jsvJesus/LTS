#pragma once

#include "Core/BaseTypes.h"

#include <array>
#include <cstddef>

namespace Platform
{
    enum class KeyCode : Core::u16
    {
        Unknown = 0,

        Backspace = 0x08,
        Tab = 0x09,
        Enter = 0x0D,
        Shift = 0x10,
        Control = 0x11,
        Alt = 0x12,
        Pause = 0x13,
        CapsLock = 0x14,
        Escape = 0x1B,
        Space = 0x20,

        PageUp = 0x21,
        PageDown = 0x22,
        End = 0x23,
        Home = 0x24,

        Left = 0x25,
        Up = 0x26,
        Right = 0x27,
        Down = 0x28,

        Insert = 0x2D,
        Delete = 0x2E,

        Key0 = '0',
        Key1 = '1',
        Key2 = '2',
        Key3 = '3',
        Key4 = '4',
        Key5 = '5',
        Key6 = '6',
        Key7 = '7',
        Key8 = '8',
        Key9 = '9',

        A = 'A',
        B = 'B',
        C = 'C',
        D = 'D',
        E = 'E',
        F = 'F',
        G = 'G',
        H = 'H',
        I = 'I',
        J = 'J',
        K = 'K',
        L = 'L',
        M = 'M',
        N = 'N',
        O = 'O',
        P = 'P',
        Q = 'Q',
        R = 'R',
        S = 'S',
        T = 'T',
        U = 'U',
        V = 'V',
        W = 'W',
        X = 'X',
        Y = 'Y',
        Z = 'Z',

        NumPad0 = 0x60,
        NumPad1 = 0x61,
        NumPad2 = 0x62,
        NumPad3 = 0x63,
        NumPad4 = 0x64,
        NumPad5 = 0x65,
        NumPad6 = 0x66,
        NumPad7 = 0x67,
        NumPad8 = 0x68,
        NumPad9 = 0x69,

        F1 = 0x70,
        F2 = 0x71,
        F3 = 0x72,
        F4 = 0x73,
        F5 = 0x74,
        F6 = 0x75,
        F7 = 0x76,
        F8 = 0x77,
        F9 = 0x78,
        F10 = 0x79,
        F11 = 0x7A,
        F12 = 0x7B,

        LeftShift = 0xA0,
        RightShift = 0xA1,
        LeftControl = 0xA2,
        RightControl = 0xA3,
        LeftAlt = 0xA4,
        RightAlt = 0xA5
    };

    enum class MouseButton : Core::u8
    {
        Left = 0,
        Right,
        Middle,
        X1,
        X2,

        Count
    };

    enum class CursorMode : Core::u8
    {
        Normal = 0,
        Hidden,
        Locked
    };

    class InputSystem final
    {
    public:
        InputSystem() = default;
        ~InputSystem();

        InputSystem(const InputSystem&) = delete;
        InputSystem& operator=(const InputSystem&) = delete;

        bool Initialize(void* nativeWindowHandle);
        void Shutdown();

        void Update();

        [[nodiscard]] bool IsInitialized() const { return mInitialized; }

        [[nodiscard]] bool IsKeyDown(KeyCode key) const;
        [[nodiscard]] bool IsKeyPressed(KeyCode key) const;
        [[nodiscard]] bool IsKeyReleased(KeyCode key) const;

        [[nodiscard]] bool IsMouseButtonDown(MouseButton button) const;
        [[nodiscard]] bool IsMouseButtonPressed(MouseButton button) const;
        [[nodiscard]] bool IsMouseButtonReleased(MouseButton button) const;

        [[nodiscard]] Core::i32 GetMouseX() const { return mMouseX; }
        [[nodiscard]] Core::i32 GetMouseY() const { return mMouseY; }

        [[nodiscard]] Core::i32 GetMouseDeltaX() const { return mMouseDeltaX; }
        [[nodiscard]] Core::i32 GetMouseDeltaY() const { return mMouseDeltaY; }

        [[nodiscard]] Core::i32 GetRawMouseDeltaX() const { return mRawMouseDeltaX; }
        [[nodiscard]] Core::i32 GetRawMouseDeltaY() const { return mRawMouseDeltaY; }

        [[nodiscard]] bool HasMousePosition() const { return mHasMousePosition; }
        [[nodiscard]] bool IsRawMouseInputAvailable() const { return mRawMouseInputAvailable; }

        void SetCursorMode(CursorMode mode);
        void ToggleCursorLock();

        [[nodiscard]] CursorMode GetCursorMode() const { return mCursorMode; }
        [[nodiscard]] bool IsCursorLocked() const { return mCursorMode == CursorMode::Locked; }
        [[nodiscard]] bool IsCursorHidden() const { return mCursorMode == CursorMode::Hidden || mCursorMode == CursorMode::Locked; }

        static void ProcessRawInputMessage(void* rawInputHandle);

    private:
        static constexpr std::size_t KeyCount = 256;
        static constexpr std::size_t MouseButtonCount =
            static_cast<std::size_t>(MouseButton::Count);

        void UpdateKeyboard();
        void UpdateMouseButtons();
        void UpdateMousePosition();
        void UpdateRawMouseDelta();

        bool RegisterRawMouseInput();
        void UnregisterRawMouseInput();
        void HandleRawInputMessage(void* rawInputHandle);

        void ApplyCursorMode();
        void ApplyCursorVisibility(bool visible);
        void UpdateCursorClip();
        void ReleaseCursorClip();

        void ClearCurrentState();
        void ClearAllState();

        [[nodiscard]] bool IsWindowFocused() const;

    private:
        bool mInitialized = false;

        void* mNativeWindowHandle = nullptr;

        std::array<bool, KeyCount> mCurrentKeys {};
        std::array<bool, KeyCount> mPreviousKeys {};

        std::array<bool, MouseButtonCount> mCurrentMouseButtons {};
        std::array<bool, MouseButtonCount> mPreviousMouseButtons {};

        Core::i32 mMouseX = 0;
        Core::i32 mMouseY = 0;

        Core::i32 mMouseDeltaX = 0;
        Core::i32 mMouseDeltaY = 0;

        Core::i32 mRawMouseDeltaX = 0;
        Core::i32 mRawMouseDeltaY = 0;

        Core::i32 mPendingRawMouseDeltaX = 0;
        Core::i32 mPendingRawMouseDeltaY = 0;

        bool mHasMousePosition = false;
        bool mRawMouseInputAvailable = false;

        CursorMode mCursorMode = CursorMode::Normal;

        bool mCursorCurrentlyVisible = true;
        bool mCursorCurrentlyClipped = false;
    };
}