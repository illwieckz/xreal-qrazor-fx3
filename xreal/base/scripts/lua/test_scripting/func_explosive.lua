-- printf wrapper
function et.G_Printf(...)
       et.G_Print(string.format(unpack(arg)))
end
--Its not used in this file, but its a handy little funcion 
--you might want to declare at the start of every script.

function et_InitGame( levelTime, randomSeed, restart )
    et.G_Print("InitGame of explosive_stuff.lua\n") -- printout our text to the console
    et.RegisterModname("ExplosiveStuff")
end

hurtCount=0
buttonState=0

--This file shows the variety of hooks, and the parameters
--Parameter names should be kept the same as a convention
--Funcion names should end in the hook type (Hurt/Touch/Spawn/etc) as a convention
--These are not technical requirements, but may make it easier to read, and manage parameters

function FuncExHurt(self, inflictor, attacker)
    hurtCount=hurtCount+1
    et.G_Print("Func Ex Hurt: Self="..self..", Inflictor="..inflictor..", Attacker="..attacker..", Count="..hurtCount.."\n")
end

function FuncExTouch(self, other)
    et.G_Print("Func Ex Touched: Self="..self..", Other="..other.."\n")
end

function FuncExSpawn(self)
    et.G_Print("Func Ex Spawned: Self="..self.."\n")
end

function FuncExFree(self)
    et.G_Print("Func Ex Freed: Self="..self.."\n")
end

function FuncExDie(self, inflictor, attacker, dmg, method)
    et.G_Print("Func Ex Die: Self="..self..", Inflictor="..inflictor..", Attacker="..attacker..", Dmg="..dmg..", MOD="..method.."\n")
end

function FuncExUse(self, other, activator)
    et.G_Print("Func Ex Use: Self="..self..", Other="..other..", Switch=!".."\n") 
end

function FuncExThink(self)
    --et.G_Print("Func Think: Self="..self..", State is 1\n")
end

--Also free the previous entity (just for the fun of it)
function FuncExTrigger(self, other)
    et.G_Print("Func Ex Trigger: Self="..self..", Other="..other.."\n")
    et.G_FreeEntity(self-1)
    et.G_Print("G_Free previous entity = "..(self-1).."\n")
end
