cmake_minimum_required(VERSION 3.16)

project(MMDVM-Multi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MMDVM-Multi MMDVM-Multi.cpp Conf.cpp UDPSocket.cpp Network.cpp
    Resampler.cpp FMMod.cpp Receiver.cpp Channelizer.cpp Rotator.cpp
    Transmitter.cpp Device.cpp DMRTiming.cpp)

add_executable(SVXBridge SVXBridge.cpp Conf.cpp UDPSocket.cpp)

target_link_libraries(MMDVM-Multi stdc++ liquid SoapySDR)
target_link_libraries(SVXBridge stdc++ liquid)
target_compile_options(MMDVM-Multi PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(SVXBridge PRIVATE -Wall -Wextra -Wpedantic)

include(GNUInstallDirs)
install(TARGETS MMDVM-Multi
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(TARGETS SVXBridge
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
