/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "vrage.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//v Rage character structure
enum
{
	Vrage_ArcMain_Idle0,
	Vrage_ArcMain_Idle1,
	Vrage_ArcMain_Left,
	Vrage_ArcMain_Down,
	Vrage_ArcMain_Up,
	Vrage_ArcMain_Right,
	Vrage_ArcMain_RightB,
	Vrage_ArcMain_Fuck0,
	Vrage_ArcMain_Fuck1,
	Vrage_ArcMain_Fuck2,
	
	Vrage_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Vrage_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Vrage;

//v rage character definitions
static const CharFrame char_vrage_frame[] = {
	{Vrage_ArcMain_Idle0, {  0,   0, 136, 107}, { 42, 183+4}}, //0 idle 1
	{Vrage_ArcMain_Idle0, {  0, 108, 136, 107}, { 43, 181+4}}, //1 idle 2
	{Vrage_ArcMain_Idle1, {  0,   0, 137, 107}, { 42, 181+4}}, //2 idle 3
	{Vrage_ArcMain_Idle1, {  0, 108, 137, 107}, { 41, 183+4}}, //3 idle 4
	
	{Vrage_ArcMain_Left, {  0,   0,  136, 106}, { 40, 185+4}}, //4 left 1
	{Vrage_ArcMain_Left, {  0,   107,  136, 108}, { 40, 185+4}}, //5 left 2
	
	{Vrage_ArcMain_Down, {  0,   0, 136, 98}, { 43, 174+4}}, //6 down 1
	{Vrage_ArcMain_Down, {  0,   99, 136, 100}, { 43, 175+4}}, //7 down 2
	
	{Vrage_ArcMain_Up, {  0,   0, 136, 105}, { 40, 196+4}}, //8 up 1
	{Vrage_ArcMain_Up, {  0,   0, 136, 108}, { 40, 194+4}}, //9 up 2
	
	{Vrage_ArcMain_Right, {  0,   0, 136, 98}, { 43, 189+4}}, //10 right 1
	{Vrage_ArcMain_Right, {  0,   99, 136, 97}, { 42, 189+4}}, //11 right 2
	{Vrage_ArcMain_RightB, {  0,   0, 137, 109}, { 43, 189+4}}, //12 right alt 1
	{Vrage_ArcMain_RightB, {  0,   110, 137, 109}, { 43, 189+4}}, //13 right alt 2

	{Vrage_ArcMain_Fuck0, {  0,  0, 136, 96}, { 40, 196+4}},
	{Vrage_ArcMain_Fuck0, {  0,  97, 136, 99}, { 40, 196+4}},
	{Vrage_ArcMain_Fuck1, {  0,  0, 136, 99}, { 40, 196+4}},
	{Vrage_ArcMain_Fuck1, {  0,  100, 137, 100}, { 40, 196+4}},
	{Vrage_ArcMain_Fuck2, {  0,  0, 137, 100}, { 40, 196+4}}

};

static const Animation char_vrage_anim[CharAnim_Max] = {
	{2, (const u8[]){ 1,  2,  3,  0, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},         //CharAnim_Down
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},         //CharAnim_Up
	{3, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{2, (const u8[]){12, 13, ASCR_BACK, 1}},         //CharAnim_RightAlt

	{3, (const u8[]){14, 15, 16, ASCR_CHGANI, CharAnim_Fuck2}},//FUCK
	{1, (const u8[]){ 17, 18, ASCR_CHGANI, CharAnim_Fuck2}}, //FUCK2
};

//v Rage character functions
void Char_Vrage_SetFrame(void *user, u8 frame)
{
	Char_Vrage *this = (Char_Vrage*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_vrage_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Vrage_Tick(Character *character)
{
	Char_Vrage *this = (Char_Vrage*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);

	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Vrage_SetFrame);
	Character_Draw(character, &this->tex, &char_vrage_frame[this->frame]);

	if (stage.flag & STAGE_FLAG_JUST_STEP)
    {   //Stage specific animations
		if (stage.note_scroll >= 0)
		{
			switch (stage.stage_id)
			{
				case StageId_1_2: //Sage fuck
				    if ((stage.song_step) == 1664)
					    character->set_anim(character, CharAnim_Fuck);
					break;
				default:
					break;
			}
		}
	}
}

void Char_Vrage_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Vrage_Free(Character *character)
{
	Char_Vrage *this = (Char_Vrage*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Vrage_New(fixed_t x, fixed_t y)
{
	//Allocate vrage object
	Char_Vrage *this = Mem_Alloc(sizeof(Char_Vrage));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Vrage_New] Failed to allocate v Rage object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Vrage_Tick;
	this->character.set_anim = Char_Vrage_SetAnim;
	this->character.free = Char_Vrage_Free;
	
	Animatable_Init(&this->character.animatable, char_vrage_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 12;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-165,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\VRAGE.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Dad_ArcMain_Idle0
		"idle1.tim", //Dad_ArcMain_Idle1
		"left.tim",  //Dad_ArcMain_Left
		"down.tim",  //Dad_ArcMain_Down
		"up.tim",    //Dad_ArcMain_Up
		"right.tim", //Dad_ArcMain_Right
		"rightb.tim",//vrage_arcmain_rightb
		"fuck0.tim",
		"fuck1.tim",
		"fuck2.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
