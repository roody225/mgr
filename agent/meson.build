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
    'src/nikss_helper.c',
]

cc = meson.get_compiler('c')

incdir = include_directories('inc')

libmnl_dep = dependency('libmnl')
nikss_dep = cc.find_library('libnikss', required: true)

executable(
    'agent',
    src_files,
    include_directories : incdir,
    dependencies : [libmnl_dep, nikss_dep]
)
