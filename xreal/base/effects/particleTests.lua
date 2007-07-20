
function TestParticleSpawn(origin)
	-- cgame.Print("testParticleSpawn called")
	
	shader = cgame.RegisterShader("teleportFlare");
	
	p = particle.Spawn()
	
	p:SetType(particle.SMOKE)
	p:SetShader(shader)
	p:SetDuration(700 + qmath.random() * 500)
	p:SetOrigin(origin)
	
	-- give random velocity
	randVec = vector.New()
	randVec[0] = qmath.crandom()		-- between 1 and -1
	randVec[1] = qmath.crandom()
	randVec[2] = qmath.crandom()
	vector.Normalize(randVec)
	
	tmpVec = vector.New()
	vector.Scale(randVec, 64, tmpVec)
	-- tmpVec[2] += 30     -- nudge the particles up a bit
	p:SetVelocity(tmpVec)

	-- add some gravity/randomness
	tmpVec[0] = qmath.crandom() * 3
	tmpVec[1] = qmath.crandom() * 3
	tmpVec[2] = 70
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
