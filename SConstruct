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

        for (key, sym) in [(x, config.syms[x]) for x in config.syms.keys()]:
            if sym.type == kconfiglib.BOOL and sym.str_value == 'y':
                content += "#define CONFIG_{} 1\n".format(key)
            elif sym.type == kconfiglib.INT:
                content += "#define CONFIG_{} {}\n".format(key, sym.str_value)
            elif sym.type == kconfiglib.STRING:
                if key in ['LV_USER_DATA_FREE', 'LV_MEM_CUSTOM_FREE', 'LV_MEM_CUSTOM_ALLOC', 'LV_TICK_CUSTOM_SYS_TIME_EXPR']:
                    value = sym.str_value.replace('"', '')
                else:
                    value = f'"{sym.str_value}"'
                content += '#define CONFIG_{} {}\n'.format(key, value)

    content += "#endif"

    with open(str(target[0]), "w") as f:
        f.write(content)


MINGW = 'mingw' in COMMAND_LINE_TARGETS
PROGRAM = "simulated.exe" if MINGW else "simulated"
MAIN = "main"
COMPONENTS = "components"
LVGL = "{}/lvgl".format(COMPONENTS)
FREERTOS = 'main/simulator/FreeRTOS/FreeRTOS'

CFLAGS = [
    "-Wall",
    "-Wextra",
    "-Wno-unused-function",
    "-g",
    "-O0",
    "-DLV_CONF_INCLUDE_SIMPLE",
    '-DESP_PLATFORM',
    '-DprojCOVERAGE_TEST=1',
    "-Wno-unused-parameter",
    "-static-libgcc",
    "-static-libstdc++",
]
LDLIBS = ["-lmingw32", "-lSDL2main",
          "-lSDL2"] if MINGW else ["-lSDL2"] + ['-lpthread']
SDLPATH = ARGUMENTS.get('sdl', None)
LIBPATH = [os.path.join(SDLPATH, 'lib')] if SDLPATH else []

FREERTOSPATH = [
    f'{FREERTOS}/Source/include',
    f'{FREERTOS}/Source/portable/ThirdParty/GCC/Posix',
    f'{FREERTOS}/Source/portable/ThirdParty/GCC/Posix/utils',
    f'{FREERTOS}/Demo/Common/include',
]

CPPPATH = [
    COMPONENTS, f"{LVGL}/src", f"{LVGL}", f'{MAIN}/simulator/port', f'#{MAIN}',
    f"#{MAIN}/config", f"{MAIN}/simulator",
] + FREERTOSPATH


FREERTOSSRC = Glob(f'{FREERTOS}/Source/*.c') + [File(f) for f in [
    # Memory manager (use malloc() / free() )
    f'{FREERTOS}/Source/portable/MemMang/heap_3.c',
    # posix port
    f'{FREERTOS}/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c',
    f'{FREERTOS}/Source/portable/ThirdParty/GCC/Posix/port.c',
    # Demo library.
    f'{FREERTOS}/Demo/Common/Minimal/AbortDelay.c',
    f'{FREERTOS}/Demo/Common/Minimal/BlockQ.c',
    f'{FREERTOS}/Demo/Common/Minimal/blocktim.c',
    f'{FREERTOS}/Demo/Common/Minimal/countsem.c',
    f'{FREERTOS}/Demo/Common/Minimal/death.c',
    f'{FREERTOS}/Demo/Common/Minimal/dynamic.c',
    f'{FREERTOS}/Demo/Common/Minimal/EventGroupsDemo.c',
    f'{FREERTOS}/Demo/Common/Minimal/flop.c',
    f'{FREERTOS}/Demo/Common/Minimal/GenQTest.c',
    f'{FREERTOS}/Demo/Common/Minimal/integer.c',
    f'{FREERTOS}/Demo/Common/Minimal/IntSemTest.c',
    f'{FREERTOS}/Demo/Common/Minimal/MessageBufferAMP.c',
    f'{FREERTOS}/Demo/Common/Minimal/MessageBufferDemo.c',
    f'{FREERTOS}/Demo/Common/Minimal/PollQ.c',
    f'{FREERTOS}/Demo/Common/Minimal/QPeek.c',
    f'{FREERTOS}/Demo/Common/Minimal/QueueOverwrite.c',
    f'{FREERTOS}/Demo/Common/Minimal/QueueSet.c',
    f'{FREERTOS}/Demo/Common/Minimal/QueueSetPolling.c',
    f'{FREERTOS}/Demo/Common/Minimal/recmutex.c',
    f'{FREERTOS}/Demo/Common/Minimal/semtest.c',
    f'{FREERTOS}/Demo/Common/Minimal/StaticAllocation.c',
    f'{FREERTOS}/Demo/Common/Minimal/StreamBufferDemo.c',
    f'{FREERTOS}/Demo/Common/Minimal/StreamBufferInterrupt.c',
    f'{FREERTOS}/Demo/Common/Minimal/TaskNotify.c',
    f'{FREERTOS}/Demo/Common/Minimal/TimerDemo.c',
]]


def main():
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
        [str(filename) for filename in Path(
            'components').rglob('Kconfig')] + ['sdkconfig'],
        generate_sdkconfig_header)

    sources = Glob('main/simulator/*.c')
    sources += Glob('main/simulator/indev/*.c')
    sources += Glob('main/simulator/display/*.c')
    sources += Glob('main/simulator/port/*.c')
    sources += [File(filename) for filename in Path('main/view').rglob('*.c')]
    sources += [File(filename)
                for filename in Path('main/network').rglob('*.c')]
    sources += [File(filename) for filename in Path('main/model').rglob('*.c')]
    sources += [
        File(filename) for filename in Path(f'{LVGL}/src').rglob('*.c')
    ]
    sources += FREERTOSSRC

    prog = env.Program(PROGRAM, sources + objects, LIBPATH=LIBPATH)
    PhonyTargets('run', './simulated', prog, env)
    env.Alias('mingw', prog)


main()
