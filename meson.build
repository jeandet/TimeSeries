project('TimeSeries', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++17'],
  license : 'GPL3')

time_series_dep = declare_dependency(include_directories:include_directories('include'))

install_headers(['include/TimeSeries.h',
    'include/ts_iterator_value.h',
    'include/ts_iterator_indexes.h',
    'include/ts_iterators.h',
    'include/ts_arithmetic.h',
    'include/ts_time.h'], subdir : 'TimeSeries')

pkg = import('pkgconfig')
pkg.generate(
             subdirs : meson.project_name(),
             version : meson.project_version(),
             name : meson.project_name(),
             filebase : meson.project_name(),
             description : 'A simple C++ time-series library.')

gtest_dep = dependency('gtest', main : true, fallback : ['gtest', 'gtest_dep'])
benchmark_dep = dependency('benchmark', fallback : ['google-benchmark', 'google_benchmark_dep'])

foreach test:['SimpleScalar','STL_ALGS','GenericTS', 'TimeSerieND', 'TSIterators', 'TSIteratorValues']
    exe = executable(test,'tests/'+test+'/main.cpp',
                    dependencies:[gtest_dep, time_series_dep],
                    install: false
                    )
    test(test, exe, args:['--gtest_output=xml:@0@_gtest_output.xml'.format(test)])
endforeach

foreach benchmark:['benchmarks']
    exe = executable(benchmark,'tests/'+benchmark+'/main.cpp',
                    dependencies:[benchmark_dep, time_series_dep],
                    install: false
                    )
    benchmark(test, exe, timeout: 120, args:['--benchmark_format=json','--benchmark_out=ts_benchmark.json'])
endforeach
