if is_plat('windows') then

target('iggy')
    set_kind('phony')
    add_includedirs('include', {public = true})
    add_linkdirs('lib', {public = true})
    add_links('iggy_w64', {public = true})

end
