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
end
