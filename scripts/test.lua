local done = false

local function dump(entity)
	for _, name in ipairs(getComponentsFrom(entity)) do
		local c = getComponent(entity, name)
		for _, field in ipairs(getComponentAttributes(name)) do
			log(name .. "." .. field .. " = " .. tostring(c[field]))
		end
	end
end

function update(entity, dt)
	if done then
		return
	end
	done = true
	dump(entity)
	log("------------------")

	setComponentAttribute(entity, "Name", "name", "Scripted")
	setComponentAttributeInt(entity, "Health", "maxHealth", 200)
	setComponentAttributeStr(entity, "Controllable", "image", "assets/sprites/thanks.png")

	dump(entity)
	log("------------------")

	local spawned = createEntity()
	addComponent(spawned, "Transform", { position = Vector2.new(200, 300) })
	addComponent(spawned, "Health", { currentHealth = 50, maxHealth = 50 })
	addComponent(spawned, "Sprite", { texturePath = "./assets/sprites/ship_1.png", size = Vector2.new(64, 64) })
	local velocity = addComponent(spawned, "Velocity")
	velocity.velocity = Vector2.new(100, 15)
	dump(spawned)
end
