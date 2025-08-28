
# See LICENSE for more information about licensing
#  Copyright 2023
#
# Author: Diego Arturo Avila Torres <diego.avila@uned.cr>
#         Luis G. Leon Vega <luis.leon@ieee.org>
#
#

# ---------------------------------------------
# Kria examples
# ---------------------------------------------

executable('matrix-multiplication-kria',
  ['zynq-mpsoc/matrix-multiplication.cpp'],
  include_directories: [projectinc],
  cpp_args : cpp_args,
  dependencies : [project_deps, libcynq_dep]
)




executable('reckon-kria',
  ['zynq-mpsoc/reckon.cpp'],
  include_directories: [projectinc],
  cpp_args : cpp_args,
  dependencies : [project_deps, libcynq_dep]
)



py3 = import('python').find_installation(pure: false)
py_dep = py3.dependency(embed: true)
pybind_dep = dependency('pybind11')

py3.extension_module('reckon_wb',
  ['zynq-mpsoc/reckon_wb.cpp'],
  include_directories: [projectinc],
  cpp_args : [cpp_args,'-std=c++20'],
  dependencies : [project_deps, 
  libcynq_dep, 
  pybind_dep,
  py_dep
  ],
  install: true
)

executable('xfopencv-warp-perspective-kria',
  ['zynq-mpsoc/xfopencv-warp-perspective.cpp'],
  include_directories: [projectinc],
  cpp_args : cpp_args,
  dependencies : [project_deps, libcynq_dep]
)

executable('ad08-sequential-kria',
  ['zynq-mpsoc/ad08-sequential.cpp'],
  include_directories: [projectinc],
  cpp_args : cpp_args,
  dependencies : [project_deps, libcynq_dep]
)

executable('ad08-streams-kria',
  ['zynq-mpsoc/ad08-streams.cpp'],
  include_directories: [projectinc],
  cpp_args : cpp_args,
  dependencies : [project_deps, libcynq_dep]
)

# ---------------------------------------------
# Alveo examples
# ---------------------------------------------

executable('vadd-example-alveo',
  ['alveo/vadd-host.cpp'],
  include_directories: [projectinc],
  cpp_args : cpp_args,
  dependencies : [project_deps, libcynq_dep]
)

# ---------------------------------------------
# Structure examples
# ---------------------------------------------

executable('execution-stream',
  ['structures/execution-stream.cpp'],
  include_directories: [projectinc],
  cpp_args : cpp_args,
  dependencies : [project_deps, libcynq_dep]
)
