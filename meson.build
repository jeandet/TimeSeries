project('TimeSeries', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

time_series_dep = declare_dependency(include_directories:include_directories('include'))

gtest_dep = dependency('gtest', main : true, fallback : ['gtest', 'gtest_dep'])

SimpleScalar = executable('SimpleScalar','tests/SimpleScalar/main.cpp', 
			dependencies:[gtest_dep, time_series_dep],
			install: true
			)
test('SimpleScalar test', SimpleScalar)