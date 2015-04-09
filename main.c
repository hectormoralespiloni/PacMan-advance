/***********************************
	PacMan Advance Demo
	Author: Hector Morales Piloni
	Date:	October 22, 2002
***********************************/
#include <mygba.h>

#include "timer.h"
#include "collision.c"
#include "dots.c"
#include "superdots.c"
#include "gfx/master.pal.c"
#include "gfx/mapa.raw.c"
#include "gfx/mapa.map.c"
#include "gfx/dot.raw.c"
#include "gfx/superdot.raw.c"
#include "gfx/inky.raw.c"
#include "gfx/pinky.raw.c"
#include "gfx/blinky.raw.c"
#include "gfx/sue.raw.c"
#include "gfx/pacman.raw.c"
#include "gfx/logo.raw.c"
#include "gfx/logo.map.c"
#include "gfx/end.raw.c"
#include "gfx/end.map.c"

//pacman direction
#define LEFT	0
#define RIGHT	1
#define UP		2
#define DOWN	3
#define NONE	4

//ghosts states
#define NORMAL	0
#define SCARED	1
#define BLINK	2
#define EATEN	3

//for pacman
#define BEGIN	0
#define EAT		1
#define CHOMP	2
#define DIE		3
#define DEAD	4

//ghosts home coords
#define HOME_X	120
#define HOME_Y	56

//max number of pills in the maze
#define MAX_PILLS	86

#define MIXER_FREQ 26757

/**************************************
	GLOBAL VARIABLES
***************************************/
u8 ghost[4],pacman[3],pill[1],super_pill[1];

u8 pacman_x,
   pacman_y;
u8 ghost_x[4],
   ghost_y[4];


//character direction
u8 pacman_dir;
u8 ghost_dir[4];

//to handle when pacman opens his mouth
u8 pacman_open;

//state of the ghosts and pacman
u8 ghost_state[4];
u8 pacman_state=BEGIN;

//character next move
u8 pacman_next;
u8 ghost_next[4];

//to konw if a character has crashed with a wall
u8 pacman_crash;
u8 ghost_crash[4];

//to redraw every 2 frames
u8 count;

//score of the game
u16 score=0;

//quite obvious...
u8 lives=3;

//to know how many pills are there in the game
u8 pills=0;

//holds all pills already eaten
u8 eaten[82];
u8 eaten2[4];

//to know how many seconds the ghosts have been scared
u8 scare_time;
u8 scare_flag;

//used to count some seconds at the beginning of the game
u8 time_begin=0;

//pointers to wave data
extern const WaveData _binary_begin_raw_start;
extern const WaveData _binary_pacchomp_raw_start;
extern const WaveData _binary_killed_raw_start;
extern const WaveData _binary_ghosteaten_raw_start;

sample_info *music_samples[4];

map_fragment_info_ptr bg_mapa;
map_fragment_info_ptr bg_logo;
map_fragment_info_ptr bg_end;

/**************************************
	GLOBAL VARIABLES END
***************************************/

void init();
void putDots();
void vblFunc(); 
void redraw_pacman();
void redraw_ghost(u8 i);
void query_keys();
void pacman_collision_test();
void ghost_collision_test();
void start_ghosts(u8 index);
void goHome(u8 index);

int main(void)
{
	int i;

    // initialize HAMlib
    ham_Init();
	ham_InitText(1);

	//set Background mode 1
	ham_SetBgMode(1);

	// Initialize palettes
	ham_LoadBGPal(&master_Palette,256);
	ham_LoadObjPal(&master_Palette,256);

	// Setup the tileset for our image
	ham_bg[0].ti = ham_InitTileSet(&logo_Tiles,
                                   SIZEOF_16BIT(logo_Tiles),1,1);

	// Setup the map for our image
	ham_bg[0].mi = ham_InitMapEmptySet(3,0);
	bg_logo = ham_InitMapFragment(&logo_Map,30,20,0,0,30,20,0);
	ham_InsertMapFragment(bg_logo,0,0,0);
	
	// Display the background
	ham_InitBg(0,1,0,0);
	while(!F_CTRLINPUT_START_PRESSED);
	ham_DeInitBg(0);

	ham_bg[0].ti = ham_InitTileSet(&mapa_Tiles,
                                   SIZEOF_16BIT(mapa_Tiles),1,1);
	ham_bg[0].mi = ham_InitMapEmptySet(3,0);
	bg_mapa = ham_InitMapFragment(&mapa_Map,30,20,0,0,30,20,0);
	ham_InsertMapFragment(bg_mapa,0,0,0);

	// Display the background
	ham_InitBg(0,1,0,0);

	//init some stuff
	init();

	//setup the sprites
	ghost[0] = ham_CreateObj(&inky_Bitmap,0,0,
                         OBJ_MODE_NORMAL,1,0,0,0,0,0,0,ghost_x[0],ghost_y[0]);
	ghost[1] = ham_CreateObj(&pinky_Bitmap,0,0,
                         OBJ_MODE_NORMAL,1,0,0,0,0,0,0,ghost_x[1],ghost_y[1]);
	ghost[2] = ham_CreateObj(&blinky_Bitmap,0,0,
                         OBJ_MODE_NORMAL,1,0,0,0,0,0,0,ghost_x[2],ghost_y[2]);
	ghost[3] = ham_CreateObj(&sue_Bitmap,0,0,
                         OBJ_MODE_NORMAL,1,0,0,0,0,0,0,ghost_x[3],ghost_y[3]);
	pacman[0] = ham_CreateObj(&pacman_Bitmap,0,0,
                         OBJ_MODE_NORMAL,1,0,0,0,0,0,0,pacman_x,pacman_y);

	putDots();

	//these are used for displaying pacman lives
	pacman[1] = ham_CreateObj(&pacman_Bitmap,0,0,
                         OBJ_MODE_NORMAL,1,0,0,0,0,0,0,8,150);
	pacman[2] = ham_CreateObj(&pacman_Bitmap,0,0,
                         OBJ_MODE_NORMAL,1,0,0,0,0,0,0,16,150);

	ham_CopyObjToOAM();
	ham_DrawText(12,11,"READY!");

	srand(time(0));

	ham_InitMixer(MIXER_FREQ);
	ham_StartIntHandler(INT_TYPE_VBL,&vblFunc);

	music_samples[0] =  ham_InitSample((u8*)_binary_begin_raw_start.data,
                        _binary_begin_raw_start.size,
                        _binary_begin_raw_start.freq>>10);
	music_samples[1] =  ham_InitSample((u8*)_binary_pacchomp_raw_start.data,
                        _binary_pacchomp_raw_start.size,
                        _binary_pacchomp_raw_start.freq>>10);
	music_samples[2] =  ham_InitSample((u8*)_binary_killed_raw_start.data,
                        _binary_killed_raw_start.size,
                        _binary_killed_raw_start.freq>>10);
	music_samples[3] =  ham_InitSample((u8*)_binary_ghosteaten_raw_start.data,
                        _binary_ghosteaten_raw_start.size,
                        _binary_ghosteaten_raw_start.freq>>10);

    while(1)
	{
    }    
}

void init()
{	
	int i;

	pacman_x=116,
	pacman_y=117;
	pacman_dir = LEFT;
	pacman_next= LEFT;
	pacman_crash=0;
	//open mouth aux variable
	pacman_open=-1; 

	scare_time=0;
	scare_flag=0;

	ham_SetObjX(pacman[0],pacman_x);
	ham_SetObjY(pacman[0],pacman_y);

	for(i=0; i<4; i++)
	{
		ghost_x[i]=117,
		ghost_y[i]=70;
		ghost_dir[i]=NONE;
		ghost_next[i]=NONE;
		ghost_crash[i]=0;
		ghost_state[i]=NORMAL;
		ham_SetObjX(ghost[i],ghost_x[i]);
		ham_SetObjY(ghost[i],ghost_y[i]);
	}

	count=0;
	ham_CopyObjToOAM();
}

void putDots()
{
	int i;

	//OAM: 5
	pill[0] = ham_CreateObj(&dot_Bitmap,0,0,
                         OBJ_MODE_NORMAL,1,0,0,0,0,0,0,56,5);
	//OAM: 6,2,...,86
	for(i=1; i<82; i++)
	{
		ham_CloneObj(pill[0],dots[i][0],dots[i][1]);
		eaten[i]=0;
	}

	//OAM: 87,88,89,90
	super_pill[0] = ham_CreateObj(&superdot_Bitmap,0,0,
                         OBJ_MODE_NORMAL,1,0,0,0,0,0,0,45,5);
	for(i=1; i<4; i++)
	{
		ham_CloneObj(super_pill[0],superdots[i][0],superdots[i][1]);
		eaten2[i]=0;
	}
}

void vblFunc()
{
	int i; 

	ham_SyncMixer();

	//begin of game or pacman diying?
	if((pacman_state!=BEGIN) && (pacman_state!=DIE)){
	//Pacman opens his mouth every 2 frames
	pacman_open++;
	if(pacman_open==0 || pacman_open==1)
		ham_UpdateObjGfx(pacman[0],
	                (void*)&pacman_Bitmap[64*(2*pacman_dir)]);
	else if(pacman_open==2 || pacman_open==3)
		ham_UpdateObjGfx(pacman[0],
	                (void*)&pacman_Bitmap[64*((2*pacman_dir)+1)]);

	if(scare_flag)
	{
		scare_time+=Timer();

		if((scare_time>100) && (scare_time<150))
		{
			for(i=0; i<4; i++)
				if(ghost_state[i]==SCARED)
					ghost_state[i]=BLINK;
		}

		if(scare_time>200)
		{
			for(i=0; i<4; i++)
				if(ghost_state[i]==BLINK)
					ghost_state[i]=NORMAL;

			scare_flag = 0;
			scare_time = 0;
		}
	}

	ham_UpdateObjGfx(ghost[0],
	(void*)&inky_Bitmap[64*ghost_state[0]]);
	ham_UpdateObjGfx(ghost[1],
	(void*)&pinky_Bitmap[64*ghost_state[1]]);
	ham_UpdateObjGfx(ghost[2],
	(void*)&blinky_Bitmap[64*ghost_state[2]]);
	ham_UpdateObjGfx(ghost[3],
	(void*)&sue_Bitmap[64*ghost_state[3]]);

	ham_CopyObjToOAM();
	query_keys();
	pacman_collision_test();
	ghost_collision_test();

	count++;

	if(count==2)
		redraw_pacman();
	
	for(i=0; i<4; i++)
	{
		//if a ghost has been eaten, increase its speed redrawing it
		//once per frame
		if((ghost_state[i]==EATEN) && (count < 2))
			redraw_ghost(i);
		else if(count==2)
			redraw_ghost(i);
	}

	if(count==2)
		count=0;

	ham_DrawText(0,1,"Score");
	ham_DrawText(0,2,"    ");
	ham_DrawText(0,2,"%d",score);
	ham_DrawText(0,17,"Lives:");

	//get a new random move for each ghost 20% of the time
	if(rand()%100 < 10)
	{
		for(i=0; i<4; i++)
		{
			if(ghost_state[i]==EATEN)
				goHome(i);
			else
			{
				if((ghost_dir[i] == LEFT) || (ghost_dir[i] == RIGHT))
					ghost_next[i]=rand()%2+2;
				else
					ghost_next[i]=rand()%2;
			}
		}

	}

	if(pacman_open==3)
		pacman_open=-1;

	}//end if pacman_state != BEGIN
	
	ham_UpdateMixer();

	//set pacman to state DEAD to display the end screen
	if(pills==MAX_PILLS)
		pacman_state=DEAD;

	switch(pacman_state){
	case CHOMP:
		if(!music_samples[1]->playing)
			ham_PlaySample(music_samples[1]);
		break;
	case EAT:
		if(!music_samples[3]->playing)
			ham_PlaySample(music_samples[3]);
		pacman_state=CHOMP;
		break;
	case DIE:
		if(!music_samples[2]->playing && time_begin==0)
		{
			ham_PlaySample(music_samples[2]);
			TimerStart();
		}
		time_begin+=Timer();
		if(time_begin>2)
		{
			pacman_state=CHOMP;

			srand(time(0));
			init();
			for(i=0; i<4; i++)
				start_ghosts(i);

			if(lives==0)
				pacman_state=DEAD;
		}
		break;
	case BEGIN:
		if(!music_samples[0]->playing)
		{
			ham_PlaySample(music_samples[0]);
			TimerStart();
			time_begin=0;
		}
		time_begin+=Timer();
		if(time_begin>3)
		{
			pacman_state=CHOMP;
			ham_DrawText(12,11,"      ");
			for(i=0; i<4; i++)
				start_ghosts(i);
		}
		break;
	case DEAD:
		//game over!
		// Setup the tileset for our image

		//delete all objects
		for(i=0; i<92; i++)
			ham_DeleteObj(i);
		ham_CopyObjToOAM();

		for(i=0; i<4; i++)
			ham_DeInitSample(music_samples[i]);

		ham_StopIntHandler(INT_TYPE_VBL);
		ham_DeInitBg(0);
		ham_DeInitText();

		ham_bg[0].ti = ham_InitTileSet(&end_Tiles,
			            SIZEOF_16BIT(end_Tiles),1,1);

		// Setup the map for our image
		ham_bg[0].mi = ham_InitMapEmptySet(3,0);
		bg_end = ham_InitMapFragment(&end_Map,30,20,0,0,30,20,0);
		ham_InsertMapFragment(bg_end,0,0,0);
		// Display the background
		ham_InitBg(0,1,0,0);
		break;
	}

	return;
}

void query_keys()
{
	if(F_CTRLINPUT_DOWN_PRESSED)
	{
		pacman_next = DOWN;
	}
	if(F_CTRLINPUT_UP_PRESSED)
	{
		pacman_next = UP;
	}
	if(F_CTRLINPUT_LEFT_PRESSED)
	{
		pacman_next = LEFT;
	}
	if(F_CTRLINPUT_RIGHT_PRESSED)
	{
		pacman_next = RIGHT;
	}
}

void pacman_collision_test()
{
	int i,j;

	switch(pacman_dir){
	case LEFT:
		if(collision[(pacman_y*240+pacman_x)-1]!=0 ||
			collision[((pacman_y+7)*240+pacman_x)-1]!=0 ||
			collision[(pacman_y+4)*240+pacman_x]!=0)
			pacman_crash=1;
		else
			pacman_crash=0;
		break;
	case RIGHT:
		if(collision[(pacman_y*240+(pacman_x+7))+1]!=0 ||
			collision[((pacman_y+7)*240+(pacman_x+7))+1]!=0 ||
			collision[(pacman_y+4)*240+pacman_x+7]!=0)
			pacman_crash=1;
		else
			pacman_crash=0;
		break;
	case UP:
		if(collision[(pacman_y-1)*240+pacman_x]!=0 ||
			collision[(pacman_y-1)*240+(pacman_x+7)]!=0)
			pacman_crash=1;
		else
			pacman_crash=0;
		break;
	case DOWN:
		if(collision[(pacman_y+8)*240+pacman_x]!=0 ||
			collision[(pacman_y+8)*240+(pacman_x+7)]!=0)
			pacman_crash=1;
		else
			pacman_crash=0;
		break;
	default:
		break;
	}

	switch(pacman_next){
	case LEFT:
		if(collision[(pacman_y*240+pacman_x)-1]==0 &&
		   collision[((pacman_y+7)*240+pacman_x)-1]==0 &&
		   collision[((pacman_y+4)*240+pacman_x)-1]==0)
			pacman_dir=LEFT;
		break;
	case RIGHT:
		if(collision[(pacman_y*240+(pacman_x+7))+1]==0 &&
			collision[((pacman_y+7)*240+(pacman_x+7))+1]==0 &&
			collision[((pacman_y+4)*240+(pacman_x+7))+1]==0)
			pacman_dir=RIGHT;
		break;
	case UP:
		if(collision[(pacman_y-1)*240+pacman_x]==0 &&
			collision[(pacman_y-1)*240+(pacman_x+7)]==0)
			pacman_dir=UP;
		break;
	case DOWN:
		if(collision[(pacman_y+8)*240+pacman_x]==0 &&
			collision[(pacman_y+8)*240+(pacman_x+7)]==0)
			pacman_dir=DOWN;
		break;
	default:
		break;
	}

	//check if pacman ate a pill
	for(i=0; i<82; i++)
	{
		if((pacman_x==dots[i][0]) && (pacman_y==dots[i][1]))
		{
			if(!eaten[i])
			{
				ham_DeleteObj(i+5);
				eaten[i]=1;
				score+=10;
				pills++;
			}
		}
	}

	//check if pacman ate a SUPER pill
	for(i=0; i<4; i++)
	{
		if((pacman_x==superdots[i][0]) && (pacman_y==superdots[i][1]))
		{
			if(!eaten2[i])
			{
				ham_DeleteObj(i+5+82);
				eaten2[i]=1;
				score+=50;
				pills++;
				for(j=0; j<4; j++)
				{
					if(ghost_state[j]!=EATEN)
						ghost_state[j]=SCARED;
				}
				//start the timer to know how many seconds the ghosts
				//have been scared
				TimerStart();
				scare_time=0;
				scare_flag=1;
				pacman_state=EAT;
			}
		}
	}

	//check if pacman ate a ghost or a ghost chased pacman
	for(i=0; i<4; i++)
	{
		if((ghost_x[i]>=pacman_x) && (ghost_x[i]<=pacman_x+7))
		{
			if((ghost_y[i]>=pacman_y) && (ghost_y[i]<=pacman_y+7))
			{
				if(ghost_state[i]==SCARED || ghost_state[i]==BLINK)
				{
					ghost_state[i]=EATEN;
					pacman_state=EAT;
				}
				else if(ghost_state[i]==NORMAL)
				{
					pacman_state=DIE;
					time_begin=0;
					lives--;
					switch(lives){
					case 2:
						ham_DeleteObj(pacman[1]);
						break;
					case 1:
						ham_DeleteObj(pacman[2]);					
						break;
					}//switch
				}
			}
		}
	}
}

void ghost_collision_test()
{
	int i;

	for(i=0; i<4; i++)
	{
		switch(ghost_dir[i]){
		case LEFT:
			if(collision[(ghost_y[i]*240+ghost_x[i])-1]!=0 ||
				collision[((ghost_y[i]+7)*240+ghost_x[i])-1]!=0)
				ghost_crash[i]=1;
			else
				ghost_crash[i]=0;
			break;
		case RIGHT:
			if(collision[(ghost_y[i]*240+(ghost_x[i]+7))+1]!=0 ||
				collision[((ghost_y[i]+7)*240+(ghost_x[i]+7))+1]!=0)
				ghost_crash[i]=1;
			else
				ghost_crash[i]=0;
			break;
		case UP:
			if(collision[(ghost_y[i]-1)*240+ghost_x[i]]!=0 ||
				collision[(ghost_y[i]-1)*240+(ghost_x[i]+7)]!=0)
				ghost_crash[i]=1;
			else
				ghost_crash[i]=0;
			break;
		case DOWN:
			if(collision[(ghost_y[i]+8)*240+ghost_x[i]]!=0 ||
				collision[(ghost_y[i]+8)*240+(ghost_x[i]+7)]!=0)
				ghost_crash[i]=1;
			else
				ghost_crash[i]=0;
			break;
		default:
			break;
		}

		switch(ghost_next[i]){
		case LEFT:
			if(collision[(ghost_y[i]*240+ghost_x[i])-1]==0 &&
			   collision[((ghost_y[i]+7)*240+ghost_x[i])-1]==0)
				ghost_dir[i]=LEFT;
			break;
		case RIGHT:
			if(collision[(ghost_y[i]*240+(ghost_x[i]+7))+1]==0 &&
				collision[((ghost_y[i]+7)*240+(ghost_x[i]+7))+1]==0)
				ghost_dir[i]=RIGHT;
			break;
		case UP:
			if(collision[(ghost_y[i]-1)*240+ghost_x[i]]==0 &&
				collision[(ghost_y[i]-1)*240+(ghost_x[i]+7)]==0)
				ghost_dir[i]=UP;
			break;
		case DOWN:
			if(collision[(ghost_y[i]+8)*240+ghost_x[i]]==0 &&
				collision[(ghost_y[i]+8)*240+(ghost_x[i]+7)]==0)
				ghost_dir[i]=DOWN;
			break;
		default:
			break;
		}
	}//for
}

void redraw_pacman()
{
	if(!pacman_crash)
	{
		switch(pacman_dir){
		case LEFT:
			pacman_x--;
			if(pacman_x<45)
				pacman_x=195;
			break;
		case RIGHT:
			pacman_x++;
			if(pacman_x>195)
				pacman_x=45;
			break;
		case UP:
			pacman_y--;
			break;
		case DOWN:
			pacman_y++;
			break;
		}
	}

	ham_SetObjX(pacman[0],pacman_x);
	ham_SetObjY(pacman[0],pacman_y);
}

void redraw_ghost(u8 i)
{
	if(!ghost_crash[i])
	{
		switch(ghost_dir[i]){
		case LEFT:
			ghost_x[i]--;
			if(ghost_x[i]<45)
				ghost_x[i]=195;
			break;
		case RIGHT:
			ghost_x[i]++;
			if(ghost_x[i]>195)
				ghost_x[i]=45;
			break;
		case UP:
			ghost_y[i]--;
			break;
		case DOWN:
			ghost_y[i]++;
			break;
		}//switch
	}
	else
	{
		//if there's not collision
		if(ghost_state[i] == EATEN)
		{
			//Special case: check if it was already trying to move
			if( ( (ghost_dir[i]==DOWN) || (ghost_dir[i]==UP) ) && 
				(ghost_next[i]!=ghost_dir[i]) )
			{
				if(ghost_next[i]==LEFT)
					ghost_next[i]=RIGHT;
				else
					ghost_next[i]=LEFT;
			}
			else
			if( ( (ghost_dir[i]==LEFT) || (ghost_dir[i]==RIGHT)) && 
				(ghost_next[i]!=ghost_dir[i]) )
			{
				if(ghost_next[i]==UP)
					ghost_next[i]=DOWN;
				else
					ghost_next[i]=UP;
			}
			/*else
				goHome(i);*/
		}
		else
		{
			if((ghost_dir[i] == LEFT) || (ghost_dir[i] == RIGHT))
				ghost_next[i]=rand()%2+2;
			else
				ghost_next[i]=rand()%2;
		}
	}

	for(i=0; i<4; i++)
	{
		ham_SetObjX(ghost[i],ghost_x[i]);
		ham_SetObjY(ghost[i],ghost_y[i]);
	}
}

void start_ghosts(u8 index)
{
	//start moving the phantoms
	//when the game starts, pacman dies or one phantom dies
	while(ghost_y[index]>HOME_Y)
	{
		ghost_y[index]--;
		ham_SetObjY(ghost[index],ghost_y[index]);
		ham_CopyObjToOAM();
	}
	ghost_dir[index]=rand()%3;
	ghost_next[index]=rand()%3;
}

void goHome(u8 index)
{
	if((ghost_dir[index] == LEFT) || (ghost_dir[index] == RIGHT))
	{
		if(ghost_y[index]<HOME_Y)
			ghost_next[index]=DOWN;
		else if(ghost_y[index]>HOME_Y)
			ghost_next[index]=UP;
		//we know we're in Y, see if we're in X
		else if(ghost_y[index]==HOME_Y)
		{
			if(ghost_x[index]<HOME_X)
				ghost_next[index]=RIGHT;
			else if(ghost_x[index]>HOME_X)
				ghost_next[index]=LEFT;
	
			if((ghost_x[index]>HOME_X-5) && (ghost_x[index]<HOME_X+5))
			{
				ghost_y[index]=70;
				ghost_state[index]=NORMAL;
				Wait(2);
				srand(time(0));
				start_ghosts(index);
			}
		}
	}
	else
	{
		if(ghost_x[index]<HOME_X)
			ghost_next[index]=RIGHT;
		else if(ghost_x[index]>HOME_X)
			ghost_next[index]=LEFT;
	}
}
