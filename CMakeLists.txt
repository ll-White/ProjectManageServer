cmake_minimum_required(VERSION 3.16)

project(ProjectManageServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MINGW)
    add_compile_options(-Wall -Wextra)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ProjectManage_Server
)

if (NOT DEFINED CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "D:/Qt/Qt5.12.0/5.12.0/mingw73_64")
endif()

find_package(Qt5 REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        Network
        Sql
        Concurrent
)

set(PROJECT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/ProjectManage_Server)
set(PUBLIC_CLASS_DIR ${PROJECT_SRC_DIR}/PublicClass)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(TRANSLATIONS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/translations)

set(APP_SOURCES
        ${PROJECT_SRC_DIR}/main.cpp
        ${PROJECT_SRC_DIR}/MananWindow.cpp
        ${PROJECT_SRC_DIR}/Shareheader.cpp
        ${PROJECT_SRC_DIR}/TaskCheckRunnable.cpp
        ${PROJECT_SRC_DIR}/TcpRunable.cpp
        ${PROJECT_SRC_DIR}/WnTCPServer.cpp
        ${PUBLIC_CLASS_DIR}/SqlQueryEx.cpp
        ${PUBLIC_CLASS_DIR}/WnDialog.cpp
        ${PUBLIC_CLASS_DIR}/WnLog.cpp
        ${PUBLIC_CLASS_DIR}/WnShareData.cpp
        ${PUBLIC_CLASS_DIR}/WnTcpClient.cpp
        ${PUBLIC_CLASS_DIR}/WnUdpClient.cpp
)

set(APP_HEADERS
        ${PROJECT_SRC_DIR}/MananWindow.h
        ${PROJECT_SRC_DIR}/ShareStructServer.h
        ${PROJECT_SRC_DIR}/Shareheader.h
        ${PROJECT_SRC_DIR}/TaskCheckRunnable.h
        ${PROJECT_SRC_DIR}/TcpRunable.h
        ${PROJECT_SRC_DIR}/WnTCPServer.h
        ${PUBLIC_CLASS_DIR}/SqlQueryEx.h
        ${PUBLIC_CLASS_DIR}/WnDebugLog.h
        ${PUBLIC_CLASS_DIR}/WnDialog.h
        ${PUBLIC_CLASS_DIR}/WnLog.h
        ${PUBLIC_CLASS_DIR}/WnShareData.h
        ${PUBLIC_CLASS_DIR}/WnTcpClient.h
        ${PUBLIC_CLASS_DIR}/WnUdpClient.h
)

set(APP_FORMS
        ${PROJECT_SRC_DIR}/MananWindow.ui
)

set(APP_RESOURCES
        ${PROJECT_SRC_DIR}/MananWindow.qrc
)

add_executable(${PROJECT_NAME} WIN32
        ${APP_SOURCES}
        ${APP_HEADERS}
        ${APP_FORMS}
        ${APP_RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${PROJECT_SRC_DIR}
        ${PUBLIC_CLASS_DIR}
        ${INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Network
        Qt5::Sql
        Qt5::Concurrent
        ws2_32
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)

if (EXISTS ${TRANSLATIONS_DIR})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/translations
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${TRANSLATIONS_DIR}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/translations
    )
endif()