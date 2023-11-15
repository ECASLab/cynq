#
# See LICENSE for more information about licensing
#  Copyright 2023
#
# Author: Diego Arturo Avila Torres <dandida95@gmail.com>
#         Luis G. Leon Vega <luis.leon@ieee.org>
#

pynq_c_sources = [
  files('pynq_api.c')
]

libpynqc = static_library('libpynqc' ,
  pynq_c_sources, 
  dependencies : [project_deps]
)

libpynqc_dep = declare_dependency(link_with: libpynqc)
project_deps += libpynqc_dep
