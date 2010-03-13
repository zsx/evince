# vim: ft=python expandtab

import os
import re
from site_init import GBuilder, Initialize

opts = Variables()
opts.Add(PathVariable('PREFIX', 'Installation prefix', os.path.expanduser('~/FOSS'), PathVariable.PathIsDirCreate))
opts.Add(BoolVariable('DEBUG', 'Build with Debugging information', 0))
opts.Add(PathVariable('PERL', 'Path to the executable perl', r'C:\Perl\bin\perl.exe', PathVariable.PathIsFile))

env = Environment(variables = opts,
                  ENV=os.environ, tools = ['default', GBuilder])
Initialize(env)
env.Append(CPPDEFINES = "HAVE_CONFIG_H")
EV_MAJOR_VERSION=2
EV_MINOR_VERSION=29
EV_MICRO_VERSION=92
EV_VERSION_STRING="%d.%d.%d" % (EV_MAJOR_VERSION, EV_MINOR_VERSION, EV_MICRO_VERSION)
EV_API_VERSION="2.29"
EV_BINARY_VERSION="2"
env['DOT_IN_SUBS'] = {'@VERSION@': EV_VERSION_STRING,
                      '@EV_MAJOR_VERSION@': str(EV_MAJOR_VERSION),
                      '@EV_MINOR_VERSION@': str(EV_MINOR_VERSION),
                      '@EV_MICRO_VERSION@': str(EV_MICRO_VERSION),
                      '@EV_API_VERSION@': EV_API_VERSION,
                      '@EV_BINARY_VERSION@': EV_BINARY_VERSION,
                      '@GLIB_REQUIRED@': "2.18",
                      '@GTK_REQUIRED@': "2.14",
                      '@prefix@': env['PREFIX'],
                      '@exec_prefix@': '${prefix}/bin',
                      '@libdir@': '${prefix}/lib',
                      '@includedir@': '${prefix}/include'}
env.DotIn('config.h', 'config.h.win32.in')
env.DotIn('libdocument/ev-version.h', 'libdocument/ev-version.h.in')
env.DotIn('evince-document-%s.pc' % EV_API_VERSION, 'evince-document.pc.in')
env.DotIn('evince-view-%s.pc' % EV_API_VERSION, 'evince-view.pc.in')
env.Depends(['config.h',
             'libdocument/ev-version.h'
             ], 'SConstruct')
env.Alias('install', env.Install('$PREFIX/lib/pkgconfig', 'evince-document-%s.pc' % EV_API_VERSION))
env.Alias('install', env.Install('$PREFIX/lib/pkgconfig', 'evince-view-%s.pc' % EV_API_VERSION))

env.Alias('install', env.Install('$PREFIX/include/evince/' + EV_API_VERSION, ['evince-view.h', 'evince-document.h']))

def gen_def(target, source, env):
    t = open(str(target[0]), 'w')
    t.write("EXPORTS\n")
    syms = set()
    if 'DEF_ADDONS' in env:
        for i in env['DEF_ADDONS']:
            syms.add(i)
    fun_regex = re.compile(r'\b(ev_\w+)\s*\(')
    for i in source:
        s = open(str(i), 'r')
        for x in s.readlines():
            mo = fun_regex.search(x)
            if mo:
                if 'BLACKLIST' not in env or mo.group(1) not in env['BLACKLIST']:
                    syms.add(mo.group(1) + '\n')
        s.close()
    syms_list = list(syms)
    syms_list.sort()
    t.writelines(syms_list)
    t.close()

SConscript(['libdocument/SConscript',
            'libview/SConscript',
            'libmisc/SConscript',
            'backend/pdf/SConscript',
            'properties/SConscript',
            'cut-n-paste/evinfobar/SConscript',
            'cut-n-paste/smclient/SConscript',
            'cut-n-paste/gimpcellrenderertoggle/SConscript',
            'cut-n-paste/toolbar-editor/SConscript',
            'cut-n-paste/totem-screensaver/SConscript',
            'cut-n-paste/zoom-control/SConscript',
            'shell/SConscript',
            'data/SConscript'],
           exports="env EV_BINARY_VERSION EV_API_VERSION gen_def")
'''
target = 'Bar' + env['MSVSSOLUTIONSUFFIX'],
                 projects = ['bar' + env['MSVSPROJECTSUFFIX']],
                 variant = 'Release'
env.MSVSSolution(target = 'evince', projects = ['evince'], variant = 'Debug')
'''
