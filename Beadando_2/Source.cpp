// Kemény Ádám R49UG5

#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// Globális változók 
std::vector<glm::vec2> controlPoints;
GLFWwindow* window;
bool dragging = false;           
int draggedPointIndex = -1;      
GLuint shaderProgram;            
const float pointDiameter = 4.0f;

// Shader forráskód beolvasása
std::string readShaderSource(const char* filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Shader program létrehozása 
GLuint createShaderProgram() {
    std::string vertShaderStr = readShaderSource("vertexShader.glsl");
    std::string fragShaderStr = readShaderSource("fragmentShader.glsl");

    const char* vertShaderSrc = vertShaderStr.c_str();
    const char* fragShaderSrc = fragShaderStr.c_str();

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vertShaderSrc, NULL);
    glCompileShader(vShader);

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fragShaderSrc, NULL);
    glCompileShader(fShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);

    glDeleteShader(vShader);
    glDeleteShader(fShader);

    return program;
}

// Kontrollpontok kirajzolása
void renderPoints(const std::vector<glm::vec2>& points, float radius, glm::vec3 color) {
    glUseProgram(shaderProgram);
    GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
    glUniform3fv(colorLoc, 1, &color[0]);

    std::vector<glm::vec2> circle;
    int segments = 30;
    for (const auto& center : points) {
        circle.clear();
        circle.push_back(center);
        for (int i = 0; i <= segments; ++i) {
            float angle = i * pointDiameter / 2 * M_PI / segments;
            circle.emplace_back(center.x + cos(angle) * radius, center.y + sin(angle) * radius);
        }

        // Buffer objektumok (VAO, VBO)
        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(circle.size() * sizeof(glm::vec2)), circle.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)circle.size());

        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }
}

// Vonalak rajzolása
void renderLines(const std::vector<glm::vec2>& points, glm::vec3 color) {
    if (points.size() < 2) return;

    glUseProgram(shaderProgram);
    GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
    glUniform3fv(colorLoc, 1, &color[0]);

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(points.size() * sizeof(glm::vec2)), points.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)points.size());

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

// A Bezier-görbe matematikai kiszámítása
void renderBezierCurve(const std::vector<glm::vec2>& points) {
    if (points.size() < 2) return;

    std::vector<glm::vec2> bezierPoints;
    int n = (int)points.size() - 1; 
    float step = 1.0f / (1000 + 50 * (float)points.size());

    for (float t = 0.0f; t <= 1.0f; t += step) {
        glm::vec2 p(0.0f);
        for (int i = 0; i <= n; ++i) {
            float binomial = 1.0f;
            for (int j = 0; j < i; ++j)
                binomial *= (float)(n - j) / (float)(j + 1);

            float blend = binomial * pow(1 - t, n - i) * pow(t, i);
            p += points[i] * blend;
        }
        bezierPoints.push_back(p);
    }

    renderLines(bezierPoints, { 0.0f, 1.0f, 0.0f });
}

void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

// Egérgombok eseménykezelése
void mouseButtonCallback(GLFWwindow*, int button, int action, int) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    xpos = (xpos / width) * 2.0 - 1.0;
    ypos = -((ypos / height) * 2.0 - 1.0);
    glm::vec2 clickPos = glm::vec2((float)xpos, (float)ypos);

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            for (int i = 0; i < (int)controlPoints.size(); ++i) {
                if (glm::distance(clickPos, controlPoints[i]) < 0.03f) {
                    dragging = true;
                    draggedPointIndex = i;
                    return;
                }
            }
            controlPoints.push_back(clickPos);
        }
        else if (action == GLFW_RELEASE) {
            dragging = false;
            draggedPointIndex = -1;
        }
    }
    // Jobb egérgomb: Pont törlése
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        for (int i = 0; i < (int)controlPoints.size(); ++i) {
            if (glm::distance(clickPos, controlPoints[i]) < 0.03f) {
                controlPoints.erase(controlPoints.begin() + i);
                return;
            }
        }
    }
}

// Egérmozgás kezelése
void cursorPosCallback(GLFWwindow*, double xpos, double ypos) {
    if (dragging && draggedPointIndex != -1) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        xpos = (xpos / width) * 2.0 - 1.0;
        ypos = -((ypos / height) * 2.0 - 1.0);
        controlPoints[draggedPointIndex] = glm::vec2((float)xpos, (float)ypos);
    }
}

int main() {

    // Irányítás
    std::cout << "===========================================" << std::endl;
    std::cout << "     SZAMITOGEPES GRAFIKA - BEADANDO: 2 " << std::endl;
    std::cout << "===========================================" << std::endl;
    std::cout << " BAL KLIKK:    Pont lehelyezese / Mozgatasa" << std::endl;
    std::cout << " JOBB KLIKK:   Pont torlese" << std::endl;
    std::cout << " ESC GOMB:     Kilepes a programbol" << std::endl;
    std::cout << "===========================================" << std::endl;

    if (!glfwInit()) {
        std::cerr << "GLFW inicializálási hiba!" << std::endl;
        return -1;
    }

    window = glfwCreateWindow(800, 600, "Beadando: 2", nullptr, nullptr);
    if (!window) {
        std::cerr << "Ablak letrehozasi hiba!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW inicializálási hiba!" << std::endl;
        return -1;
    }

    shaderProgram = createShaderProgram();

    // Eseménykezelő visszahívó függvények regisztrálása
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
    // Kilépés (Esc)
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }

        glClear(GL_COLOR_BUFFER_BIT);

        // Geometriák kirajzolása
        renderBezierCurve(controlPoints);
        renderLines(controlPoints, { 0.0f, 0.0f, 1.0f });
        renderPoints(controlPoints, 0.02f, { 1.0f, 0.0f, 0.0f });

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Erőforrások felszabadítása kilépés előtt
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}