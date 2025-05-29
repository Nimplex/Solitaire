//
// Created by pszemek on 26.05.2025.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <unordered_map>
#include <chrono>
#include <format>

namespace Logger {
    inline const std::unordered_map<std::string_view, std::string> level_colors = {
        {"INFO ", "\033[42m"},  // green
        {"DEBUG", "\033[46m"},  // cyan
        {"WARN ", "\033[43m"},  // yellow
        {"ERROR", "\033[41m"},  // red
    };
    inline const std::string reset_color = "\033[0m";

    inline std::string current_time() {
        using namespace std::chrono;

        const auto now = system_clock::now();
        const auto local_time = current_zone()->to_local(now);
        auto truncated = floor<seconds>(local_time);

        return std::format("{:%H:%M:%S}", truncated);
    }

    template <typename... Args>
    void log(const std::string_view level, Args&&... args) {
        const auto& color_it = level_colors.find(level);
        const std::string& color_code = (color_it != level_colors.end()) ? color_it->second : "";

        if (!color_code.empty()) {
            std::cout << color_code << '[' << level << ']' << reset_color << ' ';
        } else {
            std::cout << '[' << level << "] ";
        }

        std::cout << current_time() << " - ";
        (std::cout << ... << args) << '\n';
    }

    template <typename... Args>
    void info(Args&&... args) {
        log("INFO ", std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(Args&&... args) {
        log("DEBUG", std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warn(Args&&... args) {
        log("WARN ", std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(Args&&... args) {
        log("ERROR", std::forward<Args>(args)...);
    }

}

#endif // LOGGER_H
