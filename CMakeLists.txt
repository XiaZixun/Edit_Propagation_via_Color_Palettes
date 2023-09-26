cmake_minimum_required(VERSION 3.14)

project(Recolor_GUI LANGUAGES CXX)

option(CXX "enable C++ compilation" ON)
enable_language(CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

# load Qt Components
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets DataVisualization OpenGL REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets DataVisualization OpenGL REQUIRED)

# load opengl 
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# load opencv4
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# load QtColorWidgets
add_subdirectory(./include/Qt-Color-Widgets)

# find Gurobi
find_package(GUROBI REQUIRED)
include_directories(${GUROBI_INCLUDE_DIRS})

# load palette_optimizer
include_directories(./include/palette_optmizer)

include_directories(./src)
aux_source_directory(./src GUI_SRC)
message(${GUI_SRC})

add_executable(${CMAKE_PROJECT_NAME}
    main.cpp
    ${GUI_SRC}
)

target_link_libraries(${CMAKE_PROJECT_NAME} optimized ${GUROBI_CXX_LIBRARY}
                      debug ${GUROBI_CXX_DEBUG_LIBRARY})
target_link_libraries(${CMAKE_PROJECT_NAME} ${GUROBI_LIBRARY})

target_link_libraries(${CMAKE_PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DataVisualization
    Qt${QT_VERSION_MAJOR}::OpenGL
    ${OPENGL_LIBRARIES}
    ${OpenCV_LIBRARIES}
    QtColorWidgets
)

