# thorvg-freeglut
This is an example of using FreeGLUT with Thorvg. The example will load svg image and display it to the window.

<img width="428" alt="preview" src="https://user-images.githubusercontent.com/71131832/142219631-6e1aaad0-75be-4374-bcb2-905a6a10fe51.png">

## Installation
- Install freeglut: `brew install freeglut`
- Install thorvg (https://github.com/Samsung/thorvg): `meson build; ninja -C build install`
- Link Binaries with libraries (Build Phases): add `OpenGL.framework`, `thorvg/build/src/libthorvg.0.dylib` and `/usr/local/Cellar/freeglut/3.2.1_1/lib/libglut.3.11.0.dylib` 
- Add search paths (Build Setting):
  - `/usr/local/Cellar/freeglut/3.2.1_1/lib` and `/usr/local/include` for `Header Search Paths`
  - `/usr/local/Cellar/freeglut/3.2.1_1/lib` and `/usr/local/lib` for `Library Search Paths`

## Tips
Error `freeglut (...): failed to open display ''` -> install XQuartz (https://www.xquartz.org)

Warnings `OpenGL is deprecated. Consider migrating to Metal instead.` -> Disable Deprecated Functions in xcode (Build Settings -> Apple Clang - Warnings - All languages -> Deprecated Functions -> No)
