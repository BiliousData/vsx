/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "vcalm.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//v Rage character structure
enum
{
	Vcalm_ArcMain_Idle0,
	Vcalm_ArcMain_Idle1,
	Vcalm_ArcMain_Left,
	Vcalm_ArcMain_Down,
	Vcalm_ArcMain_Up,
	Vcalm_ArcMain_Right,
    Vcalm_ArcMain_Break0,
    Vcalm_ArcMain_Break1,
    Vcalm_ArcMain_Break2,
    Vcalm_ArcMain_Break3,
    Vcalm_ArcMain_Break4,	
    Vcalm_ArcMain_Break5,
	Vcalm_ArcMain_Break6,
	Vcalm_ArcMain_LeftMic,
	Vcalm_ArcMain_DownMic,
	Vcalm_ArcMain_UpMic,
	Vcalm_ArcMain_RightMic,
	Vcalm_ArcMain_IdleMic0,
	Vcalm_ArcMain_IdleMic1,


	Vcalm_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Vcalm_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Vcalm;

//v rage character definitions
static const CharFrame char_vcalm_frame[] = {
	{Vcalm_ArcMain_Idle0, {  0,   0, 136, 107}, { 30, 150}}, //0 idle 1
	{Vcalm_ArcMain_Idle0, {  0, 108, 136, 108}, { 30, 151}}, //1 idle 2
	{Vcalm_ArcMain_Idle1, {  0,   0, 136, 108}, { 30, 151}}, //2 idle 3
	{Vcalm_ArcMain_Idle1, {  0, 108, 136, 108}, { 30, 151}}, //3 idle 4

    {Vcalm_ArcMain_Left, {  0,   0,  136, 104}, { 30, 147}}, //4 left 1
	{Vcalm_ArcMain_Left, {  0,   105,  136, 105}, { 30, 148}}, //5 left 2
	
	{Vcalm_ArcMain_Down, {  0,   0, 136, 92}, { 30, 135}}, //6 down 1
	{Vcalm_ArcMain_Down, {  0,   93, 136, 94}, { 30, 137}}, //7 down 2
	
	{Vcalm_ArcMain_Up, {  0,   0, 136, 107}, { 30, 150}}, //8 up 1
	{Vcalm_ArcMain_Up, {  0,   108, 136, 109}, { 30, 152}}, //9 up 2
	
	{Vcalm_ArcMain_Right, {  0,   0, 136, 96}, { 30, 139}}, //10 right 1
	{Vcalm_ArcMain_Right, {  0,   97, 136, 97}, { 30, 140}}, //11 right 2

	{Vcalm_ArcMain_Break0, {  0,  0, 136, 106,}, { 30, 149,}},       //12 PadBreak0
	{Vcalm_ArcMain_Break0, {  0,  107, 136, 106,}, { 30, 149,}},     //13 PadBreak1
	{Vcalm_ArcMain_Break1, {  0,  0, 136, 106,}, { 30, 149,}},       //14 PadBreak2
	{Vcalm_ArcMain_Break1, {  0,  107, 136, 106,}, { 30, 149,}},          //15 PadBreak3
	{Vcalm_ArcMain_Break2, {  0,  0, 136, 106,}, { 30, 149,}},          //16 PadBreak4
	{Vcalm_ArcMain_Break2, {  0,  107, 136, 105,}, { 30, 148,}},          //17 PadBreak5
	{Vcalm_ArcMain_Break3, {  0,  0, 136, 105,}, { 30, 148,}},          //18 PadBreak6
	{Vcalm_ArcMain_Break3, {  0,  106, 136, 105,}, { 30, 148,}},          //19 PadBreak7
	{Vcalm_ArcMain_Break4, {  0,  0, 136, 105,}, { 30, 148,}},          //20 PadBreak8
	{Vcalm_ArcMain_Break4, {  0,  106, 136, 106,}, { 30, 149,}},          //21 PadBreak9
	{Vcalm_ArcMain_Break5, {  0,  0, 136, 105,}, { 30, 148,}},          //22 PadBreak10
	{Vcalm_ArcMain_Break5, {  0,  106, 136, 102,}, { 30, 145,}},          //23 PadBreak11
	{Vcalm_ArcMain_Break6, {  0,  0, 136, 106,}, { 30, 149,}},          //24 PadBreak12
	{Vcalm_ArcMain_Break6, {  0,  107, 136, 106,}, { 30, 149,}},          //25 PadBreak13

	{Vcalm_ArcMain_DownMic, {0, 0, 136, 106,}, { 30, 149,}},     //26 leftalt1
	{Vcalm_ArcMain_DownMic, {0, 107, 136, 105,}, { 30, 148,}},   //27 leftalt2
	{Vcalm_ArcMain_RightMic, {0, 0, 136, 105,}, { 30, 148,}},     //28 downalt1
	{Vcalm_ArcMain_RightMic, {0, 106, 136, 105,}, { 30, 148,}},   //29 downalt2
	{Vcalm_ArcMain_LeftMic, {0, 0, 136, 109,}, { 30, 152,}},       //30 upmic1
	{Vcalm_ArcMain_LeftMic, {0, 110, 136, 108,}, { 30, 151,}},     //31 upmic2
	{Vcalm_ArcMain_UpMic, {0, 0, 136, 104,}, { 30, 147,}},    //32 rightmic1
	{Vcalm_ArcMain_UpMic, {0, 105, 136, 104,}, { 30, 147,}},  //33 rightmic2
	//Don't ask
	//I don't know why it had to be like this

    {Vcalm_ArcMain_IdleMic0, {0, 0, 136, 105,}, { 30, 148,}},  //34 idlealt1
	{Vcalm_ArcMain_IdleMic0, {0, 106, 136, 107,}, { 30, 150}}, //35 idlealt2
	{Vcalm_ArcMain_IdleMic1, {0, 0, 136, 107,}, { 30, 150}},   //36 idlealt3
	{Vcalm_ArcMain_IdleMic1, {0, 108, 136, 107,}, { 30, 150}}, //37 idlealt4

};

static const Animation char_vcalm_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  ASCR_BACK, 0}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},         //CharAnim_Left
	{2, (const u8[]){ 26, 27, 27, ASCR_CHGANI, CharAnim_IdleAlt}},         //CharAnim_LeftAlt = up???
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},         //CharAnim_Down 
	{2, (const u8[]){ 29, 30, 30, ASCR_CHGANI, CharAnim_IdleAlt}},         //CharAnim_DownAlt = left???
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},         //CharAnim_Up 
	{2, (const u8[]){ 30, 31, 31, ASCR_CHGANI, CharAnim_IdleAlt}},        //CharAnim_UpAlt = right???
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{2, (const u8[]){32, 33, 33, ASCR_CHGANI, CharAnim_IdleAlt}},         //CharAnim_RightAlt = down???

	{3, (const u8[]){ 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, ASCR_CHGANI, CharAnim_IdleAlt}},   //CharAnim_PadBreak

	{2, (const u8[]){ 34, 35, 36, 37, 37, 37, 37,  ASCR_CHGANI, CharAnim_IdleAlt}},
};

//v Rage character functions
void Char_Vcalm_SetFrame(void *user, u8 frame)
{
	Char_Vcalm *this = (Char_Vcalm*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_vcalm_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Vcalm_Tick(Character *character)
{
	Char_Vcalm *this = (Char_Vcalm*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
	    Character_PerformIdle(character);


	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Vcalm_SetFrame);
	Character_Draw(character, &this->tex, &char_vcalm_frame[this->frame]);

	if (stage.flag & STAGE_FLAG_JUST_STEP)
    {   //Stage specific animations
		if (stage.note_scroll >= 0)
		{
			switch (stage.stage_id)
			{
				case StageId_1_3: //Harmony dance pad malfunction
				if ((stage.song_step) == 485)
					    character->set_anim(character, CharAnim_PadBreak);
					break;
				default:
					break;
			}
		}
	}
}

void Char_Vcalm_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Vcalm_Free(Character *character)
{
	Char_Vcalm *this = (Char_Vcalm*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Vcalm_New(fixed_t x, fixed_t y)
{
	//Allocate vcalm object
	Char_Vcalm *this = Mem_Alloc(sizeof(Char_Vcalm));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Vcalm_New] Failed to allocate v Rage object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Vcalm_Tick;
	this->character.set_anim = Char_Vcalm_SetAnim;
	this->character.free = Char_Vcalm_Free;
	
	Animatable_Init(&this->character.animatable, char_vcalm_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 12;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-125,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\VCALM.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Dad_ArcMain_Idle0
		"idle1.tim", //Dad_ArcMain_Idle1
		"left.tim",  //Dad_ArcMain_Left
		"down.tim",  //Dad_ArcMain_Down
		"up.tim",    //Dad_ArcMain_Up
		"right.tim", //Dad_ArcMain_Right
		"break0.tim",
		"break1.tim",
		"break2.tim",
		"break3.tim",
		"break4.tim",
		"break5.tim",
		"break6.tim",
		"upmic.tim",
		"leftmic.tim",
		"rightmic.tim",
		"downmic.tim",
		"idlemic0.tim",
		"idlemic1.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
