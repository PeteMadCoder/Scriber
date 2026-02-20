cmake_minimum_required(VERSION 3.16)
project(scriber VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components - REMOVE Core5Compat
find_package(Qt6 REQUIRED COMPONENTS Core Widgets PrintSupport) # <-- CHANGED

find_package(PkgConfig REQUIRED)
pkg_check_modules(HUNSPELL REQUIRED hunspell)

qt_standard_project_setup()

# --- MANUAL RCC STEP ---
set(RESOURCE_CPP ${CMAKE_CURRENT_BINARY_DIR}/qrc_resources.cpp)

add_custom_command(
    OUTPUT ${RESOURCE_CPP}
    COMMAND Qt6::rcc
    ARGS ${CMAKE_SOURCE_DIR}/resources.qrc -o ${RESOURCE_CPP}
    DEPENDS ${CMAKE_SOURCE_DIR}/resources.qrc
    COMMENT "Running RCC on resources.qrc"
    VERBATIM
)
# --- END MANUAL RCC ---

# --- Create the executable ---
qt_add_executable(scriber
    src/main.cpp
    include/mainwindow.h src/mainwindow.cpp
    include/editorwidget.h src/editorwidget.cpp
    include/markdownhighlighter.h src/markdownhighlighter.cpp
    include/filemanager.h src/filemanager.cpp
    include/spellchecker.h src/spellchecker.cpp
    include/thememanager.h src/thememanager.cpp
    include/themedialog.h src/themedialog.cpp
    include/outlinedelegate.h src/outlinedelegate.cpp
    include/findbarwidget.h src/findbarwidget.cpp
    include/documentoutlinewidget.h src/documentoutlinewidget.cpp
    include/sidebarfileexplorer.h src/sidebarfileexplorer.cpp
    include/toastnotification.h src/toastnotification.cpp
    ${RESOURCE_CPP}
)
# --- END Create executable ---

# Include directories
target_include_directories(scriber PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${HUNSPELL_INCLUDE_DIRS}
)

include(FetchContent)

FetchContent_Declare(
    cmark
    GIT_REPOSITORY https://github.com/commonmark/cmark.git
    GIT_TAG        0.30.2  # Use a specific tag for stability
)

FetchContent_MakeAvailable(cmark)

# --- Link libraries - REMOVE Qt6::Core5Compat ---
target_link_libraries(scriber PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::PrintSupport
    # Qt6::Core5Compat # <-- REMOVED
    ${HUNSPELL_LIBRARIES}
    cmark::cmark
)
# --- END Link libraries ---

# Integrate cmark library (uncomment/comment as needed)
# find_package(cmark REQUIRED)
# target_link_libraries(scriber PRIVATE cmark::cmark)

qt_add_executable(test_arg test_arg.cpp)
target_link_libraries(test_arg PRIVATE Qt6::Core)

qt_add_executable(test_arg_full test_arg_full.cpp)
target_link_libraries(test_arg_full PRIVATE Qt6::Core)

qt_add_executable(test_missing_arg test_missing_arg.cpp)
target_link_libraries(test_missing_arg PRIVATE Qt6::Core)
