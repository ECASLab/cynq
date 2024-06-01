#
# See LICENSE for more information about licensing
#  Copyright 2023
#
# Author: Diego Arturo Avila Torres <dandida95@gmail.com>
#         Luis G. Leon Vega <luis.leon@ieee.org>
#

wget = find_program('wget', required: true, native: true)
unzip = find_program('unzip', required: true, native: true)

res_link = 'https://www.dropbox.com/scl/fo/9qxbbc2h3cc1vhplpi5nu/h?rlkey=mj03e6xulqu3e046gke3e1bvd&dl=1'
res_output = 'resources.zip'

run_command(wget, '-nc', '-O', res_output, res_link, check: false)
run_command(unzip, res_output, check: false)

fmt = '"@0@/@1@"'

# Default xclbin for Vivado Flow
default_xclbin_location = fmt.format(meson.global_source_root(), files('kria-bitstreams/default.xclbin')[0])
cpp_args += [
  '-DEXAMPLE_KRIA_DEFAULT_XCLBIN_LOCATION=' + default_xclbin_location
]

# Examples for Kria - Ubuntu 22.04
matrix_bitstream_location = fmt.format(meson.global_source_root(), files('kria-bitstreams/matrix-multiply/multiply-stream.bit')[0])
xfopencv_warp_perspective_bitstream_location = fmt.format(meson.global_source_root(), files('kria-bitstreams/xfopencv-warp-perspective/warp.bit')[0])
xfopencv_filter2d_bitstream_location = fmt.format(meson.global_source_root(), files('kria-bitstreams/xfopencv-filter2d/filter2d.bit')[0])
ad08_location = fmt.format(meson.global_source_root(), files('kria-bitstreams/ad08/ad08.bit')[0])

cpp_args += [
  '-DEXAMPLE_MULTIPLICATION_BITSTREAM_LOCATION=' + matrix_bitstream_location,
  '-DXFOPENCV_WARP_PERSPECTIVE_BITSTREAM_LOCATION=' + xfopencv_warp_perspective_bitstream_location,
  '-DXFOPENCV_FILTER2D_BITSTREAM_LOCATION=' + xfopencv_filter2d_bitstream_location,
  '-DAD08_BITSTREAM_LOCATION=' + ad08_location,
  '-DEXAMPLE_KRIA_ACCEL_ADDR=0xa0000000',
  '-DEXAMPLE_KRIA_DMA_ADDR=0xa0010000',
]

# Examples for Alveo U250 - xilinx_u250_gen3x16_xdma_4_1_202210_1
vadd_xclbin_location = fmt.format(meson.global_source_root(), files('alveo-xclbin/vadd/vadd.xclbin')[0])
cpp_args += [
  '-DEXAMPLE_ALVEO_VADD_XCLBIN_LOCATION=' + vadd_xclbin_location,
]
