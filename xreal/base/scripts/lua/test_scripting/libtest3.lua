-- printf wrapper
function et.G_Printf(...)
   et.G_Print(string.format(unpack(arg)))
end

function et_InitGame( levelTime, randomSeed, restart )
   et.G_Printf("et_InitGame [%d] [%d] [%d]\n", levelTime, randomSeed, restart )
   et.G_Printf("lua VM id is [%d]\n", et.FindSelf())
   et.RegisterModname("LibTest3")
end

function test_Spawn(self)
    et.G_Print("et.G_Spawn Test\n")
    e = et.G_Spawn()
    et.G_Print("Spawned Ent "..e.." spawned\n")
    -- THIS CODE WILL CRASH, et.gentity_set will be disabled (for now).
    --et.gentity_set(e, "classname", 0, "func_mover");
    --et.G_Print("Spawned Ent "..e.." set classname\n")
    --et.gentity_set(e, "s.eType", 0, 5);
    --et.G_Print("Spawned Ent "..e.." set s.eType\n")
    --Setting model didn't crash, but it didn't appear (entity had no class)
    --et.gentity_set(e, "model", 0, "*4");
    --et.G_Print("Spawned Ent "..e.." set model\n")
    origin = vector.Construct(-360, -1472, 136)
    mover.SetPosition(e, origin)
    et.G_Print("Spawned Ent "..e.." set origin\n")
    et.G_AddEvent(e, 57, 0)
    et.G_Print("Spawned Ent "..e.." added event\n")
end

function test_TempEnt(self)
    et.G_Print("et.G_TempEnt Test\n")
    origin = vector.Construct(-360, -1344, 100)
    e = et.G_TempEntity(origin, 57)
    fields = {"classname", "activator", "chain", "clipmask", "target", "target_ent", "spawnflags", "clipmask", "health", "s.weapon"}
    for k,v in pairs(fields) do
        val = et.gentity_get(e, v, 0)
        if (val~=nil) then
            et.G_Print(v.." => "..val.."\n")
        else
            et.G_Print(v.." => NULL\n")
        end
    end
end

--Again use of parameters means one neat function can wrap all of these tests
--Alternatively each entity would call above functions (parameters would need to be different)
function LibTest3Hurt(self, inflictor, attacker)
    param = et.GetLuaParam(self,1)
    et.G_Print("Lib Test3: "..param.."\n")
    --pc=pc+1
    if(param=="spawn") then
        test_Spawn(self)
    elseif (param=="tempent") then
        test_TempEnt(self)
    end
end
