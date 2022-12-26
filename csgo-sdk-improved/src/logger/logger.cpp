#include <filesystem>
#include <array>

#include "logger.hpp"

#include <spdlog/sinks/basic_file_sink.h>

void Logger::Initialize()
{
    const std::filesystem::path tempDirPath = std::filesystem::temp_directory_path();

    std::string tempDirString = tempDirPath.string();
    tempDirString.append(SDK_NAME ".txt");

    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(tempDirString, true);
    fileSink->set_level(spdlog::level::trace);
    fileSink->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

    // You could add more sinks if you'd wish to.
    // I prefer log files.
    auto sinks = std::array{fileSink};

    auto logger = std::make_shared<spdlog::logger>(SDK_NAME, sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);

    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);

    SDK_INFO("Initialized logger!");
}

void Logger::Shutdown()
{
    SDK_INFO("Shutdown logger!");

    spdlog::shutdown();
}

void Logger::TurnOn()
{
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
}

void Logger::TurnOff()
{
    spdlog::set_level(spdlog::level::off);
    spdlog::flush_on(spdlog::level::off);
}
