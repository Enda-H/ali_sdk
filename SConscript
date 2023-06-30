from building import *

cwd  = GetCurrentDir()

src = Split('''
core/aiot_mqtt_api.c
core/aiot_state_api.c

core/sysdep/core_adapter.c
core/sysdep/core_sysdep.c
core/utils/core_auth.c
core/utils/core_diag.c
core/utils/core_global.c
''')

if GetDepend(['ALI_USING_DATA_MODEL']):
    src += Glob('components/data-model/aiot_dm_api.c')

src += Glob('components/dynreg/*.c')
src += Glob('components/dynreg-mqtt/*.c')
src += Glob('components/logpost/*.c')
src += Glob('components/mqtt_upload/*.c')
src += Glob('components/ntp/*.c')

src += Glob('components/ota/*.c')

src += Glob('external/mbedtls/library/*.c')
src += Glob('portfiles/aiot_port/*.c')
CPPPATH = [
cwd + '/core',
cwd + '/core/utils',
cwd + '/core/sysdep',
cwd + '/external/mbedtls/include',

cwd + '/components/dynreg',
cwd + '/components/dynreg-mqtt',
cwd + '/components/data-model',
cwd + '/components/logpost',
cwd + '/components/mqtt_upload',
cwd + '/components/nt',
cwd + '/components/ota',



cwd + '/portfiles/aiot_port',
]

group = DefineGroup('ALISDK', src, depend = ['PKG_USING_ALISDK'], CPPPATH = CPPPATH)

Return('group')
