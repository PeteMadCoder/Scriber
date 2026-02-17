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
    ${RESOURCE_CPP}
)
# --- END Create executable ---

# Include directories
target_include_directories(scriber PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${HUNSPELL_INCLUDE_DIRS}
)

# --- Link libraries - REMOVE Qt6::Core5Compat ---
target_link_libraries(scriber PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::PrintSupport
    # Qt6::Core5Compat # <-- REMOVED
    ${HUNSPELL_LIBRARIES}
)
# --- END Link libraries ---

# Integrate cmark library (uncomment/comment as needed)
# find_package(cmark REQUIRED)
# target_link_libraries(scriber PRIVATE cmark::cmark)
