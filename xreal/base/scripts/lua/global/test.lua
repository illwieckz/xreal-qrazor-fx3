function et_RunFrame(levelTime) --called every serverframe
    --et.G_Print("Hello Spam!!!\n") -- printout our text to the console
end --close et_RunFrame()

function et_InitGame( levelTime, randomSeed, restart )
    et.G_Print("InitGame of global test.lua\n") -- printout our text to the console
    et.RegisterModname("GlobalTest")
end

-- function G_LuaStartVM()
    -- 
-- end

function et_ClientBegin(clientNum) --called on ClientBegin
    et.G_Print("\n __ NEW CLIENT !!! ClientBegin="..clientNum.."\n\n") -- printout our text to the console
end --close et_RunFrame()
