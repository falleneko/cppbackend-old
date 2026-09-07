cmake_minimum_required(VERSION 3.11)

project(restaurant CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ищем Boost версии 1.78
find_package(Boost 1.78.0 REQUIRED)
if(Boost_FOUND)
  # boost найден, добавляем к каталогам заголовочных файлов проекта путь к
  # заголовочным файлам boost
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(restaurant src/main.cpp)
target_link_libraries(restaurant PRIVATE Boost::system Threads::Threads)
