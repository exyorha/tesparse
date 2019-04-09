add_executable(tesparse-cli
  nlohmann/json.hpp
  CLI11.hpp
  main.cpp
)

target_link_libraries(tesparse-cli PRIVATE tesparse)
target_include_directories(tesparse-cli PRIVATE .)
