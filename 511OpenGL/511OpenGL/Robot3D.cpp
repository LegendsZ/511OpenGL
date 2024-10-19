/*******************************************************************
           KB
********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glut.h> // Changed <gl/glut.h> to <GL/glut.h> for better portability
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "QuadMesh.h"
#include "Camera.h"

const int vWidth = 650;    // Viewport width in pixels
const int vHeight = 500;   // Viewport height in pixels

// Robot body dimensions (adjustable)
float robotBodyWidth = 8.0;
float robotBodyLength = 10.0;
float robotBodyDepth = 6.0;
float headWidth = 0.4 * robotBodyWidth;
float headLength = headWidth;
float headDepth = headWidth;
float eyeRadius = 0.1 * headWidth;
float upperArmLength = robotBodyLength * 0.75;
float upperArmWidth = 0.125 * robotBodyWidth;
float nugLength = upperArmLength / 4.0;
float nugWidth = upperArmWidth;
float nugDepth = upperArmWidth;
float legLength = robotBodyLength * 1.0;
float legWidth = 0.2 * robotBodyWidth;
float stanchionLength = robotBodyLength;
float stanchionRadius = 0.1 * robotBodyDepth;
float baseWidth = 2 * robotBodyWidth;
float baseLength = 0.25 * stanchionLength;

// Camera vars
bool firstTimeMouseMovement = true;
int previousX = 0;
int previousY = 0;
float yaw = 0.0;
float pitch = 0.0;
VECTOR3D rotateAngle = VECTOR3D(0.0, 0.0, 0.0);

// Joint control
bool shoulders = false;
bool hip = false;
bool knees = false;
bool nugSpinStop = false;

// Control Robot body rotation on base
float robotAngle = 0.0;

// Control arm rotation
float shoulderAngle = -40.0;
float nugAngle = -25.0;

// Lighting/shading and material properties
GLfloat robotBody_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 0.0f };
GLfloat robotBody_mat_specular[] = { 0.0f, 0.0f, 0.0f, 0.0f };
GLfloat robotBody_mat_diffuse[] = { 0.7f, 0.42f, 0.0f, 1.0f };
GLfloat robotBody_mat_shininess[] = { 10.0f };

GLfloat robotArm_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 0.0f };
GLfloat robotArm_mat_specular[] = { 0.0f, 0.0f, 0.0f, 0.0f };
GLfloat robotArm_mat_diffuse[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat robotArm_mat_shininess[] = { 32.0f };

GLfloat nug_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat nug_mat_diffuse[] = { 1.0f, 0.6f, 0.2f, 0.01f };
GLfloat nug_mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat nug_mat_shininess[] = { 100.0f };

GLfloat robotLowerBody_mat_ambient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
GLfloat robotLowerBody_mat_specular[] = { 0.774597f, 0.774597f, 0.774597f, 1.0f };
GLfloat robotLowerBody_mat_diffuse[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat robotLowerBody_mat_shininess[] = { 76.8f };

// Light properties
GLfloat light_position0[] = { -4.0f, 8.0f, 8.0f, 1.0f };
GLfloat light_position1[] = { 4.0f, 8.0f, 8.0f, 1.0f };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };

// Mouse button
int currentButton;

// A flat open mesh
QuadMesh* groundMesh = NULL;

// Default Mesh Size
int meshSize = 16;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void animationHandler(int param);
void drawRobot();
void drawBody();
void drawLowerBody();
void drawFoot();
void drawLeftArm();
void drawRightArm();
void nugSpinnerHandler(int param);
void printVECTOR3D(VECTOR3D* in);
void drawUpperLeg();
void drawLowerLeg();
void drawLegWithJoints();

// Clean up allocated resources
void cleanUp() {
    delete groundMesh; // Freeing the allocated ground mesh
}

int main(int argc, char** argv)
{
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(vWidth, vHeight);
    glutInitWindowPosition(200, 30);
    glutCreateWindow("3D Hierarchical Example");

    // Initialize GL
    initOpenGL(vWidth, vHeight);

    // Register callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotionHandler);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(functionKeys);

    // Start animation loop
    glutTimerFunc(0, animationHandler, 0); // Start animation handler

    // Start event loop, never returns
    glutMainLoop();

    cleanUp(); // Cleanup before exit (although this may never be called)
    return 0;
}

// Set up OpenGL. For viewport and projection setup see reshape(). 
void initOpenGL(int w, int h)
{
    // Set up and enable lighting
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);   // This second light is currently off

    // Other OpenGL setup
    glEnable(GL_DEPTH_TEST);   // Remove hidden surfaces
    glShadeModel(GL_SMOOTH);   // Use smooth shading
    glClearColor(0.4f, 0.4f, 0.4f, 0.0f);  // Color and depth for glClear
    glClearDepth(1.0f);
    glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Set up ground quad mesh
    VECTOR3D origin = VECTOR3D(0.0, 0.0, 0.0);
    groundMesh = new QuadMesh(&origin, meshSize); // Initialize QuadMesh
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear buffers

    glLoadIdentity();
    glTranslatef(0.0f, -0.5f, -25.0f); // Translate camera

    // Draw the ground plane
    groundMesh->Draw(); // Ensure Draw method is defined correctly in QuadMesh

    // Draw the robot
    drawRobot(); // Draw robot function

    // Swap buffers for double buffering
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);   // Set the viewport
    glMatrixMode(GL_PROJECTION);  // Switch to projection matrix
    glLoadIdentity();          // Load identity matrix
    gluPerspective(60.0f, (float)w / (float)h, 1.0f, 1000.0f); // Set perspective
    glMatrixMode(GL_MODELVIEW); // Switch back to modelview
}

// Function for handling mouse button events
void mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
        currentButton = button; // Track which button is pressed
    }
}

// Function for handling mouse movement
void mouseMotionHandler(int xMouse, int yMouse)
{
    if (currentButton == GLUT_LEFT_BUTTON) {
        // Control camera rotation with left mouse button
        if (firstTimeMouseMovement) {
            previousX = xMouse;
            previousY = yMouse;
            firstTimeMouseMovement = false;
        }
        else {
            yaw += (xMouse - previousX) * 0.1;  // Update yaw
            pitch -= (yMouse - previousY) * 0.1; // Update pitch
            previousX = xMouse;  // Update previous mouse position
            previousY = yMouse;
        }
        glutPostRedisplay(); // Request to redraw the scene
    }
    else {
        firstTimeMouseMovement = true; // Reset if not moving
    }
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 's':
        shoulders = !shoulders; // Toggle shoulder movement
        break;
    case 'h':
        hip = !hip; // Toggle hip movement
        break;
    case 'k':
        knees = !knees; // Toggle knee movement
        break;
    case 'n':
        nugSpinStop = !nugSpinStop; // Toggle nug spinner
        break;
    case 27: // Escape key
        exit(0); // Exit the application
        break;
    default:
        break;
    }
    glutPostRedisplay(); // Request to redraw the scene
}

void functionKeys(int key, int x, int y)
{
    switch (key) {
    case GLUT_KEY_UP:
        robotAngle += 5.0; // Rotate robot base
        break;
    case GLUT_KEY_DOWN:
        robotAngle -= 5.0; // Rotate robot base
        break;
    case GLUT_KEY_LEFT:
        nugAngle += 5.0; // Adjust nug angle
        break;
    case GLUT_KEY_RIGHT:
        nugAngle -= 5.0; // Adjust nug angle
        break;
    default:
        break;
    }
    glutPostRedisplay(); // Request to redraw the scene
}

// Function for animation loop
void animationHandler(int param)
{
    // Update robot angle based on toggles
    if (shoulders) {
        shoulderAngle += 2.0; // Adjust shoulder angle for animation
    }
    if (hip) {
        // Implement hip rotation logic
    }
    if (knees) {
        // Implement knee rotation logic
    }
    if (!nugSpinStop) {
        nugAngle += 5.0; // Spin the nug continuously
    }
    glutPostRedisplay(); // Request to redraw the scene
    glutTimerFunc(1000 / 60, animationHandler, 0); // Call function 60 times a second
}

void drawRobot()
{
    glPushMatrix();
    glRotatef(robotAngle, 0.0f, 1.0f, 0.0f); // Rotate robot base
    drawBody(); // Draw robot body
    glPopMatrix();
}

// Function to draw the robot's body
void drawBody()
{
    glPushMatrix();

    // Set material properties for the robot's body
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);

    glTranslatef(0.0f, 1.5f, 0.0f); // Move body up
    glScalef(robotBodyWidth, robotBodyDepth, robotBodyLength); // Scale to body dimensions
    glutSolidCube(1.0); // Draw body
    glPopMatrix();

    // Draw head
    glPushMatrix();
    glTranslatef(0.0f, robotBodyDepth + headDepth / 2.0f, 0.0f);
    glScalef(headWidth, headDepth, headLength);
    glutSolidCube(1.0);
    glPopMatrix();

    // Draw arms
    drawLeftArm();
    drawRightArm();

    // Draw legs
    drawLowerBody(); // Lower body includes legs
}

void drawLowerBody()
{
    glPushMatrix();
    glTranslatef(0.0f, -0.5f * robotBodyDepth, 0.0f); // Move down
    glScalef(robotBodyWidth, robotBodyDepth, robotBodyLength);
    glutSolidCube(1.0); // Draw lower body
    glPopMatrix();

    drawLeftLeg(); // Call to draw left leg
    drawRightLeg(); // Call to draw right leg
}

void drawFoot()
{
    glPushMatrix();
    glScalef(legWidth, 0.2f, legWidth);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawLeftArm()
{
    glPushMatrix();
    glTranslatef(-robotBodyWidth / 2.0f, 0.0f, 0.0f); // Position left arm
    glRotatef(shoulderAngle, 1.0f, 0.0f, 0.0f); // Rotate at shoulder
    glTranslatef(-upperArmLength / 2.0f, 0.0f, 0.0f); // Translate to shoulder joint
    glScalef(upperArmWidth, upperArmWidth, upperArmLength);
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);
    glutSolidCube(1.0); // Draw upper arm
    glPopMatrix();

    // Draw Nug
    glPushMatrix();
    glTranslatef(-upperArmLength, 0.0f, 0.0f); // Position nug
    glRotatef(nugAngle, 0.0f, 1.0f, 0.0f); // Rotate nug
    glScalef(nugWidth, nugDepth, nugLength);
    glMaterialfv(GL_FRONT, GL_AMBIENT, nug_mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, nug_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, nug_mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, nug_mat_shininess);
    glutSolidCube(1.0); // Draw nug
    glPopMatrix();
}

void drawRightArm()
{
    glPushMatrix();
    glTranslatef(robotBodyWidth / 2.0f, 0.0f, 0.0f); // Position right arm
    glRotatef(shoulderAngle, 1.0f, 0.0f, 0.0f); // Rotate at shoulder
    glTranslatef(upperArmLength / 2.0f, 0.0f, 0.0f); // Translate to shoulder joint
    glScalef(upperArmWidth, upperArmWidth, upperArmLength);
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);
    glutSolidCube(1.0); // Draw upper arm
    glPopMatrix();
}

void nugSpinnerHandler(int param)
{
    // Implement nug spinner logic here if needed
}

void drawUpperLeg() {
    glPushMatrix();
    glScalef(legWidth, legLength, legWidth);
    glutSolidCube(1.0); // Draw upper leg
    glPopMatrix();
}

void drawLowerLeg() {
    glPushMatrix();
    glScalef(legWidth, legLength, legWidth);
    glutSolidCube(1.0); // Draw lower leg
    glPopMatrix();
}

void drawLeftLeg() {
    glPushMatrix();
    glTranslatef(-robotBodyWidth / 4.0f, -robotBodyDepth / 2.0f, 0.0f); // Position left leg
    drawUpperLeg(); // Draw upper leg
    glTranslatef(0.0f, -legLength, 0.0f); // Move down for lower leg
    drawLowerLeg(); // Draw lower leg
    drawFoot(); // Draw foot
    glPopMatrix();
}

void drawRightLeg() {
    glPushMatrix();
    glTranslatef(robotBodyWidth / 4.0f, -robotBodyDepth / 2.0f, 0.0f); // Position right leg
    drawUpperLeg(); // Draw upper leg
    glTranslatef(0.0f, -legLength, 0.0f); // Move down for lower leg
    drawLowerLeg(); // Draw lower leg
    drawFoot(); // Draw foot
    glPopMatrix();
}

void initLighting() {
    glEnable(GL_LIGHTING); // Enable lighting
    glEnable(GL_LIGHT0); // Enable light source
    GLfloat light_position[] = { 0.0, 10.0, 10.0, 1.0 }; // Light position
    GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1.0 }; // Ambient light
    GLfloat light_diffuse[] = { 0.8, 0.8, 0.8, 1.0 }; // Diffuse light
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 }; // Specular light
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glEnable(GL_COLOR_MATERIAL); // Enable color material
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv); // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Set display mode
    glutInitWindowSize(800, 600); // Set window size
    glutCreateWindow("Robot Simulation"); // Create window
    glEnable(GL_DEPTH_TEST); // Enable depth testing

    initLighting(); // Initialize lighting

    // Set callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotionHandler);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(functionKeys);
    glutTimerFunc(0, animationHandler, 0); // Set timer for animation

    // Initialize QuadMesh
    initialize();

    glutMainLoop(); // Enter the main loop
    return 0;
}
