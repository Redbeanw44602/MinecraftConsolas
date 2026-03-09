add_rules('mode.debug', 'mode.release')

add_requires('zlib v1.3.2')

-- options

option('debug-menu')
    set_default(true)
    set_showmenu(true)
    set_description('Enable in-game debug menu feature.')
option_end()

option('large-world')
    set_default(true)
    set_showmenu(true)
    set_description('Enable large world feature.')
option_end()

-- global settings

set_allowedplats('windows')
set_allowedarchs('windows|x64')

set_runtimes('MT')

set_warnings('none')   -- TODO: Use 'all', 'extra'
set_languages('c++23')

if is_config('debug-menu', true) then
    add_defines('MINECRAFT_DEBUG_MENU')
end

if is_config('large-world', true) then
    add_defines('MINECRAFT_LARGE_WORLD')
end

if is_mode('debug') then
    -- never defines _DEBUG; we do not use the debug CRT (under Windows).
    -- FIXME: app instance (app.DebugPrintf) has broken.
    -- add_defines('MINECRAFT_DEBUG')
end

if is_plat('windows') then
    add_defines('_CRT_SECURE_NO_WARNINGS', '_CRT_NON_CONFORMING_SWPRINTFS', 'NOMINMAX')
    add_defines('_WINDOWS64')
end

-- targets

target('client')
    set_kind('binary')
    set_basename('minecraft')

    add_files('src/**.cpp')
    add_includedirs('src')
    add_packages(
        'zlib',
        'libsdl3'
    )

    if is_plat('windows') then
        add_deps('4jlibs', 'gdraw', 'iggy', 'mss')
        add_syslinks('user32', 'd3d11')
    end

-- subdir (deps)

includes('src-deps/*')
