local INTERVAL = 2.0
local elapsed = 0.0

function update(entity, dt)
    elapsed = elapsed + dt
    if elapsed >= INTERVAL then
        elapsed = elapsed - INTERVAL
        log(entity .. "said hi!")
    end
end
