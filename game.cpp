#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <vector> 
#include<string>
#include <sstream>
#include <cstdlib> 
#include <ctime> 
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
enum GameState { LANDING, AIMING, SHOOTING, SCORED, RESET };
GameState gameState = LANDING;
const float GRAVITY = 0.08f;
const float FRICTION = 0.99f;
const float BOUNCE_FACTOR_FLOOR = 0.7f;
const float BOUNCE_FACTOR_WALL = 0.5f;
const float BOUNCE_FACTOR_RIM = 0.6f;
const float BOUNCE_FACTOR_BACKBOARD = 0.7f;
const float MIN_VELOCITY_FOR_RESET = 0.05f;
int mouseX = 0;
int mouseY = 0;
bool mousePressed = false;
float stickFigureX = 200.0f;
float stickFigureY = 100.0f;
float stickFigureSize = 90.0f;
float armAngle = 0.0f;
float ballX = stickFigureX + stickFigureSize * 0.5f;
float ballY = stickFigureY + stickFigureSize * 0.75f;
float ballRadius = 15.0f;
float ballVelocityX = 0.0f;
float ballVelocityY = 0.0f;
bool ballInHand = true;
float ballRotationAngle = 0.0f;
float basketRotationAngle = 0.0f;
float basketMoveSpeed = 2.0f;
bool basketMovingRight = true;
float power = 0.0f;
const float MAX_POWER = 100.0f;
float basketX = 650.0f;
float basketY = 350.0f;
float basketWidth = 60.0f;
float basketHeight = 40.0f;
float rimWidth = 5.0f;
float rimLeftX = basketX - basketWidth / 2.0f;
float rimRightX = basketX + basketWidth / 2.0f;
float rimTopY = basketY + rimWidth / 2.0f;
float rimBottomY = basketY - rimWidth / 2.0f; 
float backboardWidth = 10.0f; 
float backboardHeight = 70.0f; 
float backboardLeftX = rimRightX; 
float backboardRightX = backboardLeftX + backboardWidth;
float backboardBottomY = basketY - backboardHeight / 2.0f;
float backboardTopY = basketY + backboardHeight / 2.0f;
int score = 0;
bool justScored = false;
float jumpOffset = 0.0f;
bool cheering = false; 
GLuint backgroundTexture;
int jumpCount = 0; 
const int maxJumps = 3; 
float landingTime = 0.0f;
bool ballInNet = false;
float clickCooldown = 0.0f;
const float CLICK_COOLDOWN_TIME = 0.5f; 

// Function prototypes
void init();
void display();
void reshape(int w, int h);
void mouseMove(int x, int y);
void mouseButton(int button, int state, int x, int y);
void timer(int value);
void drawStickFigure();
void drawBall();
void drawBasket();
void drawPowerMeter();
void drawScore();
void updateGame(int value);
void shootBall();
void resetGame();
bool checkAndHandleRimCollision();
bool checkAndHandleBackboardCollision();
bool checkScore();
void randomizeBasket();
void drawEnvironment();
void drawStands();
void drawBouncingBox();

void keyboardFunc(unsigned char key, int x, int y) {
    if (key == 'r' || key == 'R') {
        score = 0;
    }
}
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Basketball Game");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(mouseMove); 
    glutMotionFunc(mouseMove);
    glutMouseFunc(mouseButton);
    glutTimerFunc(16, timer, 0);
    glutKeyboardFunc(keyboardFunc);
    glutMainLoop();
    return 0;
}
void init() {
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    randomizeBasket();
}
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    drawEnvironment();
    drawStands();
    drawStickFigure();
    drawPowerMeter();
    drawScore();
    drawBall();
    drawBasket();
    if (gameState == LANDING) {
        drawBouncingBox();
    }
    if (gameState == SCORED) {
        glColor3f(0.65f, 0.85f, 0.9f);  
        glBegin(GL_QUADS);
        glVertex2f(3 * WINDOW_WIDTH / 10, WINDOW_HEIGHT - 50.0f);   
        glVertex2f(7 * WINDOW_WIDTH / 10, WINDOW_HEIGHT - 50.0f);   
        glVertex2f(7 * WINDOW_WIDTH / 10, WINDOW_HEIGHT - 200.0f);   
        glVertex2f(3 * WINDOW_WIDTH / 10, WINDOW_HEIGHT - 200.0f);   
        glEnd();
        glColor3f(0.4f, 0.7f, 0.2f);
        glRasterPos2f(WINDOW_WIDTH / 2 - 88.0f, WINDOW_HEIGHT - 130.0f);
        const char* msg = "SCORE! Click to Reset";
        for (const char* c = msg; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }
    else if (gameState == RESET) {
        glColor3f(1.0f, 0.80f, 0.65f);  
        glBegin(GL_QUADS);
        glVertex2f(3 * WINDOW_WIDTH / 10, WINDOW_HEIGHT - 50.0f);   
        glVertex2f(7 * WINDOW_WIDTH / 10, WINDOW_HEIGHT - 50.0f);   
        glVertex2f(7 * WINDOW_WIDTH / 10, WINDOW_HEIGHT - 200.0f);   
        glVertex2f(3 * WINDOW_WIDTH / 10, WINDOW_HEIGHT - 200.0f);   
        glEnd();
        glColor3f(0.8f, 0.1f, 0.1f);
        glRasterPos2f(WINDOW_WIDTH / 2 - 80.0f, WINDOW_HEIGHT - 130.0f);
        const char* msg = "Miss! Click to Reset";
        for (const char* c = msg; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }
    glutSwapBuffers();
}
void drawBouncingBox() {
    float bounce = sin(landingTime * 2.0f) * 10.0f;
    glBegin(GL_QUADS);
    glColor3f(0.2f, 0.2f, 0.2f);
    glVertex2f(30.0f, 10.0f + bounce);
    glVertex2f(WINDOW_WIDTH - 30.0f, 10.0f + bounce);
    glVertex2f(WINDOW_WIDTH - 30.0f, WINDOW_HEIGHT - 10.0f + bounce);
    glVertex2f(30.0f, WINDOW_HEIGHT - 10.0f + bounce);
    glEnd();
    glBegin(GL_QUADS);
    glColor3f(0.8f, 0.4f, 0.2f);
    glVertex2f(40.0f, 20.0f + bounce);
    glColor3f(0.9f, 0.6f, 0.3f);
    glVertex2f(WINDOW_WIDTH - 40.0f, 20.0f + bounce);
    glColor3f(0.6f, 0.3f, 0.6f);
    glVertex2f(WINDOW_WIDTH - 40.0f, WINDOW_HEIGHT - 20.0f + bounce);
    glColor3f(0.4f, 0.2f, 0.5f);
    glVertex2f(40.0f, WINDOW_HEIGHT - 20.0f + bounce);
    glEnd();
    const char* title = "Ballistic BasketBall";
    int titleLength = strlen(title);
    int titlePixelWidth = titleLength * 18;
    glColor3f(1.0f, 1.0f, 0.2f);
    glRasterPos2f(WINDOW_WIDTH / 2 - titlePixelWidth / 2.0f + 70.0f, WINDOW_HEIGHT / 2 - 80.0f + bounce);
    for (const char* c = title; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(WINDOW_WIDTH / 2 - 120.0f, WINDOW_HEIGHT / 2 - 10.0f + bounce);
    const char* msg = "Click ANYWHERE to Play!!";
    for (const char* c = msg; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
}

void mouseMove(int x, int y) {
    mouseX = x;
    mouseY = WINDOW_HEIGHT - y;
    if (gameState == AIMING) {
        float shoulderY = stickFigureY + stickFigureSize * 0.65f;
        float dx = mouseX - stickFigureX;
        float dy = mouseY - shoulderY;
        armAngle = atan2(dy, dx);
        armAngle = std::max(0.1f, std::min((float)M_PI - 0.1f, armAngle));
        if (mousePressed) {
            float distance = sqrt(dx * dx + dy * dy);
            power = std::min(distance / 2.5f, MAX_POWER);
        }
        if (ballInHand) {
            float handDist = stickFigureSize * 0.5f + ballRadius * 0.5f;
            ballX = stickFigureX + cos(armAngle) * handDist;
            ballY = shoulderY + sin(armAngle) * handDist;
        }
    }
}

void mouseButton(int button, int state, int x, int y) {
    if (clickCooldown > 0.0f) return;
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            if (gameState == LANDING) {
                gameState = AIMING;
                return;
            }
            if (gameState == AIMING && ballInHand) {
                mousePressed = true;
                mouseX = x;
                mouseY = WINDOW_HEIGHT - y;
                power = 0.0f;
                return;
            }
            if (gameState != AIMING && gameState != LANDING) {
                resetGame();
                gameState = AIMING;
                clickCooldown = CLICK_COOLDOWN_TIME;
                return;
            }
        }
        else if (state == GLUT_UP) {
            if (mousePressed && gameState == AIMING && ballInHand) {
                mousePressed = false;
                shootBall();
                gameState = SHOOTING;
                justScored = false;
                return;
            }
            mousePressed = false;
        }
    }
}

void timer(int value) {
    if (clickCooldown > 0.0f) {
        clickCooldown -= 0.016f;
        if (clickCooldown < 0.0f) clickCooldown = 0.0f;
    }
    if (cheering) {
        jumpOffset += 2.0f;
        if (jumpOffset > 15.0f) {
            jumpOffset = 15.0f;
            cheering = false;
        }
    }
    else if (jumpOffset > 0.0f) {
        jumpOffset -= 1.0f;
        if (jumpOffset <= 0.0f) {
            jumpOffset = 0.0f;
            jumpCount++;
            if (jumpCount < maxJumps) {
                cheering = true;
            }
        }
    }
    landingTime += 0.016f;
    updateGame(value);
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void drawEnvironment() {
    float plankHeight = 15.0f;
    float courtWidth = 210.0f;
    for (int i = 0; i < courtWidth / plankHeight; i++) {
        if (i % 2 == 0) glColor3f(0.8f, 0.6f, 0.4f);
        else glColor3f(0.75f, 0.55f, 0.35f);
        glBegin(GL_QUADS);
        glVertex2f(0, i * plankHeight);
        glVertex2f(WINDOW_WIDTH, i * plankHeight);
        glVertex2f(WINDOW_WIDTH, (i + 1) * plankHeight);
        glVertex2f(0, (i + 1) * plankHeight);
        glEnd();
    }
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i++) {
        float angle = M_PI * i / 180.0f;
        float x = WINDOW_WIDTH / 2 + 30 * cos(angle);
        float y = courtWidth / 2 + 12 + 18 * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
    glColor3f(0.70f, 0.35f, 0.10f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(WINDOW_WIDTH / 2, courtWidth / 2 + 12);
    for (int i = 0; i <= 360; i++) {
        float angle = M_PI * i / 180.0f;
        float x = WINDOW_WIDTH / 2 + 30 * cos(angle);
        float y = courtWidth / 2 + 12 + 18 * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex2f(WINDOW_WIDTH / 2, courtWidth);
    glVertex2f(WINDOW_WIDTH / 2, 0);
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(0, courtWidth / 2 - 30.0f);
    glVertex2f(100, courtWidth / 2 - 30.0f);
    glVertex2f(100, courtWidth / 2 + 50.0f);
    glVertex2f(0, courtWidth / 2 + 50.0f);
    glEnd();
    glColor3f(0.70f, 0.35f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(0, courtWidth / 2 - 30.0f);
    glVertex2f(100, courtWidth / 2 - 30.0f);
    glVertex2f(100, courtWidth / 2 + 50.0f);
    glVertex2f(0, courtWidth / 2 + 50.0f);
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 90; i > -91; i--) {
        float angle = M_PI * i / 180.0f;
        float x = 100 + 40 * cos(angle);
        float y = courtWidth / 2 + 10 + 40 * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
    glBegin(GL_LINE_STRIP);
    for (int i = 90; i > -91; i--) {
        float angle = M_PI * i / 180.0f;
        float x = 100 + 90 * cos(angle);
        float y = courtWidth / 2 + 10 + 65 * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(0.0f, 180.0f);
    glVertex2f(100.0f, 180.0f);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(0.0f, 50.0f);
    glVertex2f(100.0f, 50.0f);
    glEnd();
    glColor3f(0.70f, 0.35f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(WINDOW_WIDTH - 0, courtWidth / 2 - 30.0f);
    glVertex2f(WINDOW_WIDTH - 100, courtWidth / 2 - 30.0f);
    glVertex2f(WINDOW_WIDTH - 100, courtWidth / 2 + 50.0f);
    glVertex2f(WINDOW_WIDTH - 0, courtWidth / 2 + 50.0f);
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(WINDOW_WIDTH - 0, courtWidth / 2 - 30.0f);
    glVertex2f(WINDOW_WIDTH - 100, courtWidth / 2 - 30.0f);
    glVertex2f(WINDOW_WIDTH - 100, courtWidth / 2 + 50.0f);
    glVertex2f(WINDOW_WIDTH - 0, courtWidth / 2 + 50.0f);
    glEnd();
    glBegin(GL_LINE_LOOP);
    for (int i = 90; i > -91; i--) {
        float angle = M_PI * i / 180.0f;
        float x = WINDOW_WIDTH - (100 + 40 * cos(angle));
        float y = courtWidth / 2 + 10 + 40 * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
    glBegin(GL_LINE_STRIP);
    for (int i = 90; i > -91; i--) {
        float angle = M_PI * i / 180.0f;
        float x = WINDOW_WIDTH - (100 + 90 * cos(angle));
        float y = courtWidth / 2 + 10 + 65 * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(WINDOW_WIDTH - 0.0f, 180.0f);
    glVertex2f(WINDOW_WIDTH - 100.0f, 180.0f);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(WINDOW_WIDTH - 0.0f, 50.0f);
    glVertex2f(WINDOW_WIDTH - 100.0f, 50.0f);
    glEnd();
}
void drawStickFigure() {
    float skinColor[3] = { 0.96f, 0.80f, 0.69f };
    float shirtColor[3] = { 0.2f, 0.4f, 0.8f };
    float pantsColor[3] = { 0.2f, 0.2f, 0.3f };
    float shoeColor[3] = { 0.1f, 0.1f, 0.1f };
    float hairColor[3] = { 0.2f, 0.15f, 0.1f };
    float headRadius = stickFigureSize * 0.11f;
    float neckLength = stickFigureSize * 0.08f;
    float torsoWidth = stickFigureSize * 0.25f;
    float shoulderWidth = stickFigureSize * 0.35f;
    float hipWidth = stickFigureSize * 0.25f;
    float headCenterY = stickFigureY + stickFigureSize * 0.88f;
    float neckY = headCenterY - headRadius - neckLength * 0.3f;
    float shoulderY = neckY - neckLength * 0.7f;
    float waistY = stickFigureY + stickFigureSize * 0.35f;
    float hipY = waistY - stickFigureSize * 0.05f;
    float kneeY = stickFigureY + stickFigureSize * 0.18f;
    glLineWidth(2.5f);
    glPushMatrix();
    glTranslatef(stickFigureX, headCenterY, 0);
    glColor3fv(hairColor);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(-headRadius * 0.2f, headRadius * 0.3f);
    for (int i = 180; i <= 360; i += 10) {
        float angle = M_PI * i / 180.0f;
        glVertex2f(headRadius * 1.1f * cos(angle), headRadius * 1.15f * sin(angle) + headRadius * 0.2f);
    }
    glEnd();
    glColor3fv(skinColor);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (int i = 0; i <= 360; i += 10) {
        float angle = M_PI * i / 180.0f;
        glVertex2f(headRadius * cos(angle), headRadius * sin(angle));
    }
    glEnd();
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(1.5f);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    glVertex2f(-headRadius * 0.3f, headRadius * 0.1f);
    glVertex2f(headRadius * 0.3f, headRadius * 0.1f);
    glEnd();
    float mouthRadius = headRadius * 0.45f;
    float mouthYOffset = -headRadius * 0.35f;
    glBegin(GL_LINE_STRIP);
    for (int i = 200; i <= 340; i += 8) {
        float angle = M_PI * i / 180.0f;
        glVertex2f(mouthRadius * cos(angle), mouthYOffset + mouthRadius * 0.5f * sin(angle));
    }
    glEnd();
    glColor3fv(hairColor);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, headRadius * 0.5f);
    for (int i = 0; i <= 180; i += 10) {
        float angle = M_PI * i / 180.0f;
        glVertex2f(headRadius * 1.1f * cos(angle), headRadius * 1.2f * sin(angle) + headRadius * 0.2f);
    }
    glEnd();
    glPopMatrix();
    glColor3fv(skinColor);
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    glVertex2f(stickFigureX, headCenterY - headRadius);
    glVertex2f(stickFigureX, neckY - neckLength);
    glEnd();
    glColor3fv(shirtColor);
    glBegin(GL_QUADS);
    glVertex2f(stickFigureX - shoulderWidth / 2, shoulderY);
    glVertex2f(stickFigureX + shoulderWidth / 2, shoulderY);
    glVertex2f(stickFigureX + torsoWidth / 2, waistY);
    glVertex2f(stickFigureX - torsoWidth / 2, waistY);
    glEnd();
    glColor3fv(pantsColor);
    glBegin(GL_QUADS);
    glVertex2f(stickFigureX - hipWidth / 2, waistY);
    glVertex2f(stickFigureX + hipWidth / 2, waistY);
    glVertex2f(stickFigureX + hipWidth / 2, hipY);
    glVertex2f(stickFigureX - hipWidth / 2, hipY);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(stickFigureX - hipWidth / 2 + hipWidth * 0.1f, hipY);
    glVertex2f(stickFigureX - hipWidth * 0.1f, hipY);
    glVertex2f(stickFigureX - hipWidth * 0.15f, kneeY);
    glVertex2f(stickFigureX - hipWidth / 2 + hipWidth * 0.05f, kneeY);
    glVertex2f(stickFigureX - hipWidth / 2 + hipWidth * 0.05f, kneeY);
    glVertex2f(stickFigureX - hipWidth * 0.15f, kneeY);
    glVertex2f(stickFigureX - hipWidth * 0.2f, stickFigureY + stickFigureSize * 0.02f);
    glVertex2f(stickFigureX - hipWidth / 2, stickFigureY + stickFigureSize * 0.02f);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(stickFigureX + hipWidth * 0.1f, hipY);
    glVertex2f(stickFigureX + hipWidth / 2 - hipWidth * 0.1f, hipY);
    glVertex2f(stickFigureX + hipWidth / 2 - hipWidth * 0.05f, kneeY);
    glVertex2f(stickFigureX + hipWidth * 0.15f, kneeY);
    glVertex2f(stickFigureX + hipWidth * 0.15f, kneeY);
    glVertex2f(stickFigureX + hipWidth / 2 - hipWidth * 0.05f, kneeY);
    glVertex2f(stickFigureX + hipWidth / 2, stickFigureY + stickFigureSize * 0.02f);
    glVertex2f(stickFigureX + hipWidth * 0.2f, stickFigureY + stickFigureSize * 0.02f);
    glEnd();
    glColor3fv(shoeColor);
    glBegin(GL_POLYGON);
    glVertex2f(stickFigureX - hipWidth / 2, stickFigureY + stickFigureSize * 0.02f);
    glVertex2f(stickFigureX - hipWidth * 0.2f, stickFigureY + stickFigureSize * 0.02f);
    glVertex2f(stickFigureX - hipWidth * 0.15f, stickFigureY);
    glVertex2f(stickFigureX - hipWidth / 2 - stickFigureSize * 0.05f, stickFigureY);
    glVertex2f(stickFigureX - hipWidth / 2 - stickFigureSize * 0.05f, stickFigureY + stickFigureSize * 0.04f);
    glVertex2f(stickFigureX - hipWidth / 2, stickFigureY + stickFigureSize * 0.04f);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(stickFigureX + hipWidth * 0.2f, stickFigureY + stickFigureSize * 0.02f);
    glVertex2f(stickFigureX + hipWidth / 2, stickFigureY + stickFigureSize * 0.02f);
    glVertex2f(stickFigureX + hipWidth / 2, stickFigureY + stickFigureSize * 0.04f);
    glVertex2f(stickFigureX + hipWidth / 2 + stickFigureSize * 0.05f, stickFigureY + stickFigureSize * 0.04f);
    glVertex2f(stickFigureX + hipWidth / 2 + stickFigureSize * 0.05f, stickFigureY);
    glVertex2f(stickFigureX + hipWidth * 0.15f, stickFigureY);
    glEnd();
    glColor3fv(shirtColor);
    glBegin(GL_QUADS);
    glVertex2f(stickFigureX - shoulderWidth / 2, shoulderY);
    glVertex2f(stickFigureX - shoulderWidth / 2 + stickFigureSize * 0.1f, shoulderY);
    glVertex2f(stickFigureX - shoulderWidth / 2 + stickFigureSize * 0.05f, shoulderY - stickFigureSize * 0.2f);
    glVertex2f(stickFigureX - shoulderWidth / 2 - stickFigureSize * 0.05f, shoulderY - stickFigureSize * 0.2f);
    glEnd();
    glColor3fv(skinColor);
    glBegin(GL_QUADS);
    glVertex2f(stickFigureX - shoulderWidth / 2 - stickFigureSize * 0.05f, shoulderY - stickFigureSize * 0.2f);
    glVertex2f(stickFigureX - shoulderWidth / 2 + stickFigureSize * 0.05f, shoulderY - stickFigureSize * 0.2f);
    glVertex2f(stickFigureX - shoulderWidth / 2 + stickFigureSize * 0.02f, stickFigureY + stickFigureSize * 0.4f);
    glVertex2f(stickFigureX - shoulderWidth / 2 - stickFigureSize * 0.08f, stickFigureY + stickFigureSize * 0.4f);
    glEnd();
    glPushMatrix();
    glTranslatef(stickFigureX - shoulderWidth / 2 - stickFigureSize * 0.03f, stickFigureY + stickFigureSize * 0.38f, 0);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (int i = 0; i <= 360; i += 20) {
        float angle = M_PI * i / 180.0f;
        glVertex2f(stickFigureSize * 0.04f * cos(angle), stickFigureSize * 0.04f * sin(angle));
    }
    glEnd();
    glPopMatrix();
    float handDist = stickFigureSize * 0.5f;
    float handX = stickFigureX + 15 + cos(armAngle) * handDist;
    float handY = shoulderY + sin(armAngle) * handDist;
    float elbowX = stickFigureX + cos(armAngle) * handDist * 0.5f;
    float elbowY = shoulderY + sin(armAngle) * handDist * 0.5f;
    glColor3fv(shirtColor);
    glBegin(GL_QUADS);
    float perpAngle = armAngle + M_PI / 2;
    float armWidth = stickFigureSize * 0.06f;
    glVertex2f(stickFigureX + 10 + cos(perpAngle) * armWidth, shoulderY - 10 + sin(perpAngle) * armWidth);
    glVertex2f(stickFigureX + 10 - cos(perpAngle) * armWidth, shoulderY - 10 - sin(perpAngle) * armWidth);
    glVertex2f(elbowX + 10 - cos(perpAngle) * armWidth, elbowY - 10 - sin(perpAngle) * armWidth);
    glVertex2f(elbowX + 10 + cos(perpAngle) * armWidth, elbowY - 10 + sin(perpAngle) * armWidth);
    glEnd();
    glColor3fv(skinColor);
    glBegin(GL_QUADS);
    glVertex2f(elbowX + 10 + cos(perpAngle) * armWidth * 0.8, elbowY - 10 + sin(perpAngle) * armWidth * 0.8f);
    glVertex2f(elbowX + 10 - cos(perpAngle) * armWidth * 0.8f, elbowY - 10 - sin(perpAngle) * armWidth * 0.8f);
    glVertex2f(handX - cos(perpAngle) * armWidth * 0.8f, handY - sin(perpAngle) * armWidth * 0.8f);
    glVertex2f(handX + cos(perpAngle) * armWidth * 0.8f, handY + sin(perpAngle) * armWidth * 0.8f);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(handX, handY);
    for (int i = 0; i <= 360; i += 20) {
        float angle = M_PI * i / 180.0f;
        glVertex2f(handX + stickFigureSize * 0.05f * cos(angle), handY + stickFigureSize * 0.05f * sin(angle));
    }
    glEnd();
    glColor3f(0.1f, 0.1f, 0.1f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(stickFigureX - shoulderWidth / 4, shoulderY + stickFigureSize * 0.02f);
    glVertex2f(stickFigureX, shoulderY - stickFigureSize * 0.04f);
    glVertex2f(stickFigureX + shoulderWidth / 4, shoulderY + stickFigureSize * 0.02f);
    glEnd();
    glLineWidth(1.0f);
}
void drawBall() {
    glPushMatrix();
    glTranslatef(ballX, ballY, 0);
    glRotatef(ballRotationAngle, 0, 0, 1);
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 20; i++) {
        float angle = 2.0f * M_PI * i / 20;
        glVertex2f(ballRadius * cos(angle), ballRadius * sin(angle));
    }
    glEnd();
    glColor3f(1.0f, 0.5f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (int i = 0; i <= 20; i++) {
        float angle = 2.0f * M_PI * i / 20;
        glVertex2f(ballRadius * cos(angle), ballRadius * sin(angle));
    }
    glEnd();
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(-ballRadius * 0.95f, 0);
    glVertex2f(ballRadius * 0.95f, 0);
    glVertex2f(0, -ballRadius * 0.95f);
    glVertex2f(0, ballRadius * 0.95f);
    for (int i = 0; i < 2; i++) {
        float offset = (i == 0 ? 1 : -1) * ballRadius * 0.5f;
        glVertex2f(-ballRadius * 0.8f, offset);
        glVertex2f(ballRadius * 0.8f, offset);
    }
    glEnd();
    glPopMatrix();
    glLineWidth(1.0f);
}
void drawBasket() {
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(backboardLeftX, backboardBottomY);
    glVertex2f(backboardRightX, backboardBottomY);
    glVertex2f(backboardRightX, backboardTopY);
    glVertex2f(backboardLeftX, backboardTopY);
    glEnd();
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(backboardLeftX, backboardBottomY);
    glVertex2f(backboardRightX, backboardBottomY);
    glVertex2f(backboardRightX, backboardTopY);
    glVertex2f(backboardLeftX, backboardTopY);
    glEnd();
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(rimLeftX, rimBottomY);
    glVertex2f(rimRightX, rimBottomY);
    glVertex2f(rimRightX, rimTopY);
    glVertex2f(rimLeftX, rimTopY);
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(1.0f);
    float netDepth = basketHeight * 1.5f;
    float bottomWidth = basketWidth * 0.6f;
    float bottomLeftX = basketX - bottomWidth / 2.0f;
    float bottomRightX = basketX + bottomWidth / 2.0f;
    float bottomY = basketY - netDepth;
    glBegin(GL_LINES);
    int netSegments = 8;
    for (int i = 0; i <= netSegments; i++) {
        float topX = rimLeftX + i * (basketWidth / netSegments);
        float bottomX = bottomLeftX + i * (bottomWidth / netSegments);
        glVertex2f(topX, basketY);
        glVertex2f(bottomX, bottomY);
    }
    glEnd();
    glBegin(GL_LINES);
    int horizontalLines = 4;
    for (int i = 1; i <= horizontalLines; i++) {
        float y = basketY - (netDepth * i / (horizontalLines + 1));
        float progress = float(i) / (horizontalLines + 1);
        float levelWidth = basketWidth - progress * (basketWidth - bottomWidth);
        float levelLeftX = basketX - levelWidth / 2.0f;
        glVertex2f(levelLeftX, y);
        glVertex2f(levelLeftX + levelWidth, y);
    }
    glVertex2f(bottomLeftX, bottomY);
    glVertex2f(bottomRightX, bottomY);
    glEnd();
}
void drawPowerMeter() {
    if (gameState == AIMING && mousePressed) {
        float meterX = stickFigureX - 60;
        float meterY = stickFigureY + stickFigureSize * 1.2f;
        float meterWidth = 120;
        float meterHeight = 15;
        glColor3f(0.8f, 0.8f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(meterX, meterY);
        glVertex2f(meterX + meterWidth, meterY);
        glVertex2f(meterX + meterWidth, meterY + meterHeight);
        glVertex2f(meterX, meterY + meterHeight);
        glEnd();
        float fillWidth = meterWidth * (power / MAX_POWER);
        glColor3f(1.0f, 1.0f - (power / MAX_POWER), 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(meterX, meterY);
        glVertex2f(meterX + fillWidth, meterY);
        glVertex2f(meterX + fillWidth, meterY + meterHeight);
        glVertex2f(meterX, meterY + meterHeight);
        glEnd();
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(meterX, meterY);
        glVertex2f(meterX + meterWidth, meterY);
        glVertex2f(meterX + meterWidth, meterY + meterHeight);
        glVertex2f(meterX, meterY + meterHeight);
        glEnd();
    }
}
void drawScore() {
    glColor3f(0.1f, 0.1f, 0.1f);
    glRasterPos2f(20, WINDOW_HEIGHT - 30);
    std::ostringstream scoreStream;
    scoreStream << "Score: " << score;
    std::string scoreText = scoreStream.str();
    for (char c : scoreText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}
void updateGame(int value) {
    if (gameState == SHOOTING) {
        float prevBallX = ballX;
        float prevBallY = ballY;
        ballVelocityY -= GRAVITY;
        ballVelocityX *= FRICTION;
        ballVelocityY *= FRICTION;
        ballX += ballVelocityX;
        ballY += ballVelocityY;
        float netLeftX = rimLeftX + ballRadius;
        float netRightX = rimRightX - ballRadius;
        float netTopY = rimBottomY;
        float netBottomY = basketY - basketHeight * 1.5f;
        if (ballInNet) {
            if (ballX < netLeftX) {
                ballX = netLeftX;
                if (ballVelocityX < 0) ballVelocityX = 0;
            }
            if (ballX > netRightX) {
                ballX = netRightX;
                if (ballVelocityX > 0) ballVelocityX = 0;
            }
            if (ballY + ballRadius > netTopY) {
                ballY = netTopY - ballRadius;
                if (ballVelocityY > 0) ballVelocityY = 0;
            }
            if (ballY - ballRadius < netBottomY) {
                ballY = netBottomY + ballRadius;
                if (ballVelocityY < 0) ballVelocityY = 0;
            }
        }
        bool collisionOccurred = false;
        if (!justScored && checkScore()) {
            score++;
            justScored = true;
            gameState = SCORED;
            cheering = true;
            jumpCount = 0;
            ballInNet = true;
        }
        if (!collisionOccurred && checkAndHandleBackboardCollision()) {
            collisionOccurred = true;
            ballX = prevBallX + ballVelocityX;
        }
        if (!collisionOccurred && checkAndHandleRimCollision()) {
            collisionOccurred = true;
            ballX = prevBallX + ballVelocityX;
            ballY = prevBallY + ballVelocityY;
        }
        if (!collisionOccurred && !justScored && checkScore()) {
            score++;
            justScored = true;
            gameState = SCORED;
            ballInNet = true;
        }
        if (ballY - ballRadius < stickFigureY) {
            ballY = stickFigureY + ballRadius;
            if (ballVelocityY < -0.01f) {
                ballVelocityY = -ballVelocityY * BOUNCE_FACTOR_FLOOR;
                ballVelocityX *= 0.8f;
            }
            else {
                ballVelocityY = 0;
            }
            if (fabs(ballVelocityY) < MIN_VELOCITY_FOR_RESET && fabs(ballVelocityX) < MIN_VELOCITY_FOR_RESET && !justScored) {
                gameState = RESET;
            }
        }
        if (ballX - ballRadius < 0) {
            ballX = ballRadius;
            ballVelocityX = -ballVelocityX * BOUNCE_FACTOR_WALL;
        }
        if (ballX + ballRadius > WINDOW_WIDTH) {
            ballX = WINDOW_WIDTH - ballRadius;
            ballVelocityX = -ballVelocityX * BOUNCE_FACTOR_WALL;
        }
        if (ballY + ballRadius > WINDOW_HEIGHT) {
            ballY = WINDOW_HEIGHT - ballRadius;
            ballVelocityY = -ballVelocityY * BOUNCE_FACTOR_WALL;
        }
        if (gameState == SHOOTING) {
            ballRotationAngle += ballVelocityX * 5.0f;
            if (ballRotationAngle > 360.0f) ballRotationAngle -= 360.0f;
            if (ballRotationAngle < 0.0f) ballRotationAngle += 360.0f;
        }
    }
    else if (gameState == AIMING && ballInHand) {
    }
}
bool checkAndHandleRimCollision() {
    if (ballY - ballRadius < rimTopY && ballY - ballRadius > rimBottomY &&
        ballX > rimLeftX - ballRadius && ballX < rimRightX + ballRadius) {
        if (ballVelocityY < 0 && ballX > rimLeftX && ballX < rimRightX) {
            return false;
        }
        ballY = rimTopY + ballRadius;
        ballVelocityY = -ballVelocityY * BOUNCE_FACTOR_RIM;
        ballVelocityX *= 0.95f;
        return true;
    }
    float distSqLeft = (ballX - rimLeftX) * (ballX - rimLeftX) + (ballY - rimTopY) * (ballY - rimTopY);
    float distSqRight = (ballX - rimRightX) * (ballX - rimRightX) + (ballY - rimTopY) * (ballY - rimTopY);
    float radiusSq = ballRadius * ballRadius;
    if (distSqLeft < radiusSq || distSqRight < radiusSq) {
        float collisionPointX = (distSqLeft < radiusSq) ? rimLeftX : rimRightX;
        float collisionPointY = rimTopY;
        float normalX = ballX - collisionPointX;
        float normalY = ballY - collisionPointY;
        float dist = sqrt(normalX * normalX + normalY * normalY);
        if (dist < 1e-6) return false;
        normalX /= dist;
        normalY /= dist;
        float dotProduct = ballVelocityX * normalX + ballVelocityY * normalY;
        if (dotProduct < 0) {
            float reflectFactor = -(1.0f + BOUNCE_FACTOR_RIM) * dotProduct;
            ballVelocityX += reflectFactor * normalX;
            ballVelocityY += reflectFactor * normalY;
            float overlap = ballRadius - dist;
            ballX += normalX * (overlap + 0.1f);
            ballY += normalY * (overlap + 0.1f);
            return true;
        }
    }
    return false;
}
bool checkAndHandleBackboardCollision() {
    if (ballX + ballRadius > backboardLeftX &&
        ballX - ballRadius < backboardRightX &&
        ballY + ballRadius > backboardBottomY &&
        ballY - ballRadius < backboardTopY) {
        if (ballX < backboardLeftX && ballVelocityX > 0) {
            float overlap = (ballX + ballRadius) - backboardLeftX;
            ballX -= overlap + 0.1f;
            ballVelocityX = -ballVelocityX * BOUNCE_FACTOR_BACKBOARD;
        }
        else if (ballX > backboardRightX && ballVelocityX < 0) {
            float overlap = backboardRightX - (ballX - ballRadius);
            ballX += overlap + 0.1f;
            ballVelocityX = -ballVelocityX * BOUNCE_FACTOR_BACKBOARD;
        }
        else if (ballY > backboardTopY && ballVelocityY < 0) {
            float overlap = backboardTopY - (ballY - ballRadius);
            ballY += overlap + 0.1f;
            ballVelocityY = -ballVelocityY * BOUNCE_FACTOR_BACKBOARD;
        }
        else if (ballY < backboardBottomY && ballVelocityY > 0) {
            float overlap = (ballY + ballRadius) - backboardBottomY;
            ballY -= overlap + 0.1f;
            ballVelocityY = -ballVelocityY * BOUNCE_FACTOR_BACKBOARD;
        }
        else {
            float overlap = (backboardLeftX - (ballX + ballRadius));
            ballX += overlap - 0.1f;
            ballVelocityX = -fabs(ballVelocityX) * BOUNCE_FACTOR_BACKBOARD;
        }
        ballVelocityX *= 0.98f;
        ballVelocityY *= 0.98f;
        ballRotationAngle += ballVelocityX * 3.0f;
        return true;
    }
    return false;
}
bool checkScore() {
    if (ballVelocityY < 0) {
        float prevBallY = ballY - ballVelocityY;
        float prevBallTop = prevBallY + ballRadius;
        float currBallTop = ballY + ballRadius;
        if (prevBallTop >= rimBottomY && currBallTop < rimBottomY) {
            if (ballX > rimLeftX && ballX < rimRightX) {
                return true;
            }
        }
    }
    return false;
}
void shootBall() {
    if (!ballInHand) return;
    ballInHand = false;
    float initialSpeed = power * 0.15f;
    ballVelocityX = cos(armAngle) * initialSpeed;
    ballVelocityY = sin(armAngle) * initialSpeed;
    power = 0.0f;
}
void resetGame() {
    ballInHand = true;
    gameState = AIMING;
    justScored = false;
    ballInNet = false;
    float shoulderY = stickFigureY + stickFigureSize * 0.65f;
    float handDist = stickFigureSize * 0.5f + ballRadius * 0.5f;
    ballX = stickFigureX + cos(armAngle) * handDist;
    ballY = shoulderY + sin(armAngle) * handDist;
    ballVelocityX = 0.0f;
    ballVelocityY = 0.0f;
    power = 0.0f;
    randomizeBasket();
}
void randomizeBasket() {
    srand(static_cast<unsigned int>(time(0)));
    const int minX = 300;
    const int maxX = WINDOW_WIDTH - 300;
    basketX = rand() % (maxX - minX + 1) + minX;
    basketWidth = rand() % 40 + 40;
    basketHeight = rand() % 20 + 30;
    float backboardCenterY = (WINDOW_HEIGHT + 250 - 150) / 2.0f;
    backboardHeight = 70.0f;
    backboardBottomY = backboardCenterY - backboardHeight / 2.0f;
    backboardTopY = backboardCenterY + backboardHeight / 2.0f;
    float rimOffset = 5.0f;
    basketY = backboardBottomY + rimOffset + rimWidth / 2.0f;
    rimLeftX = basketX - basketWidth / 2.0f;
    rimRightX = basketX + basketWidth / 2.0f;
    rimTopY = basketY + rimWidth / 2.0f;
    rimBottomY = basketY - rimWidth / 2.0f;
    backboardLeftX = rimRightX;
    backboardRightX = backboardLeftX + backboardWidth;
}
void drawStands() {
    const int numRows = 5;
    const int seatsPerRow = 20;
    const float seatWidth = 35.0f;
    const float seatHeight = 10.0f;
    const float rowSpacing = 25.0f;
    const float startY = WINDOW_HEIGHT - 260;
    const float standsWidth = WINDOW_WIDTH;
    const float standsHeight = numRows * rowSpacing + 20.0f;
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned int>(time(0)));
        seeded = true;
    }
    glColor3f(0.35f, 0.35f, 0.4f);
    glBegin(GL_QUADS);
    glVertex2f(0, startY - standsHeight + 10.0f);
    glVertex2f(standsWidth, startY - standsHeight + 10.0f);
    glVertex2f(standsWidth, startY + 20.0f);
    glVertex2f(0, startY + 20.0f);
    glEnd();
    for (int row = 0; row < numRows; row++) {
        float y = startY - row * rowSpacing;
        glColor3f(0.75f, 0.75f, 0.75f);
        glBegin(GL_QUADS);
        glVertex2f(0, y - 5.0f);
        glVertex2f(standsWidth, y - 5.0f);
        glVertex2f(standsWidth, y + seatHeight);
        glVertex2f(0, y + seatHeight);
        glEnd();
        glColor3f(0.6f, 0.6f, 0.6f);
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        glVertex2f(0, y);
        glVertex2f(standsWidth, y);
        glEnd();
    }
    for (int row = 0; row < numRows; row++) {
        float y = startY - row * rowSpacing;
        for (int seat = 0; seat < seatsPerRow; seat++) {
            float x = seat * (seatWidth + 5.0f);
            glColor3f(0.6f, 0.3f, 0.1f);
            glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + seatWidth, y);
            glVertex2f(x + seatWidth, y + seatHeight);
            glVertex2f(x, y + seatHeight);
            glEnd();
            glColor3f(0.5f, 0.25f, 0.1f);
            glBegin(GL_QUADS);
            glVertex2f(x, y + seatHeight);
            glVertex2f(x + seatWidth, y + seatHeight);
            glVertex2f(x + seatWidth, y + seatHeight + 5.0f);
            glVertex2f(x, y + seatHeight + 5.0f);
            glEnd();
            float horizontalOffset = sin((row + seat) * 0.5f + jumpOffset * 0.1f) * 5.0f;
            float headRadius = seatWidth * 0.15f;
            int colorSeed = row * 1000 + seat * 10;
            float r = (0.2f + static_cast<float>((colorSeed * 123) % 600) / 1000.0f);
            float g = (0.2f + static_cast<float>((colorSeed * 456) % 600) / 1000.0f);
            float b = (0.2f + static_cast<float>((colorSeed * 789) % 600) / 1000.0f);
            glColor3f(r, g, b);
            glBegin(GL_QUADS);
            glVertex2f(x + seatWidth * 0.3f + horizontalOffset, y + seatHeight + jumpOffset);
            glVertex2f(x + seatWidth * 0.7f + horizontalOffset, y + seatHeight + jumpOffset);
            glVertex2f(x + seatWidth * 0.7f + horizontalOffset, y + seatHeight + seatHeight * 1.5f + jumpOffset);
            glVertex2f(x + seatWidth * 0.3f + horizontalOffset, y + seatHeight + seatHeight * 1.5f + jumpOffset);
            glEnd();
            glColor3f(0.85f, 0.68f, 0.5f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(x + seatWidth / 2 + horizontalOffset, y + seatHeight + seatHeight * 1.8f + jumpOffset);
            for (int i = 0; i <= 20; i++) {
                float angle = 2.0f * M_PI * i / 20;
                glVertex2f(x + seatWidth / 2 + horizontalOffset + headRadius * cos(angle),
                    y + seatHeight + seatHeight * 1.8f + jumpOffset + headRadius * sin(angle));
            }
            glEnd();
            glColor3f(r, g, b);
            glLineWidth(2.0f);
            glBegin(GL_LINES);
            glVertex2f(x + seatWidth * 0.3f + horizontalOffset, y + seatHeight + seatHeight + jumpOffset);
            glVertex2f(x + seatWidth * 0.1f + horizontalOffset, y + seatHeight + seatHeight * 1.3f + jumpOffset);
            glVertex2f(x + seatWidth * 0.7f + horizontalOffset, y + seatHeight + seatHeight + jumpOffset);
            glVertex2f(x + seatWidth * 0.9f + horizontalOffset, y + seatHeight + seatHeight * 1.3f + jumpOffset);
            glEnd();
        }
    }
    glLineWidth(1.0f);
}