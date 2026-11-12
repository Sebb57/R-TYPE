/**
 * @file CompileOptions.hpp
 * @brief which plugins and which binary name the game compilation produces
 *
 * @author Seb
 * @date 2026-11-11
**/

#pragma once
#ifndef COMPILEOPTIONS_HPP_
    #define COMPILEOPTIONS_HPP_

    #include <cstdint>
    #include <filesystem>
    #include <optional>
    #include <set>
    #include <string>
    #include <string_view>
    #include <vector>

namespace OrByte {

class CompileOptions {
    public:
        enum class Kind : std::uint8_t {
            System,
            Component,
        };

        static constexpr const char *DEFAULT_BINARY = "OrByte-gameClient";
        static constexpr std::size_t BINARY_MAX_LENGTH = 64;

        [[nodiscard]] static CompileOptions load(const std::filesystem::path& path);
        void save(const std::filesystem::path& path) const;

        void scan(const std::filesystem::path& pluginsDirectory);

        [[nodiscard]] const std::string& getBinaryName() const noexcept { return this->_binaryName; }
        void setBinaryName(std::string name) { this->_binaryName = std::move(name); }

        [[nodiscard]] const std::vector<std::string>& getPlugins(Kind kind) const noexcept;
        [[nodiscard]] bool isEnabled(Kind kind, const std::string& name) const;
        void setEnabled(Kind kind, const std::string& name, bool enabled);
        void setAllEnabled(Kind kind, bool enabled);
        [[nodiscard]] std::vector<std::string> getEnabled(Kind kind) const;

        [[nodiscard]] std::optional<std::string> checkBinaryName(const std::filesystem::path& root) const;
        [[nodiscard]] std::string cmakeArguments() const;

        [[nodiscard]] bool operator==(const CompileOptions& other) const;

    private:
        std::string _binaryName = DEFAULT_BINARY;
        std::vector<std::string> _systems;
        std::vector<std::string> _components;
        std::set<std::string> _disabledSystems;
        std::set<std::string> _disabledComponents;

        [[nodiscard]] std::set<std::string>& _disabled(Kind kind) noexcept;
        [[nodiscard]] const std::set<std::string>& _disabled(Kind kind) const noexcept;
};

[[nodiscard]] bool isPluginName(std::string_view name) noexcept;

} // namespace OrByte

#endif /* COMPILEOPTIONS_HPP_ */
