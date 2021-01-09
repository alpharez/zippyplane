/*This source code copyrighted by Lazy Foo' Productions (2004-2020)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard math, and strings

#include "main.hpp"

//The window we'll be rendering to
SDL_Window *gWindow = NULL;

//The window renderer
SDL_Renderer *gRenderer = NULL;

//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 3200;

//Game Controller 1 handler
SDL_Joystick *gGameController = NULL;

//Globally used font
TTF_Font *gFont = NULL;

//Scene textures
//LTexture gTimeTextTexture;
//LTexture gPromptTextTexture;
LTexture gFPSTextTexture;

//Scene sprites
LTexture gSpriteSheetTexture;
SDL_Rect gRedPlaneClips;
SDL_Rect gBackgroundClips;
SDL_Rect gRockDownClips;
SDL_Rect gRockUpClips;
SDL_Rect gRockGrassDownClips;
SDL_Rect gRockGrassUpClips;

std::vector<Entity> rocks;
std::vector<Entity> coins;
//Entity plane;
bool gameOver;
bool isCollision;
bool playExplosion;
int score;

//The music that will be played
Mix_Music *gMusic = NULL;

//The sound effects that will be used
Mix_Chunk *g1up = NULL;
Mix_Chunk *gHigh = NULL;
Mix_Chunk *gMedium = NULL;
Mix_Chunk *gLow = NULL;

void addRock()
{
	SDL_Rect grect;
	int rockStartX = SCREEN_WIDTH + 100;
	int y;
	// Rocks have to be at least 80 pxls apart
	// check last rock
	Entity lastrock = rocks.back();
	if (lastrock.getX() > rockStartX - 150)
		return;
	// make a down or up rock

	grect.w = ROCKW;
	grect.h = ROCKH;
	int rock_x, rock_y, rockdown_x, rockdown_y;
	int rock_type = rand() % 4 + 1;
	switch (rock_type)
	{
	case 1:
		rock_x = ROCKGRASSX;
		rock_y = ROCKGRASSY;
		rockdown_x = ROCKGRASSDOWNX;
		rockdown_y = ROCKGRASSDOWNY;
		break;
	case 2:
		rock_x = ROCKX;
		rock_y = ROCKY;
		rockdown_x = ROCKDOWNX;
		rockdown_y = ROCKDOWNY;
		break;
	case 3:
		rock_x = ROCKICEX;
		rock_y = ROCKICEY;
		rockdown_x = ROCKICEDOWNX;
		rockdown_y = ROCKICEDOWNY;
		break;
	case 4:
		rock_x = ROCKSNOWX;
		rock_y = ROCKSNOWY;
		rockdown_x = ROCKSNOWDOWNX;
		rockdown_y = ROCKSNOWDOWNY;
		break;
	}

	if (rand() % 2 + 1 == 1) // down
	{
		grect.x = rock_x;
		grect.y = rock_y;
		y = ROCKH - (rand() % 10 + 1);
	}
	else // up
	{
		grect.x = rockdown_x;
		grect.y = rockdown_y;
		y = rand() % 10 + 1;
	}

	Entity e = Entity(rockStartX, y, 0, grect);
	rocks.push_back(e);
}

void addCoin()
{
	SDL_Rect grect;
	int coinStartX = SCREEN_WIDTH + 100;
	int y = rand() % SCREEN_HEIGHT + 1;
	//x="369" y="1444" width="39" height="37"
	grect.x = 369;
	grect.y = 1444;
	grect.w = 39;
	grect.h = 37;
	Entity e = Entity(coinStartX, y, 0, grect);
	coins.push_back(e);
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		// Check for joysticks
		if (SDL_NumJoysticks() < 1)
		{
			printf("Warning: No joysticks connected!\n");
		}
		else
		{
			// Load Joystick
			gGameController = SDL_JoystickOpen(0);
			if (gGameController == NULL)
			{
				printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
			}
		}
		//Create window
		gWindow = SDL_CreateWindow("ZippyPlane", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_mage Error: %s\n", IMG_GetError());
					success = false;
				}
				//Initialize SDL_ttf
				if (TTF_Init() == -1)
				{
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					success = false;
				}
				//Initialize SDL_mixer
				if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
				{
					printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}

bool checkTriangleCollision(SDL_Rect plane, SDL_Rect rock)
{
	//The sides of the rectangles
	int leftA, rightA, topA, bottomA;
	int point1x, base1x, base2x;
	int point1y, base1y, base2y;
	//Calculate the sides of rect A
	SDL_Rect p; // smaller plane for CD
	int sizeFactor = 50;
	p.x = plane.x + (PLANEW / 4) + 10;
	p.w = PLANEW / 4;
	//p.y = plane.y + (PLANEH / 4) - 10;
	p.y = plane.y;
	p.h = PLANEH / 4;
	// calculate the sides of triangle rock
	if (rock.y < 20)
	{
		// this rock points down
		point1x = rock.x + rock.w / 2 + 10;
		point1y = rock.h;
		base1x = rock.x;
		base1y = 0;
		//base2x = rock.w;
		//base2y = 0;
		//printf("%d %d\n", base1x, rock.y);
		if (SDL_IntersectRectAndLine(&p, &point1x, &point1y, &base1x, &base1y))
			return true;
	}
	else
	{
		// this rock points up
		point1x = rock.x + rock.w / 2 + 10;
		point1y = SCREEN_HEIGHT - rock.h + 15;
		base1x = rock.x;
		base1y = SCREEN_HEIGHT;
		if (SDL_IntersectRectAndLine(&plane, &point1x, &point1y, &base1x, &base1y))
			return true;
	}
	return false;
}

bool checkCollision(SDL_Rect plane, SDL_Rect b)
{
	//The sides of the rectangles
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	//Calculate the sides of rect A
	leftA = plane.x;
	rightA = plane.x + plane.w;
	topA = plane.y;
	bottomA = plane.y + plane.h;

	//Calculate the sides of rect B
	leftB = b.x;
	rightB = b.x - 20 + b.w;
	topB = b.y;
	bottomB = b.y + b.h;
	//If any of the sides from A are outside of B
	if (bottomA <= topB)
	{
		return false;
	}

	if (topA >= bottomB)
	{
		return false;
	}

	if (rightA <= leftB)
	{
		return false;
	}

	if (leftA >= rightB)
	{
		return false;
	}

	//If none of the sides from A are outside B
	return true;
}

void resetGame()
{
	gameOver = false;
	isCollision = false;
	playExplosion = true;
	score = 0;
	SDL_Rect rect1;
	// clear rocks
	rocks.clear();
	// clear coins
	coins.clear();
	// add some starting rocks
	rect1.x = 264; //up
	rect1.y = 986;
	rect1.w = 108;
	rect1.h = 239;
	rocks.push_back(Entity(SCREEN_WIDTH / 2, 0, 0, rect1));
	rocks.push_back(Entity(SCREEN_WIDTH / 2 + 400, 0, 0, rect1));
	rect1.x = 0;
	rect1.y = 1757;
	rect1.w = 108;
	rect1.h = 239;
	rocks.push_back(Entity(SCREEN_WIDTH / 2 + 200, 239, 0, rect1));
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Open the font
	/*gFont = TTF_OpenFont("./lazy.ttf", 28);
	if (gFont == NULL)
	{
		printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
		success = false;
	}
	else
	{
		//Set text color as black
		SDL_Color textColor = {0, 0, 0, 255};
	}*/

	//Load sprite sheet texture
	//if( !gSpriteSheetTexture.loadFromFile( "dots.png" ) )
	if (!gSpriteSheetTexture.loadFromFile("Spritesheet/sheet.png"))
	{
		printf("Failed to load sprite sheet texture!\n");
		success = false;
	}

	//Load music
	gMusic = Mix_LoadMUS("propplane2.wav");
	if (gMusic == NULL)
	{
		printf("Failed to load propplane! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	gHigh = Mix_LoadWAV("high.wav");
	if (gHigh == NULL)
	{
		printf("Failed to load high sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	g1up = Mix_LoadWAV("1up.wav");
	if (g1up == NULL)
	{
		printf("Failed to load 1up sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	return success;
}

void close()
{
	//Free loaded images
	gSpriteSheetTexture.free();

	//Close game controller
	SDL_JoystickClose(gGameController);
	gGameController = NULL;

	//Free global font
	//TTF_CloseFont(gFont);
	//gFont = NULL;

	//Destroy window
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	Mix_FreeMusic(gMusic);
	Mix_FreeChunk(gHigh);
	Mix_FreeChunk(g1up);
	gMusic = NULL;
	gHigh = NULL;
	g1up = NULL;

	//Quit SDL subsystems
	Mix_Quit();
	//TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char *args[])
{
	srand(time(NULL));
	//Start up SDL and create window
	//int planeAngle = 0;
	//int planeX = 20;
	//int planeY = SCREEN_HEIGHT / 2;

	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		SDL_Rect rect1, rect2, rect3;
		SDL_Rect gameOverRect;
		SDL_Rect rectOne, rectTwo, rectThree, rectFour, rectFive, rectSix, rectSeven;
		SDL_Rect rectEight, rectNine, rectZero;

		// Numbers
		rectZero.x = 432;
		rectZero.y = 1743;
		rectZero.w = 53;
		rectZero.h = 78;
		rectOne.x = 512;
		rectOne.y = 1093;
		rectOne.w = 37;
		rectOne.h = 76;
		rectTwo.x = 477;
		rectTwo.y = 1350;
		rectTwo.w = 51;
		rectTwo.h = 77;
		rectThree.x = 485;
		rectThree.y = 1679;
		rectThree.w = 51;
		rectThree.h = 78;
		rectFour.x = 432;
		rectFour.y = 1537;
		rectFour.w = 55;
		rectFour.h = 76;
		rectFive.x = 485;
		rectFive.y = 1823;
		rectFive.w = 50;
		rectFive.h = 76;
		rectSix.x = 432;
		rectSix.y = 1885;
		rectSix.w = 53;
		rectSix.h = 77;
		rectSeven.x = 478;
		rectSeven.y = 1173;
		rectSeven.w = 51;
		rectSeven.h = 76;
		rectEight.x = 461;
		rectEight.y = 899;
		rectEight.w = 51;
		rectEight.h = 78;
		rectNine.x = 458;
		rectNine.y = 1962;
		rectNine.w = 51;
		rectNine.h = 78;

		//Set plane sprite
		rect1.w = PLANEW;
		rect1.h = PLANEH;
		rect2.w = PLANEW;
		rect2.h = PLANEH;
		rect3.w = PLANEW;
		rect3.h = PLANEH;
		int rndPlane = rand() % 3 + 1;
		//int rndPlane = 3;
		if (rndPlane == 1)
		{ // red
			rect1.x = 216;
			rect1.y = 1878;
			rect2.x = 372;
			rect2.y = 1059;
			rect3.x = 372;
			rect3.y = 986;
		}
		else if (rndPlane == 2)
		{ // yellow
			rect1.x = 304;
			rect1.y = 1967;
			rect2.x = 330;
			rect2.y = 1298;
			rect3.x = 330;
			rect3.y = 1225;
		}
		else if (rndPlane == 3)
		{ // green
			rect1.x = 114;
			rect1.y = 1639;
			rect2.x = 216;
			rect2.y = 1951;
			rect3.x = 222;
			rect3.y = 1489;
		}
		else
		{ // blue
			rect1.x = 330;
			rect1.y = 1371;
			rect2.x = 372;
			rect2.y = 1132;
			rect3.x = 222;
			rect3.y = 1526;
		}

		Entity plane = Entity(20, SCREEN_HEIGHT / 2, 0, rect1, rect2, rect3);

		gameOverRect.x = 0;
		gameOverRect.y = 835;
		gameOverRect.w = 412;
		gameOverRect.h = 78;
		gBackgroundClips.x = 0;
		gBackgroundClips.y = 355;
		gBackgroundClips.w = 800;
		gBackgroundClips.h = 480;
		//Load media
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			//Main loop flag
			bool quit = false;
			int planeYdir = 0;
			resetGame();
			//Event handler
			SDL_Event e;

			//Set text color as black
			SDL_Color textColor = {0, 0, 0, 255};

			//Current time start time
			Uint32 startTime = 0;

			//The frames per second timer
			LTimer fpsTimer;
			//The frames per second cap timer
			LTimer capTimer;
			//In memory text stream
			std::stringstream timeText;

			//Start counting frames per second
			int countedFrames = 0;
			fpsTimer.start();

			//The background scrolling offset
			int scrollingOffset = 0;

			//While application is running
			while (!quit)
			{
				//Start cap timer
				capTimer.start();

				//Play the music
				if (Mix_PlayingMusic() == 0)
				{
					Mix_PlayMusic(gMusic, -1);
				}
				//angle ;
				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
					else if (e.type == SDL_JOYBUTTONDOWN)
					{
						//planeAngle -= 25;
						//startTime = SDL_GetTicks();
						if (gameOver)
							resetGame();
					}
					else if (e.type == SDL_JOYAXISMOTION)
					{
						//Motion on controller 0
						if (e.jaxis.which == 0)
						{
							//X axis motion
							if (e.jaxis.axis == 1)
							{
								//Left of dead zone
								if (e.jaxis.value < -JOYSTICK_DEAD_ZONE)
								{
									if (plane.getY() > 20 && !gameOver)
									{
										planeYdir = -3;
										plane.setAngle(-8);
										//printf("planeY: %d\n", plane.getY());
									}
									else
									{
										planeYdir = 0;
										plane.setAngle(-8);
									}
								}
								//Right of dead zone
								else if (e.jaxis.value > JOYSTICK_DEAD_ZONE)
								{
									if (plane.getY() < SCREEN_HEIGHT - 20 && !gameOver)
									{
										planeYdir = 3;
										plane.setAngle(8);
										//printf("planeY: %d\n", plane.getY());
									}
									else
									{
										planeYdir = 0;
										plane.setAngle(8);
									}
								}
								else
								{
									//planeYdir = 0;
									//plane.setAngle(0);
								}
							}
						}
					}
					else if (e.type == SDL_KEYDOWN)
					{
						switch (e.key.keysym.sym)
						{
						case SDLK_LEFT:
							printf("LEFT");
							break;
						case SDLK_RIGHT:
							printf("RIGHT");
							break;
						case SDLK_SPACE:
							printf("ZAP!");
							break;
						}
					}
				}
				//Calculate and correct fps
				float avgFPS = countedFrames / (fpsTimer.getTicks() / 1000.f);
				if (avgFPS > 2000000)
				{
					avgFPS = 0;
				}
				//Scroll background
				if (!gameOver)
					--scrollingOffset;
				if (scrollingOffset < -BG_WIDTH)
				{
					scrollingOffset = 0;
				}
				//planeY += planeYdir;
				plane.setY(plane.getY() + planeYdir);
				//Set text to be rendered
				//timeText.str("");
				//timeText << "FPS " << avgFPS;

				//Render text
				//if (!gFPSTextTexture.loadFromRenderedText(timeText.str().c_str(), textColor))
				//{
				//	printf("Unable to render FPS texture!\n");
				//}

				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);
				SDL_RenderClear(gRenderer);

				//Render background
				gSpriteSheetTexture.render(scrollingOffset, 0, &gBackgroundClips, 0, NULL, SDL_FLIP_NONE);
				gSpriteSheetTexture.render(scrollingOffset + BG_WIDTH, 0, &gBackgroundClips, 0, NULL, SDL_FLIP_NONE);

				//render plane
				gSpriteSheetTexture.render(plane.getX(), plane.getY(), plane.getCurrentFrame(), plane.getAngle(), NULL, SDL_FLIP_NONE);

				// loop through rock vector
				if (!rocks.empty())
				{
					// get rid of rocks past us
					// list of rocks > 20 then pop_front
					if (rocks.size() > 20)
						rocks.erase(rocks.begin());
					// render other rocks
					for (std::vector<Entity>::iterator it = rocks.begin(); it != rocks.end(); ++it)
					{
						int x = it->getX();
						if (!isCollision)
							//isCollision = checkCollision(plane.getBox(), it->getBox());
							isCollision = checkTriangleCollision(plane.getBox(), it->getBox());
						// render rock
						gSpriteSheetTexture.render(x, it->getY(), it->getCurrentFrame(), 0, NULL, SDL_FLIP_NONE);
						if (!gameOver)
							if (score > 60)
								it->setX(x - 6);
							else if (score > 30)
								it->setX(x - 4);
							else
								it->setX(x - 2);
						// check for collisions with plane

						// if top of plane
					}
				}
				// loop through coins vector
				if (!coins.empty())
				{
					//printf("Coins: %d\n", coins.size());
					if (coins.size() > 50)
						coins.erase(coins.begin());
					for (std::vector<Entity>::iterator it = coins.begin(); it != coins.end();)
					{
						int x = it->getX();
						if (it->inGame())
						{
							gSpriteSheetTexture.render(x, it->getY(), it->getCurrentFrame(), 0, NULL, SDL_FLIP_NONE);
							if (!gameOver)
								if (score > 60)
									it->setX(x - 6);
								else if (score > 30)
									it->setX(x - 4);
								else
									it->setX(x - 2);
						}
						bool coinCollision = checkCollision(plane.getBox(), it->getBox());
						if (coinCollision)
						{
							score++;
							coins.erase(it);
							Mix_PlayChannel(-1, g1up, 0);
						}
						else
						{
							++it; // iterate if we're not erasing
						}
					}
				}
				//gSpriteSheetTexture.render(200, 0, &gRockDownClips, 0, NULL, SDL_FLIP_NONE);
				//gSpriteSheetTexture.render(400, 239, &gRockUpClips, 0, NULL, SDL_FLIP_NONE);
				//gSpriteSheetTexture.render(600, 239, &gRockGrassDownClips, 0, NULL, SDL_FLIP_NONE);
				//gSpriteSheetTexture.render(500, 0, &gRockGrassUpClips, 0, NULL, SDL_FLIP_NONE);

				// render FPS text
				//gFPSTextTexture.render((SCREEN_WIDTH - gFPSTextTexture.getWidth()) - 20, (SCREEN_HEIGHT - gFPSTextTexture.getHeight()) - 20);
				// render Score
				int temp_score = score;
				int num_digits = 0;
				while (temp_score > 0)
				{
					num_digits++;
					int digit = temp_score % 10;
					temp_score /= 10;
					switch (digit)
					{
					case 0:
						gSpriteSheetTexture.render((SCREEN_WIDTH - num_digits * rectZero.w) - 20, 20, &rectZero, 0, NULL, SDL_FLIP_NONE);
						break;
					case 1:
						gSpriteSheetTexture.render((SCREEN_WIDTH - num_digits * rectOne.w) - 20, 20, &rectOne, 0, NULL, SDL_FLIP_NONE);
						break;
					case 2:
						gSpriteSheetTexture.render((SCREEN_WIDTH - num_digits * rectTwo.w) - 20, 20, &rectTwo, 0, NULL, SDL_FLIP_NONE);
						break;
					case 3:
						gSpriteSheetTexture.render((SCREEN_WIDTH - num_digits * rectThree.w) - 20, 20, &rectThree, 0, NULL, SDL_FLIP_NONE);
						break;

					case 4:
						gSpriteSheetTexture.render((SCREEN_WIDTH - num_digits * rectFour.w) - 20, 20, &rectFour, 0, NULL, SDL_FLIP_NONE);
						break;
					case 5:
						gSpriteSheetTexture.render((SCREEN_WIDTH - num_digits * rectFive.w) - 20, 20, &rectFive, 0, NULL, SDL_FLIP_NONE);
						break;
					case 6:
						gSpriteSheetTexture.render((SCREEN_WIDTH - num_digits * rectSix.w) - 20, 20, &rectSix, 0, NULL, SDL_FLIP_NONE);
						break;
					case 7:
						gSpriteSheetTexture.render((SCREEN_WIDTH - num_digits * rectSeven.w) - 20, 20, &rectSeven, 0, NULL, SDL_FLIP_NONE);
						break;
					case 8:
						gSpriteSheetTexture.render((SCREEN_WIDTH - num_digits * rectEight.w) - 20, 20, &rectEight, 0, NULL, SDL_FLIP_NONE);
						break;
					case 9:
						gSpriteSheetTexture.render((SCREEN_WIDTH - num_digits * rectNine.w) - 20, 20, &rectNine, 0, NULL, SDL_FLIP_NONE);
						break;
					default:
						gSpriteSheetTexture.render((SCREEN_WIDTH - num_digits * rectZero.w) - 20, 20, &rectZero, 0, NULL, SDL_FLIP_NONE);
						break;
					}
				}
				if (isCollision)
				{
					if (playExplosion)
					{
						Mix_PlayChannel(-1, gHigh, 0);
						playExplosion = false;
					}
					gameOver = true;

					//Stop the music
					Mix_HaltMusic();
				}
				if (gameOver)
				{
					gSpriteSheetTexture.render(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 40, &gameOverRect, 0, NULL, SDL_FLIP_NONE);
				}
				//Update screen
				SDL_RenderPresent(gRenderer);
				++countedFrames;
				//If frame finished early
				int frameTicks = capTimer.getTicks();
				// Randomly add a rock
				if (rand() % 100 + 1 == 1 && !gameOver)
				{
					addRock();
				}
				if (rand() % 50 + 1 == 1 && !gameOver)
				{
					addCoin();
				}
				if (frameTicks < SCREEN_TICKS_PER_FRAME)
				{
					//Wait remaining time
					SDL_Delay(SCREEN_TICKS_PER_FRAME - frameTicks);
				}
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
