find_package(benchmark REQUIRED)

add_executable(chess_bench benchmark_main.cpp is_king_threatened.cpp bm_threats.cpp)

target_link_libraries(chess_bench PRIVATE wisdom::chess)
target_link_libraries(chess_bench PRIVATE benchmark::benchmark)
