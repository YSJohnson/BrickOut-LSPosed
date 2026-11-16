cmake_minimum_required(VERSION 3.22.1)
project(brickout C)
add_library(brickout SHARED items.c)
target_link_libraries(brickout log dl)
