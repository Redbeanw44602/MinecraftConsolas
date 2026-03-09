if is_plat('windows') then

target('xinput')
    set_kind('phony')
    add_linkdirs('lib', {public = true})
    add_links('xinput', {public = true})

end
