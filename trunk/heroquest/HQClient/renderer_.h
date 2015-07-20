//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: renderer.qt.hh                                             |
//  ==================================================================

#ifndef RENDERER_HH
#define RENDERER_HH

#include <math.h>
#include "SDL.h"
#include <iostream>
#include <list>

#include "vector.hh"

using std::list;

class InputField;
class Button;
class Inventory;

//! gives an OpenGL drawing area from SDL
/*! Besides drawing the scene, it handles also key and mouse events
 */
class Renderer{

public:
  //! create a new renderer
  Renderer();
  //! Copy Constructor
  Renderer(const Renderer& r);
  //! Destructor
  ~Renderer();
  //! setup OpenGL state machine for this render context
  void initializeGL();
  //! viewport has been resized
  void resizeGL( int w, int h );
  //! draw the entire scene
  void paintGL();
  //! quits the application
  void quit();
  //! if the renderer has finished
  inline bool isFinished() {return quit_;}
  //! calculates frame rate
  void calculateFrameRate();
  //! returns the time interval of the last frame
  float getFrameInterval() {return frameInterval_;}
  //! returns frames per second
  short getFPS() {return fps_;}
  
  //! handles pressed keys immediately
  void handleKeys();
  
  //! adds an input field for mouse click checking
  void addInputListener(InputField* in, bool immediate=true){if (immediate)listeners_.push_back(in);
  	else newIn_.push_back(in);}
  //! adds a button for mouse click checking
  void addButtonListener(Button* button, bool immediate=true){if (immediate)buttons_.push_back(button);
  	else newBut_.push_back(button);}
  //! remove the input listener at position idx
  void removeInputListener(int idx);
  //! remove the button listener at position idx
  void removeButtonListener(int idx);
  //! remove all button listeners beginning at idx
  void removeButtonListeners(int idx);
  //! clears the input and button listeners
  void clearListeners(bool immediate=true);
  //! displays main ingame menu
  void mainMenu();
  
  //! a key is pressed down
  void keyPressEvent(SDL_keysym *key);
  //1 a key is released
  void keyReleaseEvent(SDL_keysym *key);
  //! the mouse has been pressed
  void mousePressEvent(SDL_MouseButtonEvent *m);
  //! the mouse has been released
  void mouseReleaseEvent(SDL_MouseButtonEvent *m);
  //! the mouse has been moved
  void mouseMoveEvent(SDL_MouseMotionEvent *m);
  //! ignore the next mouse move
  inline void ignoreNextMouse() {ignoreMouse_ = true;}
  //! set the view to 3D
  void setViewTo3D(bool threeD);

  //! get current mouse position
  inline Vector2D getMousePos() {return Vector2D(mousex_, mousey_);}
  //! reset mouse position
  inline void resetMousePos() {mousex_ = -1; mousey_ = -1;}

#include "menu.hh"

protected:
  //! change to orthographic projection with virtual screen size 1024x768
  void ortho();
  //! change the projection back to perspective
  void back();

  //! set user event in event queue
  void setEvent();

  //! where should the Keyboard Input go to?
  InputField* input_;
  //! waiting input fields
  list<InputField*> listeners_;
  //! waiting buttons
  list<Button*> buttons_;
  //! should the GUI be cleared
  bool clear_;
  //! the new input fields
  list<InputField*> newIn_;
  //! the new buttons
  list<Button*> newBut_;

  //! dimension of current viewport
  int width_, height_;
  //! aspect ratio
  float aspect_;
  //! field of view
  float fieldOV_;
  //! render 3d
  bool threeD_;
  //! if the renderer should quit
  bool quit_;

  //! the time interval of the current frame
  float frameInterval_;
  //! frames per second as float
  float framesPerSecond_;
  //! the last frame time
  float lastTime_;
  //! the current frame time
  float frameTime_;
  //! frames per second as integer
  short fps_;

  //! should the mouse be movable and visible
  bool mousePointer_;
  //! mouse position
  int mousex_, mousey_;
  //! clicked field in 2D map
  Vector2D cf_;
  //! mouse press and release control
  bool buttonPressed_;
  //! if map click is awaited
  void (Renderer::*awaitMapClick_)(Vector2D click);
  //! contains key state for all keys (pressed or not)
  bool keys_[256];
  //! security question before quitting application
  bool ask_;
  //! should the next mouse move be ignored
  bool ignoreMouse_;

  //! the inventory that should be rendered
  Inventory* inventory_;
};

extern Renderer* gl;

#endif
