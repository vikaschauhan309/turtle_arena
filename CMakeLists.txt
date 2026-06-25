cmake_minimum_required(VERSION 3.8)
project(turtle_arena)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(turtlesim REQUIRED)
find_package(std_srvs REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Box2D has no stable apt packaging across Ubuntu releases; vendor a pinned
# release so the physics API and behavior are identical for everyone who
# builds this package, rather than drifting with whatever libbox2d-dev
# happens to be in the distro at build time.
include(FetchContent)
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  box2d
  GIT_REPOSITORY https://github.com/erincatto/box2d.git
  GIT_TAG v2.4.1
)
FetchContent_MakeAvailable(box2d)

include_directories(include)

add_executable(turtle_arena_node
  src/main.cpp
  src/arena_world.cpp
  src/renderer.cpp
  src/robot_entity.cpp
  src/arena_node.cpp
  src/lidar_sensor.cpp
  src/world_loader.cpp
  src/challenge_manager.cpp
)
target_link_libraries(turtle_arena_node
  sfml-graphics sfml-window sfml-system
  box2d
  yaml-cpp
)
ament_target_dependencies(turtle_arena_node
  rclcpp
  geometry_msgs
  nav_msgs
  sensor_msgs
  std_msgs
  std_srvs
  turtlesim
)

install(TARGETS turtle_arena_node
  DESTINATION lib/${PROJECT_NAME}
)
install(DIRECTORY launch worlds
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # Per-file copyright boilerplate isn't this project's convention -- a single
  # LICENSE file at the repo root covers it. ament_cmake_lint_cmake is
  # excluded too: it flags FetchContent_Declare/FetchContent_MakeAvailable as
  # "mixed case commands", but mixed case is the documented modern-CMake
  # convention for these calls, not a style violation.
  set(AMENT_LINT_AUTO_EXCLUDE ament_cmake_copyright ament_cmake_cpplint ament_cmake_lint_cmake)
  ament_lint_auto_find_test_dependencies()

  find_package(ament_cmake_cpplint REQUIRED)
  # build/include_order is also excluded here: cpplint's header-classifier
  # treats third-party libs ending in .h (box2d/box2d.h, yaml-cpp/yaml.h) as
  # C system headers, then flags them for appearing after C++ system headers
  # like <vector> -- a false positive, not an actual ordering problem.
  ament_cpplint(FILTERS "-legal/copyright" "-build/include_order")

  find_package(launch_testing_ament_cmake REQUIRED)
  add_launch_test(test/test_cmd_vel_to_odom.py
    TARGET test_cmd_vel_to_odom
  )
endif()

ament_package()
