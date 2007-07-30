
function tr3b_rain_RainGrid128(origin, dir)
	
	shader = cgame.RegisterShader("particles/rain01");
	
	-- spawn particles in a 128x128 grid
	org = vector.New()
	for i = -64, 64, 8 do
		for j = -64, 64, 8 do
		
			org[0] = origin[0] + i
			org[1] = origin[1] + j
			org[2] = origin[2]
		
			if qmath.random() > 0.7 then
				tr3b_rain_SpawnRainParticle(org, shader)
			end
		end
	end
end

function tr3b_rain_SpawnRainParticle(origin, shader)
	
	p = particle.Spawn()

	if not p then
		return
	end
	
	p:SetType(particle.SPARK)
	p:SetShader(shader)
	p:SetDuration(10000)
	p:SetOrigin(origin)
	
	
	vel = vector.Construct(0, 0, -30)
	p:SetVelocity(vel)

	-- add some gravity/randomness
	accel = vector.New()
	accel[0] = 0 --qmath.crandom() * 3
	accel[1] = 0 --qmath.crandom() * 3
	accel[2] = -70 - qmath.random() * 50
	p:SetAcceleration(accel)
	
	-- set color
	p:SetColor(1.0, 1.0, 1.0, 1.0)

	-- size it
	p:SetWidth(5)
	p:SetHeight(0.3)
	
	p:SetEndWidth(10 + qmath.random() * 10)
	p:SetEndHeight(0.4 + qmath.random() * 0.2)
	
end
