-- The scene model is silent; each duplicate owns one independent sound.
local lifetime = 0

function update(entity, dt)
    local name = getComponent(entity, "Name")
    if not name or not name.name:match("^Effet_Sonore_") then return end
    local music = getComponent(entity, "Music")
    lifetime = lifetime + math.max(0, math.min(dt, 0.05))
    -- The timeout also releases sounds when the audio system is not selected.
    if not music or music.state == MusicState.Stopped or lifetime >= 5 then
        destroyEntity(entity)
    end
end
