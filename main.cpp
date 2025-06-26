#include "logger/include/MyLogger.hpp"
#include <iostream>

int main(int, char**){
     INIT_MY_LOGGER("MyApplication", MyLogger::DEBUG_L, true, "logs/app.log");
    //  LOG_DEBUG << std::string("Application started") << 1;
     LOG_INFO << "This is an info message";
}
