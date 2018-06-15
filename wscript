#!/usr/bin/env python


def depends(ctx):
    ctx('pyhmf', 'pyhmf')
    ctx('euter')
    ctx('ester', 'pyester')
    if ctx.options.with_ester:
        # w/ communication between layers
        ctx('lib-rcf', branch='v2')
    else:
        # directly linked to marocco (and leave out rcf stuff too)
        ctx('marocco')
        ctx('marocco', 'pymarocco')


def options(opt):
    opt.load('compiler_cxx')
    opt.load('gtest')
    opt.load('boost')
    opt.load('python')

    hopts = opt.add_option_group('Ester Options')
    hopts.add_withoption('ester', default=False,
                   help='Enable/Disable use of RCF xor link ester/marocco directly into pyhmf')


def configure(cfg):
    cfg.load('compiler_cxx')
    cfg.load('gtest')
    cfg.load('boost')
    cfg.load('python')

    cfg.check_boost(
        lib='serialization system thread program_options filesystem',
        uselib_store='BOOST4ESTER')
    cfg.check_boost(lib='python', uselib_store='BOOST4EXAMPLE')


def build(bld):
    cxxflags = [
        '-Wno-literal-suffix', # squelch warnings from openMPI's mpi.h
        '-ftemplate-depth=800'
    ]

    common_defines = list()

    bld(
        target          = 'ester_inc',
        export_includes = '.',
    )

    if bld.options.with_ester:
        # in case of ester, we build the real thing (with RFC-based and MPI stuff)
        bld(
            target          = 'ester-client',
            features        = 'cxx cxxshlib',
            source          = 'experiment.cpp jobmgrif.cpp',
            use             = 'BOOST4ESTER ester_inc euter rcf-boost-only pyester_obj'.split(),
            install_path    = 'lib',
            export_includes = '.',
            defines         = common_defines + ['__ESTER_SERVER_MODE__'],
            cxxflags        = cxxflags
        )
    else:
        # in case of non-ester, we build something that links to marocco directly
        common_defines.append('__ESTER_BREACHED__')
        bld(
            target          = 'ester-client',
            features        = 'cxx cxxshlib',
            source          = 'experiment.cpp',
            use             = 'BOOST4ESTER marocco ester_inc pyester_obj'.split(),
            install_path    = 'lib',
            export_includes = '.',
            defines         = common_defines,
            cxxflags        = cxxflags
        )

    bld(
        target          = 'ester',
        features        = 'cxx cxxprogram',
        source          = [
            'main.cpp',
            'job.cpp',
            'jobmgr.cpp',
        ],
        use             = 'BOOST4ESTER ester-client euter rcf-boost-only'.split(),
        install_path    = 'bin',
        export_includes = '.',
        defines         = common_defines,
        cxxflags        = cxxflags
    )

    import os
    bld(
        target          = 'example',
        features        = 'cxx cxxprogram',
        source          = 'examples/example.cpp',
        use             = ['BOOST4EXAMPLE', 'pyhmf_impl', 'euter'],
        defines         = common_defines + ['__ESTER_SERVER_MODE__'],
        cxxflags        = cxxflags,
        rpath           = os.path.abspath('lib'), # WTF
        install_path    = '${PREFIX}/bin',
    )

    bld (target          = 'ester-submit',
         features        = 'cxx cxxshlib',
         source          =  bld.path.ant_glob('submit.cpp'),
         use = [
            'euter',
            'ester_inc',
            'ester-client',
            'boost_serialization',
            ],
         defines         = common_defines,
         cxxflags        = cxxflags,
         install_path    = '${PREFIX}/lib',
    )
