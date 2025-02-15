#include "Viewer.h"

PPMImage loadPPM(const std::string& filename) {

    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }

    std::string line;
    std::getline(file, line); // P3 Header

    if (line != "P3") {
        std::cerr << "Not a valid PPM P3 file" << std::endl;
    }

    int width, height, max_value;
    file >> width >> height >> max_value;
    file.ignore();

    std::vector<unsigned char> data(width * height * 3);

    for (int i = 0; i < width * height * 3; ++i) {
        int value;
        file >> value;
        data[i] = static_cast<unsigned char>(value);
    }

    PPMImage img;
    img.width = width;
    img.height = height;
    img.data = std::move(data);
    return img;
}

void initOpenGL() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
}

GLuint createTexture(const PPMImage& img) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}

std::string readShaderSource(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filename << std::endl;
        exit(1);
    }

    std::stringstream shaderStream;
    shaderStream << file.rdbuf();
    return shaderStream.str();
}

void view() {

    // Load the PPM image

    PPMImage img = loadPPM("base.ppm");

    // Initialize GLFW and create a window

    initOpenGL();
    GLFWwindow* window = glfwCreateWindow(img.width, img.height, "PPM Viewer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
    }

    GLuint texture = createTexture(img);

    // Define vertex data for the quad

    GLfloat vertices[] = {
        -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 0.0f
    };

    GLuint indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Load and compile shaders

    std::string vertexShaderSource = readShaderSource("vertex_shader.glsl");
    std::string fragmentShaderSource = readShaderSource("fragment_shader.glsl");

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderCode = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderCode, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentShaderCode = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    // Render loop

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}
