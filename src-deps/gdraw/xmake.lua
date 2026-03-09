if is_plat('windows') then

target('gdraw')
    set_kind('static')
    add_files('src/**.cpp')
    add_deps('mss', 'iggy')
    add_includedirs('include', {public = true})

end
