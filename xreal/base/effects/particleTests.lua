
function TestParticleSpawn(origin)
	-- cgame.Print("testParticleSpawn called")
	
	shader = cgame.RegisterShader("teleportFlare");
	
	-- spawn particles in a small circle
	up = vector.Construct(0, 0, 1)
	forward = vector.Construct(20, 0, 0)
	dst = vector.New()
	vel = vector.New()
	for angle = 0, 360, 30 do
		
		vector.RotatePointAround(dst, up, forward, angle);
		
		-- move into center
		vel[0] = -dst[0] * 0.4
		vel[1] = -dst[1] * 0.4
		vel[2] = -dst[2] * 0.4
		
		-- add circle origin to world origin
		dst = dst + origin
		
		SpawnTeleportParticle(dst, shader, vel)
	end
end

function SpawnTeleportParticle(origin, shader, vel)
	
	p = particle.Spawn()
	
	p:SetType(particle.SMOKE)
	p:SetShader(shader)
	p:SetDuration(1900 + qmath.random() * 100)
	p:SetOrigin(origin)
	
	-- give random velocity
	--randVec = vector.New()
	--randVec[0] = qmath.crandom()		-- between 1 and -1
	--randVec[1] = qmath.crandom()
	--randVec[2] = qmath.crandom()
	--vector.Normalize(randVec)
	--tmpVec = vector.New()
	--vector.Scale(randVec, 16, tmpVec)
	-- tmpVec[2] += 30     -- nudge the particles up a bit
	p:SetVelocity(vel)

	-- add some gravity/randomness
	tmpVec = vector.New()
	tmpVec[0] = 0 --qmath.crandom() * 3
	tmpVec[1] = 0 --qmath.crandom() * 3
	tmpVec[2] = 30
	p:SetAcceleration(tmpVec)
	
	-- don't fade too fast
	p:SetAlpha(1.0)
	p:SetAlphaVelocity(0)

	-- size it
	dim = 3 + qmath.random() * 2
	p:SetWidth(dim)
	p:SetHeight(dim)
	
	dim = dim * 0.2
	p:SetEndWidth(dim)
	p:SetEndHeight(dim)
	
	-- rotate it by some degrees
	p:SetRotation(qmath.rand() % 179)
	
end
