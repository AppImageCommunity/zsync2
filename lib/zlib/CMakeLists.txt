cmake_minimum_required(VERSION 3.5)

set(CMAKE_POSITION_INDEPENDENT_CODE true)
set(CMAKE_INCLUDE_CURRENT_DIR true)

add_library(zsync2_libz STATIC inflate.c adler32.c inftrees.c zutil.c crc32.c deflate.c compress.c trees.c gzio.c)
