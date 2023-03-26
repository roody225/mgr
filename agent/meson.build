project(
    'agent',
    'c',
    default_options : ['warning_level=3']
)

src_files = [
    'src/main.c',
    'src/offload.c',
]

incdir = include_directories('inc')

libmnl_dep = dependency('libmnl')
threads_dep = dependency('threads')

executable(
    'agent',
    src_files,
    include_directories : incdir,
    dependencies : [libmnl_dep, threads_dep]
)
