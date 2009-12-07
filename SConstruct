# vim: ft=python expandtab

import os
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
EV_MICRO_VERSION=3
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
env.Alias('install', env.Install('$PREFIX/lib/pkgconfig', 'evince-document-%s.pc' % EV_API_VERSION))
env.Alias('install', env.Install('$PREFIX/lib/pkgconfig', 'evince-view-%s.pc' % EV_API_VERSION))

env.Alias('install', env.Install('$PREFIX/include/evince/' + EV_API_VERSION, ['evince-view.h', 'evince-document.h']))

SConscript(['libdocument/SConscript'], exports="env EV_BINARY_VERSION EV_API_VERSION")
