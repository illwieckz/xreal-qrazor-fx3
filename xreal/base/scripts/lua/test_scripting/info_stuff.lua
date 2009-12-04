-- printf wrapper
function et.G_Printf(...)
       et.G_Print(string.format(unpack(arg)))
end

function et_InitGame( levelTime, randomSeed, restart )
    et.G_Print("InitGame of info_stuff.lua\n") -- printout our text to the console
    et.RegisterModname("InfoStuff")
end

hurtCount=0
buttonState=0

function InfoHurt(self, inflictor, attacker)
    hurtCount=hurtCount+1
    et.G_Print("Info Hurt: Self="..self..", Inflictor="..inflictor..", Attacker="..attacker..", Count="..hurtCount.."\n")
    et.G_Printf("g_speed=[%s]\n", et.trap_Cvar_Get("g_speed"))
    test = et.gentity_get (self, "s.origin", 0)
    test[0] = test[0] + 10
    et.gentity_set(self, "s.origin", 0, test)
    et.G_Printf("time(ms)=[%s]\n",et.trap_Milliseconds())
end

function InfoTouch(self, other)
    et.G_Print("Info Touched: Self="..self..", Other="..other.."\n")
    et.G_Printf("g_entity[%d].classname = [%s]\n", self, et.gentity_get( self, "classname" ) )
end

function InfoSpawn(self)
    et.G_Print("Info Spawned: Self="..self.."\n")
end

function InfoFree(self)
    et.G_Print("Info Freed: Self="..self.."\n")
end

function InfoDie(self, inflictor, attacker, dmg, method)
    et.G_Print("Info Die: Self="..self..", Inflictor="..inflictor..", Attacker="..attacker..", Dmg="..dmg..", MOD="..method.."\n")
end

function InfoUse(self, other, activator)
    et.G_Print("Info Use: Self="..self..", Other="..other..", Switch=!".."\n") --..buttonState
    --if (buttonState==0) then
    --    buttonState = 1
    --else
    --    buttonState = 0
    --end
end

function InfoThink(self)
    if (buttonState==1) then
        et.G_Print("Info Think: Self="..self..", State is 1\n")
    end
end

function InfoTrigger(self, other)
    et.G_Print("Info Trigger: Self="..self..", Other="..other.."\n")
end
