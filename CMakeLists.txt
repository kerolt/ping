project(PING)

add_executable(ping main.cpp ping.cpp) 

target_link_libraries(ping "ws2_32") # 链接ws2_32.lib