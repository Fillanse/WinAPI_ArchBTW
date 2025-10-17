cmake_minimum_required(VERSION 3.16)

project(WinAPI_Workspace VERSION 1.0.0 LANGUAGES C CXX)

# Общие настройки
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Кросс-компиляция
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER /usr/bin/x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER /usr/bin/x86_64-w64-mingw32-windres)
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32/include)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_definitions(-DUNICODE -D_UNICODE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -municode")

# Автоматическое добавление всех подпапок с CMakeLists.txt
file(GLOB PROJECT_DIRS LIST_DIRECTORIES true "*")
foreach(PROJECT_DIR ${PROJECT_DIRS})
    if(IS_DIRECTORY ${PROJECT_DIR} AND 
       EXISTS "${PROJECT_DIR}/CMakeLists.txt" AND
       NOT ${PROJECT_DIR} STREQUAL ${CMAKE_BINARY_DIR})
        get_filename_component(PROJECT_NAME ${PROJECT_DIR} NAME)
        message(STATUS "Adding project: ${PROJECT_NAME}")
        add_subdirectory(${PROJECT_DIR})
    endif()
endforeach()

# Псевдо-таргет для сборки всех проектов
add_custom_target(BuildAll)

# Получаем все добавленные executable таргеты
get_property(ALL_TARGETS DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
foreach(TARGET ${ALL_TARGETS})
    get_target_property(TARGET_TYPE ${TARGET} TYPE)
    if(TARGET_TYPE STREQUAL "EXECUTABLE")
        add_dependencies(BuildAll ${TARGET})
    endif()
endforeach()

message(STATUS "Workspace configured with ${CMAKE_PROJECT_NAME}")