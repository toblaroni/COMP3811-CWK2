#ifndef USER_INTERFACE_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9
#define USER_INTERFACE_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9

#include <glad/glad.h>

#include <vector>
#include <string>

#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"

struct Button {
    std::string text;
    Vec2f corner1;
    Vec2f corner2;
    Vec4f color;
	size_t state = 0;
};

struct Text {
    std::string text;
    Vec2f position;
    Vec4f color;
};

class UserInterface {
public:
    std::vector<Button> buttons;
    std::vector<Text> texts;

    // Method to add a button
    void add_button(const std::string& text, const Vec2f& corner1, const Vec2f& corner2, const Vec4f& color) {
        buttons.push_back({text, corner1, corner2, color});
    }

    // Method to add text
    void add_text(const std::string& text, const Vec2f& position, const Vec4f& color) {
        texts.push_back({text, position, color});
    }

};

// Function to create a VAO for the UI
GLuint create_UI_vao(UserInterface& ui);

#endif // USER_INTERFACE_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9
