# For Fun Graphics

Just a fun side project for exploring 3D computer graphics. A Win32 app that reads in .obj files,
projects them to a first-person perspective, and displays them using Direct2D to draw to the screen.
Also allows for object translation/rotation, and moving the PoV in real-time.

Current progress demo (click for video):

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/7DC83wx4E7U/0.jpg)](https://www.youtube.com/watch?v=7DC83wx4E7U)


## Features
* Read in 3D objects from .obj files
* Project 3D objects to first-person PoV, drawn on screen using Direct2D
* Movable/rotatable PoV, controlled with keyboard
* Add real-time spin and translation to objects
* FPS counter

## Potential Future Work
* Performance improvements
* GPU acceleration
* Face shading
* Foreground/background differentiation
* Physics
* Support more .obj file features
* More interactibility

## References
* https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix.html
* https://learn.microsoft.com/en-us/windows/win32/learnwin32/learn-to-program-for-windows
* https://learn.microsoft.com/en-us/windows/win32/direct2d/reference
