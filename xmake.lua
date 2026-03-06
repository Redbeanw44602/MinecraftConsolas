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

-- targets (lib)

local deps = 'src/deps'

if is_plat('windows') then

target('4jlibs')
    set_kind('phony')
    add_includedirs(deps .. '/4jlibs/include', {public = true})
    add_linkdirs(deps .. '/4jlibs/lib', {public = true})
    add_links(
        '4J_Render',
        '4J_Storage',
        '4J_Input',
        {public = true}
    )

target('gdraw')
    set_kind('static')
    add_files(deps .. '/gdraw/src/**.cpp')
    add_deps('mss', 'iggy')
    add_includedirs(deps .. '/gdraw/include', {public = true})

target('mss')
    set_kind('phony')
    add_includedirs(deps .. '/mss/include', {public = true})
    add_links(deps .. '/mss/lib/mss64.lib', {public = true})

target('iggy')
    set_kind('phony')
    add_includedirs(deps .. '/iggy/include', {public = true})
    add_links(deps .. '/iggy/lib/iggy_w64.lib', {public = true})

target('xinput')
    set_kind('phony')
    add_links(deps .. '/xinput/lib/xinput.lib', {public = true})

end

-- targets (binary)

target('client')
    set_kind('binary')
    set_basename('minecraft')

    add_files('src/**.cpp')
    add_packages('zlib')
    add_includedirs('src')

    remove_files(deps .. '/**')

    if is_plat('windows') then
        add_deps('4jlibs', 'gdraw', 'iggy', 'mss', 'xinput')
        add_syslinks('user32', 'd3d11')
    end
