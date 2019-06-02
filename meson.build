project('TimeSeries', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

time_series_dep = declare_dependency(include_directories:include_directories('include'))

gtest_dep = dependency('gtest', main : true, fallback : ['gtest', 'gtest_dep'])
benchmark_dep = dependency('benchmark', fallback : ['google-benchmark', 'google_benchmark_dep'])

foreach test:['SimpleScalar','STL_ALGS','GenericTS', 'TimeSerieND']
    exe = executable(test,'tests/'+test+'/main.cpp',
                    dependencies:[gtest_dep, time_series_dep],
                    install: true
                    )
    test(test, exe)
endforeach

foreach benchmark:['benchmarks']
    exe = executable(benchmark,'tests/'+benchmark+'/main.cpp',
                    dependencies:[benchmark_dep, time_series_dep],
                    install: true
                    )
    benchmark(test, exe, timeout: 120)
endforeach

