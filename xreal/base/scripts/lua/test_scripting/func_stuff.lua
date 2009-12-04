-- printf wrapper
function et.G_Printf(...)
       et.G_Print(string.format(unpack(arg)))
end

function et_InitGame( levelTime, randomSeed, restart )
    et.G_Print("InitGame of func_stuff.lua\n") -- printout our text to the console
    et.RegisterModname("FuncStuff")
end

hurtCount=0
buttonState=0

function FuncHurt(self, inflictor, attacker)
    hurtCount=hurtCount+1
    et.G_Print("Func Hurt: Self="..self..", Inflictor="..inflictor..", Attacker="..attacker..", Count="..hurtCount.."\n")
    et.G_Printf("g_speed=[%s]\n", et.trap_Cvar_Get("g_speed"))
    test = et.gentity_get (self, "s.origin", 0)
    test[0] = test[0] + 10
    et.gentity_set(self, "s.origin", 0, test) 
    et.G_Printf("time(ms)=[%s]\n",et.trap_Milliseconds())
end

function FuncTouch(self, other)
    et.G_Print("Func Touched: Self="..self..", Other="..other.."\n")
    et.G_Printf("g_entity[%d].classname = [%s]\n", self, et.gentity_get( self, "classname" ) )
end

function FuncSpawn(self)
    et.G_Print("Func Spawned: Self="..self.."\n")
end

function FuncFree(self)
    et.G_Print("Func Freed: Self="..self.."\n")
end

function FuncDie(self, inflictor, attacker, dmg, method)
    et.G_Print("Func Die: Self="..self..", Inflictor="..inflictor..", Attacker="..attacker..", Dmg="..dmg..", MOD="..method.."\n")
end

function FuncUse(self, other, activator)
    et.G_Print("Func Use: Self="..self..", Other="..other..", Switch=!".."\n") --..buttonState
    --if (buttonState==0) then
    --    buttonState = 1
    --else
    --    buttonState = 0
    --end
end

function FuncThink(self)
    if (buttonState==1) then
        et.G_Print("Func Think: Self="..self..", State is 1\n")
    end
end

function FuncTrigger(self, other)
    et.G_Print("Func Trigger: Self="..self..", Other="..other.."\n")
end
