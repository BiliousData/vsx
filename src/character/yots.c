/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "yots.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"


//Yotsuba snowgear player types
enum
{
	Yots_ArcMain_Hit0,  //Idle left down
	Yots_ArcMain_Hit1, //right up
	Yots_ArcMain_Miss, //miss
	
	Yots_ArcMain_Max,
};

enum
{
	Yots_ArcBan_Ban1, //Ignore lol
	
	Yots_ArcBan_Max,
};

#define Yots_Arc_Max Yots_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Yots_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
} Char_Yots;

//Yotsuba snowgear player definitions
static const CharFrame char_yots_frame[] = {
	{Yots_ArcMain_Hit0, {  0,   0, 59, 97}, { 53,  92}}, //0 idle 1
	{Yots_ArcMain_Hit0, { 60,   0, 58, 97}, { 51,  92}}, //1 idle 2
	{Yots_ArcMain_Hit0, {119, 0, 58, 98}, { 51,  93}}, //2 idle 3
	{Yots_ArcMain_Hit0, {178, 0, 59, 98}, { 51,  93}}, //3 idle 4
	
	{Yots_ArcMain_Hit0,  {135,  99,  59,  96}, { 57,  91}}, //4 left 1
	{Yots_ArcMain_Hit0,  {195,  99,  59,  96}, { 56,  91}}, //5 left 2
	{Yots_ArcMain_Miss,  { 63,   0,  58,  95}, { 55,  94}}, //6 left miss 1
	
	{Yots_ArcMain_Hit0,  {  0,  98,  67,  89}, { 58,  84}}, //7 down 1
	{Yots_ArcMain_Hit0,  { 68,  98,  66,  90}, { 59,  85}}, //8 down 2
	{Yots_ArcMain_Miss,  {  0,   0,  62,  92}, { 57,  87}}, //9 down miss 1
	
	{Yots_ArcMain_Hit1,  {  0,  96,  67,  97}, { 51,  92}}, //10 up 1
	{Yots_ArcMain_Hit1,  { 68,  96,  66,  97}, { 52,  92}}, //11 up 2
	{Yots_ArcMain_Miss,  {183,   0,  66,  97}, { 51,  92}}, //12 up miss 1
	
	{Yots_ArcMain_Hit1,  {  0,   0,  62,  95}, { 47,  90}}, //13 right 1
	{Yots_ArcMain_Hit1,  { 63,   0,  60,  95}, { 47,  90}}, //14 right 2
	{Yots_ArcMain_Miss,  {122,   0,  60,  96}, { 50,  94}}, //15 right miss 1

	{Yots_ArcMain_Hit0,  { 0,  0,  1, 1}, {53, 92}}, //16 dead lol

};

static const Animation char_yots_anim[PlayerAnim_Max] = {
	{1, (const u8[]){ 0,  1,  2,  2,  3, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},             //CharAnim_Left
	{1, (const u8[]){ 4,  5,  6,  6, ASCR_BACK, 1}},     //CharAnim_LeftAlt
	{2, (const u8[]){ 7,  8, ASCR_BACK, 1}},             //CharAnim_Down
	{1, (const u8[]){ 7,  8, 9, 9, ASCR_BACK, 1}},     //CharAnim_DownAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},             //CharAnim_Up
	{1, (const u8[]){10, 11, 12, 12, ASCR_BACK, 1}},     //CharAnim_UpAlt
	{2, (const u8[]){13, 14, ASCR_BACK, 1}},             //CharAnim_Right
	{1, (const u8[]){13, 14, 15, 15, ASCR_BACK, 1}},     //CharAnim_RightAlt
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //PlayerAnim_Peace
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //PlayerAnim_Sweat
	
	{1, (const u8[]){ 16, ASCR_CHGANI, PlayerAnim_Dead1}}, //CharAnim_Idle
	{0, (const u8[]){ ASCR_CHGANI, PlayerAnim_Dead1}}, //CharAnim_Idle
	{0, (const u8[]){ ASCR_CHGANI, PlayerAnim_Dead1}}, //CharAnim_Idle
	{0, (const u8[]){ ASCR_CHGANI, PlayerAnim_Dead1}}, //CharAnim_Idle
	{0, (const u8[]){ ASCR_CHGANI, PlayerAnim_Dead1}}, //CharAnim_Idle
	{0, (const u8[]){ ASCR_CHGANI, PlayerAnim_Dead1}}, //CharAnim_Idle
	{0, (const u8[]){ ASCR_CHGANI, PlayerAnim_Dead1}}, //CharAnim_Idle
};

//Boyfriend /v/ player functions
void Char_Yots_SetFrame(void *user, u8 frame)
{
	Char_Yots *this = (Char_Yots*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_yots_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Yots_Tick(Character *character)
{
	Char_Yots *this = (Char_Yots*)character;
	
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
	Animatable_Animate(&character->animatable, (void*)this, Char_Yots_SetFrame);
	Character_Draw(character, &this->tex, &char_yots_frame[this->frame]);
}

void Char_Yots_SetAnim(Character *character, u8 anim)
{
	Char_Yots *this = (Char_Yots*)character;

	//Perform animation checks
	switch (anim)
	{
		case PlayerAnim_Dead0:
			//Begin reading dead.arc and adjust focus
			character->focus_x = FIXED_DEC(0,1);
			character->focus_y = FIXED_DEC(-40,1);
			character->focus_zoom = FIXED_DEC(125,100);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Yots_Free(Character *character)
{
	Char_Yots *this = (Char_Yots*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Yots_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_Yots *this = Mem_Alloc(sizeof(Char_Yots));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Yots_New] Failed to allocate yotsuba object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Yots_Tick;
	this->character.set_anim = Char_Yots_SetAnim;
	this->character.free = Char_Yots_Free;
	
	Animatable_Init(&this->character.animatable, char_yots_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	
	this->character.health_i = 11;
	
	this->character.focus_x = FIXED_DEC(-50,1);
	this->character.focus_y = FIXED_DEC(-85,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\YOTS.ARC;1");
	
	const char **pathp = (const char *[]){
		"hit0.tim",  //Everything except miss lol
		"hit1.tim",
		"miss.tim", //Yots_ArcMain_Miss0
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize player state
	this->retry_bump = 0;
	
	
	return (Character*)this;
}
