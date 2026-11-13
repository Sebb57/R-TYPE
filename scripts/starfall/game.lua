-- Arena controller attached to Camera_Arene_Fixe: rounds, waves, pickups and HUD.
-- All entity ids may be retained; component references are reacquired per callback.
local DURATION, INTRO, RESTART, TRANSITION = 30, 3, 6, 3
local DIFFICULTIES = {
    { name = "Facile", counts = {2, 2, 3}, delays = {1.5, 1.35, 1.2}, speeds = {160, 185, 210}, healDelay = 12, limit = 24 },
    { name = "Normal", counts = {2, 3, 4}, delays = {1.05, 0.9, 0.8}, speeds = {205, 235, 265}, healDelay = 14, limit = 36 },
    { name = "Difficile", counts = {3, 4, 5}, delays = {0.8, 0.7, 0.6}, speeds = {245, 280, 315}, healDelay = 16, limit = 48 },
}
local difficulty, selecting, lastDirection = 1, true, 0
local player, hud, projectileTemplate, healTemplate
local difficultyTemplate, transitionTemplate, soundTemplate, menu, menuSource
local stage, startingStage = 1, 1
local campaign = false
local actors = {}
local elapsed, intro, cooldown, healClock = 0, INTRO, 0, 0
local outcome, restartClock, wave, sequence = nil, 0, 0, 0
local randomState = math.random(1, 2147483646)
local shotSound = 1

-- Disabled components and missing entities both return nil; never retain references.
local function component(id, name)
    if id then return getComponent(id, name) end
end

local function playSound(asset)
    if not component(soundTemplate, "Music") then return end
    local id = duplicateEntity(soundTemplate)
    if not id then return end
    local name, music = component(id, "Name"), component(id, "Music")
    if not name or not music then destroyEntity(id); return end
    name.name = "Effet_Sonore_" .. asset
    music.path = "assets/sounds/Assets_Sound_Fx_" .. asset .. ".wav"
    music.volume, music.pitch, music.loop, music.state = 75, 1, false, MusicState.Playing
end

local function setPlayerSpeed(speed)
    local controls = component(player, "Controllable")
    if controls then controls.speed = speed end
    local velocity = component(player, "Velocity")
    if velocity and speed == 0 then velocity.velocity = Vector2.new(0, 0) end
end

local function random()
    randomState = (randomState * 48271) % 2147483647
    return randomState / 2147483647
end

local function clearActors()
    for _, id in ipairs(actors) do destroyEntity(id) end
    actors = {}
end

local function hideMenu()
    if menu then destroyEntity(menu) end
    menu, menuSource = nil, nil
end

local function showMenu(template, name, animation)
    if not template then return false end
    if menuSource ~= template or not component(menu, "Transform") then
        hideMenu()
        menu = duplicateEntity(template)
        if not menu then return false end
        menuSource = template
        local label = component(menu, "Name")
        if label then label.name = name end
    end
    local transform = component(menu, "Transform")
    if not transform then hideMenu(); return false end
    transform.position = Vector2.new(960, 540)
    local animator = component(menu, "Animator")
    if animation and animator then animator.currentAnimation = animation end
    return true
end

local function beginStage()
    clearActors()
    stage = startingStage
    elapsed, intro, cooldown, healClock = 0, INTRO, 0, 0
    outcome, restartClock, wave, sequence = nil, 0, 0, 0
    local transform = component(player, "Transform")
    if transform then transform.position = Vector2.new(960, 820) end
    setPlayerSpeed(0)
    log("STARFALL | Stage " .. stage .. " | Survivre " .. DURATION .. " secondes")
end

local function reset()
    beginStage()
    local health = component(player, "Health")
    if health then health.currentHealth = health.maxHealth end
    selecting, lastDirection = true, 0
    showMenu(difficultyTemplate, "Menu_Difficulte_Actif", "difficulty_" .. difficulty)
    local controls = component(menu, "Controllable")
    local input = controls and (component(menu, "Velocity") or addComponent(menu, "Velocity"))
    if controls and input then
        controls.speed = 1
    else
        -- Without menu input, start the default/retained difficulty.
        selecting = false
        hideMenu()
        setPlayerSpeed(430)
    end
    log("STARFALL | Choisir: gauche/droite | Fleche haut pour lancer | " .. DIFFICULTIES[difficulty].name)
    log("STARFALL | Fleches pour se deplacer | 30 secondes par stage | 3 vies")
    log("STARFALL | Barre: temps restant. Couleur: vert=3 vies, jaune=2, rouge=1.")
end

local function initialize()
    -- Reuse runtime actors if the controller script is hot-reloaded.
    for _, id in ipairs(getEntities()) do
        local name = component(id, "Name")
        if name then
            if name.name == "Joueur_Pilote" then player = id
            elseif name.name == "Camera_Arene_Fixe_Stage_2" then startingStage = 2
            elseif name.name == "Modele_Projectile_Ennemi" then projectileTemplate = id
            elseif name.name == "Modele_Soin" then healTemplate = id
            elseif name.name == "Modele_Effet_Sonore" then soundTemplate = id
            elseif name.name == "Modele_Menu_Difficulte" then difficultyTemplate = id
            elseif name.name == "Modele_Menu_Transition_Stage_2" then transitionTemplate = id
            elseif name.name == "HUD_Temps_Et_Vies" then hud = id
            elseif name.name:match("^Menu_.*_Actif$") then destroyEntity(id)
            elseif name.name:match("^Danger_") or name.name:match("^Bonus_Soin_") then
                actors[#actors + 1] = id
            end
        end
    end
    if not player then return false end
    if not hud then
        hud = createEntity()
        local label = component(hud, "Name")
        if label then label.name = "HUD_Temps_Et_Vies" end
        addComponent(hud, "Transform", { position = Vector2.new(960, 65) })
        addComponent(hud, "Controllable", { size = Vector2.new(1100, 14), speed = 0 })
    end
    -- Plain Lua data survives the replacement; entity/component references do not.
    local transfer = _G.starfallTransfer
    _G.starfallTransfer = nil
    campaign = startingStage == 1 or (transfer and transfer.campaign) or false
    if transfer then difficulty = math.max(1, math.min(#DIFFICULTIES, transfer.difficulty)) end
    reset()
    if transfer and transfer.resume and startingStage == 2 then
        local health = component(player, "Health")
        if health and transfer.health then health.currentHealth = math.max(0, math.min(health.maxHealth, transfer.health)) end
        selecting, intro = false, 0
        setPlayerSpeed(430)
        hideMenu()
    end
    return true
end

local function changeScene(path, transfer)
    _G.starfallTransfer = transfer
    local ok, message = loadScene(path)
    if not ok then
        _G.starfallTransfer = nil
        restartClock = RESTART
        log("STARFALL | Chargement impossible: " .. message)
    end
    return ok
end

local function cleanActors()
    local existing, alive = {}, {}
    for _, id in ipairs(getEntities()) do existing[id] = true end
    for _, id in ipairs(actors) do
        if existing[id] then alive[#alive + 1] = id end
    end
    actors = alive
end

local function bullet(x, y, angle, speed)
    if #actors >= DIFFICULTIES[difficulty].limit then return end
    sequence = sequence + 1
    if not projectileTemplate then return end
    local id = duplicateEntity(projectileTemplate)
    if not id then return end
    local label = component(id, "Name")
    if label then label.name = string.format("Danger_Vague_%d_Projectile_%04d", wave, sequence) end
    local transform = component(id, "Transform")
    if not transform then destroyEntity(id); return end
    transform.position = Vector2.new(x, y)
    transform.rotation = angle * 180 / math.pi
    -- ColliderSystem uses axis-aligned boxes: enclose the rotated template.
    local collider = component(id, "Collider")
    if collider then
        local width, height = collider.size.x, collider.size.y
        local c, s = math.abs(math.cos(angle)), math.abs(math.sin(angle))
        collider.size = Vector2.new(width * c + height * s, width * s + height * c)
    end
    local velocity = component(id, "Velocity")
    if velocity then velocity.velocity = Vector2.new(math.cos(angle) * speed, math.sin(angle) * speed) end
    local animator = component(id, "Animator")
    if animator then animator.currentFrame, animator.elapsed, animator.playing = 0, 0, true end
    actors[#actors + 1] = id
end

local function shoot()
    local settings = DIFFICULTIES[difficulty]
    local transform = component(player, "Transform")
    if not transform then return end
    local p = transform.position
    local countBefore = #actors
    for shot = 1, settings.counts[wave] do
        -- Every shot gets a new position across the top edge, without fixed lanes.
        local x, y = 100 + random() * 1720, 100 + random() * 40
        -- Leave reaction space even when the player is near the top edge.
        if (x - p.x)^2 + (y - p.y)^2 < 220^2 then
            x = 100 + (x - 100 + 500) % 1720
        end
        local angle = math.pi / 2 + (random() - 0.5) * (wave == 3 and 1.2 or 0.7)
        if wave == 2 then
            -- Aim near the player rather than precisely at its center.
            angle = math.atan(p.y - y, p.x + (random() - 0.5) * 500 - x)
        end
        bullet(x, y, angle, settings.speeds[wave] * (0.9 + random() * 0.2))
    end
    if #actors > countBefore then
        playSound("shot" .. shotSound)
        shotSound = 3 - shotSound
    end
end

local function heal()
    if #actors >= DIFFICULTIES[difficulty].limit then return end
    sequence = sequence + 1
    if not healTemplate then return end
    local id = duplicateEntity(healTemplate)
    if not id then return end
    local label = component(id, "Name")
    if label then label.name = string.format("Bonus_Soin_%04d", sequence) end
    local transform = component(id, "Transform")
    if not transform then destroyEntity(id); return end
    transform.position = Vector2.new(180 + random() * 1560, 140)
    local velocity = component(id, "Velocity")
    if velocity then velocity.velocity = Vector2.new(0, 95) end
    local animator = component(id, "Animator")
    if animator then animator.currentFrame, animator.elapsed, animator.playing = 0, 0, true end
    actors[#actors + 1] = id
end

local function finish(result)
    outcome, restartClock = result, result == "Stage 2" and TRANSITION or RESTART
    clearActors()
    setPlayerSpeed(0)
    if result == "Stage 2" then
        log("STARFALL | Stage 1 termine | Passage au stage 2 dans 3 s")
    else
        log("STARFALL | " .. result .. " | Stage " .. stage .. " | Retour au menu dans 6 s")
    end
end

local function updateHud()
    local health = component(player, "Health")
    local visual = component(hud, "Controllable")
    if selecting then
        if visual then visual.color.a = 0 end
        showMenu(difficultyTemplate, "Menu_Difficulte_Actif", "difficulty_" .. difficulty)
        return
    end
    if outcome == "Stage 2" then
        if visual then visual.color.a = 0 end
        showMenu(transitionTemplate, "Menu_Transition_Actif")
        return
    end
    hideMenu()
    if not visual then return end
    visual.color.a = 255
    local fraction = outcome and restartClock / RESTART or (intro > 0 and intro / INTRO or 1 - elapsed / DURATION)
    local width = math.max(2, 1100 * fraction)
    visual.size.x = width
    local transform = component(hud, "Transform")
    if transform then transform.position.x = 410 + width / 2 end
    visual.color.r, visual.color.g, visual.color.b = 85, 225, 150
    if health and health.currentHealth == 2 then visual.color.r, visual.color.g, visual.color.b = 245, 205, 70 end
    if health and health.currentHealth <= 1 then visual.color.r, visual.color.g, visual.color.b = 245, 75, 90 end
    if outcome == "Victoire" then visual.color.r, visual.color.g, visual.color.b = 90, 200, 255 end
end


function update(entity, dt)
    if not player and not initialize() then return end
    local health = component(player, "Health")
    local playerExists = false
    for _, id in ipairs(getEntities()) do
        if id == player then playerExists = true; break end
    end
    dt = math.max(0, math.min(dt, 0.05))
    if selecting then
        -- Read the active menu's controls; the offscreen model stays unchanged.
        local input = component(menu, "Velocity")
        if not input or not component(menu, "Controllable") then
            selecting = false
            hideMenu()
            setPlayerSpeed(430)
            updateHud()
            return
        end
        local direction = input and (input.velocity.x < 0 and -1 or (input.velocity.x > 0 and 1 or 0)) or 0
        if direction ~= 0 and direction ~= lastDirection then
            difficulty = math.max(1, math.min(#DIFFICULTIES, difficulty + direction))
            playSound("hoverButton")
            log("STARFALL | Difficulte: " .. DIFFICULTIES[difficulty].name)
        end
        lastDirection = direction
        -- Vary the sequence with time spent choosing as well as the Lua seed.
        randomState = 1 + (randomState + math.floor(dt * 100000)) % 2147483646
        if input then
            if input.velocity.y < 0 then
                playSound("hoverButton")
                selecting = false
                local controls = component(menu, "Controllable")
                if controls then controls.speed = 0 end
                setPlayerSpeed(430)
                log("STARFALL | Depart en " .. DIFFICULTIES[difficulty].name .. " dans 3 s")
            end
            input.velocity = Vector2.new(0, 0)
        end
    elseif outcome then
        restartClock = math.max(0, restartClock - dt)
        if restartClock == 0 then
            if outcome == "Stage 2" then
                changeScene("configs/starfall_stage_2.obe", {
                    difficulty = difficulty, health = health and health.currentHealth,
                    campaign = true, resume = true,
                })
                return -- The old world is replaced once this callback returns.
            elseif stage == 2 and campaign then
                changeScene("configs/starfall.obe", { difficulty = difficulty })
                return
            elseif not playerExists then
                changeScene(stage == 2 and "configs/starfall_stage_2.obe" or "configs/starfall.obe", {
                    difficulty = difficulty,
                })
                return
            else
                reset()
            end
        end
    elseif intro > 0 then
        intro = math.max(0, intro - dt)
    elseif not playerExists or (health and health.currentHealth <= 0) then
        finish("Defaite")
    else
        elapsed = math.min(DURATION, elapsed + dt)
        cleanActors()
        if elapsed >= DURATION then
            finish(stage == 1 and "Stage 2" or "Victoire")
        else
            local nextWave = math.min(3, math.floor(elapsed / (DURATION / 3)) + 1)
            if nextWave ~= wave then wave = nextWave; log("STARFALL | Vague " .. wave .. "/3") end
            cooldown, healClock = cooldown - dt, healClock + dt
            if cooldown <= 0 then
                shoot()
                cooldown = DIFFICULTIES[difficulty].delays[wave] * (0.85 + random() * 0.3)
            end
            local healDelay = DIFFICULTIES[difficulty].healDelay
            if healClock >= healDelay then healClock = healClock - healDelay; heal() end
        end
    end
    updateHud()
end
