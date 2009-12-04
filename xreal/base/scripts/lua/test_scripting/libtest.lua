-- printf wrapper
function et.G_Printf(...)
   et.G_Print(string.format(unpack(arg)))
end

function et_InitGame( levelTime, randomSeed, restart )
   et.G_Printf("et_InitGame [%d] [%d] [%d]\n", levelTime, randomSeed, restart )
   et.G_Printf("lua VM id is [%d]\n", et.FindSelf())
   et.RegisterModname("LibTest")
end

function et_IPCReceive(otherVM, message)
    et.G_Print("LibTest received: '"..message.."'\n")
    if (message == "yo dawg...") then
        et.G_Print("LibTest to LibTest2: 'yo dawg, I heard you yo dawg.'\n")
        et.IPCSend(otherVM, "yo dawg, I heard you yo dawg.")
    end
end

linkTestEnt = -1
foundEntLinked = 1

function LibTestLinkerSpawn(self)
    linkTestEnt = self
end

function Test_UnlinkEntity()
    --trap_UnlinkEntity
    --Calls unlinkEntity on a specified entity (fist param)
    --Unlinks from network (invisible to clients)
    et.trap_UnlinkEntity(linkTestEnt)
    et.G_Print("Test_UnlinkEntity; unlinked test ent\n")
end

function Test_LinkEntity()
    --trap_LinkEntity
    --Calls LlinkEntity on a specified entity (fist param)
    --Links to network (visible to clients)
    et.trap_LinkEntity(linkTestEnt)
    et.G_Print("Test_LinkEntity; linked test ent\n")
end

function Test_G_FreeEntity(self)
    --G_FreeEntity
    --Calls free entity on a specified entity (first param)
    et.G_FreeEntity(self)
    et.G_Print("Test_G_FreeEntity; freed entity\n")
end

gravityRising = 0

function Test_Cvar()
    --trap_Cvar_Get
    --Gets the value of a specified cvar (cvar name is first param)
    gravity = (et.trap_Cvar_Get("g_gravity"))
    if(gravityRising==1) then
        gravity = gravity + 100
    else
        gravity = gravity - 100
    end
    
    if(gravity >= 1000) then
        gravityRising = 0
    elseif (gravity <50) then
        gravityRising = 1
    end
    
    et.trap_Cvar_Set("g_gravity", gravity)
    et.G_Print("Test_Cvar; Changed g_gravity to "..gravity.."\n")
end

function Test_G_Damage(self, playerWhoAttackedTrigger)
    --G_Damage
    --Calls G_Damage on an entity (param1). Can be players, or other entitys)
    et.G_Damage(playerWhoAttackedTrigger, self, self, 10, 0, 0) 
    --Second param = inflictor
    --Third param = attacker
    --4th param = damage
    --5th param = damage flags (see gamecode)
    --6th param = method of death (see gamecode)
    et.G_Print("Test_G_Damage; delt 10 dmg to player "..attacker.."\n")    
end

function Test_G_PrintLog(self, attacker)
    --G_LogPrint
    --Prints to the log in the user's base dir games.log
    --(eg on Windows, %appdata%\XreaL\base\games.log )
    eg.G_LogPrint("LOG PRINT TEST. GameTime="..et.trap_Milliseconds().."ms\n")
    et.G_Print("Test_G_LogPrint; printed GameTime="..et.trap_Milliseconds().." to games.log\n")    
end

function Test_G_Sound(self, attacker)
    soundNum = 1
    --G_Sound
    --TODO: work out how to use this.
    --Possible need to set info strings to sound path?
    et.G_Sound(self, soundNum)
    et.G_Print("Test_G_Sound self "..soundNum.."\n")    
end

--GetLuaFunction and SetLuaFunction
--These can be used to change the lua function which is called when a hook is triggered.
function Test_GetLuaFunction(self)
    funcName = et.GetLuaFunction(self, "luaHurt")
    et.G_Print("Test_GetLuaFunction ent="..self..", luaHurt="..funcName.."\n")
end

function Test_SetLuaFunction(self)
    funcName = et.GetLuaFunction(self, "luaHurt")
    if (funcName == "LibTestHurt") then
        funcName = "LibTestDifferentHurt"
    else
        funcName = "LibTestHurt"
    end
    et.SetLuaFunction(self, "luaHurt", funcName)
    et.G_Print("Test_SetLuaFunction ent="..self..", change luaHurt="..funcName.."\n")
end

function LibTestDifferentHurt(self, inflictor, attacker)
    Test_SetLuaFunction(self)
end

--GetEntityByName
--Used to find an entity by it's script name.
function Test_GetEntityByName(self)
    entity = et.GetEntityByName("func_explosive_9")
    if (foundEntLinked == 1) then
        et.trap_UnlinkEntity(entity)
        foundEntLinked = 0
        et.G_Print("Test_GetEntityByName entityfound="..entity.." Unlinked.\n")
    else
        et.trap_LinkEntity(entity)
        foundEntLinked = 1
        et.G_Print("Test_GetEntityByName entityfound="..entity.." Linked.\n")
    end
end

--GameEnd
--Used to end the game.
function Test_GameEnd(self)
    et.G_Print("Test_GameEnd doing its thing...\n")
    game.EndRound()
end

--GetVMsGamePrint
--Tests a few things
function Test_GetVMsGamePrint(self)
    et.G_Print("Test_GameEnd TESTING qmath\n")
    et.G_Printf("random=[%f] sin(45)=[%f]\n", (qmath.rand() % 100), (qmath.sin(45)))

    Test_FindMods()    

    et.G_Print("Test_GameEnd TESTING GAME.PRINT\n")
    game.Print("GAME.PRINT\n")
end

function Test_FindMods()
    vmnum=0
    et.G_Print("Test_FindMods starting\n")
    while (vmnum < 20) do
        modname, signature = et.FindMod(vmnum)
        if(modname ~= nil) then
            et.G_Printf("lua VM [%d] found, [%s]\n", vmnum, modname)
        else
            et.G_Printf("lua VM [%d] is nil\n", vmnum)
            vmnum = 99
        end
        vmnum = vmnum + 1
    end
    et.G_Print("Thats all the mods found.\n")
end

--Again use of parameters means one neat function can wrap all of these tests
--Alternatively each entity would call above functions (parameters would need to be different)
function LibTestHurt(self, inflictor, attacker)
    param = et.GetLuaParam(self,1)
    et.G_Print("Lib Test: "..param.."\n")
    --pc=pc+1
    if(param=="sound") then
        Test_G_Sound(self, attacker)
    elseif (param=="damage") then
        Test_G_Damage(self, attacker)
    elseif (param=="logprint") then
        Test_G_LogPrint(self, attacker)
    elseif (param=="cvar") then
        Test_Cvar()
    elseif (param=="free") then
        Test_G_FreeEntity(self)
    elseif (param=="link") then
        Test_LinkEntity()
    elseif (param=="unlink") then
        Test_UnlinkEntity()
    elseif (param=="getfunc") then
        Test_GetLuaFunction(self)
    elseif (param=="setfunc") then
        Test_SetLuaFunction(self)
    elseif (param=="getentitybyname") then
        Test_GetEntityByName(self)
    elseif (param=="endgame") then
        Test_GameEnd(self)
    elseif (param=="getvmgprint") then
        Test_GetVMsGamePrint(self)
    end
end
