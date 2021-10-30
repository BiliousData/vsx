/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "sneed.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Sneed character structure
enum
{
	Sneed_ArcMain_Idle0,
	Sneed_ArcMain_Idle1,
	Sneed_ArcMain_Left,
	Sneed_ArcMain_Down,
	Sneed_ArcMain_Up,
	Sneed_ArcMain_Right,
	
	Sneed_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Sneed_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Sneed;

//Sneed character definitions
static const CharFrame char_sneed_frame[] = {
	{Sneed_ArcMain_Idle0, {  0,   0, 109, 155}, { 42, 183}}, //0 idle 1
	{Sneed_ArcMain_Idle0, {110,   0, 111, 157}, { 42, 183}}, //1 idle 2
	{Sneed_ArcMain_Idle1, {  0,   0, 111, 156}, { 42, 183}}, //2 idle 3
	{Sneed_ArcMain_Idle1, {112,   0, 112, 156}, { 42, 183}}, //3 idle 4
	
	{Sneed_ArcMain_Left, {  0,   0, 113, 158}, { 42, 183}}, //4 left 1
	{Sneed_ArcMain_Left, {114,   0, 112, 157}, { 42, 183}}, //5 left 2
	
	{Sneed_ArcMain_Down, {  0,   0, 114, 154}, { 42, 183}}, //6 down 1
	{Sneed_ArcMain_Down, {115,   0, 115, 154}, { 42, 183}}, //7 down 2
	
	{Sneed_ArcMain_Up, {  0,   0, 115, 158}, { 42, 183}}, //8 up 1
	{Sneed_ArcMain_Up, {116,   0, 114, 157}, { 42, 183}}, //9 up 2
	
	{Sneed_ArcMain_Right, {  0,   0, 126, 158}, { 42, 183}}, //10 right 1
	{Sneed_ArcMain_Right, {127,   0, 123, 158}, { 42, 183}}, //11 right 2
};

static const Animation char_sneed_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Sneed character functions
void Char_Sneed_SetFrame(void *user, u8 frame)
{
	Char_Sneed *this = (Char_Sneed*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_sneed_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Sneed_Tick(Character *character)
{
	Char_Sneed *this = (Char_Sneed*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Sneed_SetFrame);
	Character_Draw(character, &this->tex, &char_sneed_frame[this->frame]);
}

void Char_Sneed_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Sneed_Free(Character *character)
{
	Char_Sneed *this = (Char_Sneed*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Sneed_New(fixed_t x, fixed_t y)
{
	//Allocate sneed object
	Char_Sneed *this = Mem_Alloc(sizeof(Char_Sneed));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Sneed_New] Failed to allocate sneed object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Sneed_Tick;
	this->character.set_anim = Char_Sneed_SetAnim;
	this->character.free = Char_Sneed_Free;
	
	Animatable_Init(&this->character.animatable, char_sneed_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SNEED.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Sneed_ArcMain_Idle0
		"idle1.tim", //Sneed_ArcMain_Idle1
		"left.tim",  //Sneed_ArcMain_Left
		"down.tim",  //Sneed_ArcMain_Down
		"up.tim",    //Sneed_ArcMain_Up
		"right.tim", //Sneed_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
