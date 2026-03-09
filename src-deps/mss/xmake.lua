if is_plat('windows') then

target('mss')
    set_kind('phony')
    add_includedirs('include', {public = true})
    add_linkdirs('lib', {public = true})
    add_links('mss64', {public = true})

end
