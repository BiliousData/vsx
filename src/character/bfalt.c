/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bfalt.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Boyfriend /v/ unused skull fragments
static SkullFragment char_bfalt_skull[15] = {
	{ 1 * 8, -87 * 8, -13, -13},
	{ 9 * 8, -88 * 8,   5, -22},
	{18 * 8, -87 * 8,   9, -22},
	{26 * 8, -85 * 8,  13, -13},
	
	{-3 * 8, -82 * 8, -13, -11},
	{ 8 * 8, -85 * 8,  -9, -15},
	{20 * 8, -82 * 8,   9, -15},
	{30 * 8, -79 * 8,  13, -11},
	
	{-1 * 8, -74 * 8, -13, -5},
	{ 8 * 8, -77 * 8,  -9, -9},
	{19 * 8, -75 * 8,   9, -9},
	{26 * 8, -74 * 8,  13, -5},
	
	{ 5 * 8, -73 * 8, -5, -3},
	{14 * 8, -76 * 8,  9, -6},
	{26 * 8, -67 * 8, 15, -3},
};

//Boyfriend /v/ player types
enum
{
	BFALT_ArcMain_Idle,
	BFALT_ArcMain_Up,
	BFALT_ArcMain_Left,
	BFALT_ArcMain_Down,
	BFALT_ArcMain_Right,
	BFALT_ArcMain_Upmiss,
	BFALT_ArcMain_Leftmiss,
	BFALT_ArcMain_Downmiss,
	BFALT_ArcMain_Rmiss,
	
	BFALT_ArcMain_Max,
};

enum
{
	BFALT_ArcDead_Dead1, //Mic Drop
	BFALT_ArcDead_Dead2, //Twitch
	BFALT_ArcDead_Retry, //Retry prompt
	
	BFALT_ArcDead_Max,
};

#define BFALT_Arc_Max BFALT_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[BFALT_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_bfalt_skull)];
	u8 skull_scale;
} Char_BFALT;

//Boyfriend /v/ player definitions
static const CharFrame char_bfalt_frame[] = {
	{BFALT_ArcMain_Idle,     {  0,   0, 121, 105},   { 53,  92}}, //0 idle 1
	{BFALT_ArcMain_Idle,     {122,   0, 120, 108},   { 53,  95}}, //1 idle 2
	{BFALT_ArcMain_Idle,     {  0, 106, 117, 109},     { 51,  96}}, //2 idle 3
	{BFALT_ArcMain_Idle,     {120, 109, 117, 109},     { 49,  96}}, //3 idle 4
	
	{BFALT_ArcMain_Left,     {  0,   0, 154, 108}, { 86,  95}}, //4 left 1
	{BFALT_ArcMain_Left,     {  0, 109, 154, 108}, { 86,  95}}, //5 left 2
	{BFALT_ArcMain_Leftmiss, {  0,   0, 154, 108}, { 86,  95}}, //6 left miss 1
	{BFALT_ArcMain_Leftmiss, {  0, 109, 153, 108}, { 85,  95}}, //7 left miss 2
	
	{BFALT_ArcMain_Down,     {  0,   0, 139, 107}, { 71,  89}}, //8 down 1
	{BFALT_ArcMain_Down,     {  0, 108, 139, 108}, { 71,  93}}, //9 down 2
	{BFALT_ArcMain_Downmiss, {  0,   0, 138, 109}, { 71,  91}}, //10 down miss 1
	{BFALT_ArcMain_Downmiss, {  0, 110, 140, 109}, { 72,  91}}, //11 down miss 2
	
	{BFALT_ArcMain_Up,       {  0,   0, 142, 118}, { 74, 105}}, //12 up 1
	{BFALT_ArcMain_Up,       {  0, 119, 141, 114}, { 73, 101}}, //13 up 2
	{BFALT_ArcMain_Upmiss,   {  0,   0, 142, 114}, { 74, 105}}, //14 up miss 1
	{BFALT_ArcMain_Upmiss,   {  0, 115, 142, 114}, { 74, 105}}, //15 up miss 2
	
	{BFALT_ArcMain_Right,    {  0,   0, 138, 110}, { 70,  95}}, //16 right 1
	{BFALT_ArcMain_Right,    {  0, 111, 137, 111}, { 69,  95}}, //17 right 2
	{BFALT_ArcMain_Rmiss,    {  0,   0, 137, 111}, { 69,  96}}, //18 right miss 1
	{BFALT_ArcMain_Rmiss,    {  0, 112, 137, 111}, { 69,  95}}, //19 right miss 2

};

static const Animation char_bfalt_anim[PlayerAnim_Max] = {
	{1, (const u8[]){ 0,  1,  2,  2,  3, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},             //CharAnim_Left
	{1, (const u8[]){ 4,  6,  6,  7, ASCR_BACK, 1}},     //CharAnim_LeftAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},             //CharAnim_Down
	{1, (const u8[]){ 8, 10, 10, 11, ASCR_BACK, 1}},     //CharAnim_DownAlt
	{2, (const u8[]){12, 13, ASCR_BACK, 1}},             //CharAnim_Up
	{1, (const u8[]){12, 14, 14, 15, ASCR_BACK, 1}},     //CharAnim_UpAlt
	{2, (const u8[]){16, 17, ASCR_BACK, 1}},             //CharAnim_Right
	{1, (const u8[]){16, 18, 18, 19, ASCR_BACK, 1}},     //CharAnim_RightAlt
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //PlayerAnim_Peace
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //PlayerAnim_Sweat
	
	{2, (const u8[]){ 0,  1,  2,  3, 4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 0,  1,  2,  3, 4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 0,  1,  2,  3, 4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 0,  1,  2,  3, 4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 0,  1,  2,  3, 4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 0,  1,  2,  3, 4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 0,  1,  2,  3, 4, ASCR_BACK, 1}}, //CharAnim_Idle
};

//Boyfriend /v/ player functions
void Char_BFALT_SetFrame(void *user, u8 frame)
{
	Char_BFALT *this = (Char_BFALT*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bfalt_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BFALT_Tick(Character *character)
{
	Char_BFALT *this = (Char_BFALT*)character;
	
	//Handle animation updates
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_Right))
		Character_CheckEndSing(character);
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform idle dance
		if (Animatable_Ended(&character->animatable) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt) &&
			(stage.song_step & 0x7) == 0)
			character->set_anim(character, CharAnim_Idle);
		
		//Stage specific animations
		if (stage.note_scroll >= 0)
		{
			switch (stage.stage_id)
			{
				case StageId_V_1: //uhh Vidyagaems peace??? Not anymore.
				    if ((stage.song_step) == 188)
					    character->set_anim(character, PlayerAnim_Peace);
					break;
				default:
					break;
			}
		}
	}
	
	//Retry screen
	if (character->animatable.anim >= PlayerAnim_Dead3)
	{
		//Tick skull fragments
		if (this->skull_scale)
		{
			SkullFragment *frag = this->skull;
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFALT, skull); i++, frag++)
			{
				//Draw fragment
				RECT frag_src = {
					(i & 1) ? 112 : 96,
					(i >> 1) << 4,
					16,
					16
				};
				fixed_t skull_dim = (FIXED_DEC(16,1) * this->skull_scale) >> 6;
				fixed_t skull_rad = skull_dim >> 1;
				RECT_FIXED frag_dst = {
					character->x + (((fixed_t)frag->x << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.x,
					character->y + (((fixed_t)frag->y << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.y,
					skull_dim,
					skull_dim,
				};
				Stage_DrawTex(&this->tex_retry, &frag_src, &frag_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
				
				//Move fragment
				frag->x += frag->xsp;
				frag->y += ++frag->ysp;
			}
			
			//Decrease scale
			this->skull_scale--;
		}
		
		//Draw input options
		u8 input_scale = 16 - this->skull_scale;
		if (input_scale > 16)
			input_scale = 0;
		
		RECT button_src = {
			 0, 96,
			16, 16
		};
		RECT_FIXED button_dst = {
			character->x - FIXED_DEC(32,1) - stage.camera.x,
			character->y - FIXED_DEC(88,1) - stage.camera.y,
			(FIXED_DEC(16,1) * input_scale) >> 4,
			FIXED_DEC(16,1),
		};
		
		//Cross - Retry
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Circle - Blueball
		button_src.x = 16;
		button_dst.y += FIXED_DEC(56,1);
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Draw 'RETRY'
		u8 retry_frame;
		
		if (character->animatable.anim == PlayerAnim_Dead6)
		{
			//Selected retry
			retry_frame = 2 - (this->retry_bump >> 3);
			if (retry_frame >= 3)
				retry_frame = 0;
			if (this->retry_bump & 2)
				retry_frame += 3;
			
			if (++this->retry_bump == 0xFF)
				this->retry_bump = 0xFD;
		}
		else
		{
			//Idle
			retry_frame = 1 +  (this->retry_bump >> 2);
			if (retry_frame >= 3)
				retry_frame = 0;
			
			if (++this->retry_bump >= 55)
				this->retry_bump = 0;
		}
		
		RECT retry_src = {
			(retry_frame & 1) ? 48 : 0,
			(retry_frame >> 1) << 5,
			48,
			32
		};
		RECT_FIXED retry_dst = {
			character->x -  FIXED_DEC(7,1) - stage.camera.x,
			character->y - FIXED_DEC(92,1) - stage.camera.y,
			FIXED_DEC(48,1),
			FIXED_DEC(32,1),
		};
		Stage_DrawTex(&this->tex_retry, &retry_src, &retry_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
	}
	
	//Animate and draw character
	Animatable_Animate(&character->animatable, (void*)this, Char_BFALT_SetFrame);
	Character_Draw(character, &this->tex, &char_bfalt_frame[this->frame]);
}

void Char_BFALT_SetAnim(Character *character, u8 anim)
{
	Char_BFALT *this = (Char_BFALT*)character;
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_BFALT_Free(Character *character)
{
	Char_BFALT *this = (Char_BFALT*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_BFALT_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_BFALT *this = Mem_Alloc(sizeof(Char_BFALT));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BFALT_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BFALT_Tick;
	this->character.set_anim = Char_BFALT_SetAnim;
	this->character.free = Char_BFALT_Free;
	
	Animatable_Init(&this->character.animatable, char_bfalt_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	
	this->character.health_i = 0;
	
	this->character.focus_x = FIXED_DEC(-93,1);
	this->character.focus_y = FIXED_DEC(-99,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BFALT.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle.tim",
		"up.tim", 
		"left.tim",
		"down.tim",
		"right.tim",
		"upmiss.tim",
		"leftmiss.tim",
		"downmiss.tim",
		"rmiss.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize player state
	this->retry_bump = 0;
	
	//Copy skull fragments
	memcpy(this->skull, char_bfalt_skull, sizeof(char_bfalt_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFALT, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
