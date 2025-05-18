# José Miguel Rodríguez Marchena (josemirm)

cmake_minimum_required(VERSION 3.4)
project(ctoml 	VERSION     0.1.0
				DESCRIPTION "CTOML"
				LANGUAGES   C
)

# Configure basic build settings
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
#set(CMAKE_BUILD_TYPE Release)

# Set the source files.
set(CTOML_SRC src/booleans.c src/proc_stments.c src/strings.c src/utils.c src/toml.c)

# Show more warnings and analyze the code looking for common mistakes
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /analyze")
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra -fanalyzer")
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra")
endif()

# Set the library
add_library(ctoml STATIC ${CTOML_SRC})
target_include_directories(ctoml PUBLIC src)

# Set the tests
include(CTest)
enable_testing()

# add_executable(typesTester tests/types_tester.c)
# target_link_libraries(typesTester ctoml)
# add_test(NAME typesTester
# 		 COMMAND $<TARGET_FILE:typesTester>
# )

add_executable(ctomlTest tests/tester.c)
target_link_libraries(ctomlTest ctoml)
add_test(NAME ctomlTest
		 COMMAND $<TARGET_FILE:ctomlTest>
)
