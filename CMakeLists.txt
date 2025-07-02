cmake_minimum_required(VERSION 3.16)
project(CodeCommentsPlatform LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -no-pie")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(CodeCommentsPlatform
  src/main.cpp
  src/MainWindow.cpp
  src/CommentExtractor.cpp
  src/CommentSaver.cpp
  include/MainWindow.h
  include/CommentExtractor.h
  include/CommentSaver.h
)

target_include_directories(CodeCommentsPlatform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(CodeCommentsPlatform PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Threads::Threads)
