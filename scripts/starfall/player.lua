-- Player behavior: arena boundaries, banking, invulnerability and contacts.
-- Reacquire components each callback; only scalar state persists here.
local invulnerable, previousHealth = 0, 3
local flash, flashTime = "", 0

local function explode(entity)
    local position = getComponent(entity, "Transform")
    if not position then return end
    for _, id in ipairs(getEntities()) do
        local name = getComponent(id, "Name")
        if name and name.name == "Modele_Explosion_Joueur" then
            if not getComponent(id, "Animator") then return end
            local explosion = duplicateEntity(id)
            if not explosion then return end
            local label = getComponent(explosion, "Name")
            local transform = getComponent(explosion, "Transform")
            local animator = getComponent(explosion, "Animator")
            if not label or not transform or not animator then destroyEntity(explosion); return end
            label.name = "Explosion_Joueur"
            transform.position = Vector2.new(position.position.x, position.position.y)
            animator.currentAnimation = "explosion"
            animator.currentFrame, animator.elapsed, animator.playing = 0, 0, true
            return
        end
    end
end

local function playSound(asset, pitch)
    for _, id in ipairs(getEntities()) do
        local name = getComponent(id, "Name")
        if name and name.name == "Modele_Effet_Sonore" then
            if not getComponent(id, "Music") then return end
            local sound = duplicateEntity(id)
            if not sound then return end
            local label = getComponent(sound, "Name")
            local music = getComponent(sound, "Music")
            if not label or not music then destroyEntity(sound); return end
            label.name = "Effet_Sonore_" .. asset
            music.path = "assets/sounds/Assets_Sound_Fx_" .. asset .. ".wav"
            music.volume, music.pitch, music.loop, music.state = 75, pitch or 1, false, MusicState.Playing
            return
        end
    end
end

function update(entity, dt)
    local health = getComponent(entity, "Health")
    local controls = getComponent(entity, "Controllable")
    local transform = getComponent(entity, "Transform")
    local velocity = getComponent(entity, "Velocity")
    dt = math.max(0, math.min(dt, 0.05))
    if health then
        if health.currentHealth > previousHealth then invulnerable = 0 end
        previousHealth = health.currentHealth
    end
    invulnerable = math.max(0, invulnerable - dt)
    flashTime = math.max(0, flashTime - dt)
    local animator = getComponent(entity, "Animator")
    if animator then
        local horizontal = velocity and velocity.velocity.x or 0
        local animation = horizontal < 0 and "moveup" or (horizontal > 0 and "movedown" or "moveforward")
        -- Use the existing tinted sheets, with the same 0.08 s flash as OrByte.
        if flashTime > 0 and math.floor((0.48 - flashTime) / 0.08) % 2 == 0 then
            animation = animation .. flash
        end
        if animator.currentAnimation:match("_stage_2$") then animation = animation .. "_stage_2" end
        if animator.currentAnimation ~= animation then
            animator.currentAnimation = animation
            animator.currentFrame, animator.elapsed, animator.playing = 0, 0, true
        end
    end
    if transform then
        transform.position.x = math.max(80, math.min(1840, transform.position.x))
        transform.position.y = math.max(180, math.min(980, transform.position.y))
        if velocity then
            if (transform.position.x <= 80 and velocity.velocity.x < 0) or
               (transform.position.x >= 1840 and velocity.velocity.x > 0) then velocity.velocity.x = 0 end
            if (transform.position.y <= 180 and velocity.velocity.y < 0) or
               (transform.position.y >= 980 and velocity.velocity.y > 0) then velocity.velocity.y = 0 end
        end
    end
    if velocity and ((health and health.currentHealth <= 0) or (controls and controls.speed == 0)) then
        velocity.velocity.x, velocity.velocity.y = 0, 0
    end
end


function onCollisionEnter(entity, other)
    local name = getComponent(other, "Name")
    local health = getComponent(entity, "Health")
    local controls = getComponent(entity, "Controllable")
    if not name or not health or health.currentHealth <= 0 or (controls and controls.speed == 0) then return end
    if name.name:match("^Danger_") then
        if invulnerable <= 0 then
            destroyEntity(other)
            health.currentHealth = math.max(0, health.currentHealth - 1)
            playSound(health.currentHealth == 0 and "explosion" or "hit")
            if health.currentHealth == 0 then explode(entity) end
            invulnerable = 1.4
            flash, flashTime = "_red", 0.48
            log("STARFALL | Impact | " .. health.currentHealth .. "/3 vies")
        end
    elseif name.name:match("^Bonus_Soin_") and health.currentHealth < health.maxHealth then
        health.currentHealth = math.min(health.maxHealth, health.currentHealth + 1)
        destroyEntity(other)
        playSound("hoverButton", 1.5)
        flash, flashTime = "_blue", 0.48
        log("STARFALL | Soin | " .. health.currentHealth .. "/3 vies")
    end
end
