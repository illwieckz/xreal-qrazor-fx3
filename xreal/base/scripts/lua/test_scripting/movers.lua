-- printf wrapper
function et.G_Printf(...)
       et.G_Print(string.format(unpack(arg)))
end
--See et_InitGame function below for what this does, 
--its pretty much like printf in C (with different syntax).

--Global variables
--These variables are visible to all the script in this file.
--This is because this file runs in it's own lua VM.
--These variables are not visible to other lua VMs (eg, other files).
--There is (or will be) IPC functionality for inter lua VM communication.
moverState=0

plat=-1

--Tables (aka Arrays)
--See lua reference for more info on all the lua stuff
-- http://lua-users.org/wiki/TutorialDirectory
pathCorners = {} 
pathCornersLoop = {[1]=-1, [2]=-1, [3]=-1}

pc=3

haltOnThree = 1

--LOLOL, shows variable names not important for functionality here, only code quality
moverStopIndicatorLOLOL = -1

-- built in function name called by the init game trigger.
function et_InitGame( levelTime, randomSeed, restart )
   et.G_Printf("et_InitGame [%d] [%d] [%d]\n", levelTime, randomSeed, restart )
   et.G_Printf("lua VM id is [%d]\n", et.FindSelf())
   et.G_Print("Initilization of mover\n")
end

--Custom lua functions can have any name, 
--but there should probably be a convention to end them in the hook type (ie, Spawn/Hurt/Trigger/Die/etc)
--The number of parameters must match the hook type
--The parameter names should probably be consistent with the hook type
function moversPlatSpawn(self)
    plat=self
end

function moverStopIndicatorSpawn(self)
    moverStopIndicatorLOLOL = self
end

function moverPathCornerSpawn(self)
    et.G_Print("Mover PC Spawn pathcorners["..et.GetLuaParam(self,1).."]="..self.."\n")
    pathCorners[et.GetLuaParam(self,1)] = self
end

function moverLoopSpawn(self)
    --By using parameters, one function can be reused by multiple entities.
    --There are a maximum of 4 parameters per entity.
    --If you need more you need more than 4 you could probably 
    --use one to reference a table in the lua scope or something.
    param = et.GetLuaParam(self,1)
    --In some cases lua may require explicit casting
    if (tonumber(param)==1) then
        circleStart = self
    end
    et.G_Print("Mover Loop Spawn pathcorners["..param.."]="..self.."\n")
    pathCornersLoop[et.GetLuaParam(self,1)] = self
end

function movers1Hurt(self, inflictor, attacker)
    param = et.GetLuaParam(self,1)
    path = pathCorners[param]
    
    et.G_Print("Movers 1 Hurt: Self="..self..", Path="..path.." ")
    
    angles =  et.gentity_get(plat, "s.pos.trBase", 0)
    et.G_Print("s.pos.trBase="..angles[0]..","..angles[1]..","..angles[2].." ")
    
    angles =  et.gentity_get(plat, "r.currentAngles", 0)
    et.G_Print("Angles="..angles[0]..","..angles[1]..","..angles[2].." ")
    
    origin =  et.gentity_get(plat, "r.currentOrigin", 0)
    et.G_Print("Origin="..origin[0]..","..origin[1]..","..origin[2].."\n")
        
    mover.AsTrain(plat, path, 150)
    mover.ToAngles(plat, (90*param), 0, 0, 20)
    et.G_Print("Movers 1 Hurt (after angles): Self="..self..", Path="..path.."\n")
end

--This function is defined in this file for use from within this file
--Not all functions need be called from a trigger. 
--Calling functions from other lua code, as usual, works fine too.
function haltPlatform()
    --MoveHalt
    --Halts all movement of a given entity (first param)
    mover.Halt(plat)
    --note the plat variable is declared in this file, outside functions.
    --it is visible to all functions in this file.
    et.G_Print("HALT!\n")
end

function haltPlatformAngles()
    --MoveHalt
    --Halts all angular movement of a given entity (first param)
    mover.HaltAngles(plat)
    et.G_Print("HALT ANGLES!\n")
end

function entity_Functions(self)
    et.G_Print("entity_Functions Test\n")
    fields = {"classname", "activator", "target", "target_ent", "spawnflags", "health", "nextTrain", "prevTrain"}
    for k,v in pairs(fields) do
        val = et.gentity_get(self, v, 0)
        if(val~=nil) then
            et.G_Print(v.." => "..val.."\n")
        else
            et.G_Print(v.." => NULL\n")
        end
    end
    et.GetEntityByName(et.gentity_get(self, "target", 0))
end

function moverLoopTrigger(self, other)
    pcid=et.GetLuaParam(self,1)
    et.G_Print("Loop PathCorner "..pcid.."\n")
    if (tonumber(pcid)==3) then
        if(haltOnThree==1) then
            entity_Functions(self)
            haltPlatform()
        end
    end
end

function moverCircle(self, inflictor, attacker)
    et.G_Print("Path Circle\n")
    --SetPosition
    --Sets the position of the entity (snap to the position instantly)
    --New origin can be specified by 1 vector parameter or 3 numeric parameters
    newOrg = vector.Construct(896, -1088, 80)
    et.G_Print("Vector created newOrg="..newOrg[0]..","..newOrg[1]..","..newOrg[2].."\n")
    mover.SetPosition(plat, newOrg)
    et.G_Print("SetPosition to vector")
    --The same as:
    --mover.SetPosition(plat, 896, -1088, 80)
    
    --MoveAsTrain
    --Moves the entity (first param) to the path corner (second param) at a given speed (third param).
    --If the target parth corner has a target, then the entity will follow the path of that second target.
    mover.AsTrain(plat, circleStart, 150)
end

function moverStop(self, inflictor, attacker)
    haltPlatform()
end

function moverStopAngles(self, inflictor, attacker)
    haltPlatformAngles()
end

function moverToggleStop(self, inflictor, attacker)
    if(haltOnThree==1) then
        haltOnThree=0
        --MoveToPosition
        --Moves an entity (first param) to specific coordinates (2nd, 3rd, 4th = x, y, z).
        --Entity moves instantly.
        mover.ToPosition(moverStopIndicatorLOLOL, 1024, -960, -16, 40)
        et.G_Print("Loop will continue at Loop PathCorner 3.\n")
    else
        haltOnThree=1
        mover.ToPosition(moverStopIndicatorLOLOL, 1024, -960, 16, 40)
        et.G_Print("Loop will halt at Loop PathCorner 3.\n")
    end
end
