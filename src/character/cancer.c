/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "cancer.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Cancer Lord character structure
enum
{
	Cancer_ArcMain_Idle0,
	Cancer_ArcMain_Idle1,
	Cancer_ArcMain_Left,
	Cancer_ArcMain_Down,
	Cancer_ArcMain_Up0,
	Cancer_ArcMain_Up1,
	Cancer_ArcMain_Right0,
	Cancer_ArcMain_Right1,
	
	Cancer_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Cancer_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Cancer;

//Cancer Lord character definitions
static const CharFrame char_cancer_frame[] = {
	{Cancer_ArcMain_Idle0, {  0,   0, 118, 126}, { 42, 183}}, //0 idle 1
	{Cancer_ArcMain_Idle0, {119,   0, 113, 135}, { 42, 191}}, //1 idle 2
	{Cancer_ArcMain_Idle1, {  0,   0, 113, 135}, { 42, 191}}, //2 idle 3
	{Cancer_ArcMain_Idle1, {114,   0, 113, 135}, { 42, 191}}, //3 idle 4
	
	{Cancer_ArcMain_Left, {  0,   0,  125, 169}, { 43, 226}}, //4 left 1
	{Cancer_ArcMain_Left, {126,   0,  128, 166}, { 46, 222}}, //5 left 2
	
	{Cancer_ArcMain_Down, {  0,   0, 134, 121}, { 44, 180}}, //6 down 1
	{Cancer_ArcMain_Down, {0,   122, 131, 130}, { 44, 189}}, //7 down 2
	
	{Cancer_ArcMain_Up0, {  0,   0, 131, 179}, { 48, 235}}, //8 up 1
	{Cancer_ArcMain_Up1, {  0,   0, 137, 180}, { 49, 236}}, //9 up 2
	
	{Cancer_ArcMain_Right0, {  0,   0, 153, 145}, { 42, 202}}, //10 right 1
	{Cancer_ArcMain_Right1, {  0,   0, 144, 143}, { 42, 201}}, //11 right 2
};

static const Animation char_cancer_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1,  2,  3,  ASCR_BACK, 0}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Cancer character functions
void Char_Cancer_SetFrame(void *user, u8 frame)
{
	Char_Cancer *this = (Char_Cancer*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_cancer_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Cancer_Tick(Character *character)
{
	Char_Cancer *this = (Char_Cancer*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Cancer_SetFrame);
	Character_Draw(character, &this->tex, &char_cancer_frame[this->frame]);
}

void Char_Cancer_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Cancer_Free(Character *character)
{
	Char_Cancer *this = (Char_Cancer*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Cancer_New(fixed_t x, fixed_t y)
{
	//Allocate cancer object
	Char_Cancer *this = Mem_Alloc(sizeof(Char_Cancer));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Cancer_New] Failed to allocate cancer object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Cancer_Tick;
	this->character.set_anim = Char_Cancer_SetAnim;
	this->character.free = Char_Cancer_Free;
	
	Animatable_Init(&this->character.animatable, char_cancer_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 10;
	
	this->character.focus_x = FIXED_DEC(85,1);
	this->character.focus_y = FIXED_DEC(-125,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\CANCER.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Cancer_ArcMain_Idle0
		"idle1.tim", //Cancer_ArcMain_Idle1
		"left.tim",  //Cancer_ArcMain_Left
		"down.tim",  //Cancer_ArcMain_Down
		"up0.tim",    //Cancer_ArcMain_Up
		"up1.tim",
		"right0.tim", //Cancer_ArcMain_Right
		"right1.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
