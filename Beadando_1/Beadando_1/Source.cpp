// Kemény Ádám R49UG5

#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>



using namespace std;

// Globális azonosítók az OpenGL objektumokhoz

GLuint renderingProgram;
GLuint vao, vbo; // VAO/VBO a körhöz
GLuint vao_line, vbo_line; // VAO/VBO a vonalhoz

// Mozgáshoz a változók

bool isMoving = false;
bool isRotated = false;

// Kör változók(pozíció, sebesség, sugár)

float circleCenterX = 300.0f;
float circleCenterY = 300.0f;
float circleVelocityX = 0.10f;
float circleVelocityY = 0.0f;
const float radius = 50.0f;

// Vonal változók(pozíció, sebesség)

float lineCenterY = 0.0f;
float lineCenterX = 0.0f;
const float lineSpeed = 1.0f / 300.0f;

// Shader beolvasás

string readShaderSource(const char* filePath) {
    ifstream file(filePath);
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint createShaderProgram() {
    string vertShaderStr = readShaderSource("vertexShader.glsl");
    string fragShaderStr = readShaderSource("fragmentShader.glsl");

    const char* vertShaderSrc = vertShaderStr.c_str();
    const char* fragShaderSrc = fragShaderStr.c_str();

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vShader, 1, &vertShaderSrc, NULL);
    glShaderSource(fShader, 1, &fragShaderSrc, NULL);

    glCompileShader(vShader);
    glCompileShader(fShader);

    GLuint vfProgram = glCreateProgram();
    glAttachShader(vfProgram, vShader);
    glAttachShader(vfProgram, fShader);

    glLinkProgram(vfProgram);

    glDeleteShader(vShader);
    glDeleteShader(fShader);

    return vfProgram;
}

// Billenytûzet eseménykezelõ

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, true);
        }
        if (key == GLFW_KEY_UP) {
            lineCenterY += lineSpeed;
        }
        if (key == GLFW_KEY_DOWN) {
            lineCenterY -= lineSpeed;
        }
        if (key == GLFW_KEY_S) {
            if (!isRotated) {

                float angleInDegrees = 25.0f;
                float angleInRadians = angleInDegrees * M_PI / 180.0f;
                circleVelocityX = cos(angleInRadians) * 0.10f;
                circleVelocityY = sin(angleInRadians) * 0.10f;
                isRotated = true;
            }
        }
    }
}


void init() {
    renderingProgram = createShaderProgram();

    // Kör csúcspontjainak kiszámítása (egységsugarú kör)

    const int numVertices = 100;
    GLfloat vertices[2 * numVertices];

    for (int i = 0; i < numVertices; i++) {
        float angle = 2.0f * M_PI * i / numVertices;
        vertices[2 * i] = cos(angle);
        vertices[2 * i + 1] = sin(angle);
    }

    // Kör puffer (VBO) és leíró (VAO) beállítása

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Pozíció attribútum összekötése a shaderrel

    GLuint positionAttrib = glGetAttribLocation(renderingProgram, "position");
    glVertexAttribPointer(positionAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(positionAttrib);

    GLfloat lineVertices[] = {
        -0.3333f + (lineCenterX),
        (lineCenterY),

        0.3333f + (lineCenterX),
        (lineCenterY)
    };

    glGenBuffers(1, &vbo_line);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_line);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao_line);
    glBindVertexArray(vao_line);
    glVertexAttribPointer(positionAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(positionAttrib);
}

// Mozgás és ütközés a falakkal

void update() {

    circleCenterX += circleVelocityX;
    circleCenterY += circleVelocityY;

    if (circleCenterX + radius > 600.0f || circleCenterX - radius < 0.0f) {
        circleVelocityX = -circleVelocityX;
    }

    if (circleCenterY + radius > 600.0f || circleCenterY - radius < 0.0f) {
        circleVelocityY = -circleVelocityY;
    }

    GLfloat lineVertices[] = {
        -0.3333f + lineCenterX, lineCenterY,
        0.3333f + lineCenterX, lineCenterY
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo_line);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_DYNAMIC_DRAW);
}

// Kirajzolás

void display(GLFWwindow* window) {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(renderingProgram);

    float normalizedCircleCenterX = (circleCenterX - 300.0f) / 300.0f;
    float normalizedCircleCenterY = (circleCenterY - 300.0f) / 300.0f;

    float lineStartX = -0.3333f + lineCenterX;
    float lineEndX = 0.3333f + lineCenterX;

    bool xInRange = (normalizedCircleCenterX >= lineStartX && normalizedCircleCenterX <= lineEndX);
    float yDistance = abs(normalizedCircleCenterY - lineCenterY);
    bool yInRange = (yDistance <= radius / 300.0f);
    bool isIntersecting = xInRange && yInRange;

    GLint colorSwapLoc = glGetUniformLocation(renderingProgram, "colorSwap");
    int colorSwapValue = isIntersecting ? 0 : 1;
    glUniform1i(colorSwapLoc, colorSwapValue);

    GLint isLineLoc = glGetUniformLocation(renderingProgram, "isLine");
    GLint circleCenterLoc = glGetUniformLocation(renderingProgram, "circleCenter");
    GLint radiusLoc = glGetUniformLocation(renderingProgram, "radius");

    // Kör kirajzolása

    glUniform1i(isLineLoc, GL_FALSE);
    glUniform2f(circleCenterLoc, circleCenterX, circleCenterY);
    glUniform1f(radiusLoc, radius);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 100);

    // Vonal kirajzolása

    glUniform1i(isLineLoc, GL_TRUE);
    glBindVertexArray(vao_line);
    glEnableVertexAttribArray(0);
    glLineWidth(3.0f);
    glDrawArrays(GL_LINES, 0, 2);
    glDisableVertexAttribArray(0);

    glfwSetKeyCallback(window, key_callback);
    glfwSwapBuffers(window);
}


int main() {

    
    if (!glfwInit()) return -1;

    // Irányítás 
    cout << "===========================================" << endl;
    cout << "        SZAMITOGEPES GRAFIKA - BEADANDO: 1   " << endl;
    cout << "===========================================" << endl;
    cout << "[S] billentyu:   Kor inditasa 25 fokos szogben" << endl;
    cout << "[FEL nyil]:      Kek szakasz mozgatasa FEL" << endl;
    cout << "[LE nyil]:       Kek szakasz mozgatasa LE" << endl;
    cout << "[ESC billentyu]: KILEPES a programbol" << endl;
    cout << "===========================================" << endl;

    // Ablak létrehozása (600x600 pixel)

    GLFWwindow* window = glfwCreateWindow(600, 600, "Beadando: 1", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();

    glClearColor(0.85f, 0.75f, 0.15f, 1.0f); 
    init();

    while (!glfwWindowShouldClose(window)) {
        update();
        display(window);
        glfwPollEvents();
    }

    // Erõforrások felszabadítása

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
