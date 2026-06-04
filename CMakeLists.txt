cmake_minimum_required(VERSION 3.16)

project(MMDVM-Multi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MMDVM-Multi main.cpp Conf.cpp
    resampler.cpp fmmod.cpp receiver.cpp channelizer.cpp rotator.cpp
    transmitter.cpp device.cpp dmrtiming.cpp)
target_link_libraries(MMDVM-Multi stdc++ zmq liquid SoapySDR)
target_compile_options(MMDVM-Multi PRIVATE -Wall -Wextra -Wpedantic)

include(GNUInstallDirs)
install(TARGETS MMDVM-Multi
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
