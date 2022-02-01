find_package(benchmark REQUIRED)

add_executable(chess_bench benchmark_main.cpp)

target_link_libraries(chess_bench PRIVATE chess_lib)
target_link_libraries(chess_bench PRIVATE benchmark::benchmark)
