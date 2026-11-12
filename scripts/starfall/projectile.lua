-- Attached to the template and inherited by every duplicate.
-- The offscreen template is a model, so it must never delete itself.
function update(entity, dt)
    local name = getComponent(entity, "Name")
    if not name or not name.name:match("^Danger_") then return end
    local transform = getComponent(entity, "Transform")
    if not transform then return end
    local position = transform.position
    if position.x < 25 or position.x > 1895 or position.y < 85 or position.y > 1035 then
        destroyEntity(entity)
    end
end
