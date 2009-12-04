-- printf wrapper
function et.G_Printf(...)
   et.G_Print(string.format(unpack(arg)))
end

lastHit=-1

function et_InitGame( levelTime, randomSeed, restart )
   et.G_Printf("et_InitGame [%d] [%d] [%d]\n", levelTime, randomSeed, restart )
   et.G_Printf("lua VM id is [%d]\n", et.FindSelf())
   et.RegisterModname("LibTest2")
end

function vector_Test1(self)
    et.G_Print("vector_Test1\n")
    
    forward = vector.Construct(300.0, 0.0, 400.0)
    len = (vector.Length(forward))
    et.G_Printf("new vector={%f %f %f}, length=%f\n", forward[0], forward[1], forward[2], len)
    forward[0] = 100
    forward[1] = 0
    forward[2] = 200
    et.G_Print("Edited new vector.\n")
    len = (vector.Length(forward))
    et.G_Printf("new vector={%f %f %f}, length=%f\n", forward[0], forward[1], forward[2], len)
end

function game_Functions(self)
    game.Print("Testing game.Leveltime\n")
    game.Print("game.Print: level time is "..game.Leveltime().."\n")
end

function entity_Functions(self)
    et.G_Print("entity_Functions Test\n")
    fields = {"classname", "activator", "chain", "clipmask", "target", "target_ent", "spawnflags", "clipmask", "health", }
    for k,v in pairs(fields) do
        val = et.gentity_get(self, v, 0)
        if(val~=nil) then
            et.G_Print(v.." => "..val.."\n")
        else
            et.G_Print(v.." => NULL\n")
        end
    end
end

function lua_IPC(self)
    et.G_Print("Testing lua IPC. Global test should fail, LibTest should work.\n")
    otherVM = et.FindModname("GlobalTest")
    et.G_Printf("GlobalTest is VMnumber=%d\n", otherVM)
    et.G_Print("LibTest2 to GlobalTest: 'yo dawg' ")
    result = et.IPCSend(otherVM, "yo dawg...")
    if (result==1) then
        et.G_Print("send sucessful\n")
    else
        --Global test does not have a et_IPCRecieve(vmnum, message) defined.
        --Therefore it cannot be called and IPC returns 0, failure.
        et.G_Print("send failed\n")
    end
    
    otherVM = et.FindModname("LibTest")
    et.G_Printf("LibTest is VMnumber=%d\n", otherVM)
    et.G_Print("LibTest2 to LibTest: 'yo dawg'\n")
    result = et.IPCSend(otherVM, "yo dawg...")
    if (result==1) then
        et.G_Print("send sucessful\n")
    else
        et.G_Print("send failed\n")
    end
end

function entity_FindNumber(self)
    et.G_Print("entity_FindNumber Test\n")
    ent = entity.FindNumber(self)
    et.G_Print("after entity.FindNumber\n")
    name = ent:GetClassName()
    et.G_Print("entity found, classname was "..name.."\n")
end

function game_SetConfigString(self)
    et.G_Print("game_SetConfigString Test\n")
    et.trap_SetConfigstring(666, "WOOT")
    et.trap_SetConfigstring(667, "LOL")
    et.trap_SetConfigstring(668, ":D")
end

function game_GetConfigString(self)
    et.G_Print("game_GetConfigString Test\n")
    et.G_Print("666="..et.trap_GetConfigstring(666).." \n")
    et.G_Print("667="..et.trap_GetConfigstring(667).." \n")
    et.G_Print("668="..et.trap_GetConfigstring(668).." \n")
    et.G_Print("5="..et.trap_GetConfigstring(5).." \n")
    et.G_Print("1="..et.trap_GetConfigstring(1).." \n")
end

function game_SendConsole(self, attacker)    
    --et.G_Print("G_Say Test\n")
    -- -1 = all clients, 0=mode?
    --et.G_Say(-1, 0, "BLEH G_SAY\n")
    et.G_Print("game_GetUserInfo Test\n")
    userinfo = et.trap_GetUserinfo(attacker)
    et.G_Print(userinfo.."\n")
    oldname = et.Info_ValueForKey(userinfo,"name")
    userinfo = et.Info_SetValueForKey(userinfo, "name", (oldname.."_WITH_MORE!"))
    et.trap_SetUserinfo(attacker, userinfo)
    -- after setting the user info, you need to do ClientUserinfochanged for clients to use update
    et.G_Print("et.ClientUserinfoChanged Test\n")
    et.ClientUserinfoChanged(attacker)
end

function et_test1(self, attacker)
    et.G_Print("game_SendServer Test\n")
    -- -1 = all clients
    et.trap_SendServerCommand(attacker, "cp \"HELLOOOOOO UPDATED server command\n\"")
end

function et_test2(self, attacker)
    et.G_Print("G_Sound Test\n")
    sidx = et.G_SoundIndex("sound/movers/doors/dr1_strt.ogg")
    et.G_Print("Playing sound index="..sidx.."\n")
    et.G_Sound(self, sidx)
    midx = et.G_ModelIndex("models/mapobjects/fabz0r/gates/stargate.ase")
    et.G_Print("Found model index="..midx.."\n")
end

function et_test3(self)
    et.G_Print("game_Info_ValueForKey Test\n")
    cstr = et.trap_GetConfigstring(1)
    et.G_Print(et.Info_ValueForKey(cstr,"g_gravity").."\n")
    
    et.G_Print("game_Info_RemoveKey Test\n")
    cstr = et.trap_GetConfigstring(1)
    et.G_Print(cstr.."\n")
    cstr2 = et.Info_RemoveKey(cstr,"g_gravity")
    et.G_Print(cstr2.."\n")
end

function et_test4(self)
    et.G_Print("game_Info_SetValueForKey Test\n")
    cstr = et.trap_GetConfigstring(1)
    et.G_Print(cstr2.."\n")
    cstr2 = et.Info_SetValueForKey(cstr,"lolwut", "tehValue")
    et.G_Print(cstr2.."\n")
end

function et_test5(self, attacker)
    --et.trap_DropClient(attacker, "test", 0)
end

function et_test6(self)
    et.G_Print("et.Q_CleanStr Test\n")
    str = "^1fsd^2fds^^^^sdfs^3sfd"
    et.G_Print(str.."\n")
    et.G_Print(et.Q_CleanStr(str).."\n")
    et.G_Print("et.trap_Argc Test\n")
    et.G_Print(et.trap_Argc().."\n")
    et.G_Print("et.trap_Argv Test\n")
    et.G_Print(et.trap_Argv(0).."\n")
end

-- hook /moomoo
function et_ConsoleCommand(cmd)
    if(cmd == "moomoo") then
        et.G_Print("MOOMOO et.trap_Argc Test\n")
        narg = et.trap_Argc()
        et.G_Print(narg.."\n")
        et.G_Print("MOOMOO et.trap_Argv Test\n")
        ai = 0
        while (ai < narg) do
            et.G_Print(ai..")"..et.trap_Argv(ai).."\n")
            et.G_Print("concat,"..ai..")"..et.ConcatArgs(ai).."\n")
            ai = ai + 1
        end
        et.G_Print("MOOMOO et.ConcatArgs Test\n")
        return 1
    else
        return 0
    end
end

function et_IPCReceive(otherVM, message)
    et.G_Print("LibTest2 received: '"..message.."'\n")
    if (message == "yo dawg, I heard you yo dawg.") then
        et.G_Print("LibTest2 to LibTest: '... I heard YOU yo dawg.'\n")
        et.IPCSend(otherVM, "... I heard YOU yo dawg.")
    end
end

function DamageLast()
    if (lastHit>0) then
        hp=et.gentity_get(lastHit, "health", 0)
        et.gentity_set(lastHit, "health", hp-200)
    end
end

--Again use of parameters means one neat function can wrap all of these tests
--Alternatively each entity would call above functions (parameters would need to be different)
function LibTest2Hurt(self, inflictor, attacker)
    param = et.GetLuaParam(self,1)
    et.G_Print("Lib Test: "..param.." on ent="..self.."\n")
    --pc=pc+1
    if(param=="vec1") then
        vector_Test1(self)
    elseif (param=="game") then
        game_Functions(self)
    elseif (param=="entityGet") then
        entity_Functions(self)
    elseif (param=="luaIPC") then
        lua_IPC(self)
    elseif (param=="entityFindNum") then
        entity_FindNumber(self)
    elseif (param=="setConfigString") then
        game_SetConfigString(self)
    elseif (param=="getConfigString") then
        game_GetConfigString(self)
    elseif (param=="sendconsole") then
        game_SendConsole(self, attacker)
    elseif (param=="test1") then
        et_test1(self, attacker)
    elseif (param=="test2") then
        et_test2(self, attacker)
    elseif (param=="test3") then
        et_test3(self)
    elseif (param=="test4") then
        et_test4(self)
    elseif (param=="test5") then
        et_test5(self, attacker)
    elseif (param=="test6") then
        et_test6(self)
    end
    DamageLast()
    lastHit=self
end
