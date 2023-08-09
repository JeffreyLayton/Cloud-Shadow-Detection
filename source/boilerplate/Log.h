#pragma once
#include <fmt/color.h>
#include <fmt/format.h>

namespace Log {
template<typename S1, typename S2, typename S3, typename... Args>
void _log(const S1 &prefix, const S2 &c, const S3 &format_str, Args &&...args) {
    auto colored_prefix = fmt::format(fg(c), "[{}]", prefix);
    auto white_message
        = fmt::format(fg(fmt::color::white), format_str, std::forward<Args>(args)...);
    fmt::print("{} {}\n", colored_prefix, white_message);
}

template<typename S, typename... Args>
void debug(const S &format_str, Args &&...args) {
    _log("DEBUG", fmt::color::green, format_str, args...);
}

template<typename S, typename... Args>
void info(const S &format_str, Args &&...args) {
    _log("INFO", fmt::color::light_gray, format_str, args...);
}

template<typename S, typename... Args>
void warning(const S &format_str, Args &&...args) {
    _log("WARN", fmt::color::yellow, format_str, args...);
}
template<typename S, typename... Args>
void warn(const S &format_str, Args &&...args) {
    _log("WARN", fmt::color::yellow, format_str, args...);
}

template<typename S, typename... Args>
void error(const S &format_str, Args &&...args) {
    _log("ERROR", fmt::color::red, format_str, args...);
}
}  // namespace Log
