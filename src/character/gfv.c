/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "gfv.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//GFV character structure
enum
{
	GFV_ArcMain_Gfv,
	
	GFV_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[GFV_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
} Char_GFV;

//GFV character definitions
static const CharFrame char_gfv_frame[] = {
	{GFV_ArcMain_Gfv, {  0,   0,  52, 54}, { 40,  73}}, //0 bop left 1
	{GFV_ArcMain_Gfv, { 53,   0,  52, 54}, { 40,  73}}, //1 bop left 2
	{GFV_ArcMain_Gfv, {106,   0,  53, 55}, { 41,  74}}, //2 bop left 3
	{GFV_ArcMain_Gfv, {159,   0,  53, 55}, { 41,  74}}, //3 bop left 4
	{GFV_ArcMain_Gfv, {  0,  55,  53, 54}, { 41,  74}}, //4 bop left 5
	{GFV_ArcMain_Gfv, { 54,  55,  54, 55}, { 42,  74}}, //5 bop left 6
	
	{GFV_ArcMain_Gfv, {109,  56,  53, 55}, { 41,  74}}, //6 bop right 1
	{GFV_ArcMain_Gfv, {163,  56,  53, 55}, { 41,  74}}, //7 bop right 2
	{GFV_ArcMain_Gfv, {  0, 110,  52, 56}, { 40,  75}}, //8 bop right 3
	{GFV_ArcMain_Gfv, { 53, 111,  53, 55}, { 41,  74}}, //9 bop right 4
	{GFV_ArcMain_Gfv, {107, 112,  52, 55}, { 40,  74}}, //10 bop right 5
	{GFV_ArcMain_Gfv, {160, 112,  52, 55}, { 40,  74}}, //11 bop right 6
	
	{GFV_ArcMain_Gfv, {  0, 167,  50,  55}, { 41,  74}}, //12 cheer
	{GFV_ArcMain_Gfv, { 51, 167,  51,  54}, { 40,  73}}, //13 cheer
};

static const Animation char_gfv_anim[CharAnim_Max] = {
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_Idle
	{1, (const u8[]){ 0,  0,  1,  1,  2,  2,  3,  4,  4,  5, ASCR_BACK, 1}}, //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_LeftAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                                  //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_DownAlt
	{3, (const u8[]){ 12, 13, ASCR_BACK, 1}},                           //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                                //CharAnim_UpAlt
	{1, (const u8[]){ 6,  6,  7,  7,  8,  8,  9, 10, 10, 11, ASCR_BACK, 1}}, //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_RightAlt
};

//GFV character functions
void Char_GFV_SetFrame(void *user, u8 frame)
{
	Char_GFV *this = (Char_GFV*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_gfv_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_GFV_Tick(Character *character)
{
	Char_GFV *this = (Char_GFV*)character;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform dance
		if ((stage.song_step % stage.gf_speed) == 0)
		{
			//Switch animation
			if (character->animatable.anim == CharAnim_Left)
				character->set_anim(character, CharAnim_Right);
			else
				character->set_anim(character, CharAnim_Left);
		}
	}
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_GFV_SetFrame);
	Character_Draw(character, &this->tex, &char_gfv_frame[this->frame]);

	if (stage.flag & STAGE_FLAG_JUST_STEP)
    {   //Stage specific animations
		if (stage.note_scroll >= 0)
		{
			switch (stage.stage_id)
			{
				case StageId_1_1: //Vidyagaems cheer
				if ((stage.song_step) == 188)
					    character->set_anim(character, CharAnim_Up);
					break;
				default:
					break;
			}
		}
	}
	
}

void Char_GFV_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_GFV_Free(Character *character)
{
	Char_GFV *this = (Char_GFV*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_GFV_New(fixed_t x, fixed_t y)
{
	//Allocate gfv object
	Char_GFV *this = Mem_Alloc(sizeof(Char_GFV));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_GFV_New] Failed to allocate gfv object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_GFV_Tick;
	this->character.set_anim = Char_GFV_SetAnim;
	this->character.free = Char_GFV_Free;
	
	Animatable_Init(&this->character.animatable, char_gfv_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(16,1);
	this->character.focus_y = FIXED_DEC(-50,1);
	this->character.focus_zoom = FIXED_DEC(13,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\GFV.ARC;1");
	
	const char **pathp = (const char *[]){
		"gfv.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;

	return (Character*)this;
}
