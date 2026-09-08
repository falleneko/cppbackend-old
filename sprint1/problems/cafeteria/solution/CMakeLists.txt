cmake_minimum_required(VERSION 3.11)

project(cafeteria CXX)
set(CMAKE_CXX_STANDARD 20)

# Ищем Boost версии 1.78
find_package(Boost 1.78.0 REQUIRED)
if(Boost_FOUND)
  # boost найден, добавляем к каталогам заголовочных файлов проекта путь к
  # заголовочным файлам boost
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(cafeteria
	src/main.cpp
	src/cafeteria.h
	src/result.h
	src/hotdog.h
	src/gascooker.h
	src/ingredients.h
	src/clock.h
)
target_link_libraries(cafeteria PRIVATE Threads::Threads)
