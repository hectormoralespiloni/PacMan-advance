PAC-MAN ADVANCE (Oct-2002)
--------------------------

![](https://github.com/hectormoralespiloni/PacMan-advance/blob/master/pacman_full.jpg)

1. SUMMARY 
	This is a demo of the oldie classic Pac-Man. 
	Ported to the Game boy advance and made with:
	* HAM (www.ngine.de)
	* Visual HAM (www.console-dev.de)

2. REQUIREMENTS TO RUN THE GBA ROM
	* I recommend Visual Boy Advance (vba.ngemu.com)
	* Sound Card
	* You can copy the GBA to a Flash card and play it on your GBA hardware
	
3. HOW TO PLAY THE DEMO
	* Left/Right 	=> moves pacman left/right
	* Up/Down	=> moves pacman up/down
	
4. HOW TO COMPILE
	* The easiest way to compile the game is to get HAM which includes Visual HAM
	from www.ngine.de
	* Install HAM, run Visual HAM and open the project file pacman.vhw
	* Press F5 to compile or F7 to compile and run on the GBA emulator

5. CODE STURCTURE
	* All the graphics (gfx directory) were converted from bitmaps to char 
	representation using the tool gfx2gba included with HAM.
	* There's only one main.c file basically the main function initializes
	the HAM engine, the game variables and load all the graphics and sounds.
	* The main game loop is performed whenever a Vblank interrupt occurs; in HAM
	you define a callback function for that purpose and where we process input
	and update graphics, sound, collision detection, etc.
	* To handle collisions with the map, there's a file called collision.c which
	represents the game level and values different from zero represent walls
	so basically we test the square of pacman sprite against the map to check for
	collisions (the same applies for ghosts).
	* There are also one pills and one superpills file which represent the location
	of the pills on the map, we test collision against those to change the state of
	the game.
	* Last but not least, timer needs some fixing...
