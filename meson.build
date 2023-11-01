#
# See LICENSE for more information about licensing
#  Copyright 2023
#
# Author: Diego Arturo Avila Torres <dandida95@gmail.com>
#         Luis G. Leon Vega <luis.leon@ieee.org>
#


project('cynq','cpp',version : '0.1.0',
  default_options : ['warning_level=3',
                     'cpp_std=c++17',
                     'cpp_args=-Werror'])

add_global_arguments(
  '-Wno-ignored-qualifiers',
  language : 'cpp'
)

#############################
## Dependencies definition ##
#############################

# options

build_tests = get_option('build_tests')

# directories
projectinc = include_directories('.',
'include')

subdir('examples')

# Testing suite
if build_tests
  gtest_proj = subproject('gtest')
  gtest_dep = gtest_proj.get_variable('gtest_dep')
  gmock_dep = gtest_proj.get_variable('gmock_dep')
  subdir('tests')
endif
