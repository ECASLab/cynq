#
# See LICENSE for more information about licensing
#  Copyright 2023
#
# Author: Diego Arturo Avila Torres <dandida95@gmail.com>
#         Luis G. Leon Vega <luis.leon@ieee.org>
#

sources = []

subdir('cynq')

libcynq = static_library('libcynq' ,
  sources, 
  cpp_args : cpp_args,
  install : true,
  include_directories : [projectinc],
  dependencies : [project_deps]
)

libcynq_dep = declare_dependency(link_with: libcynq)
