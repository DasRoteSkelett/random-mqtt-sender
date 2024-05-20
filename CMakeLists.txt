cmake_minimum_required(VERSION 3.28)
project(random_mqtt_sender)

include (GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)

find_package(PahoMqttCpp REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PahoMqttCpp::paho-mqttpp3)
install(TARGETS random_mqtt_sender
        RUNTIME          
          COMPONENT Runtime
          DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )