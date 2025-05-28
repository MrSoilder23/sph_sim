#pragma once
// C++ standard libraries
#include <fstream>
#include <iostream>

// Third_party libraries
#include <nlohmann/json.hpp>

struct WindowData {
    WindowData(std::string& filePath) {
        std::ifstream configFile(filePath);
        if (!configFile) {
            std::cerr << "Failed to open configuration file." << std::endl;
            exit(1);
        }

        nlohmann::json config;
        configFile >> config;

        mScreenWidth = config["window"]["width"];
        mScreenHeight = config["window"]["height"];
    }

    float mScreenWidth;
    float mScreenHeight;
};