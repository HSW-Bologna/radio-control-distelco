import kconfiglib
import os
import multiprocessing
from pathlib import Path


def PhonyTargets(
    target,
    action,
    depends,
    env=None,
):
    # Creates a Phony target
    if not env:
        env = DefaultEnvironment()
    t = env.Alias(target, depends, action)
    env.AlwaysBuild(t)


def getKconfig(kconfig):
    if os.path.isfile('sdkconfig'):
        config = kconfiglib.Kconfig(kconfig)
        config.load_config('sdkconfig')
    else:
        print('Not configured!')
        exit(1)
    return config


def generate_sdkconfig_header(target, source, env):
    content = """
#ifndef SDKCONFIG_H_INCLUDED
#define SDKCONFIG_H_INCLUDED
// Automatically generated file. Do not edit.

    """
    for config in list(
            map(lambda x: getKconfig(str(x)),
                [kconf for kconf in source if 'Kconfig' in str(kconf)])):
        config.load_config('sdkconfig')

        for (key, sym) in [(x, config.syms[x]) for x in config.syms.keys()
                           if x.startswith('LVGL_')]:
            if sym.type == kconfiglib.BOOL and sym.str_value == 'y':
                content += "#define CONFIG_{} 1\n".format(key)
            elif sym.type == kconfiglib.INT:
                content += "#define CONFIG_{} {}\n".format(key, sym.str_value)
            elif sym.type == kconfiglib.STRING:
                content += '#define CONFIG_{} "{}"\n'.format(
                    key, sym.str_value)

    content += "#endif"

    with open(str(target[0]), "w") as f:
        f.write(content)


MINGW = 'mingw' in COMMAND_LINE_TARGETS
PROGRAM = "simulated.exe" if MINGW else "simulated"
MAIN = "main"
COMPONENTS = "components"
LVGL = "{}/lvgl".format(COMPONENTS)
CFLAGS = [
    "-Wall",
    "-Wextra",
    "-Wno-unused-function",
    "-g",
    "-O0",
    "-DLV_CONF_INCLUDE_SIMPLE",
    '-DLV_CONF_SKIP',
    '-DLV_USE_USER_DATA',
    "-DMILLIS_H_INCLUDED",
    "-Wno-unused-parameter",
    "-static-libgcc",
    "-static-libstdc++",
]
LDLIBS = ["-lmingw32", "-lSDL2main", "-lSDL2"] if MINGW else ["-lSDL2"]
SDLPATH = ARGUMENTS.get('sdl', None)
LIBPATH = [os.path.join(SDLPATH, 'lib')] if SDLPATH else []

CPPPATH = [
    COMPONENTS, f"#{LVGL}/src", f"#{LVGL}", f"#{MAIN}",
    f"#{MAIN}/config", f"#{MAIN}/simulator", f"#{COMPONENTS}/gel",
]

if MINGW and SDLPATH:
    CPPPATH.append(os.path.join(SDLPATH, 'include'))
num_cpu = multiprocessing.cpu_count()
SetOption('num_jobs', num_cpu)
print("Running with -j {}".format(GetOption('num_jobs')))

env_options = {
    "ENV": os.environ,
    "CC": ARGUMENTS.get('cc', 'gcc'),
    "ENV": os.environ,
    "CPPPATH": CPPPATH,
    'CPPDEFINES': [],
    "CCFLAGS": CFLAGS,
    "LIBS": LDLIBS,
}

env = Environment(**env_options)

gel_env = env
gel_selected = ['pagemanager', 'collections']
(objects, include) = SConscript(f'{COMPONENTS}/generic_embedded_libs/SConscript',
                                exports=['gel_env', 'gel_selected'])
env['CPPPATH'] += [include]

sdkconfig = env.Command(
    "main/simulator/sdkconfig.h",
    [str(filename) for filename in Path('.').rglob('Kconfig')] + ['sdkconfig'],
    generate_sdkconfig_header)

sources = [str(filename) for filename in Path('main/simulator').rglob('*.c')]
sources += [str(filename) for filename in Path('main/view').rglob('*.c')]
sources += [str(filename) for filename in Path('main/model').rglob('*.c')]
sources += [
    str(filename) for filename in Path(f'{LVGL}/src').rglob('*.c')
]

prog = env.Program(PROGRAM, sources + objects, LIBPATH=LIBPATH)
PhonyTargets('run', './simulated', prog, env)
env.Alias('mingw', prog)
