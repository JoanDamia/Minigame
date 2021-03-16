// -------------------------------------------------------------------------
// Awesome simple game with SDL
// Lesson 2 - Input Events
//
// SDL API: http://wiki.libsdl.org/APIByCategory
// -------------------------------------------------------------------------

#include <stdio.h>			// Required for: printf()
#include <stdlib.h>			// Required for: EXIT_SUCCESS
#include <math.h>			// Required for: sinf(), cosf()
#include <time.h>

// Include SDL libraries
#include "SDL/include/SDL.h"				// Required for SDL base systems functionality
#include "SDL_image/include/SDL_image.h"	// Required for image loading functionality
#include "SDL_mixer/include/SDL_mixer.h"	// Required for audio loading and playing functionality

// Define libraries required by linker
// WARNING: Not all compilers support this option and it couples 
// source code with build system, it's recommended to keep both 
// separated, in case of multiple build configurations
//#pragma comment(lib, "SDL/lib/x86/SDL2.lib")
//#pragma comment(lib, "SDL/lib/x86/SDL2main.lib")
//#pragma comment(lib, "SDL_image/lib/x86/SDL2_image.lib")
//#pragma comment( lib, "SDL_mixer/libx86/SDL2_mixer.lib" )

// -------------------------------------------------------------------------
// Defines, Types and Globals
// -------------------------------------------------------------------------
#define SCREEN_WIDTH		901
#define SCREEN_HEIGHT		901

#define MAX_KEYBOARD_KEYS	 256
#define MAX_MOUSE_BUTTONS	   5
#define JOYSTICK_DEAD_ZONE  8000

#define SHIP_SPEED			   8
#define MAX_SHIP_SHOTS		 32
#define SHOT_SPEED		12
#define SCROLL_SPEED		   19

enum WindowEvent
{
	WE_QUIT = 0,
	WE_HIDE,
	WE_SHOW,
	WE_COUNT
};

enum KeyState
{
	KEY_IDLE = 0,		// DEFAULT
	KEY_DOWN,			// PRESSED (DEFAULT->DOWN)
	KEY_REPEAT,			// KEEP DOWN (sustained)
	KEY_UP				// RELEASED (DOWN->DEFAULT)
};
enum GameScreen
{
	TITLE = 0,
	GAMEPLAY,
	ENDING
};

struct Projectile
{
	int x, y;
	bool alive;
};

// Global context to store our game state data
struct GlobalState
{
	// Window and renderer
	SDL_Window* window;
	SDL_Surface* surface;
	SDL_Renderer* renderer;

	// Input events
	KeyState* keyboard;
	KeyState mouse_buttons[MAX_MOUSE_BUTTONS];
	int mouse_x;
	int mouse_y;
	SDL_Joystick* gamepad;
	int gamepad_axis_x_dir;
	int gamepad_axis_y_dir;
	bool window_events[WE_COUNT];

	// Texture variables
	SDL_Texture* background;
	SDL_Texture* ship;
	SDL_Texture* shot;
	SDL_Texture* gameover;
	SDL_Texture* playgame;
	int background_width;
	int background_height;

	// Audio variables
	Mix_Music* music;
	Mix_Music* ending;

	// Game elements
	int ship_x;
	int ship_y;
	int scroll;
	Projectile shots[MAX_SHIP_SHOTS];
	int last_shot;
	int shot_w;
	int shot_h;
	
	
	int ship_w;
	int ship_h;

	GameScreen currentScreen;		// 0-LOGO, 1-TITLE, 2-GAMEPLAY, 3-ENDING
};

// Global game state variable
GlobalState state;
int Contador = 0;

// Functions Declarations
// Some helpful functions to draw basic shapes
// -------------------------------------------------------------------------
static void DrawRectangle(int x, int y, int width, int height, SDL_Color color);
static void Spawn(int x, int y, int width, int height, SDL_Color color);
static void DrawLine(int x1, int y1, int x2, int y2, SDL_Color color);
static void DrawCircle(int x, int y, int radius, SDL_Color color);

// Functions Declarations and Definition
// -------------------------------------------------------------------------
void Start()
{

	// Initialize SDL internal global state
	SDL_Init(SDL_INIT_EVERYTHING);

	// Init input events system
	//if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0) printf("SDL_EVENTS could not be initialized! SDL_Error: %s\n", SDL_GetError());

	// Init window
	state.window = SDL_CreateWindow("2021: Space Odyssey", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	state.surface = SDL_GetWindowSurface(state.window);

	// Init renderer
	state.renderer = SDL_CreateRenderer(state.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawColor(state.renderer, 100, 149, 237, 255);		// Default clear color: Cornflower blue

	// L2: DONE 1: Init input variables (keyboard, mouse_buttons)
	state.keyboard = (KeyState*)calloc(sizeof(KeyState) * MAX_KEYBOARD_KEYS, 1);
	for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) state.mouse_buttons[i] = KEY_IDLE;

	// L2: DONE 2: Init input gamepad 
	// Check SDL_NumJoysticks() and SDL_JoystickOpen()
	if (SDL_NumJoysticks() < 1) printf("WARNING: No joysticks connected!\n");
	else
	{
		state.gamepad = SDL_JoystickOpen(0);
		if (state.gamepad == NULL) printf("WARNING: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
	}

	// Init image system and load textures
	IMG_Init(IMG_INIT_PNG);
	state.background = SDL_CreateTextureFromSurface(state.renderer, IMG_Load("Assets/Definitivisimo.png"));
	state.ship = SDL_CreateTextureFromSurface(state.renderer, IMG_Load("Assets/ship.png"));
	state.shot = SDL_CreateTextureFromSurface(state.renderer, IMG_Load("Assets/shot.png"));
	state.gameover = SDL_CreateTextureFromSurface(state.renderer, IMG_Load("Assets/Game_Over.png"));
	state.playgame = SDL_CreateTextureFromSurface(state.renderer, IMG_Load("Assets/Play_Game.png"));
	SDL_QueryTexture(state.background, NULL, NULL, &state.background_width, &state.background_height);

	// L4: TODO 1: Init audio system and load music/fx
	// EXTRA: Handle the case the sound can not be loaded!
	Mix_Init(MIX_INIT_OGG);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
	state.music = Mix_LoadMUS("Assets/Music.ogg");
	state.ending = Mix_LoadMUS("Assets/final.ogg");
	// L4: TODO 2: Start playing loaded music
	Mix_PlayMusic(state.music, -1);


	// Init game variables
	state.ship_x = SCREEN_WIDTH / 2;
	state.ship_y = SCREEN_HEIGHT / 1.3;
	state.scroll = 0;
	state.last_shot = 0;
	
	
	state.ship_w = 64;
	state.ship_h = 64;

	srand(time(NULL));
	if (state.last_shot == MAX_SHIP_SHOTS) state.last_shot = 0;

	for (int i = 0; i < MAX_SHIP_SHOTS; ++i)
	{




		srand(time(NULL));
		if (state.last_shot == MAX_SHIP_SHOTS) state.last_shot = 0;

		int a;

		a = rand() % 5;

		switch (a) {
		case 0:

			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 286;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 406;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 526;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 646;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;


			break;

		case 1:
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 166;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;

			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 406;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 526;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 646;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;


			break;
		case 2:
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 166;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 286;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;

			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 526;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 646;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;

			break;
		case 3:
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 166;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 286;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 406;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;

			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 646;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;

			break;
		case 4:
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 166;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 286;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 406;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;
			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = 526;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;



			break;


		}


	}Contador++;

}

// ----------------------------------------------------------------
void Finish()
{
	// L4: TODO 3: Unload music/fx and deinitialize audio system
	Mix_FreeMusic(state.music);
	Mix_CloseAudio();
	Mix_Quit();

	// Unload textures and deinitialize image system
	SDL_DestroyTexture(state.background);
	SDL_DestroyTexture(state.ship);
	IMG_Quit();

	// L2: DONE 3: Close game controller
	SDL_JoystickClose(state.gamepad);
	state.gamepad = NULL;

	// Deinitialize input events system
	//SDL_QuitSubSystem(SDL_INIT_EVENTS);

	// Deinitialize renderer and window
	// WARNING: Renderer should be deinitialized before window
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);

	// Deinitialize SDL internal global state
	SDL_Quit();

	// Free any game allocated memory
	free(state.keyboard);
}

// ----------------------------------------------------------------
bool CheckInput()
{
	// Update current mouse buttons state 
	// considering previous mouse buttons state
	for (int i = 0; i < MAX_MOUSE_BUTTONS; ++i)
	{
		if (state.mouse_buttons[i] == KEY_DOWN) state.mouse_buttons[i] = KEY_REPEAT;
		if (state.mouse_buttons[i] == KEY_UP) state.mouse_buttons[i] = KEY_IDLE;
	}

	// Gather the state of all input devices
	// WARNING: It modifies global keyboard and mouse state but 
	// its precision may be not enough
	//SDL_PumpEvents();

	// Poll any currently pending events on the queue,
	// including 'special' events like window events, joysticks and 
	// even hotplug events for audio devices and joysticks,
	// you can't get those without inspecting event queue
	// SDL_PollEvent() is the favored way of receiving system events
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		switch (event.type)
		{
		case SDL_QUIT: state.window_events[WE_QUIT] = true; break;
		case SDL_WINDOWEVENT:
		{
			switch (event.window.event)
			{
				//case SDL_WINDOWEVENT_LEAVE:
			case SDL_WINDOWEVENT_HIDDEN:
			case SDL_WINDOWEVENT_MINIMIZED:
			case SDL_WINDOWEVENT_FOCUS_LOST: state.window_events[WE_HIDE] = true; break;
				//case SDL_WINDOWEVENT_ENTER:
			case SDL_WINDOWEVENT_SHOWN:
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			case SDL_WINDOWEVENT_MAXIMIZED:
			case SDL_WINDOWEVENT_RESTORED: state.window_events[WE_SHOW] = true; break;
			case SDL_WINDOWEVENT_CLOSE: state.window_events[WE_QUIT] = true; break;
			default: break;
			}
		} break;
		// L2: DONE 4: Check mouse events for button state
		case SDL_MOUSEBUTTONDOWN: state.mouse_buttons[event.button.button - 1] = KEY_DOWN; break;
		case SDL_MOUSEBUTTONUP:	state.mouse_buttons[event.button.button - 1] = KEY_UP; break;
		case SDL_MOUSEMOTION:
		{
			state.mouse_x = event.motion.x;
			state.mouse_y = event.motion.y;
		} break;
		case SDL_JOYAXISMOTION:
		{
			// Motion on controller 0
			if (event.jaxis.which == 0)
			{
				// X axis motion
				if (event.jaxis.axis == 0)
				{
					if (event.jaxis.value < -JOYSTICK_DEAD_ZONE) state.gamepad_axis_x_dir = -1;
					else if (event.jaxis.value > JOYSTICK_DEAD_ZONE) state.gamepad_axis_x_dir = 1;
					else state.gamepad_axis_x_dir = 0;
				}
				// Y axis motion
				else if (event.jaxis.axis == 1)
				{
					if (event.jaxis.value < -JOYSTICK_DEAD_ZONE) state.gamepad_axis_y_dir = -1;
					else if (event.jaxis.value > JOYSTICK_DEAD_ZONE) state.gamepad_axis_y_dir = 1;
					else state.gamepad_axis_y_dir = 0;
				}
			}
		} break;
		default: break;
		}
	}

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	// L2: DONE 5: Update keyboard keys state
	// Consider previous keys states for KEY_DOWN and KEY_UP
	for (int i = 0; i < MAX_KEYBOARD_KEYS; ++i)
	{
		// A value of 1 means that the key is pressed and a value of 0 means that it is not
		if (keys[i] == 1)
		{
			if (state.keyboard[i] == KEY_IDLE) state.keyboard[i] = KEY_DOWN;
			else state.keyboard[i] = KEY_REPEAT;
		}
		else
		{
			if (state.keyboard[i] == KEY_REPEAT || state.keyboard[i] == KEY_DOWN) state.keyboard[i] = KEY_UP;
			else state.keyboard[i] = KEY_IDLE;
		}
	}

	// L2: DONE 6: Check ESCAPE key pressed to finish the game
	if (state.keyboard[SDL_SCANCODE_ESCAPE] == KEY_DOWN) return false;

	// Check QUIT window event to finish the game
	if (state.window_events[WE_QUIT] == true) return false;

	return true;
}

// ----------------------------------------------------------------
void MoveStuff()
{
	switch (state.currentScreen)
	{
	case TITLE:
	{
		Mix_FadeOutMusic(100);
		if (state.keyboard[SDL_SCANCODE_RETURN] == KEY_DOWN) {
			state.currentScreen = GAMEPLAY;
			Mix_PlayMusic(state.music, -1);
		}
	} break;
	case GAMEPLAY:
	{
		if ((state.ship_x >= 155) && (state.ship_x <= 680)) {
		if (state.keyboard[SDL_SCANCODE_LEFT] == KEY_REPEAT) state.ship_x -= SHIP_SPEED;
		else if (state.keyboard[SDL_SCANCODE_RIGHT] == KEY_REPEAT) state.ship_x += SHIP_SPEED;
		//if (state.keyboard[SDL_SCANCODE_UP] == KEY_REPEAT) state.ship_y -= SHIP_SPEED;
		//else if (state.keyboard[SDL_SCANCODE_DOWN] == KEY_REPEAT) state.ship_y += SHIP_SPEED;
	}
	else if (state.ship_x < 155 && state.keyboard[SDL_SCANCODE_RIGHT] == KEY_REPEAT && state.keyboard[SDL_SCANCODE_LEFT] == KEY_IDLE) {
		state.ship_x = 155;
	}
	else if (state.ship_x > 155 && state.keyboard[SDL_SCANCODE_LEFT] == KEY_REPEAT && state.keyboard[SDL_SCANCODE_RIGHT] == KEY_IDLE) {
		state.ship_x = 680;
	}

		for (int i = 0; i < MAX_SHIP_SHOTS; ++i)
		{
			if (state.shots[i].alive)
			{
				if (state.shots[i].y < SCREEN_HEIGHT) { state.shots[i].y += SHOT_SPEED; }
				else if (state.shots[i].y > SCREEN_HEIGHT + 100) { state.shots[i].alive = false; }
				else {


					srand(time(NULL));
					if (state.last_shot == MAX_SHIP_SHOTS) state.last_shot = 0;

					int a;

					a = rand() % 5;

					switch (a) {
					case 0:

						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 286;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 406;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 526;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 646;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;



						break;

					case 1:
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 166;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;

						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 406;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 526;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 646;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;



						break;
					case 2:
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 166;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 286;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;

						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 526;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 646;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;


						break;
					case 3:
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 166;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 286;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 406;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;

						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 646;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;

						break;
					case 4:
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 166;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 286;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 406;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;
						state.shots[state.last_shot].alive = true;
						state.shots[state.last_shot].x = 526;
						state.shots[state.last_shot].y = -20;
						state.last_shot++;
						break;






					}
				}
			}
			if (state.ship_x< state.shots[i].x + state.shot_w && state.ship_x + state.ship_w>state.shots[i].x && state.ship_y<state.shots[i].y + state.shot_h && state.ship_h + state.ship_y>state.shots[i].y)
			{
				state.currentScreen = ENDING;
				Mix_FadeOutMusic(100);
				Mix_PlayMusic(state.ending, -1);
			}
		}



		// L4: TODO 4: Play sound fx_shoot
		if (state.keyboard[SDL_SCANCODE_RETURN] == KEY_DOWN) {
			
		}
	}break;
	// Update active shots



	case ENDING:
	{
		if (state.keyboard[SDL_SCANCODE_RETURN] == KEY_DOWN) {
			state.currentScreen = TITLE;

		}
	} break;
	default: break;
	}
}




// ----------------------------------------------------------------
void Draw()
{
	// Clear screen to Cornflower blue
	SDL_SetRenderDrawColor(state.renderer, 100, 149, 237, 255);
	SDL_RenderClear(state.renderer);

	switch (state.currentScreen)
	{
	case TITLE:
	{
		SDL_Rect rec = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		SDL_RenderCopy(state.renderer, state.playgame, NULL, &rec);
	} break;
	case GAMEPLAY:
	{
		// Draw background and scroll
		state.scroll -= SCROLL_SPEED;
		if (state.scroll <= 0)	state.scroll = state.background_height;

		// Draw background texture (two times for scrolling effect)
		// NOTE: rec rectangle is being reused for next draws
		SDL_Rect rec = { 0, -state.scroll, state.background_width, state.background_height };
		SDL_RenderCopy(state.renderer, state.background, NULL, &rec);
		rec.y += state.background_height;
		SDL_RenderCopy(state.renderer, state.background, NULL, &rec);

		// Draw ship rectangle
		//DrawRectangle(state.ship_x, state.ship_y, 250, 100, { 255, 0, 0, 255 });

		// Draw ship texture
		rec.x = state.ship_x; rec.y = state.ship_y; rec.w = 64; rec.h = 64;
		SDL_RenderCopy(state.renderer, state.ship, NULL, &rec);

		// L2: DONE 9: Draw active shots
		rec.w = 86; rec.h = 124;
		for (int i = 0; i < MAX_SHIP_SHOTS; ++i)
		{
			if (state.shots[i].alive)
			{
				//DrawRectangle(state.shots[i].x, state.shots[i].y, 50, 20, { 0, 250, 0, 255 });
				rec.x = state.shots[i].x; rec.y = state.shots[i].y;
				SDL_RenderCopy(state.renderer, state.shot, NULL, &rec);
			}
		}

	} break;
	case ENDING:
	{
		SDL_Rect rec = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		SDL_RenderCopy(state.renderer, state.gameover, NULL, &rec);
	} break;
	default: break;
	}

	// Finally present framebuffer
	SDL_RenderPresent(state.renderer);
}


// Main Entry point
// -------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	Start();

	while (CheckInput())
	{
		MoveStuff();

		Draw();
	}

	Finish();

	return(EXIT_SUCCESS);
}

// Functions Definition
// -------------------------------------------------------------------------
void DrawRectangle(int x, int y, int width, int height, SDL_Color color)
{
	SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(state.renderer, color.r, color.g, color.b, color.a);

	SDL_Rect rec = { x, y, width, height };

	int result = SDL_RenderFillRect(state.renderer, &rec);

	if (result != 0) printf("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
}
// -------------------------------------------------------------------------
void Spawn(int x, int y, int width, int height, SDL_Color color) {
	SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(state.renderer, color.r, color.g, color.b, color.a);

	SDL_Rect rec = { x, y, width, height };

	int result = SDL_RenderFillRect(state.renderer, &rec);

	if (result != 0) printf("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
}
// ----------------------------------------------------------------
void DrawLine(int x1, int y1, int x2, int y2, SDL_Color color)
{
	SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(state.renderer, color.r, color.g, color.b, color.a);

	int result = SDL_RenderDrawLine(state.renderer, x1, y1, x2, y2);

	if (result != 0) printf("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
}

// ----------------------------------------------------------------
void DrawCircle(int x, int y, int radius, SDL_Color color)
{
	SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(state.renderer, color.r, color.g, color.b, color.a);

	SDL_Point points[360];
	float factor = (float)M_PI / 180.0f;

	for (int i = 0; i < 360; ++i)
	{
		points[i].x = (int)(x + radius * cosf(factor * i));
		points[i].y = (int)(y + radius * sinf(factor * i));
	}

	int result = SDL_RenderDrawPoints(state.renderer, points, 360);

	if (result != 0) printf("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
}