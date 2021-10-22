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


//Boyfriend /v/ player types
enum
{
	BFALT_ArcMain_Idle0,
	BFALT_ArcMain_Idle1,
	BFALT_ArcMain_Up,
	BFALT_ArcMain_Left,
	BFALT_ArcMain_Down,
	BFALT_ArcMain_Right,
	BFALT_ArcMain_Upmiss,
	BFALT_ArcMain_Leftmiss,
	BFALT_ArcMain_Downmiss,
	BFALT_ArcMain_Rmiss,
	BFALT_ArcMain_Ban0,
	BFALT_ArcMain_Ban1,
	BFALT_ArcMain_Ban2,
	
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
	
} Char_BFALT;

//Boyfriend /v/ player definitions
static const CharFrame char_bfalt_frame[] = {
	{BFALT_ArcMain_Idle0,     {  0,   0, 121, 105},   { 53,  92}}, //0 idle 1
	{BFALT_ArcMain_Idle1,     {122,   0, 120, 108},   { 53,  95}}, //1 idle 2
	{BFALT_ArcMain_Idle1,     {  0, 106, 117, 109},     { 51,  96}}, //2 idle 3
	{BFALT_ArcMain_Idle1,     {120, 109, 117, 109},     { 49,  96}}, //3 idle 4
	
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
	{BFALT_ArcMain_Upmiss,   {  0,   0, 142, 114}, { 74, 101}}, //14 up miss 1
	{BFALT_ArcMain_Upmiss,   {  0, 115, 142, 114}, { 74, 101}}, //15 up miss 2
	
	{BFALT_ArcMain_Right,    {  0,   0, 138, 110}, { 70,  95}}, //16 right 1
	{BFALT_ArcMain_Right,    {  0, 111, 137, 111}, { 69,  95}}, //17 right 2
	{BFALT_ArcMain_Rmiss,    {  0,   0, 137, 111}, { 69,  96}}, //18 right miss 1
	{BFALT_ArcMain_Rmiss,    {  0, 112, 137, 111}, { 69,  95}}, //19 right miss 2

	{BFALT_ArcMain_Ban0,  { 0,  0,  161, 129}, {93, 122}}, //20
	{BFALT_ArcMain_Ban0,  { 0,  127,  161, 129}, {93, 122}}, //21

	{BFALT_ArcMain_Ban1,  { 0,  0,  161, 129}, {93, 122}},//22
	{BFALT_ArcMain_Ban1,  { 0,  127,  161, 129}, {93, 122}},//23

	{BFALT_ArcMain_Ban2,  { 0,  0,  161, 129}, {93, 122}},//24
	{BFALT_ArcMain_Ban2,  { 0,  127,  161, 129}, {93, 122}},//25

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
	
	{3, (const u8[]){ 20, 21, ASCR_CHGANI, PlayerAnim_Dead1}}, //CharAnim_Idle
	{3, (const u8[]){ 22, 23, ASCR_CHGANI, PlayerAnim_Dead2}}, //CharAnim_Idle
	{20, (const u8[]){ 24, ASCR_CHGANI, PlayerAnim_Dead3}}, //CharAnim_Idle
	{1, (const u8[]){ 25, ASCR_CHGANI, PlayerAnim_Dead3}}, //CharAnim_Idle
	{2, (const u8[]){ ASCR_CHGANI, PlayerAnim_Dead3}}, //CharAnim_Idle
	{2, (const u8[]){ ASCR_CHGANI, PlayerAnim_Dead3}}, //CharAnim_Idle
	{2, (const u8[]){ ASCR_CHGANI, PlayerAnim_Dead3}}, //CharAnim_Idle
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
		"idle0.tim",
		"idle1.tim",
		"up.tim", 
		"left.tim",
		"down.tim",
		"right.tim",
		"upmiss.tim",
		"leftmiss.tim",
		"downmiss.tim",
		"rmiss.tim",
		"ban0.tim",
		"ban1.tim",
		"ban2.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	
	
	return (Character*)this;
}
