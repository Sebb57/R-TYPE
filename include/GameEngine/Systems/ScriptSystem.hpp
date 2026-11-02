/**
 * @file ScriptSystem.hpp
 * @brief Runs the Lua script of every entity owning a Script component
 * @author Seb
 * @date 2026-11-02
**/

#ifndef SCRIPTSYSTEM_HPP_
    #define SCRIPTSYSTEM_HPP_

    #include "Chronometer.hpp"
    #include "GameEngine/ECS/World.hpp"
    #include "plugins/ASystem.hpp"
    #include <sol/sol.hpp>
    #include <exception>
    #include <filesystem>
    #include <string>
    #include <string_view>
    #include <unordered_map>

namespace OrByte {

class ScriptSystem : public ASystem {
    public:
        /// Mandatory function of every script: update(entity, dt)
        static constexpr const char *UPDATE = "update";

        ScriptSystem(World& world, std::filesystem::path scriptsDir = "scripts");
        ~ScriptSystem() override = default;

        void update() override;

        class ScriptException : public std::exception {
            protected:
                std::string _msg;
            public:
                explicit ScriptException(std::string msg) : _msg(std::move(msg)) {}

                [[nodiscard]] const char* what() const noexcept override { return this->_msg.c_str(); }
        };

        class ScriptLoadFailed : public ScriptException {
            public:
                ScriptLoadFailed(const std::string& p, const std::string& why) : ScriptException("Failed to load script '" + p + "' : " + why) {}
        };

        class MissingUpdate : public ScriptException {
            public:
                explicit MissingUpdate(const std::string& p) : ScriptException("Missing mandatory 'update' method in file : " + p) {}
        };

    private:
        struct Instance {
            std::string path;                            // Script::path it was loaded from
            std::filesystem::file_time_type lastWrite;   // for hot reload
            sol::environment env;                        // the entity's sandbox
            sol::protected_function update;              // cached env["update"]
            bool valid = false;                          // false: load failed, wait for a file change
        };

        World& _world;
        std::filesystem::path _scriptsDir;
        Chronometer _chrono;
        sol::state _lua;
        std::unordered_map<int, Instance> _instances;

        void _collect();
        void _sync(int entity, const std::string& path);
        [[nodiscard]] Instance _load(const std::string& path);
        void _call(int entity, Instance& instance, double deltaTime);
        void _bindApi();

        [[nodiscard]] std::filesystem::path _resolve(const std::string& path) const { return this->_scriptsDir / path; }
        static void _reportError(int entity, std::string_view what) { std::cerr << "[ScriptSystem] entity " << entity << ": " << what << std::endl; }
};

} // namespace OrByte

#endif /* SCRIPTSYSTEM_HPP_ */

