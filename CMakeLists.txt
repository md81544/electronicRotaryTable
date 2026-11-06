cmake_minimum_required(VERSION 3.20)

set(APP_NAME ert)
project(${APP_NAME} CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Release)
endif()

# FAKE mocks out pigpio and the stepper, so the app runs on a PC. The
# Makefile turns it on for Debug
option(FAKE "Mock the GPIO hardware" OFF)

find_package(Curses REQUIRED)
find_package(Threads REQUIRED)

# GNU readline. On macOS the system "readline" is really libedit, which lacks
# parts of the GNU API, so prefer Homebrew's (keg-only) GNU readline
if(APPLE)
	execute_process(COMMAND brew --prefix readline
		OUTPUT_VARIABLE READLINE_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
endif()
find_path(READLINE_INCLUDE_DIR readline/readline.h HINTS ${READLINE_PREFIX}/include)
find_library(READLINE_LIBRARY readline HINTS ${READLINE_PREFIX}/lib)
if(NOT READLINE_INCLUDE_DIR OR NOT READLINE_LIBRARY)
	message(FATAL_ERROR "GNU readline not found (apt install libreadline-dev, "
		"or brew install readline)")
endif()

# ---------------------------------------------------------------------------
# stepperControl library, fetched at configure time. To build against a local
# checkout instead (e.g. when working on both), use
#   make STEPPERCONTROL_DIR=../stepperControl
# which sets FETCHCONTENT_SOURCE_DIR_STEPPERCONTROL
# ---------------------------------------------------------------------------
include(FetchContent)
set(STEPPERCONTROL_GIT_REPOSITORY "https://github.com/md81544/stepperControl.git"
	CACHE STRING "Git repository to fetch stepperControl from")
FetchContent_Declare(stepperControl
	GIT_REPOSITORY ${STEPPERCONTROL_GIT_REPOSITORY}
	GIT_TAG        v1.0.0
	GIT_SHALLOW    TRUE
)
# FAKE builds use MockGpio, so don't need pigpio
if(FAKE)
	set(STEPPERCONTROL_USE_PIGPIO OFF)
else()
	set(STEPPERCONTROL_USE_PIGPIO ON)
endif()
FetchContent_MakeAvailable(stepperControl)

# ---------------------------------------------------------------------------
# The app
# ---------------------------------------------------------------------------
add_executable(${APP_NAME}
	configreader.cpp
	curses.cpp
	input.cpp
	log.cpp
	main.cpp
)
target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra -Werror -Wpedantic)
if(FAKE)
	target_compile_definitions(${APP_NAME} PRIVATE FAKE)
endif()
target_include_directories(${APP_NAME} PRIVATE ${CURSES_INCLUDE_DIRS} ${READLINE_INCLUDE_DIR})
target_link_libraries(${APP_NAME} PRIVATE
	stepperControl::stepperControl
	${CURSES_LIBRARIES}
	${READLINE_LIBRARY}
	Threads::Threads
)

# ---------------------------------------------------------------------------
# Symlinks in the source root to the last-built executable (it loads ert.cfg
# from the current directory, so is run from here) and to
# compile_commands.json, for clangd and VS Code
# ---------------------------------------------------------------------------
add_custom_command(
	TARGET ${APP_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE:${APP_NAME}>
		${CMAKE_SOURCE_DIR}/${APP_NAME}
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/compile_commands.json
		${CMAKE_SOURCE_DIR}/compile_commands.json
	COMMENT "Updating symlinks: ${APP_NAME}, compile_commands.json"
)
