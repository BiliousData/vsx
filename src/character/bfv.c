/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bfv.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"


//Boyfriend /v/ player types
enum
{
	BFV_ArcMain_Hit,  //Left Down
	BFV_ArcMain_Miss, //Left Down
	BFV_ArcMain_Ban0, //BREAK
	BFV_ArcMain_Ban1,
	BFV_ArcMain_Ban2,
	
	BFV_ArcMain_Max,
};

enum
{
	BFV_ArcBan_Ban1, //Mic Drop
	BFV_ArcBan_Ban2, //Twitch
	
	BFV_ArcBan_Max,
};

#define BFV_Arc_Max BFV_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[BFV_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
} Char_BFV;

//Boyfriend /v/ player definitions
static const CharFrame char_bfv_frame[] = {
	{BFV_ArcMain_Hit, {  0,   0, 58, 60}, { 53,  92}}, //0 idle 1
	{BFV_ArcMain_Hit, { 59,   0, 57, 60}, { 52,  92}}, //1 idle 2
	{BFV_ArcMain_Hit, {117, 0, 57, 61}, { 52,  93}}, //2 idle 3
	{BFV_ArcMain_Hit, {175, 0, 57, 63}, { 52,  95}}, //3 idle 4
	
	{BFV_ArcMain_Hit,  {117,  62,  57,  61}, { 52, 93}}, //4 left 1
	{BFV_ArcMain_Hit,  {175,  64,  56,  61}, { 51, 93}}, //5 left 2
	{BFV_ArcMain_Miss, {119,   0,  56,  65}, { 52, 93}}, //6 left miss 1
	{BFV_ArcMain_Miss, {176,   0,  56,  65}, { 51, 93}}, //7 left miss 2
	
	{BFV_ArcMain_Hit,  {  0,  61,  58,  55}, { 53,  87}}, //8 down 1
	{BFV_ArcMain_Hit,  { 59,  61,  57,  56}, { 52,  88}}, //9 down 2
	{BFV_ArcMain_Miss, {  0,   0,  58,  61}, { 53,  87}}, //10 down miss 1
	{BFV_ArcMain_Miss, { 59,   0,  59,  60}, { 52,  88}}, //11 down miss 2
	
	{BFV_ArcMain_Hit,  {113, 124,  50,  65}, { 43, 97}}, //12 up 1
	{BFV_ArcMain_Hit,  {164, 126,  51,  64}, { 44, 96}}, //13 up 2
	{BFV_ArcMain_Miss, {115,  66,  52,  63}, { 43, 97}}, //14 up miss 1
	{BFV_ArcMain_Miss, {168,  67,  52,  64}, { 44, 96}}, //15 up miss 2
	
	{BFV_ArcMain_Hit,  {  0, 117,  56,  60}, { 45, 92}}, //16 right 1
	{BFV_ArcMain_Hit,  { 57, 118,  55,  61}, { 45, 93}}, //17 right 2
	{BFV_ArcMain_Miss, {  0,  62,  56,  64}, { 45, 92}}, //18 right miss 1
	{BFV_ArcMain_Miss, { 57,  61,  57,  64}, { 45, 93}}, //19 right miss 2

	{BFV_ArcMain_Ban0,  { 0,  0,  161, 129}, {53, 92}}, //20
	{BFV_ArcMain_Ban0,  { 0,  127,  161, 129}, {53, 92}}, //21

	{BFV_ArcMain_Ban1,  { 0,  0,  161, 129}, {53, 92}},//22
	{BFV_ArcMain_Ban1,  { 0,  127,  161, 129}, {53, 92}},//23

	{BFV_ArcMain_Ban2,  { 0,  0,  161, 129}, {53, 92}},//24
	{BFV_ArcMain_Ban2,  { 0,  127,  161, 129}, {53, 92}},//25

};

static const Animation char_bfv_anim[PlayerAnim_Max] = {
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
void Char_BFV_SetFrame(void *user, u8 frame)
{
	Char_BFV *this = (Char_BFV*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bfv_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BFV_Tick(Character *character)
{
	Char_BFV *this = (Char_BFV*)character;
	
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
	Animatable_Animate(&character->animatable, (void*)this, Char_BFV_SetFrame);
	Character_Draw(character, &this->tex, &char_bfv_frame[this->frame]);
}

void Char_BFV_SetAnim(Character *character, u8 anim)
{
	Char_BFV *this = (Char_BFV*)character;

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

void Char_BFV_Free(Character *character)
{
	Char_BFV *this = (Char_BFV*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_BFV_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_BFV *this = Mem_Alloc(sizeof(Char_BFV));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BFV_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BFV_Tick;
	this->character.set_anim = Char_BFV_SetAnim;
	this->character.free = Char_BFV_Free;
	
	Animatable_Init(&this->character.animatable, char_bfv_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	
	this->character.health_i = 0;
	
	this->character.focus_x = FIXED_DEC(-50,1);
	this->character.focus_y = FIXED_DEC(-85,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BFV.ARC;1");
	
	const char **pathp = (const char *[]){
		"hit.tim",  //Everything except miss lol
		"miss.tim", //BFV_ArcMain_Miss0
		"ban0.tim",
		"ban1.tim",
		"ban2.tim",
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
