#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fontstash.h>
#include <vector>


class GLFontStash {
public:
    GLFontStash(int width, int height) 
        : texture(0), atlasWidth(width), atlasHeight(height) 
    {
        FONSparams params{};
        params.width = width;
        params.height = height;

        // Pass instance-specific functions directly
        params.userPtr = this; 
        context = fonsCreateInternal(&params);

        // Initialize VAO and VBO
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // Allocate buffer size (dynamic allocation if needed later)
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * 1024, nullptr, GL_DYNAMIC_DRAW); // Adjust size as needed

        // Vertex positions (x, y)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);

        // Texture coordinates (s, t)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    ~GLFontStash() {
        Delete();
    }

    // Public methods to manage fontstash rendering
    int Create(int width, int height) {

        // deletes the texture if it exists
        if (texture != 0) {
            glDeleteTextures(1, &texture);
            texture = 0;
        }
        // generates new texture and stores its id in 'texture'
        glGenTextures(1, &texture);
        if (!texture) return 0;

        atlasWidth = width;
        atlasHeight = height;
        glBindTexture(GL_TEXTURE_2D, texture);

        // first 0 means BORDER
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        return 1;
    }

    // Run this to add the data to the glTexImage2D
    void Update(int* rect, const unsigned char* data) {
        int w = rect[2] - rect[0];
        int h = rect[3] - rect[1];
        glBindTexture(GL_TEXTURE_2D, texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, rect[0], rect[1], w, h, GL_ALPHA, GL_UNSIGNED_BYTE, data);
    }

    void Draw(const float* verts, const float* tcoords, const unsigned int* colors, int nverts) {
        // Interleave vertex and texture coordinate data
        std::vector<float> interleavedData(nverts * 4); // x, y, s, t
        for (int i = 0; i < nverts; ++i) {
            interleavedData[i * 4 + 0] = verts[i * 2 + 0];
            interleavedData[i * 4 + 1] = verts[i * 2 + 1];
            interleavedData[i * 4 + 2] = tcoords[i * 2 + 0];
            interleavedData[i * 4 + 3] = tcoords[i * 2 + 1];
        }

        // Update VBO with new data
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, interleavedData.size() * sizeof(float), interleavedData.data());

        // Bind texture and VAO
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao);

        // Render
        glDrawArrays(GL_TRIANGLES, 0, nverts);

        // Unbind VAO
        glBindVertexArray(0);
    }

    void Delete() {
        if (texture != 0) {
            glDeleteTextures(1, &texture);
            texture = 0;
        }
        if (context) {
            fonsDeleteInternal(context);
            context = nullptr;
        }
    }

    // Font loading and management methods
    int AddFont(const char* name, const char* path) {
        return fonsAddFont(context, name, path);
    }

    void SetFont(int fontHandle) {
        fonsSetFont(context, fontHandle);
    }

    void SetSize(float size) {
        fonsSetSize(context, size);
    }

    void SetColor(unsigned int color) {
        fonsSetColor(context, color);
    }

    float DrawText(float x, float y, const char* text) {
        return fonsDrawText(context, x, y, text, nullptr);
    }

private:
    GLuint texture;
    GLuint vao;
    GLuint vbo;
    int atlasWidth;
    int atlasHeight;
    FONScontext* context;
};
