cmake_minimum_required(VERSION 3.21)
project(network C)

set(CMAKE_C_STANDARD 90)

add_executable(network crc_encoder.c crc_decoder.c)
