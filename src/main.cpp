#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <MazeGenerator.hpp>
#include <MazeMesh.hpp>

// Updation of viewport size when the window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Input processing
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main()
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Window creation
    GLFWwindow* window = glfwCreateWindow(800, 600, "Museum", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // GLAD initialization
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    
    // ### Maze Generation and Mesh Creation ###

        // Generate and print the maze to console
        MazeGenerator maze(21, 21);
        maze.generate();
        maze.printToConsole();

        // Generate 3D maze mesh
        MazeMesh mazeMesh;
        mazeMesh.build(maze.getGrid());

        // Level 1 - Set polygon mode to line
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // ##########################################


    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // 1. Clear the color buffer
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 2. Draw the maze mesh
        mazeMesh.draw();


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}