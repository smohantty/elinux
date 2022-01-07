subdir('utils')
subdir('surface')
subdir('window')
subdir('rendering')

source_file = [
   'elinux_view.cc',
]

src_dep = declare_dependency(
                                  include_directories : include_directories('.'),
                                  dependencies : [utils_dep, surface_dep, window_dep, rendering_dep],
                                  sources : source_file
                               )