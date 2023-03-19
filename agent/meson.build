project(
    'agent',
    'cpp',
    default_options : ['warning_level=3', 'cpp_std=c++17']
)

src_files = [
    'src/main.cc',
]

incdir = include_directories('inc')

libmnl_dep = dependency('libmnl')

executable(
    'agent',
    src_files,
    include_directories : incdir,
    dependencies : [libmnl_dep]
)
