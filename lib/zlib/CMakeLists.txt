cmake_minimum_required(VERSION 3.2)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(z STATIC inflate.c adler32.c inftrees.c zutil.c crc32.c deflate.c compress.c trees.c gzio.c)
