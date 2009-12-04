-- printf wrapper
function et.G_Printf(...)
       et.G_Print(string.format(unpack(arg)))
end

function et_InitGame( levelTime, randomSeed, restart )
    et.G_Print("InitGame of trigger_stuff.lua\n") -- printout our text to the console
    et.RegisterModname("TriggerStuff")
end

hurtCount=0
buttonState=0

function TriggerHurt(self, inflictor, attacker)
    hurtCount=hurtCount+1
    et.G_Print("Trigger Hurt: Self="..self..", Inflictor="..inflictor..", Attacker="..attacker..", Count="..hurtCount.."\n")
    et.G_Printf("g_speed=[%s]\n", et.trap_Cvar_Get("g_speed"))
    test = et.gentity_get (self, "s.origin", 0)
    test[0] = test[0] + 10
    et.gentity_set(self, "s.origin", 0, test)
    et.G_Printf("time(ms)=[%s]\n",et.trap_Milliseconds())
end

function TriggerTouch(self, other)
    --et.G_Print("Trigger Touched: Self="..self..", Other="..other.."\n")
    --et.G_Printf("g_entity[%d].classname = [%s]\n", self, et.gentity_get( self, "classname" ) )
end

function TriggerSpawn(self)
    et.G_Print("Trigger Spawned: Self="..self.."\n")
end

function TriggerFree(self)
    et.G_Print("Trigger Freed: Self="..self.."\n")
end

function TriggerDie(self, inflictor, attacker, dmg, method)
    et.G_Print("Trigger Die: Self="..self..", Inflictor="..inflictor..", Attacker="..attacker..", Dmg="..dmg..", MOD="..method.."\n")
end

function TriggerUse(self, other, activator)
    et.G_Print("Trigger Use: Self="..self..", Other="..other..", Activator="..activator.."\n")
end

function TriggerThink(self)
    if (buttonState==1) then
        et.G_Print("Trigger Think: Self="..self..", State is 1\n")
    end
end

function TriggerTrigger(self, other)
    et.G_Print("Trigger Trigger: Self="..self..", Other="..other.."\n")
end
