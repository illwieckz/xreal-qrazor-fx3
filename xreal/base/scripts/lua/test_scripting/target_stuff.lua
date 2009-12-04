-- printf wrapper
function et.G_Printf(...)
       et.G_Print(string.format(unpack(arg)))
end

function et_InitGame( levelTime, randomSeed, restart )
    et.G_Print("InitGame of target_stuff.lua\n") -- printout our text to the console
    et.RegisterModname("TargetStuff")
end

hurtCount=0
buttonState=0

function target_usestuff(self, inflictor, attacker)
    entity = et.GetEntityByName("target_kill_1")
    et.G_Print("RunUse, target_kill: target_kill="..entity.." attacker="..attacker.."\n")
    et.RunUse(entity, inflictor, attacker)
end

function TargetHurt(self, inflictor, attacker)
    hurtCount=hurtCount+1
    et.G_Print("Target Hurt: Self="..self..", Inflictor="..inflictor..", Attacker="..attacker..", Count="..hurtCount.."\n")
    et.G_Printf("g_speed=[%s]\n", et.trap_Cvar_Get("g_speed"))
    test = et.gentity_get (self, "s.origin", 0)
    test[0] = test[0] + 10
    et.gentity_set(self, "s.origin", 0, test)
    et.G_Printf("time(ms)=[%s]\n",et.trap_Milliseconds())
end

function TargetTouch(self, other)
    et.G_Print("Target Touched: Self="..self..", Other="..other.."\n")
    et.G_Printf("g_entity[%d].classname = [%s]\n", self, et.gentity_get( self, "classname" ) )
end

function TargetSpawn(self)
    et.G_Print("Target Spawned: Self="..self.."\n")
end

function TargetFree(self)
    et.G_Print("Target Freed: Self="..self.."\n")
end

function TargetDie(self, inflictor, attacker, dmg, method)
    et.G_Print("Target Die: Self="..self..", Inflictor="..inflictor..", Attacker="..attacker..", Dmg="..dmg..", MOD="..method.."\n")
end

function TargetUse(self, other, activator)
    et.G_Print("Target Use: Self="..self..", Other="..other..", Activator="..activator.."\n") --..buttonState
    --if (buttonState==0) then
    --    buttonState = 1
    --else
    --    buttonState = 0
    --end
end

function TargetThink(self)
    if (buttonState==1) then
        et.G_Print("Target Think: Self="..self..", State is 1\n")
    end
end

function TargetTrigger(self, other)
    et.G_Print("Target Trigger: Self="..self..", Other="..other.."\n")
end
