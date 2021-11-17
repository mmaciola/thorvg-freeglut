/*
 * This is an example of using FreeGLUT with Thorvg
 * The example will load svg image and display it to the window.
 * FreeGLUTThorvg, Created by Michal Maciola (m.maciola@samsung.com) on 17/11/2021.
 */

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <thorvg.h>
#include <thread>

using namespace std;
using namespace tvg;

#define WIDTH 400
#define HEIGHT 400

GLubyte* PixelBuffer = new GLubyte[WIDTH * HEIGHT * 4];

static unique_ptr<SwCanvas> swCanvas;

void createThorvgView() {
    // Initialize the engine
    CanvasEngine tvgEngine = CanvasEngine::Sw;
    auto threads = thread::hardware_concurrency();
    if (threads > 0) --threads;
    
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
    if (picture->load("/Users/mtm/Development/samsung/thorvg-mm/src/examples/images/tiger.svg") != Result::Success) return;
    picture->size(WIDTH, HEIGHT);
    swCanvas->push(move(picture));
    
    // Draw
    if (swCanvas->draw() == Result::Success) {
        swCanvas->sync();
    }
}

// Draw the buffer
void display() {
    // Clear the buffer (not needed as buffer is full window sized)
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Flip vertically, as (0,0) is in the topleft corner
    glRasterPos2f(-1,1);
    glPixelZoom(1, -1);
    
    // Draw pixels
    glDrawPixels(WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, PixelBuffer);
    
    // Flush
    glFlush();
}

// Initializes GLUT, the display mode, and main window; registers callbacks;
// enters the main event loop.
int main(int argc, char** argv) {
    // Use a single buffered window in RGB mode
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

    // Position, size and entitle window
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("FreeGLUT with Thorvg");
    
    // Create thorvg view
    createThorvgView();

    // Call the function display() on GLUT window repaint
    glutDisplayFunc(display);

    // Start main loop
    glutMainLoop();
}
