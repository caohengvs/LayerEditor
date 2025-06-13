#include "logger/ILogger.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int main(int, char**){
    std::cout << "Hello, from project1!\n";

    CHUtils::ILogger& logger = CHUtils::ILogger::s_CreateInstance();
    logger.addLogger("main_logger", "main.log");
    logger.setDefaultLogger("main_logger");

    logger.info({"main.cpp", 42, "main"}, "This is a log message with value");

    CHUtils::ILogger::s_DestroyInstance();

}
