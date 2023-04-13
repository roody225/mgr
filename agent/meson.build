project(
    'agent',
    'c',
    default_options : ['warning_level=3']
)

src_files = [
    'src/main.c',
    'src/offload.c',
    'src/netlink_handlers.c',
    'src/p4_handlers.c',
]

incdir = include_directories('inc')

libmnl_dep = dependency('libmnl')

executable(
    'agent',
    src_files,
    include_directories : incdir,
    dependencies : [libmnl_dep]
)
