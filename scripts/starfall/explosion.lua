-- Keep the scene model; remove each visual effect once its animation finishes.
local lifetime = 0

function update(entity, dt)
    local name = getComponent(entity, "Name")
    if not name or name.name ~= "Explosion_Joueur" then return end
    local animator = getComponent(entity, "Animator")
    lifetime = lifetime + math.max(0, math.min(dt, 0.05))
    if not animator or not animator.playing or lifetime >= 0.6 then
        destroyEntity(entity)
    end
end
