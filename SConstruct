# vim: ft=python expandtab

import os
import re
from site_init import *
from xml.etree.ElementTree import Element, SubElement, XMLTreeBuilder, tostring
from xml.dom import minidom
from uuid import uuid4

opts = Variables()
opts.Add(PathVariable('PREFIX', 'Installation prefix', os.path.expanduser('~/FOSS'), PathVariable.PathIsDirCreate))
opts.Add(BoolVariable('DEBUG', 'Build with Debugging information', 0))
opts.Add(PathVariable('PERL', 'Path to the executable perl', r'C:\Perl\bin\perl.exe', PathVariable.PathIsFile))

env = Environment(variables = opts,
                  ENV=os.environ, tools = ['default', GBuilder])
Initialize(env)
env.Append(CPPDEFINES = "HAVE_CONFIG_H")
EV_MAJOR_VERSION=2
EV_MINOR_VERSION=30
EV_MICRO_VERSION=0
EV_VERSION_STRING="%d.%d.%d" % (EV_MAJOR_VERSION, EV_MINOR_VERSION, EV_MICRO_VERSION)
EV_API_VERSION="2.30"
EV_BINARY_VERSION="2"
env['DOT_IN_SUBS'] = {'@VERSION@': EV_VERSION_STRING,
                      '@EV_MAJOR_VERSION@': str(EV_MAJOR_VERSION),
                      '@EV_MINOR_VERSION@': str(EV_MINOR_VERSION),
                      '@EV_MICRO_VERSION@': str(EV_MICRO_VERSION),
                      '@EV_API_VERSION@': EV_API_VERSION,
                      '@EV_BINARY_VERSION@': EV_BINARY_VERSION,
                      '@GLIB_REQUIRED@': "2.18",
                      '@GTK_REQUIRED@': "2.20",
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

dev_root = Element("Wix", xmlns='http://schemas.microsoft.com/wix/2006/wi')
dev_m = SubElement(dev_root, 'Module', Id='EnvinceDev', Language='1033', Version=EV_VERSION_STRING)
dev_p = SubElement(dev_m, 'Package', Id=str(uuid4()).upper(), Description='envince devlopement package',
                Comments='This is a windows installer for Evince devlopment files',
                Manufacturer='Gnome4Win', InstallerVersion='200')
dev_target = SubElement(dev_m, 'Directory', Id='TARGETDIR', Name='SourceDir')

run_root = Element("Wix", xmlns='http://schemas.microsoft.com/wix/2006/wi')
run_m = SubElement(run_root, 'Module', Id='EvinceRun', Language='1033', Version=EV_VERSION_STRING)
run_p = SubElement(run_m, 'Package', Id=str(uuid4()).upper(), Description='Evince runtime package',
                Comments='This is a windows installer for Evince runtime files',
                Manufacturer='Gnome4Win', InstallerVersion='200')
run_target = SubElement(run_m, 'Directory', Id='TARGETDIR', Name='SourceDir')

env.Alias('install', env.Install('$PREFIX/lib/pkgconfig', 'evince-document-%s.pc' % EV_API_VERSION))
env.Alias('install', env.Install('$PREFIX/lib/pkgconfig', 'evince-view-%s.pc' % EV_API_VERSION))
dev_lib = SubElement(dev_target, 'Directory', Id='lib', Name='lib')
dev_pkgconfig = SubElement(dev_lib, 'Directory', Id='pkgconfig', Name='pkgconfig')
dev_cpkgconfig = SubElement(dev_pkgconfig, "Component", Id='pkgconfigs', Guid=str(uuid4()).upper())
FileElement(dev_cpkgconfig, ['evince-document-%s.pc' % EV_API_VERSION, 'evince-view-%s.pc' % EV_API_VERSION], 'lib/pkgconfig', env)

env.Alias('install', env.Install('$PREFIX/include/evince/' + EV_API_VERSION, ['evince-view.h', 'evince-document.h']))
dev_dinclude = SubElement(dev_target, 'Directory', Id='include', Name='include')
dev_devince = SubElement(dev_dinclude, 'Directory', Id='evince', Name='evince')
dev_dapi = SubElement(dev_devince, 'Directory', Id='_' + EV_API_VERSION, Name=EV_API_VERSION)
dev_cheader = SubElement(dev_dapi, "Component", Id='headers', Guid=str(uuid4()).upper())
FileElement(dev_cheader, ['evince-view.h', 'evince-document.h'], 'include/evince/' + EV_API_VERSION, env)

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
            #'cut-n-paste/evinfobar/SConscript',
            'cut-n-paste/smclient/SConscript',
            'cut-n-paste/gimpcellrenderertoggle/SConscript',
            'cut-n-paste/toolbar-editor/SConscript',
            'cut-n-paste/totem-screensaver/SConscript',
            'cut-n-paste/zoom-control/SConscript',
            'shell/SConscript',
            'data/SConscript'],
           exports="env EV_BINARY_VERSION EV_API_VERSION gen_def dev_target run_target")
'''
target = 'Bar' + env['MSVSSOLUTIONSUFFIX'],
                 projects = ['bar' + env['MSVSPROJECTSUFFIX']],
                 variant = 'Release'
env.MSVSSolution(target = 'evince', projects = ['evince'], variant = 'Debug')
'''
env_dev = env.Clone(XML=dev_root)
env_dev.Command('evincedev.wxs', [], generate_wxs)

env_run = env.Clone(XML=run_root)
env_run.Command('evincerun.wxs', [], generate_wxs)

env.Depends(['evincerun.wxs', 'evincedev.wxs'], 
            ['SConstruct', 
             'libdocument/SConscript',
             'libview/SConscript',
             'backend/pdf/SConscript',
             'shell/SConscript',
             'data/SConscript'])
env.Alias('install', env.Install('$PREFIX/wxs', ['evincerun.wxs', 'evincedev.wxs']))
