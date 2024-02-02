#
# See LICENSE for more information about licensing
#  Copyright 2023
#
# Author: Diego Arturo Avila Torres <dandida95@gmail.com>
#         Luis G. Leon Vega <luis.leon@ieee.org>
#


project('cynq',['c','cpp'],version : '0.2.0',
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
build_tests = get_option('build-tests')

# directories
projectinc = [include_directories('.', 'include')]

# cflags
cpp_args = []
c_args = []

# dependencies
project_deps = []

# Install pre-commit hooks
developer_mode = get_option('developer-mode')
if developer_mode
  pre_commit = find_program('pre-commit', required: true, native: true)
  run_command(pre_commit, 'install', check: false)
  run_command(pre_commit, 'install', '--hook-type', 'commit-msg', check: false)
endif

if not get_option('build-docs-only')
  # Imports pkgconfig module
  pkgconfig = import('pkgconfig')

  subdir('third-party')
  subdir('include/cynq')
  subdir('src')
  subdir('examples')

  pkgconfig_install_dir = join_paths(get_option('libdir'), 'pkgconfig')
  pkgconfig.generate(libcynq, requires: ['xrt'])
endif

if get_option('build-docs') or get_option('build-docs-only')
  subdir('docs')
endif

# Testing suite
if build_tests
  gtest_proj = subproject('gtest')
  gtest_dep = gtest_proj.get_variable('gtest_dep')
  gmock_dep = gtest_proj.get_variable('gmock_dep')
  subdir('tests')
endif
