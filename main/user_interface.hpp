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


class UserInterface {
public:
    std::vector<Button> buttons;

    // Method to add a button
    void add_button(const std::string& text, const Vec2f& corner1, const Vec2f& corner2, const Vec4f& color) {
        buttons.push_back({text, corner1, corner2, color});
    }

};

// Function to create a VAO for the UI
GLuint create_UI_vao(UserInterface& ui);

#endif // USER_INTERFACE_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9
