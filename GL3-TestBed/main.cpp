#include "Shader.hpp"
#include "Camera.h"

#include <reig/context.h>
#include <reig/reference_widget.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

namespace g {
    template <typename T = int>
    constexpr T windowWidth {800};
    
    template <typename T = int>
    constexpr T windowHeight {600};
}

class Test {
public:
    Test() {
        glfw_init();
        reig_init();
    }

    int run() {
        Shader vertex {GL_VERTEX_SHADER,     "shaders/base.vert", ShaderFromFile{}};
        Shader fragment {GL_FRAGMENT_SHADER, "shaders/base.frag", ShaderFromFile{}};
        ShaderProgram shader {vertex, fragment};
        if(!shader.ready()) {
            std::cout << "vertex: " << vertex.get_log() << '\n' <<
                         "fragment: " << fragment.get_log() << '\n' <<
                         "shader: " << shader.get_log() << std::endl;
            return -1;
        }
        
        GLuint woodTex;
        {
            glGenTextures(1, &woodTex);
            glBindTexture(GL_TEXTURE_2D, woodTex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
            GLint width, height;
            GLubyte* pixels = SOIL_load_image("gfx/wood.jpg", &width, &height, 0, SOIL_LOAD_RGB);
            if(!pixels) {
                std::cout << "Failed to load wood texture!" << std::endl;
                return -1;
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
            glGenerateMipmap(GL_TEXTURE_2D);
            SOIL_free_image_data(pixels);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        
        GLfloat vertices[] = {
            // position                // tex coords
            -0.5f, -0.5f, -0.5f,       0.0f, 0.0f, // lbn00
             0.5f, -0.5f, -0.5f,       1.0f, 0.0f, // rbn10
             0.5f,  0.5f, -0.5f,       1.0f, 1.0f, // rtn11
            -0.5f,  0.5f, -0.5f,       0.0f, 1.0f, // ltn01
            -0.5f, -0.5f,  0.5f,       0.0f, 0.0f, // lbf00
             0.5f, -0.5f,  0.5f,       1.0f, 0.0f, // rbf10
             0.5f,  0.5f,  0.5f,       1.0f, 1.0f, // rtf11
            -0.5f,  0.5f,  0.5f,       0.0f, 1.0f, // ltf01
            -0.5f,  0.5f,  0.5f,       1.0f, 0.0f, // ltf10
            -0.5f,  0.5f, -0.5f,       1.0f, 1.0f, // ltn11
            -0.5f, -0.5f, -0.5f,       0.0f, 1.0f, // lbn01
             0.5f,  0.5f,  0.5f,       1.0f, 0.0f, // rtf10
             0.5f, -0.5f, -0.5f,       0.0f, 1.0f, // rbn01
             0.5f, -0.5f,  0.5f,       0.0f, 0.0f, // rbf00
             0.5f, -0.5f, -0.5f,       1.0f, 1.0f, // rbn11
            -0.5f,  0.5f,  0.5f,       0.0f, 0.0f  // ltf00
        };
        enum tops {
            lbn00, rbn10, rtn11, ltn01, 
            lbf00, rbf10, rtf11, ltf01, 
            ltf10, ltn11, lbn01, rtf10, 
            rbn01, rbf00, rbn11, ltf00
        };
        GLuint indices[] = {
            rtn11, rbn10, lbn00, lbn00, ltn01, rtn11, // back face
            lbf00, rbf10, rtf11, rtf11, ltf01, lbf00, // front face
            ltf10, ltn11, lbn01, lbn01, lbf00, ltf10, // left face
            rbn01, rtn11, rtf10, rtf10, rbf00, rbn01, // right face
            lbn01, rbn11, rbf10, rbf10, lbf00, lbn01, // bottom
            rtf10, rtn11, ltn01, ltn01, ltf00, rtf10  // top
        };
        
        std::vector<glm::vec3> cubePositions {
            { 0.f,   0.f,   0.f  },
            { 2.0f,  5.0f, -15.0f}, 
            {-1.5f, -2.2f, -2.5f },  
            {-3.8f, -2.0f, -12.3f},  
            { 2.4f, -0.4f, -3.5f },  
            {-1.7f,  3.0f, -7.5f },  
            { 1.3f, -2.0f, -2.5f },  
            { 1.5f,  2.0f, -2.5f }, 
            { 1.5f,  0.2f, -1.5f }, 
            {-1.3f,  1.0f, -1.5f }
        };
        
        struct { GLuint vao, vbo, ebo; } cube;
        glGenVertexArrays(1, &cube.vao);
        glGenBuffers(1, &cube.vbo);
        glGenBuffers(1, &cube.ebo);
        
        glBindVertexArray(cube.vao);
        {
            glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(sizeof(GLfloat) * 3));
            
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);
        }
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        
        glEnable(GL_DEPTH_TEST);
        
        while(!glfwWindowShouldClose(window)) {
            double currentTime = glfwGetTime();
            deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            ctx.start_frame();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            glfwPollEvents();
            move_camera();
            
            static float winX = 10, winY = 10;
            static bool check = true;
            static float scaling = 1.0f;
            static float cubeColor[3] {255.f, 255.f, 255.f};
            static float rotation[3] { 0.f, 0.f, 0.f };

            namespace widget = reig::reference_widget;
            namespace colors = reig::primitive::colors;

            float yline = 0; float step = 28;
            ctx.start_window("Cube manipulation", winX, winY);
            widget::checkbox{{0, yline, 25, 25}, colors::darkGrey, check}.use(ctx);
            
            if(check) {
                if(widget::button{"S", {31, yline, 60, 25}, colors::mediumGrey}.use(ctx)) {
                    scaling = 1.f;
                }
                if(widget::button{"R", {97, yline, 60, 25}, colors::mediumGrey}.use(ctx)) {
                    rotation[0] = rotation[1] = rotation[2] = 0.f;
                }
                if(widget::button{"C", {163, yline, 60, 25}, colors::mediumGrey}.use(ctx)) {
                    cubeColor[0] = cubeColor[1] = cubeColor[2] = 255.f;
                }
                
                yline += step;
                widget::label{"Scale:", {0, yline, 230, 25}}.use(ctx);
                yline += step;
                widget::slider{{0, yline, 230, 25}, colors::lightGrey, scaling, 0.1f, 2.5f, 0.1f}.use(ctx);
                
                yline += step;
                widget::label{"Rotation:", {0, yline, 230, 25}}.use(ctx);
                yline += step;
                for(int i = 0; i < 3; ++i) {
                    widget::slider{{0, yline, 230, 25}, colors::darkGrey, rotation[i], 0.f, 360.f, 5.f}.use(ctx);
                    yline += step;
                }

                widget::label{"Color:", {0, yline, 230, 25}}.use(ctx);
                widget::slider{{0, yline += step, 230, 25}, colors::red, cubeColor[0], 0.f, 255.f, 10.f}.use(ctx);
                widget::slider{{0, yline += step, 230, 25}, colors::green, cubeColor[1], 0.f, 255.f, 10.f}.use(ctx);
                widget::slider{{0, yline + step, 230, 25}, colors::blue, cubeColor[2], 0.f, 255.f, 10.f}.use(ctx);
            }
            
            shader.use();
            
            float supRotation = 0;
            for(auto const& transVec : cubePositions) {
                glUniform3f(shader.uniform("color"), cubeColor[0], cubeColor[1], cubeColor[2]);
                using namespace glm;
                mat4 model;
                model = translate(model, transVec);
                model = rotate(model, radians(float(currentTime * rotation[0]) + supRotation), vec3(1.f, 0.f, 0.f));
                model = rotate(model, radians(float(currentTime * rotation[1]) + supRotation), vec3(0.f, 1.f, 0.f));
                model = rotate(model, radians(float(currentTime * rotation[2]) + supRotation), vec3(0.f, 0.f, 1.f));
                model = scale(model, vec3(scaling, scaling, scaling));
                supRotation += 30;
                mat4 view = camera.get_view();
                mat4 projection = perspective(
                    radians(75.f), 
                    g::windowWidth<float> / g::windowHeight<>, 
                    0.1f, 100.f
                );
                
                glUniformMatrix4fv(shader.uniform("model"), 1, GL_FALSE, value_ptr(model));
                glUniformMatrix4fv(shader.uniform("view"), 1, GL_FALSE, value_ptr(view));
                glUniformMatrix4fv(shader.uniform("projection"), 1, GL_FALSE, value_ptr(projection));
                
                glBindVertexArray(cube.vao);
                glBindTexture(GL_TEXTURE_2D, woodTex);
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
            }
            
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);
            shader.unuse();

            ctx.end_frame();
            
            glfwSwapBuffers(window);
        }
        
        glfwTerminate();
        
        return 0;
    }
    
    void move_camera() {
        if(keysPressed[GLFW_KEY_W])          camera.move(Camera::forward,  deltaTime);
        if(keysPressed[GLFW_KEY_S])          camera.move(Camera::backward, deltaTime);
        if(keysPressed[GLFW_KEY_D])          camera.move(Camera::right,    deltaTime);
        if(keysPressed[GLFW_KEY_A])          camera.move(Camera::left,     deltaTime);
        if(keysPressed[GLFW_KEY_SPACE])      camera.move(Camera::up,       deltaTime);
        if(keysPressed[GLFW_KEY_LEFT_SHIFT]) camera.move(Camera::down,     deltaTime);
    }
    
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int) {
        Test* self = static_cast<Test*>(glfwGetWindowUserPointer(window));
        
        if(self->mouse.free && button == GLFW_MOUSE_BUTTON_LEFT) {
            if(action == GLFW_PRESS) {
                self->ctx.mouse.leftButton.press(self->mouse.lastX, self->mouse.lastY);
            }
            else if(action == GLFW_RELEASE) {
                self->ctx.mouse.leftButton.release();
            }
        }
    }
    
    static void mouse_scroll_callback(GLFWwindow* window, double, double y) {
        Test* self = static_cast<Test*>(glfwGetWindowUserPointer(window));

        if(self->mouse.free) {
            self->ctx.mouse.scroll(-y);
        }
    }
    
    static void mouse_motion_callback(GLFWwindow* window, double xpos, double ypos) {
        Test* self = static_cast<Test*>(glfwGetWindowUserPointer(window));
        
        float offsetx = xpos - self->mouse.lastX;
        float offsety = self->mouse.lastY - ypos;
        self->mouse.lastX = xpos;
        self->mouse.lastY = ypos;
            
        if(self->mouse.free) {
            self->ctx.mouse.place(xpos, ypos);
        }
        else {
            self->camera.rotate(offsetx, offsety);
        }
    }
    
    static void  key_button_callback(GLFWwindow* window, int key, int, int action, int) {
        Test* self = static_cast<Test*>(glfwGetWindowUserPointer(window));
        
        if(action == GLFW_PRESS) {
            switch(key) {
                case GLFW_KEY_ESCAPE:
                glfwWindowShouldClose(window);
                break;
                
                case GLFW_KEY_TAB:
                self->mouse.free = !self->mouse.free;
                if(self->mouse.free) {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
                else {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
                break;
                
                default:
                self->keysPressed[key] = true;
            }
        }
        else if(action == GLFW_RELEASE) {
            switch(key) {
                default:
                self->keysPressed[key] = false;
            }
        }
    }
    
    static void render_handler(reig::Context::DrawData const& drawData, std::any& userPtr) {
        Test* self = std::any_cast<Test*>(userPtr);
        
        struct {
            GLint shader, vao, vbo, ebo, texture, blendsrc, blenddst;
            GLboolean depthtest, stenciltest, blend;
        } last;
        glGetIntegerv(GL_CURRENT_PROGRAM, &last.shader);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last.vao);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last.vbo);
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last.ebo);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last.texture);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &last.blendsrc);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &last.blenddst);
        glGetBooleanv(GL_DEPTH_TEST, &last.depthtest);
        glGetBooleanv(GL_STENCIL_TEST, &last.stenciltest);
        glGetBooleanv(GL_BLEND, &last.blend);
        
        self->gui.shader.use();
        glBindVertexArray(self->gui.vao);
        glBindBuffer(GL_ARRAY_BUFFER, self->gui.vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(reig::primitive::Vertex), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(reig::primitive::Vertex), (void*)(offsetof(reig::primitive::Vertex, texCoord)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(reig::primitive::Vertex), (void*)(offsetof(reig::primitive::Vertex, color)));
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->gui.ebo);
        
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        for(auto const& figure : drawData) {
            auto const& vertices = figure.vertices().data();
            auto const& indices  = figure.indices().data();
            auto vnumber = figure.vertices().size();
            auto inumber = figure.indices().size();
            
            glUniform1ui(self->gui.shader.uniform("fragTexId"), figure.texture());
            glBindTexture(GL_TEXTURE_2D, figure.texture());
            
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vnumber, vertices, GL_STATIC_DRAW);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * inumber, indices, GL_STATIC_DRAW);
            glDrawElements(GL_TRIANGLES, inumber, GL_UNSIGNED_INT, nullptr);
        }
        
        glUseProgram(last.shader);
        glBindVertexArray(last.vao);
        glBindBuffer(GL_ARRAY_BUFFER, last.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last.ebo);
        glBindTexture(GL_TEXTURE_2D, last.texture);
        if(last.depthtest) glEnable(GL_DEPTH_TEST);
        if(last.stenciltest) glEnable(GL_STENCIL_TEST);
        if(!last.blend) glDisable(GL_BLEND);
        else glBlendFunc(last.blendsrc, last.blenddst);
    }
    
    void reig_init() {
        ctx.set_user_ptr(this);
        ctx.set_render_handler(&render_handler);
        
        glGenTextures(1, &font.tex);
        auto f = ctx.set_font("/usr/share/fonts/TTF/impact.ttf", font.tex, 20.f);
        glBindTexture(GL_TEXTURE_2D, font.tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, f.width, f.height, 0, GL_RED, GL_UNSIGNED_BYTE, f.bitmap.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        GLint swizzle[] {GL_ONE, GL_ONE, GL_ONE, GL_RED};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        Shader vertexShader {GL_VERTEX_SHADER,     "shaders/gui.vert", ShaderFromFile{}};
        Shader fragmentShader {GL_FRAGMENT_SHADER, "shaders/gui.frag", ShaderFromFile{}};
        gui.shader.attach(vertexShader, fragmentShader);
        if(!gui.shader.ready()) {
            std::cout << vertexShader.get_log() << '\n' <<
                         fragmentShader.get_log() << '\n' <<
                         gui.shader.get_log() << std::endl;
        }
        
        glCreateVertexArrays(1, &gui.vao);
        glCreateBuffers(1, &gui.vbo);
        glCreateBuffers(1, &gui.ebo);
        
        gui.shader.use();
        glUniformMatrix4fv(
            gui.shader.uniform("vertexProjection"), 1, GL_FALSE, 
            glm::value_ptr(
                glm::ortho(0.f, g::windowWidth<float>, g::windowHeight<float>, 0.f)
            )
        );
        gui.shader.unuse();
    }
    
    void glfw_init() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        
        window = glfwCreateWindow(
            g::windowWidth<>, g::windowHeight<>, "reig in OpenGL test", nullptr, nullptr
        );
        glfwMakeContextCurrent(window);
        
        glfwSetWindowUserPointer(window, this);
        
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetMouseButtonCallback(window, &mouse_button_callback);
        glfwSetCursorPosCallback  (window, &mouse_motion_callback);
        glfwSetScrollCallback     (window, &mouse_scroll_callback);
        glfwSetKeyCallback        (window, &key_button_callback);
        
        glewExperimental = GL_TRUE;
        glewInit();
        
        glClearColor(0.1f, 0.1f, 0.1f, 1.f);
        glViewport(0, 0, g::windowWidth<unsigned>, g::windowHeight<unsigned>);
        
        for(int i = 0; i < GLFW_KEY_LAST + 1; ++i) {
            keysPressed[i] = false;
        }
    }
    
private:
    GLFWwindow* window = nullptr;
    
    Camera camera {{0.f, 0.f, 3.f}};
    bool keysPressed[GLFW_KEY_LAST + 1];
    
    double lastTime  {};
    double deltaTime {};

    struct mouse {
        double lastX = g::windowWidth<float> / 2.f;
        double lastY = g::windowHeight<float> / 2.f;
        
        bool neverUsed = true;
        bool free = false;
    }
    mouse;
    
    struct gui {
        ShaderProgram shader;
        GLuint vao {}, vbo {}, ebo {};
        
        ~gui() {
            if(vao) glDeleteVertexArrays(1, &vao);
            if(vbo) glDeleteBuffers(1, &vbo);
            if(ebo) glDeleteBuffers(1, &ebo);
        }
    }
    gui;
    
    reig::Context ctx;
    
    struct font {
        GLuint tex = 0;
    }
    font;
};

int main(int, char*[]) {
    Test test;
    return test.run();
}
