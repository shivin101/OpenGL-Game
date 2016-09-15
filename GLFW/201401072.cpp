#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include<bits/stdc++.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
        GLuint VertexArrayID;
        GLuint VertexBuffer;
        GLuint ColorBuffer;

        GLenum PrimitiveMode;
        GLenum FillMode;
        int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
        glm::mat4 projection;
        glm::mat4 model;
        glm::mat4 view;
        GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

        // Create the shaders
        GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

        // Read the Vertex Shader code from the file
        std::string VertexShaderCode;
        std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
        if(VertexShaderStream.is_open())
        {
                std::string Line = "";
                while(getline(VertexShaderStream, Line))
                        VertexShaderCode += "\n" + Line;
                VertexShaderStream.close();
        }

        // Read the Fragment Shader code from the file
        std::string FragmentShaderCode;
        std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
        if(FragmentShaderStream.is_open()){
                std::string Line = "";
                while(getline(FragmentShaderStream, Line))
                        FragmentShaderCode += "\n" + Line;
                FragmentShaderStream.close();
        }

        GLint Result = GL_FALSE;
        int InfoLogLength;

        // Compile Vertex Shader
        printf("Compiling shader : %s\n", vertex_file_path);
        char const * VertexSourcePointer = VertexShaderCode.c_str();
        glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
        glCompileShader(VertexShaderID);

        // Check Vertex Shader
        glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        std::vector<char> VertexShaderErrorMessage(InfoLogLength);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

        // Compile Fragment Shader
        printf("Compiling shader : %s\n", fragment_file_path);
        char const * FragmentSourcePointer = FragmentShaderCode.c_str();
        glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
        glCompileShader(FragmentShaderID);

        // Check Fragment Shader
        glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

        // Link the program
        fprintf(stdout, "Linking program\n");
        GLuint ProgramID = glCreateProgram();
        glAttachShader(ProgramID, VertexShaderID);
        glAttachShader(ProgramID, FragmentShaderID);
        glLinkProgram(ProgramID);

        // Check the program
        glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

        glDeleteShader(VertexShaderID);
        glDeleteShader(FragmentShaderID);

        return ProgramID;
}

static void error_callback(int error, const char* description)
{
        fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
        struct VAO* vao = new struct VAO;
        vao->PrimitiveMode = primitive_mode;
        vao->NumVertices = numVertices;
        vao->FillMode = fill_mode;

        // Create Vertex Array Object
        // Should be done after CreateWindow and before any other GL calls
        glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
        glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
        glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

        glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
        glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
        glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
        glVertexAttribPointer(
                        0,                  // attribute 0. Vertices
                        3,                  // size (x,y,z)
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        (void*)0            // array buffer offset
                        );

        glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
        glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
        glVertexAttribPointer(
                        1,                  // attribute 1. Color
                        3,                  // size (r,g,b)
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        (void*)0            // array buffer offset
                        );

        return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
        GLfloat* color_buffer_data = new GLfloat [3*numVertices];
        for (int i=0; i<numVertices; i++) {
                color_buffer_data [3*i] = red;
                color_buffer_data [3*i + 1] = green;
                color_buffer_data [3*i + 2] = blue;
        }

        return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
        // Change the Fill Mode for this object
        glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

        // Bind the VAO to use
        glBindVertexArray (vao->VertexArrayID);

        // Enable Vertex Attribute 0 - 3d Vertices
        glEnableVertexAttribArray(0);
        // Bind the VBO to use
        glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

        // Enable Vertex Attribute 1 - Color
        glEnableVertexAttribArray(1);
        // Bind the VBO to use
        glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

        // Draw the geometry !
        glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/
int score=0;
int no_of_balls=0;
bool mouse_fire=false;
vector<int> sevensegdecoder[10];
float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
bool ball_translate_status = false;
bool cannon_rotate_status = false;
bool fired = false;
int vy=0;
int cannon_rotation_dir = 0;
bool cannon_fired=false;
float ball_v=0;
float fire_press_time;
float fire_release_time;
float MAX_POWER=4;
float zoom=1,pan=0;
VAO *sevenseg[15];
typedef struct object
{
        struct VAO* sprite;
        float posx,posy;
        float x,y;
        float vx,vy;
        float radius;
        float mass;
        float rotation;
}object;

void update_object(object*);
void checkCollision(object*,object*);

object ball,goal;
object cannon;
vector <object> blocks,platforms,path;
//object cannon;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
double min(double a,double b)
{
    if(a<b)
            return a;
    return b;
}
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
        // Function is called first on GLFW_PRESS.

        if (action == GLFW_RELEASE) {
                switch (key) {
                        case GLFW_KEY_C:
                                rectangle_rot_status = !rectangle_rot_status;
                                break;
                        case GLFW_KEY_P:
                                triangle_rot_status = !triangle_rot_status;
                                break;
                        case GLFW_KEY_W:
                                ball.vx=0.1;
                                break;
                        case GLFW_KEY_SPACE:
                                fire_release_time=glfwGetTime();
                                cannon_fired=true;
                                break;
                        case GLFW_KEY_A:
                                cannon_rotate_status=false;
                                break;
                        case GLFW_KEY_B:
                                cannon_rotate_status=false;
                                break;
                        default:
                                break;
                }
        }
        else if (action == GLFW_PRESS) {
                switch (key) {
                        case GLFW_KEY_ESCAPE:
                                quit(window);
                                break;
                        case GLFW_KEY_A:
                                cannon_rotate_status=true;
                                cannon_rotation_dir=1;
                                break;
                        case GLFW_KEY_B:
                                cannon_rotate_status=true;
                                cannon_rotation_dir=-1;
                                break;
                        case GLFW_KEY_SPACE:
                                fire_press_time=glfwGetTime();
                                fired=true;
                                break;
                        case GLFW_KEY_UP:
                                zoom+=0.1;
                                break;
                        case GLFW_KEY_DOWN:
                                zoom-=0.1;
                                break;
                        case GLFW_KEY_LEFT:
                                pan-=0.1;
                                break;
                        case GLFW_KEY_RIGHT:
                                pan+=0.1;
                                break;
                        default:
                                break;
                }
        }
}
void gravity(object *ball,bool surface)
{

        float g= 0.009;
        if(!surface)
        { 
                ball->vy=ball->vy-g;
                ball->vy = 0.97*ball->vy;
                ball->vx=0.98*ball->vx;
        }
        if(surface)
        {
                ball->y=-3.001+ball->radius;
                ball->vy=abs(ball->vy);
                ball->vy =0.95*ball->vy;
                if(ball->vy<0.1)
                        ball->vy=0;
        }

}
void update_blocks()
{
        int i;
        for(i=0;i<blocks.size();i++)
        {
                update_object(&blocks[i]);
        }

}
/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
        switch (key) {
                case 'Q':
                case 'q':
                        quit(window);
                        break;
                default:
                        break;
        }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
        switch (button) {
                case GLFW_MOUSE_BUTTON_LEFT:
                        if (action == GLFW_PRESS)
                        {   
                                mouse_fire=true;
                                cannon_fired=true;
                        }
                        break;
                case GLFW_MOUSE_BUTTON_RIGHT:
                        if (action == GLFW_RELEASE) {
                                rectangle_rot_dir *= -1;
                        }
                        break;
                default:
                        break;
        }
}
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
                xpos = xpos/100 - 4;
                ypos = ypos/100 - 4;
                if(xpos <-4 || xpos > 4)
                        if(ypos <-4 || ypos > 4)
                                      return;
                xpos = xpos - cannon.x;
                ypos = ypos + cannon.y;
                ypos *= -1;
                double angle = atan2(ypos, xpos);
                double distance = sqrt(xpos*xpos+ypos*ypos);
                distance = min(MAX_POWER, distance/2);
                ball_v = distance/7;
                angle = (angle*180)/M_PI;
                angle = max(min(90.0, angle), -90.0);
                cannon.rotation = angle;
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
        int fbwidth=width, fbheight=height;
        /* With Retina display on Mac OS X, GLFW's FramebufferSize
           is different from WindowSize */
        glfwGetFramebufferSize(window, &fbwidth, &fbheight);

        GLfloat fov = 90.0f;

        // sets the viewport of openGL renderer
        glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

        // set the projection matrix as perspective
        /* glMatrixMode (GL_PROJECTION);
           glLoadIdentity ();
           gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
        // Store the projection matrix in a variable for future use
        // Perspective projection for 3D views
        // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

        // Ortho projection for 2D views
        Matrices.projection = glm::ortho(-4.0f*zoom+pan, 4.0f*zoom+pan, -4.0f*zoom, 4.0f*zoom, 0.1f, 500.0f);
}

VAO *triangle, *rectangle,*base,*powerbar;
vector <object> aesthetics;
// Creates the triangle object used in this sample code
void createTriangle ()
{
        /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

        /* Define vertex array as used in glBegin (GL_TRIANGLES) */
        static const GLfloat vertex_buffer_data [] = {
                -3.5,-3,0, // vertex 0
                -3,-3,0, // vertex 1
                -3.25,-2.25,0, // vertex 2
        };

        static const GLfloat color_buffer_data [] = {
                1,0,0, // color 0
                0,1,0, // color 1
                0,0,1, // color 2
        };

        // create3DObject creates and returns a handle to a VAO that can be used later
        triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

object createPlatform(float x,float y)
{
        object platform;
        platform.x=x;
        platform.y=y;
        platform.posx=0.5;
        platform.posy=0.2;
        int dir=rand()%2;
        if(!dir)
             dir=-1;   
        platform.vx=0.01*dir;
        static const GLfloat vertex_buffer_data [] = {
                -platform.posx,-platform.posy,0, // vertex 1
                -platform.posx,platform.posy,0, // vertex 2
                platform.posx,-platform.posy,0, // vertex 4
                platform.posx, platform.posy,0, // vertex 3
        };
        static const GLfloat color_buffer_data [] = {
                1,0.43,0, // color 1
                1,0.43,0, // color 1
                1,0.43,0, // color 1
                1,0.43,0, // color 1

        };

        platform.sprite = create3DObject(GL_TRIANGLE_STRIP, 4, vertex_buffer_data, color_buffer_data, GL_FILL);
        return platform;     
}
void createPlatforms()
{
        platforms.push_back(createPlatform(2,2));
        platforms.push_back(createPlatform(2,-2));
        platforms.push_back(createPlatform(2,0));
        platforms.push_back(createPlatform(-1,1));
        platforms.push_back(createPlatform(-0.15,-1));
}
void collidePlatforms2(object *C)
{
        for(int i=0;i<platforms.size();i++)
        {
                glm::vec2 center(C->x, C->y);
                glm::vec2 aabb_half_extents(platforms[i].posx, platforms[i].posy);
                glm::vec2 aabb_center(platforms[i].x, platforms[i].y);
                glm::vec2 difference = center - aabb_center;
                glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
                glm::vec2 closest = aabb_center + clamped;
                difference = closest - center;

                float overlap = glm::length(difference)-C->radius;
                glm::vec2 offset = glm::normalize(difference)*overlap;
                glm::vec2 rebound = glm::normalize(difference)*-0.01f;
                enum direction {UP=0, LEFT=1, DOWN=2, RIGHT=3};
                float dot_prod[4];
                dot_prod[0] = glm::dot(difference, glm::vec2(0, 1));
                dot_prod[1] = glm::dot(difference, glm::vec2(-1, 0));
                dot_prod[2] = glm::dot(difference, glm::vec2(0, -1));
                dot_prod[3] = glm::dot(difference, glm::vec2(1, 0));
                if (glm::length(difference) <= C->radius) {
                        switch(distance(dot_prod, max_element(dot_prod, dot_prod+4)))
                        {
                                case UP:
                                case DOWN:
                                        C->vx *=0.97;
                                        C->vy = 0.94*C->vy*-1;
                                        if(abs(C->vy) < 0.05)
                                                C->vy = 0;
                                        if(abs(C->vx)<= abs(platforms[i].vx))
                                            C->vx=platforms[i].vx;
                                        break;
                                case LEFT:
                                case RIGHT:
                                        C->vx += rebound[0];
                                        C->x += offset[0]*2;
                                        C->y += offset[1];
                                        C->vy *= 0.97;
                                        C->vx = 0.9*C->vx*-1;
                                        if(abs(C->vx) < 0.05)
                                                C->vx = 0;
                                        if(abs(C->vx) <= abs(platforms[i].vx))
                                            C->vx =platforms[i].vx;
                                        break;
                        }
                    return;
                }
        
}
}
void createPath()
{
    object dot;
    dot.x=ball.x;
    dot.y=ball.y;
    static const GLfloat vertex_buffer_data [] = {
                -0.02,-0.02,0, // vertex 1
                -0.02,0.02,0, // vertex 1
                0.02,-0.02,0, // vertex 2
                0.02,0.02,0, // vertex 1
        };
    static const GLfloat color_buffer_data [] = {
                0,0,0, // color 1
                0,0,0, // color 1
                0,0,0, // color 1
                0,0,0, // color 1

        };

    dot.sprite = create3DObject(GL_TRIANGLE_STRIP, 4, vertex_buffer_data, color_buffer_data, GL_FILL);
    path.push_back(dot);
}
void createBase()
{
        static const GLfloat vertex_buffer_data [] = {
                -10,-4,0, // vertex 1
                -10,-3,0, // vertex 2
                10,-4,0, // vertex 4
                10, -3,0, // vertex 3
        };
        static const GLfloat color_buffer_data [] = {
                0.42,0.26,0.15, // color 1
                0.42,0.26,0.15, // color 1
                0.42,0.26,0.15, // color 1
                0.42,0.26,0.15, // color 1

        };

        base = create3DObject(GL_TRIANGLE_STRIP, 4, vertex_buffer_data, color_buffer_data, GL_FILL);
        // create3DObject creates and returns a handle to a VAO that can be used later
}
void createGoal()
{
        static const GLfloat vertex_buffer_data [] = {
                3.2,-2,0, // vertex 1
                5,-2,0, // vertex 2
                5, -4,0, // vertex 3
                3.2,-4,0, // vertex 4
        };
        static const GLfloat color_buffer_data [] = {
                0.42,0.35,0.80, // color 1
                0.42,0.35,0.80, // color 1
                0.42,0.35,0.80, // color 1
                0.42,0.35,0.80, // color 1

        };

        goal.sprite = create3DObject(GL_TRIANGLE_STRIP, 4, vertex_buffer_data, color_buffer_data, GL_FILL);
     
}
bool checkGoal(object *ball)
{
    if(ball->x>3.75&&ball->y<-1.5)
            return true;
    return false;
}
int blocks_count = 0;
void createBlock(float x,float y,float l)
{
        float x1,y1;
        int k=0,i=0;
        object block;
        block.mass=1;
        blocks.push_back(block);
        blocks[blocks_count].x=x;
        blocks[blocks_count].y=y;
        blocks[blocks_count].posx=x;
        blocks[blocks_count].posy=y;
        blocks[blocks_count].rotation=0;
        blocks[blocks_count].radius=0.25;
        GLfloat *vertex_buffer_data = (GLfloat*)malloc(sizeof(GLfloat)*1100);
        GLfloat *color_buffer_data = (GLfloat*)malloc(sizeof(GLfloat)*1100);
        for(float angle=0.0f;angle<2*M_PI;angle+=0.2)
        {

                x1=blocks[blocks_count].x+cos(angle)*blocks[blocks_count].radius;
                y1=blocks[blocks_count].y+sin(angle)*blocks[blocks_count].radius;

                vertex_buffer_data[k]=x1;
                color_buffer_data[k]=0.4;
                k++;
                vertex_buffer_data[k]=y1;
                color_buffer_data[k]=0.9;
                k++;
                vertex_buffer_data[k]=0;
                color_buffer_data[k]=0.1;
                k++;
                i++;

        }
        blocks[blocks_count].sprite = create3DObject(GL_TRIANGLE_FAN, i, vertex_buffer_data, color_buffer_data, GL_FILL);
        blocks_count++;
}
void createGrass()
{
        object grass;
        float i;
        
        for(i=-10;i<10;){
        grass.x=i;
        grass.y=-3;
        static const GLfloat vertex_buffer_data [] = {
                0,0,0, // vertex 0
                0.075,0,0, // vertex 2
                0.037,-0.25,0, // vertex 1
        };

        static const GLfloat color_buffer_data [] = {
                0,1,0, // color 0
                0,1,0, // color 1
                0,1,0, // color 2
        };

        // create3DObject creates and returns a handle to a VAO that can be used later
        grass.sprite= create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
        aesthetics.push_back(grass);
        i=i+0.05;
        }

}

void defineBlocks()
{
        createBlock(2.2,2.75,1);
        createBlock(2.2,0.75,1);
        createBlock(2.2,-1.25,1);
        createBlock(-1,1.5,1);
        createBlock(-0.1,-0.25,1);
}

object createBall(object ball)
{
        ball.x = 2;
        ball.y = 2;
        ball.mass = 1;
        ball.posx=ball.x;
        ball.posy=ball.y;
        ball.rotation=0;
        float x1,y1;
        int k=0,i=0;
        ball.radius=0.15;
        ball.vx=0;
        ball.vy=0;
        GLfloat *vertex_buffer_data = (GLfloat*)malloc(sizeof(GLfloat)*1100);
        GLfloat *color_buffer_data = (GLfloat*)malloc(sizeof(GLfloat)*1100);
        for(float angle=0.0f;angle<2*M_PI;angle+=0.2)
        {

                x1=ball.x+cos(angle)*ball.radius;
                y1=ball.y+sin(angle)*ball.radius;

                vertex_buffer_data[k]=x1;
                color_buffer_data[k]=0.9;
                k++;
                vertex_buffer_data[k]=y1;
                color_buffer_data[k]=0.1;
                k++;
                vertex_buffer_data[k]=0;
                color_buffer_data[k]=0;
                k++;
                i++;

        }
        ball.sprite = create3DObject(GL_TRIANGLE_FAN, i, vertex_buffer_data, color_buffer_data, GL_FILL);
        return ball;
}
void createPowerbar(float l)
{
        static const GLfloat vertex_buffer_data [] = {
                l/2,0.2,0, // vertex 1
                l/2,-0.2,0, // vertex 2
                -l/2, 0.2,0, // vertex 3
                -l/2,-0.2,0, // vertex 4
        };
        static const GLfloat color_buffer_data [] = {
                1,1,0, // color 1
                0,1,0, // color 2
                0,0,1, // color 3

                0.3,1,0.8, // color 4
        };

        powerbar = create3DObject(GL_TRIANGLE_STRIP, 4, vertex_buffer_data, color_buffer_data, GL_FILL);

}
// Creates the rectangle object used in this sample code
void createRectangle ()
{
        // GL3 accepts only Triangles. Quads are not supported
        static const GLfloat vertex_buffer_data [] = {
                -10,-2,0, // vertex 1
                -2.4,-2,0, // vertex 2
                -2.4, 0,0, // vertex 3

                -2.4, 0,0, // vertex 3
                -1.2, 0,0, // vertex 4
                -3.6,-2,0  // vertex 1
        };

        static const GLfloat color_buffer_data [] = {
                1,0,0, // color 1
                0,0,1, // color 2
                0,1,0, // color 3

                0,1,0, // color 3
                0.3,0.3,0.3, // color 4
                1,0,0  // color 1
        };

        // create3DObject creates and returns a handle to a VAO that can be used later
        rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createCannon()
{

        static const GLfloat vertex_buffer_data [] = {
                -3.5,-2.5,0, // vertex 1
                -2.5,-2.5,0, // vertex 2
                -2.5, -2.25,0, // vertex 3

                -3.5,-2.5,0, // vertex 1
                -3.5,-2.2,0, // vertex 4
                -2.5, -2.25,0, // vertex 3

        };
        static const GLfloat color_buffer_data [] = {
                1,0,0, // color 1
                1,0,0, // color 2
                1,0,0, // color 3

                1,0,0, // color 3
                0.3,0.3,0.3, // color 4
                1,0,0  // color 1
        };
        cannon.x = -3.25;
        cannon.y = -2.5;
        cannon.rotation=0;

        // create3DObject creates and returns a handle to a VAO that can be used later
        cannon.sprite = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void friction(object *ball,bool surface)
{


        int dir=1;
        float friction_value=0.005;
        if(ball->x>4 || ball->x<-4)
        {
                if(!(ball->x>4&&ball->y<-2))
                    ball->x=-ball->x;
        }
        if(abs(ball->vx)<0.05&&surface)
        {
                ball->vx=0;
        }
        else if(surface)
        {
                if(ball->vx>0)
                        dir = -1;
                ball->vx += dir*friction_value;

        }
}


void blocks_collision()
{
        for(int i=0;i<blocks.size();i++)
        {
                checkCollision(&ball,&blocks[i]);
                for(int j=0;j<blocks.size();j++)
                {
                        if(i!=j)
                                checkCollision(&blocks[i],&blocks[j]);
                }
        }
}


void checkCollision(object *b1,object *b2)
{
        float dis = (pow((b1->x-b2->x),2)+pow((b1->y-b2->y),2));
        if(dis <= pow(b1->radius+b2->radius,2))
        {
                float xdis=b1->x-b2->x;
                float ydis=b1->y-b2->y;
                float xvel=b2->vx-b1->vx;
                float yvel=b2->vy-b1->vy;
                float dotproduct = xdis*xvel + ydis*yvel;
                if(dotproduct>0)
                {
                        double collisionScale = dotproduct / dis;
                        double xCollision = xdis * collisionScale;
                        double yCollision = ydis * collisionScale;
                        double combinedMass = b1->mass + b2->mass;
                        double collisionWeightA = 2 * b2->mass / combinedMass;
                        double collisionWeightB = 2 * b1->mass / combinedMass;

                        b1->vx += collisionWeightA * xCollision;
                        b1->vy += collisionWeightA * yCollision;
                        b2->vx -= collisionWeightB * xCollision;
                        b2->vy -= collisionWeightB * yCollision;

                        b1->x+=b1->vx;
                        b1->y+=b1->vy;
                        b2->x+=b2->vx;
                        b2->y+=b2->vy;
                }


        }    
}
void update_object(object * ball)
{
        bool surface = false;
        if((ball->y-ball->radius)<-3)
                surface = true;
        gravity(ball,surface);
        friction(ball,surface); 
        collidePlatforms2(ball);
        ball->y+=ball->vy;
        ball->x+=ball->vx; 
}
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
float cannon_rotation = 0;


VAO * createRectangle (float r1,float g1,float b1,float r2,float g2,float b2,float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4)
{
	VAO * rectangle;
	GLfloat vertex_buffer_data [] = { x1,y1,0, x2,y2,0 , x3,y3,0, x4,y4,0, x3,y3,0 , x2,y2,0};
	GLfloat color_buffer_data [] = {  r1,g1,b1, r1,g1,b1, r2,g2,b2,  r2,g2,b2, r2,g2,b2, r1,g1,b1}; // 1 , 2 , 3 , 4 , 3, 2 
	rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	return rectangle;
}
void createSevenSeg()
{
	sevenseg[0]=createRectangle(0.0,0.0,0.0,0.0,0.0,0.0, 0.0f/3,0.0f/3, 0.8f/3,0.0f/3 , 0.1f/3,0.1f/3, 0.7f/3,0.1f/3);
	sevenseg[1]=createRectangle(0.0,0.0,0.0,0.0,0.0,0.0, 0.0f/3,0.1f/3, 0.1f/3,0.2f/3 , 0.0f/3,.9f/3, 0.1f/3,0.8f/3);
	sevenseg[2]=createRectangle(0.0,0.0,0.0,0.0,0.0,0.0, 0.0f/3,1.0f/3, 0.1f/3,1.1f/3 , 0.0f/3,1.8f/3, 0.1f/3,1.7f/3);
	sevenseg[3]=createRectangle(0.0,0.0,0.0,0.0,0.0,0.0, 0.1f/3,1.8f/3, 0.7f/3,1.8f/3 , 0.0f/3,1.9f/3, 0.8f/3,1.9f/3);
	sevenseg[4]=createRectangle(0.0,0.0,0.0,0.0,0.0,0.0, 0.7f/3,1.1f/3, 0.8f/3,1.0f/3 , 0.7f/3,1.7f/3, 0.8f/3,1.8f/3);
	sevenseg[5]=createRectangle(0.0,0.0,0.0,0.0,0.0,0.0, 0.7f/3,0.2f/3, 0.8f/3,0.1f/3 , 0.7f/3,0.8f/3, 0.8f/3,0.9f/3);
	sevenseg[6]=createRectangle(0.0,0.0,0.0,0.0,0.0,0.0, 0.1f/3,0.95f/3, 0.7f/3,0.95f/3 , 0.2f/3,1.0f/3, 0.6f/3,1.0f/3);
	sevenseg[7]=createRectangle(0.0,0.0,0.0,0.0,0.0,0.0, 0.2f/3,0.9f/3, 0.6f/3,0.9f/3 , 0.1f/3,0.95f/3, 0.7f/3,0.95f/3);


	for(int i=0;i<6;i++)
		sevensegdecoder[0].push_back(i);
	sevensegdecoder[1].push_back(4);
	sevensegdecoder[1].push_back(5);
	
	sevensegdecoder[2].push_back(3);
	sevensegdecoder[2].push_back(4);
	sevensegdecoder[2].push_back(6);
	sevensegdecoder[2].push_back(7);
	sevensegdecoder[2].push_back(1);
	sevensegdecoder[2].push_back(0);

	sevensegdecoder[3].push_back(3);
	sevensegdecoder[3].push_back(4);
	sevensegdecoder[3].push_back(6);
	sevensegdecoder[3].push_back(7);
	sevensegdecoder[3].push_back(5);
	sevensegdecoder[3].push_back(0);

	sevensegdecoder[4].push_back(2);
	sevensegdecoder[4].push_back(6);
	sevensegdecoder[4].push_back(7);
	sevensegdecoder[4].push_back(4);
	sevensegdecoder[4].push_back(5);


	sevensegdecoder[5].push_back(3);
	sevensegdecoder[5].push_back(2);
	sevensegdecoder[5].push_back(6);
	sevensegdecoder[5].push_back(7);
	sevensegdecoder[5].push_back(5);
	sevensegdecoder[5].push_back(0);
	
	
	sevensegdecoder[6].push_back(3);
	sevensegdecoder[6].push_back(2);
	sevensegdecoder[6].push_back(6);
	sevensegdecoder[6].push_back(7);
	sevensegdecoder[6].push_back(5);
	sevensegdecoder[6].push_back(1);
	sevensegdecoder[6].push_back(0);
	
	
	sevensegdecoder[7].push_back(3);
	sevensegdecoder[7].push_back(4);
	sevensegdecoder[7].push_back(5);

	for(int i=0;i<8;i++)
		sevensegdecoder[8].push_back(i);
	
	sevensegdecoder[9].push_back(2);
	sevensegdecoder[9].push_back(3);
	sevensegdecoder[9].push_back(4);
	sevensegdecoder[9].push_back(6);
	sevensegdecoder[9].push_back(7);
	sevensegdecoder[9].push_back(5);
	
	}
/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
        // clear the color and depth in the frame buffer
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // use the loaded shader program
        // Don't change unless you know what you are doing
        glUseProgram (programID);

        // Eye - Location of camera. Don't change unless you are sure!!
        glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
        // Target - Where is the camera looking at.  Don't change unless you are sure!!
        glm::vec3 target (0, 0, 0);
        // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
        glm::vec3 up (0, 1, 0);

        // Compute Camera matrix (view)
        // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
        //  Don't change unless you are sure!!
        Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

        // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
        //  Don't change unless you are sure!!
        glm::mat4 VP = Matrices.projection * Matrices.view;

        // Send our transformation to the currently bound shader, in the "MVP" uniform
        //  Don't change unless you are sure!!
        //
        glm::mat4 MVP;	// MVP = Projection * View * Model
        Matrices.projection = glm::ortho(-4.0f*zoom+pan, 4.0f*zoom+pan, -4.0f*zoom, 4.0f*zoom, 0.1f, 500.0f);
        // Load identity to model matrix
        Matrices.model = glm::mat4(1.0f);

        /* Render your scene */

        glm::mat4 translateTriangle = glm::translate (glm::vec3(0, 0, 0)); // glTranslatef
        glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
        Matrices.model *= triangleTransform; 
        MVP = VP * Matrices.model; // MVP = p * V * M

        //  Don't change unless you are sure!!
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // draw3DObject draws the VAO given to it using current MVP matrix
        draw3DObject(triangle);
        draw3DObject(base);
        draw3DObject(goal.sprite);
        for(int i=0;i<aesthetics.size();i++)
        {
                
                Matrices.model = glm::mat4(1.0f);
                glm::mat4 translateBlock = glm::translate (glm::vec3(aesthetics[i].x,aesthetics[i].y, 0)); // glTranslatef
                Matrices.model *= translateBlock; 
                MVP = VP * Matrices.model; // MVP = p * V * M
                glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
                draw3DObject(aesthetics[i].sprite);
        }
        for(int i=0;i<path.size();i++)
        {
                
                Matrices.model = glm::mat4(1.0f);
                glm::mat4 translateBlock = glm::translate (glm::vec3(path[i].x,path[i].y, 0)); // glTranslatef
                Matrices.model *= translateBlock; 
                MVP = VP * Matrices.model; // MVP = p * V * M
                glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
                draw3DObject(path[i].sprite);
        }
        for(int i = 0;i < blocks.size();i++)
        {
                Matrices.model = glm::mat4(1.0f);
                glm::mat4 translateBlock = glm::translate (glm::vec3(blocks[i].x-blocks[i].posx, blocks[i].y-blocks[i].posy, 0)); // glTranslatef
                Matrices.model *= translateBlock; 
                MVP = VP * Matrices.model; // MVP = p * V * M
                glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
                draw3DObject(blocks[i].sprite);
                if(checkGoal(&blocks[i])){
					blocks.erase(blocks.begin()+i--);
					score+=5;	
							}
                     
        }

        for(int i=0;i<platforms.size();i++)
        {
                Matrices.model = glm::mat4(1.0f);
                glm::mat4 translateBlock = glm::translate (glm::vec3(platforms[i].x,platforms[i].y, 0)); // glTranslatef
                Matrices.model *= translateBlock; 
                MVP = VP * Matrices.model; // MVP = p * V * M
                glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
                draw3DObject(platforms[i].sprite);
                platforms[i].x+=platforms[i].vx;
                if(platforms[i].x>3.2||platforms[i].x<-2)
                        platforms[i].vx=-platforms[i].vx;
        }



        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateCannon1 = glm::translate (glm::vec3(cannon.x,cannon.y, 0)); // glTranslatef
        glm::mat4 rotatecannon = glm::rotate((float)(cannon.rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
        glm::mat4 translateCannon2 = glm::translate (glm::vec3(-cannon.x,-cannon.y, 0)); // glTranslatef
        Matrices.model *= rotatecannon;
        MVP = VP * translateCannon1*Matrices.model*translateCannon2;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(cannon.sprite);
        // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
        // glPopMatrix ();
        Matrices.model = glm::mat4(1.0f);

        glm::mat4 translateRectangle = glm::translate (glm::vec3(ball.x-ball.posx, ball.y-ball.posy, 0));        // glTranslatef
        glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
        Matrices.model *= (translateRectangle * rotateRectangle);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // draw3DObject draws the VAO given to it using current MVP matrix
        //draw3DObject(rectangle);
        draw3DObject(ball.sprite);
        //cout<<ball.y<<ball.vy<<"\n";
        // Increment angles
        float increments = 1;
        update_object(&ball);
        update_blocks();
        cannon.rotation = cannon.rotation + increments*cannon_rotation_dir*cannon_rotate_status; 

        if(fired) 
        {
                float size=(glfwGetTime()-fire_press_time)*5;
                if(size>3.5)
                        size=3.5;
                Matrices.model = glm::mat4(1.0f);
                glm::mat4 translateBlock = glm::translate (glm::vec3(0, 3, 0)); // glTranslatef
                glm::mat4 scalebar = glm::scale(glm::vec3(size,1,0));
                Matrices.model *= scalebar*translateBlock; 
                MVP = VP * Matrices.model; // MVP = p * V * M
                glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
                draw3DObject(powerbar);
                size=0;
        }
        if(cannon_fired)
        {
                path.clear();
                no_of_balls+=1;
                if(!mouse_fire)
                    ball_v=(fire_release_time-fire_press_time)*0.5;
                ball.x=-3.25;
                ball.y=-2.25;
                ball.vx=cos(cannon.rotation*M_PI/180.0)*ball_v;
                ball.vy=sin(cannon.rotation*M_PI/180.0)*ball_v;
                cannon_fired=false;
                fire_release_time=0;
                fire_press_time=0;
                fired=false;
                mouse_fire=false;
        }
        blocks_collision();
        if(abs(ball.vx)>0.01||abs(ball.vy)>0.02)
            createPath();
       int temp=score;
	   float scorex=0.0f;
	   while(1)
		{

		for(int i=0;i<sevensegdecoder[temp%10].size();i++)
		{
			Matrices.model = glm::mat4(1.0f);
			glm::mat4 pscore = glm::translate (glm::vec3(3.0f+scorex,3.0f, 0.0f)); 
			Matrices.model*= pscore;
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(sevenseg[sevensegdecoder[temp%10][i]]);
		}
		temp/=10;
		if(!temp)
			break;
		scorex-=1;
		}
        temp= no_of_balls;
        scorex=0.0f;
        while(1)
		{

		for(int i=0;i<sevensegdecoder[temp%10].size();i++)
		{
			Matrices.model = glm::mat4(1.0f);
			glm::mat4 pscore = glm::translate (glm::vec3(-3.0f+scorex,3.0f, 0.0f)); 
			Matrices.model*= pscore;
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(sevenseg[sevensegdecoder[temp%10][i]]);
		}
		temp/=10;
		if(!temp)
			break;
		scorex-=1;
		}

        //camera_rotation_angle++; // Simulating camera rotation
        //triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
        //rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
        GLFWwindow* window; // window desciptor/handle

        glfwSetErrorCallback(error_callback);
        if (!glfwInit()) {
                exit(EXIT_FAILURE);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

        if (!window) {
                glfwTerminate();
                exit(EXIT_FAILURE);
        }

        glfwMakeContextCurrent(window);
        gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
        glfwSwapInterval( 1 );

        /* --- register callbacks with GLFW --- */

        /* Register function to handle window resizes */
        /* With Retina display on Mac OS X GLFW's FramebufferSize
           is different from WindowSize */
        glfwSetFramebufferSizeCallback(window, reshapeWindow);
        glfwSetWindowSizeCallback(window, reshapeWindow);

        /* Register function to handle window close */
        glfwSetWindowCloseCallback(window, quit);

        /* Register function to handle keyboard input */
        glfwSetKeyCallback(window, keyboard);      // general keyboard input
        glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

        /* Register function to handle mouse click */
        glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
        glfwSetCursorPosCallback (window,cursor_position_callback);
        return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
        /* Objects should be created before any other gl function and shaders */
        // Create the models
        createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
        defineBlocks();
        createRectangle ();
        createCannon();
        createBase();
        ball=createBall(ball);
        createPowerbar(1);
        createPlatforms();
        createGrass();
        createGoal();
        createSevenSeg();
        // Create and compile our GLSL program from the shaders
        programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
        // Get a handle for our "MVP" uniform
        Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


        reshapeWindow (window, width, height);

        // Background color of the scene
        glClearColor (0.0f, 0.5f, 0.99f, 0.0f); // R, G, B, A
        glClearDepth (1.0f);

        glEnable (GL_DEPTH_TEST);
        glDepthFunc (GL_LEQUAL);

        cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
        cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
        cout << "VERSION: " << glGetString(GL_VERSION) << endl;
        cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
        int width = 600;
        int height = 600;

        GLFWwindow* window = initGLFW(width, height);

        initGL (window, width, height);

        double last_update_time = glfwGetTime(), current_time;

        /* Draw in loop */
        while (!glfwWindowShouldClose(window)) {

                // OpenGL Draw commands
                draw();

                // Swap Frame Buffer in double buffering
                glfwSwapBuffers(window);

                // Poll for Keyboard and mouse events
                glfwPollEvents();
                if(!blocks.size())
                {
                    float ratio=(float)no_of_balls/(float)score;
                    printf("Final score : 25\nNo of balls Fired = %d\n ",no_of_balls);
                    printf("Balls to score ratio = %f",ratio);
                    exit(0);
                }

                // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
                current_time = glfwGetTime(); // Time in seconds
                if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
                        // do something every 0.5 seconds ..
                        last_update_time = current_time;
                }
        }

        glfwTerminate();
        exit(EXIT_SUCCESS);
}
