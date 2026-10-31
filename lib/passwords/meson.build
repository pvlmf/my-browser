libephypasswords_sources = [
  'ephy-password-export.c',
  'ephy-password-import.c',
  'ephy-password-manager.c',
  'ephy-password-record.c'
]

libephypasswords_deps = [
  ephymisc_dep,
  glib_dep,
  libsecret_dep,
  nettle_dep,
]

libephypasswords_includes = include_directories(
  '.',
  '..',
  '../history',
  '../..'
)

libephypasswords = shared_library('ephypasswords',
  libephypasswords_sources,
  dependencies: libephypasswords_deps,
  include_directories: libephypasswords_includes,
  install: true,
  install_dir: pkglibdir,
  install_rpath: pkglibdir
)

ephypasswords_dep = declare_dependency(
  link_with: libephypasswords,
  dependencies: libephypasswords_deps,
  include_directories: libephypasswords_includes
)
