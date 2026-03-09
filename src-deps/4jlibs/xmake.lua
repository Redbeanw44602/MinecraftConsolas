if is_plat('windows') then

target('4jlibs')
    set_kind('phony')
    add_deps('xinput')
    add_includedirs('include', {public = true})
    add_linkdirs('lib', {public = true})
    add_links(
        '4J_Render',
        '4J_Storage',
        '4J_Input',
        {public = true}
    )

end
