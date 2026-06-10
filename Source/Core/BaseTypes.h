#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Core
{
    using i8  = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

    using u8  = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using f32 = float;
    using f64 = double;

    using usize = std::size_t;
    using isize = std::ptrdiff_t;

    using String = std::string;
    using StringView = std::string_view;
    using Path = std::filesystem::path;

    template <typename T>
    using UniquePtr = std::unique_ptr<T>;

    template <typename T>
    using SharedPtr = std::shared_ptr<T>;

    template <typename T>
    using WeakPtr = std::weak_ptr<T>;

    template <typename T>
    using Optional = std::optional<T>;

    template <typename T>
    using Vector = std::vector<T>;

    template <typename T, usize Size>
    using Array = std::array<T, Size>;

    template <typename Key, typename Value>
    using HashMap = std::unordered_map<Key, Value>;

    enum class BuildConfiguration : u8
    {
        Debug,
        Release,
        Final
    };

    [[nodiscard]] constexpr BuildConfiguration GetBuildConfiguration()
    {
    #if defined(GAME_FINAL)
        return BuildConfiguration::Final;
    #elif defined(GAME_RELEASE)
        return BuildConfiguration::Release;
    #else
        return BuildConfiguration::Debug;
    #endif
    }

    [[nodiscard]] constexpr StringView GetBuildConfigurationName()
    {
        switch (GetBuildConfiguration())
        {
        case BuildConfiguration::Debug:
            return "Debug";
        case BuildConfiguration::Release:
            return "Release";
        case BuildConfiguration::Final:
            return "Final";
        default:
            return "Unknown";
        }
    }

    [[nodiscard]] constexpr bool IsDebugBuild()
    {
        return GetBuildConfiguration() == BuildConfiguration::Debug;
    }

    [[nodiscard]] constexpr bool IsReleaseBuild()
    {
        return GetBuildConfiguration() == BuildConfiguration::Release;
    }

    [[nodiscard]] constexpr bool IsFinalBuild()
    {
        return GetBuildConfiguration() == BuildConfiguration::Final;
    }
}

static_assert(sizeof(Core::i8) == 1);
static_assert(sizeof(Core::u8) == 1);
static_assert(sizeof(Core::i16) == 2);
static_assert(sizeof(Core::u16) == 2);
static_assert(sizeof(Core::i32) == 4);
static_assert(sizeof(Core::u32) == 4);
static_assert(sizeof(Core::i64) == 8);
static_assert(sizeof(Core::u64) == 8);
static_assert(sizeof(Core::f32) == 4);
static_assert(sizeof(Core::f64) == 8);