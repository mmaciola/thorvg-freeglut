/*
 * This is an example of using FreeGLUT with Thorvg
 * The example will load svg image and display it to the window.
 * FreeGLUTThorvg, Created by Michal Maciola (m.maciola@samsung.com) on 17/11/2021.
 */

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include <thorvg.h>
#include <thread>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

// For benchmark
#define BENCHMARK

#ifdef BENCHMARK
#include <chrono>
#include "string.h"
#endif

using namespace std;
using namespace tvg;

#define WIDTH 800
#define HEIGHT 800
#define FPS 60
#define INTERVAL (1000/FPS)
#define MAX_SINGLE_PATH_SHAPE_COUNT 512

GLubyte* PixelBuffer = new GLubyte[WIDTH * HEIGHT * 4];

static CanvasEngine tvgEngine = CanvasEngine::Sw;
static unique_ptr<SwCanvas> swCanvas;
static bool needInvalidation = false;
static Shape* PathShape = nullptr;
static int PathShapeCounter = 0;
static int PathShapePoint[2] = {};

void drawDemoShape() {
    int x = WIDTH/2, y = HEIGHT/2;
    auto shape = Shape::gen();
    shape->moveTo(x, y);
    shape->stroke(255, 255, 0, 255);
    shape->stroke(4);
    
    for (int i = 0; i < 1000; i++) {
        x += (rand() % 100) - 50;
        y += (rand() % 100) - 50;
        if (x > WIDTH || x < 0) x = WIDTH/2;
        if (y > HEIGHT || y < 0) y = HEIGHT/2;
        shape->lineTo(x, y);
    }
    
    swCanvas->push(move(shape));
}

void createThorvgView(uint32_t threads) {
    // Initialize the engine
    if (Initializer::init(tvgEngine, threads) != Result::Success) {
        printf("Thorvg init failed: Engine is not supported\n");
        return;
    }
    
    // Create canvas using ABGR8888. Note: ABGR8888 is only partly supported, default is ARGB8888
    swCanvas = SwCanvas::gen();
    swCanvas->target((uint32_t *) PixelBuffer, WIDTH, WIDTH, HEIGHT, SwCanvas::ABGR8888);
    
    // Push white background
    auto shape = Shape::gen();
    shape->appendRect(0, 0, WIDTH, HEIGHT, 0, 0);
    shape->fill(255, 255, 255, 255);
    swCanvas->push(move(shape));
    
    // Push picture
    auto picture = Picture::gen();
    if (picture->load("tiger.svg") == Result::Success) {
        picture->size(WIDTH, HEIGHT);
        swCanvas->push(move(picture));
    }
    
    //drawDemoShape(); // DEMO
    
    // Call for invalidation
    needInvalidation = true;
}

#ifdef BENCHMARK
string getVmSizeValue() {
    FILE* file = fopen("/proc/self/status", "r");
    if (!file) return string("UNKNOWN");
    char* result = nullptr;
    char line[128];

    while (fgets(line, 128, file) != NULL) {
        if (strncmp(line, "VmSize:", 7) == 0) {
            result = line + 7;
            while (*result < '0' || *result > '9') result++;
            char* p = strchr(result, 'B');
            if (p) p[1] = '\0';
            break;
        }
    }
    fclose(file);
    return string(result);
}
#endif

// Draw the buffer
void display() {
#ifdef BENCHMARK
    auto begin = chrono::steady_clock::now();
#endif
    
    // Draw, will be synced later
    swCanvas->draw();
    
    // Sync thorvg drawing
    swCanvas->sync();
    
    // Clear the buffer (not needed as buffer is full window sized)
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Flip vertically, as (0,0) is in the topleft corner
    glRasterPos2f(-1,1);
    glPixelZoom(1, -1);
    
    // Draw pixels
    glDrawPixels(WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, PixelBuffer);
    
    // Flush
    glFlush();
    
#ifdef BENCHMARK
    auto end = chrono::steady_clock::now();
    //auto memory = getVmSizeValue();
    //cout << "Time difference = " << chrono::duration_cast<chrono::microseconds>(end - begin).count() << "[µs]" /*<< ", Memory used = " << memory*/ << endl;
    
    cout << PathShapeCounter << "\t" << chrono::duration_cast<chrono::microseconds>(end - begin).count() << endl;
#endif
}

// Handle mouse events
void mouse(int button, int state, int x, int y) {
    // Handle only left mouse button
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            // Store begin mouse position
            PathShapePoint[0] = x; PathShapePoint[1] = y;
            PathShape = Shape::gen().release();
            swCanvas->push(unique_ptr<Shape>(PathShape));
            PathShape->moveTo(x, y);
            PathShape->stroke(255, 0, 0, 255);
            PathShape->stroke(4);
        } else {
            // Reset rect
            PathShape = nullptr;
            PathShapeCounter = 0;
        }
    }
}
void mouseMotion(int mx, int my) {
    // Draw line
    if (PathShapeCounter < MAX_SINGLE_PATH_SHAPE_COUNT) {
        PathShapePoint[0] = mx; PathShapePoint[1] = my;
        PathShape->lineTo(mx, my);
        swCanvas->update(PathShape);
        PathShapeCounter += 1;
    } else {
        PathShape = Shape::gen().release();
        swCanvas->push(unique_ptr<Shape>(PathShape));
        PathShape->moveTo(PathShapePoint[0], PathShapePoint[1]);
        PathShape->lineTo(mx, my);
        PathShape->stroke(255, 0, 0, 255);
        PathShape->stroke(4);
        PathShapeCounter = 0;
    }
    
    // Call for invalidation
    needInvalidation = true;
}

// Handle the timer event
void timer(int value) {
    // Invalidate if needed
    if (needInvalidation) {
        glutPostRedisplay();
        needInvalidation = false;
    }
    
    // Call the function timer() after INTERVAL time
    glutTimerFunc(INTERVAL, timer, value);
}

// Handle close event
void close() {
    swCanvas->clear(false);
    Initializer::term(tvgEngine);
}

// Initializes GLUT, the display mode, and main window
int main(int argc, char** argv) {
    // Use a single buffered window in RGB mode
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

    // Position, size and entitle window
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("FreeGLUT with Thorvg");
    
    // Create thorvg view
    auto threads = thread::hardware_concurrency();
    if (threads > 0) --threads;
    if (argc >= 2) {
        threads = atoi(argv[1]);
    }
    cout << "Using " << threads << " threads" << endl;
    createThorvgView(threads);

    // Call the function display() on GLUT window repaint
    glutDisplayFunc(display);
    
    // Call the function mouse() on mouse button pressed and mouseMotion() on mouse move with pressed button
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    
    // Call the function timer() after INTERVAL time
    glutTimerFunc(INTERVAL, timer, 0);
    
    // Set closing function
    glutWMCloseFunc(close);

    // Start main loop
    glutMainLoop();
}
