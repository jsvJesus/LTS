#pragma once

#include "Core/BaseTypes.h"

namespace Platform
{
    struct WindowCreateInfo final
    {
        Core::String Title = "Application";
        Core::i32 Width = 1280;
        Core::i32 Height = 720;

        bool StartCentered = true;
        bool Resizable = true;
        bool VisibleOnCreate = false;
    };

    class Window final
    {
    public:
        Window() = default;
        ~Window();

        Window(const Window&) = delete;
        Window(Window&&) = delete;

        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&) = delete;

        bool Create(const WindowCreateInfo& createInfo);
        void Destroy();

        void Show();
        void Hide();

        [[nodiscard]] bool PollEvents();

        [[nodiscard]] bool IsOpen() const;
        [[nodiscard]] bool IsCloseRequested() const;
        [[nodiscard]] bool HasFocus() const;

        [[nodiscard]] Core::i32 GetWidth() const;
        [[nodiscard]] Core::i32 GetHeight() const;

        [[nodiscard]] void* GetNativeHandle() const;

        void SetTitle(Core::StringView title);

    private:
        friend long long PlatformWindowProcedureBridge(
            void* nativeHandle,
            unsigned int message,
            unsigned long long wParam,
            long long lParam
        );

        void* mNativeHandle = nullptr;
        void* mInstanceHandle = nullptr;

        Core::String mTitle;

        Core::i32 mWidth = 0;
        Core::i32 mHeight = 0;

        bool mIsOpen = false;
        bool mCloseRequested = false;
        bool mHasFocus = false;
    };
}