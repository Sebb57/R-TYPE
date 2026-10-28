/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Event.hpp
*/

#pragma once
#ifndef EVENT_HPP_
    #define EVENT_HPP_

namespace OrByte {

struct Event {
    enum class Type {
        None,
        Closed,
        KeyPressed,
        KeyReleased,
        TextEntered,
        MouseWheelScrolled,
        MouseMoved,
        MouseButtonPressed,
        MouseButtonReleased,
        Resized,
    };

    enum class Key {
        Unknown,
        Escape,
        Space,
        Backspace,
        Shift,
        Control,
        Alt,
        Enter,
        Left,
        Right,
        Up,
        Down,
        Delete,
        Insert,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
    };

    enum class MouseButton {
        None,
        Left,
        Right,
        Middle
    };

    Type type = Type::None;
    Key key = Key::Unknown;
    unsigned keyChar = '\0';
    MouseButton mouseButton = MouseButton::None;
    double wheelDelta = 0.0;
    int mouseX = 0;
    int mouseY = 0;
    int width = 0;
    int height = 0;
    int windowX = 0;
    int windowY = 0;
};

}

#endif /* EVENT_HPP_ */
