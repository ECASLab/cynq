#
# See LICENSE for more information about licensing
#  Copyright 2023
#
# Author: Luis G. Leon Vega <luis.leon@ieee.org>
#

doxygen = find_program('doxygen', required: true)

# Find plots dependencies
dot = find_program('dot', required: true)
java = find_program('java', required: true)
epstopdf = find_program('epstopdf', required: true)

# Download plant UML
plantumlurl = 'https://github.com/plantuml/plantuml/releases/download/v1.2022.12/plantuml-1.2022.12.jar'
plantumlpath = meson.current_build_dir() + '/plantuml.jar'
run_command('wget', '-O', plantumlpath, plantumlurl)

# Set the environment variable for running the docs
env = environment()
env.set('PLANTUML_JAR_PATH', meson.current_build_dir())


brief = 'Framework to develop FPGA applications in C++ with the easiness of PYNQ'

cdata = configuration_data()
cdata.set('VERSION', meson.project_version())
cdata.set('PROJECT_NAME', meson.project_name().to_upper())
cdata.set('PROJECT_BRIEF', brief)
cdata.set('TOP_SRCDIR', meson.project_source_root())
cdata.set('PLANTUML_JAR_PATH', meson.current_build_dir())
cdata.set('TOP_BUILDDIR', meson.project_build_root())

doxyfile = configure_file(input: 'Doxyfile.in',
                          output: 'Doxyfile',
                          configuration: cdata,
                          install: false)

datadir = join_paths('docs', 'cynq')

html_target = custom_target('cynq-docs',
                            input: doxyfile,
                            output: 'html',
                            command: [doxygen, doxyfile],
                            install: true,
                            install_dir: datadir)
