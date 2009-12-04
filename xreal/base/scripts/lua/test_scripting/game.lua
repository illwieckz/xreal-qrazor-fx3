--ETPro test
-- called when game inits
function et_InitGame( levelTime, randomSeed, restart )
       et.G_Printf("et_InitGame [%d] [%d] [%d]\n", levelTime, randomSeed, restart )
       et.G_Printf("lua VM id is [%d]\n", et.FindSelf())
       --et.RegisterModname( "bani qagame " .. et.FindSelf() )
--     test_lua_functions()
end

--ETPro test
-- called every server frame
function et_RunFrame( levelTime )
       if math.mod( levelTime, 10000 ) == 0 then
             et.G_Printf("et_RunFrame [%d]\n", levelTime )
       end
end
