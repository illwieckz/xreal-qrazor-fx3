-- printf wrapper
function et.G_Printf(...)
       et.G_Print(string.format(unpack(arg)))
end

function et_InitGame( levelTime, randomSeed, restart )
    et.G_Print("InitGame of team_stuff.lua\n") -- printout our text to the console
    et.RegisterModname("TeamStuff")
end

hurtCount=0
buttonState=0

function TeamHurt(self, inflictor, attacker)
    hurtCount=hurtCount+1
    et.G_Print("Team Hurt: Self="..self..", Inflictor="..inflictor..", Attacker="..attacker..", Count="..hurtCount.."\n")
    et.G_Printf("g_speed=[%s]\n", et.trap_Cvar_Get("g_speed"))
    test = et.gentity_get (self, "s.origin", 0)
    test[0] = test[0] + 10
    et.gentity_set(self, "s.origin", 0, test)
    et.G_Printf("time(ms)=[%s]\n",et.trap_Milliseconds())
end

function TeamTouch(self, other)
    --et.G_Print("Team Touched: Self="..self..", Other="..other.."\n")
    --et.G_Printf("g_entity[%d].classname = [%s]\n", self, et.gentity_get( self, "classname" ) )
end

function TeamSpawn(self)
    et.G_Print("Team Spawned: Self="..self.."\n")
end

function TeamFree(self)
    et.G_Print("Team Freed: Self="..self.."\n")
end

function TeamDie(self, inflictor, attacker, dmg, method)
    et.G_Print("Team Die: Self="..self..", Inflictor="..inflictor..", Attacker="..attacker..", Dmg="..dmg..", MOD="..method.."\n")
end

function TeamUse(self, other, activator)
    et.G_Print("Trigger Use: Self="..self..", Other="..other..", Activator="..activator.."\n")
end

function TeamThink(self)
    if (buttonState==1) then
        et.G_Print("Team Think: Self="..self..", State is 1\n")
    end
end

function TeamTrigger(self, other)
    et.G_Print("Team Trigger: Self="..self..", Other="..other.."\n")
end
