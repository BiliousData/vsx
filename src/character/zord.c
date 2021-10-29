/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "zord.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Dad character structure
enum
{
	Zord_ArcMain_Idle0,
	Zord_ArcMain_Idle1,
	
	Zord_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Zord_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Zord;

//Zord character definitions
static const CharFrame char_zord_frame[] = {
	{Zord_ArcMain_Idle0, {  0,   0, 332, 187}, { 0, 0}}, //0 idle 1
	{Zord_ArcMain_Idle0, {332,   0, 332, 187}, { 0, 0}}, //1 idle 2
	{Zord_ArcMain_Idle1, {  0,   0, 332, 187}, { 0, 0}}, //2 idle 3
	{Zord_ArcMain_Idle1, {332,   0, 332, 187}, { 0, 0}}, //3 idle 4
	
};

static const Animation char_zord_anim[CharAnim_Max] = {
	{2, (const u8[]){ 1,  2,  3,  0, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Dad character functions
void Char_Zord_SetFrame(void *user, u8 frame)
{
	Char_Zord *this = (Char_Zord*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_zord_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Zord_Tick(Character *character)
{
	Char_Zord *this = (Char_Zord*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Zord_SetFrame);
	Character_Draw(character, &this->tex, &char_zord_frame[this->frame]);
}

void Char_Zord_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Zord_Free(Character *character)
{
	Char_Zord *this = (Char_Zord*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Zord_New(fixed_t x, fixed_t y)
{
	//Allocate dad object
	Char_Zord *this = Mem_Alloc(sizeof(Char_Zord));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Zord_New] Failed to allocate big fucking robot object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Zord_Tick;
	this->character.set_anim = Char_Zord_SetAnim;
	this->character.free = Char_Zord_Free;
	
	Animatable_Init(&this->character.animatable, char_zord_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(85,1);
	this->character.focus_y = FIXED_DEC(-60,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\ZORD.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Dad_ArcMain_Idle0
		"idle1.tim", //Dad_ArcMain_Idle1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
