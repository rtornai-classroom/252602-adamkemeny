// Kemény Ádám R49UG5

#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // Mátrixműveletekhez 
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#define GLM_FORCE_RADIANS

// Globális konstansok az ablakmérethez és a sebességekhez
const GLuint WIDTH = 800, HEIGHT = 600;
const float angleSpeed = 0.0005f;
const float heightSpeed = 0.005f;

// Kamera paraméterek (Henger koordináta-rendszerben)
float cameraAngle = 0.0f;
float cameraHeight = 0.0f;
const float cameraRadius = 9.0f;

// Fényforrás paraméterei
float lightAngle = 0.0f;
const float lightRadius = 2.0f;
const float lightSpeed = 1.0f;
bool lightOn = false;

// Függvény prototípusok
GLuint loadShader(const char* vertexPath, const char* fragmentPath);
void processInput(GLFWwindow* window);

int main() {

    // Irányítás
    std::cout << "===========================================" << std::endl;
    std::cout << "        SZAMITOGEPES GRAFIKA - BEADANDO: 3 " << std::endl;
    std::cout << "===========================================" << std::endl;
    std::cout << " NYILAK (BAL/JOBB): Kamera forgatasa" << std::endl;
    std::cout << " NYILAK (FEL/LE):   Kamera emelese/sullyesztese" << std::endl;
    std::cout << " 'L' BILLENTYU:     Vilagitas BE/KI kapcsolasa" << std::endl;
    std::cout << " ESC GOMB:          Kilepes a programbol" << std::endl;
    std::cout << "===========================================" << std::endl;

    // GLFW inicializálása
    if (!glfwInit()) {
        std::cerr << "GLFW inicializálás nem sikerült!" << std::endl;
        return -1;
    }

    // Ablak létrehozása
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Beadando: 3", nullptr, nullptr);
    if (!window) {
        std::cerr << "A GLFW ablak létrehozása nem sikerült!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // GLEW inicializálása 
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW inicializálás nem sikerült!" << std::endl;
        return -1;
    }

    // Mélységteszt engedélyezése 
    glEnable(GL_DEPTH_TEST);

    // Shaderek betöltése
    GLuint shaderProgram = loadShader("vertexShader.glsl", "fragmentShader.glsl");

    // Kocka csúcspontok: 
    float cubeVertices[] = {
        // Pozíciók           // Normálvektorok (melyik irányba néz a felület)
        -0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
         0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
         0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
        -0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,

        -0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
         0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
         0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,

        -0.5f,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

         0.5f,-0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, 0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, 0.5f, 0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,-0.5f, 0.5f,  1.0f, 0.0f, 0.0f,

         -0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,
          0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,
          0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
         -0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,

         -0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,
          0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,
          0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
         -0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f
    };

    // Index puffer: Meghatározza, melyik csúcspontokból álljanak a háromszögek
    GLuint cubeIndices[] = {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        8,9,10, 10,11,8,
        12,13,14, 14,15,12,
        16,17,18, 18,19,16,
        20,21,22, 22,23,20
    };

    // Buffer objektumok (VAO, VBO, EBO) létrehozása
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    // 0-s attribútum:
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 1-s attribútum:
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window); // Billentyűzet kezelése

        glUseProgram(shaderProgram);

        // Világítás állapotának átadása a shadernek
        GLuint lightOnLoc = glGetUniformLocation(shaderProgram, "lightOn");
        glUniform1i(lightOnLoc, lightOn);

        // Fényforrás pozíciójának kiszámítása
        lightAngle += lightSpeed * deltaTime;
        glm::vec3 lightPos = glm::vec3(lightRadius * cos(lightAngle), lightRadius * sin(lightAngle), 0.0f);

        // Képernyő törlése
        glClearColor(0.0f, 0.0f, 0.0f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Kamera pozíció kiszámítása
        glm::vec3 cameraPos = glm::vec3(
            cameraRadius * cos(cameraAngle),
            cameraRadius * sin(cameraAngle),
            cameraHeight
        );

        // View mátrix:
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // Projection mátrix: (55 fokos látószög)
        glm::mat4 projection = glm::perspective(glm::radians(55.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);

        // Mátrixok és vektorok átadása a shadernek
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        GLuint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        glUniform3fv(lightPosLoc, 1, &lightPos[0]);

        GLuint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
        glUniform3fv(viewPosLoc, 1, &cameraPos[0]);

        glBindVertexArray(VAO);

        // 1. Megjelenítjük a kockát a fényforrás helyén
        glm::mat4 modelLight = glm::translate(glm::mat4(1.0f), lightPos);
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelLight[0][0]);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // 2. Három kocka kirajzolása különböző magasságokban
        std::vector<glm::vec3> positions = {
            {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 2.0f},
            {0.0f, 0.0f,-2.0f}
        };

        for (auto& pos : positions) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// Irányítás:
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  cameraAngle -= angleSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) cameraAngle += angleSpeed;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    cameraHeight += heightSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  cameraHeight -= heightSpeed;

    // Világítás állapota (on/off)
    static bool lPressed = false;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        if (!lPressed) {
            lightOn = !lightOn;
            lPressed = true;
            std::cout << "[INFO] Vilagitas: " << (lightOn ? "BE" : "KI") << std::endl;
        }
    }
    else {
        lPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Shader fájlok beolvasása, fordítása és linkelése
GLuint loadShader(const char* vertexPath, const char* fragmentPath) {
    std::ifstream vFile(vertexPath), fFile(fragmentPath);
    std::stringstream vStream, fStream;
    vStream << vFile.rdbuf(); fStream << fFile.rdbuf();
    std::string vStr = vStream.str(), fStr = fStream.str();
    const char* vSrc = vStr.c_str(); const char* fSrc = fStr.c_str();

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vSrc, nullptr);
    glCompileShader(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fSrc, nullptr);
    glCompileShader(fragment);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return program;
}