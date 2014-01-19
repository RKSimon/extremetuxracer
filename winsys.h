/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tuxracer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifndef WINSYS_H
#define WINSYS_H

#include "bh.h"

#define NUM_RESOLUTIONS 10

typedef void (*TInitFuncN)    (void);
typedef void (*TLoopFuncN)    (double time_step);
typedef void (*TTermFuncN)    (void);
typedef void (*TMouseFuncN)   (int button, int state, int x, int y);
typedef void (*TMotionFuncN)  (int x, int y);
typedef void (*TKeybFuncN)    (unsigned int key, bool special, bool release, int x, int y);
typedef void (*TJAxisFuncN)   (int axis, double value);
typedef void (*TJButtFuncN)   (int button, int state);
typedef void (*TKeybFuncS)    (SDL_Keysym sym, bool release);

typedef struct {
	TInitFuncN   init; 
	TLoopFuncN   loop; 
	TTermFuncN   term;
	TKeybFuncN   keyb;
	TMouseFuncN  mouse;
	TMotionFuncN motion;
	TJAxisFuncN  jaxis;
	TJButtFuncN  jbutt;
	TKeybFuncS   keyb_spec;
} TModeFuncsN;

class CWinsys {
private:
	// time
	float clock_time;
	float cur_time;
	float lasttick;
	float elapsed_time;
	int remain_ticks;

	// joystick
	SDL_Joystick *joystick;	
	int numJoysticks;
	bool joystick_active;

	// sdl window
 	TScreenRes resolution[NUM_RESOLUTIONS];
	int auto_x_resolution;
	int auto_y_resolution;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_GLContext context;
	TScreenRes MakeRes (int width, int height);
	double CalcScreenScale ();

	// modes and loop
	TModeFuncsN modefuncs [NUM_GAME_MODES];
	TGameMode new_mode;
	void IdleFunc ();
	void PollEvent ();
	void ChangeMode ();
	void CallLoopFunction ();
public:
	CWinsys ();
	~CWinsys ();

	// sdl window
	TScreenRes GetResolution (int idx);
	string GetResName (int idx);
	void Init ();
	void SetupVideoMode (TScreenRes resolution);
	void SetupVideoMode (int idx);
	void SetupVideoMode (int width, int height);
	void KeyRepeat (bool repeat);
	void SetFonttype ();
	void PrintJoystickInfo ();
	void ShowCursor (bool visible) {SDL_ShowCursor (visible);}
	void SwapBuffers ();
	void Delay (unsigned int ms);
	void Quit ();
	void InitJoystick ();
	void CloseJoystick ();
	void SetModeFuncs (
			TGameMode mode, TInitFuncN init, TLoopFuncN loop, TTermFuncN term,
			TKeybFuncN keyb, TMouseFuncN mouse, TMotionFuncN motion,
 			TJAxisFuncN jaxis, TJButtFuncN jbutt, TKeybFuncS keyb_spec);
	void EventLoop ();
	void SetMode (TGameMode mode) {new_mode = mode;}
	bool ModePending ();
	double ClockTime () {return SDL_GetTicks() * 1.e-3; } 
};

extern CWinsys Winsys;

#endif
