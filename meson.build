project('elinux', ['c','cpp'],
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_ELINUX']

subdir('inc')
subdir('src')

egl_dep = dependency('egl')
wayland_dep = [dependency('wayland-client'), dependency('wayland-egl'),  dependency('wayland-cursor') ]

elinux_lib_dep = [src_dep, egl_dep, wayland_dep]
inc = [include_directories('inc')]

elinux_lib = shared_library('elinux',
  include_directories   : inc,
  dependencies : elinux_lib_dep,
  install : true,
  cpp_args : lib_args,
  gnu_symbol_visibility : 'hidden',
)

subdir('example')

# Make this library usable as a Meson subproject.
elinux_dep = declare_dependency(
  include_directories: include_directories('.'),
  link_with : elinux_lib)

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name : 'elinux',
  filebase : 'elinux',
  description : 'Meson sample project.',
  subdirs : 'elinux',
  libraries : elinux_lib,
  version : '0.1',
)
