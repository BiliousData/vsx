/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "vtown.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"

// /v/ town background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //Uhh everything
} Back_Vtown;

// /v/ town 1 background functions
void Back_Vtown_DrawBG(StageBack *back)
{
	Back_Vtown *this = (Back_Vtown*)back;
	
	fixed_t fx, fy;
	
	//Draw stage
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	//Draw window light
	RECT stage_src = {0, 128, 256, 128};
	RECT_FIXED stage_dst = {
		FIXED_DEC(-130,1) - fx,
		FIXED_DEC(44,1) - fy,
		FIXED_DEC(350,1),
		FIXED_DEC(148,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &stage_src, &stage_dst, stage.camera.bzoom);
	
	//Draw back
	fx = stage.camera.x * 2 / 3;
	fy = stage.camera.y * 2 / 3;
	
	RECT backl_src = {0, 59, 121, 105};
	RECT_FIXED backl_dst = {
		FIXED_DEC(-190,1) - fx,
		FIXED_DEC(-100,1) - fy,
		FIXED_DEC(121,1),
		FIXED_DEC(105,1)
	};
	RECT backr_src = {121, 59, 136, 120};
	RECT_FIXED backr_dst = {
		FIXED_DEC(60,1) - fx,
		FIXED_DEC(-110,1) - fy,
		FIXED_DEC(136,1),
		FIXED_DEC(120,1)
	};
	RECT backf_src = {0, 59, 1, 1};
	RECT backf_dst = {
		0,
		0,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
	};
	
	Stage_DrawTex(&this->tex_back0, &backl_src, &backl_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_back0, &backr_src, &backr_dst, stage.camera.bzoom);
	Gfx_DrawTex(&this->tex_back0, &backf_src, &backf_dst);
}

void Back_Vtown_Free(StageBack *back)
{
	Back_Vtown *this = (Back_Vtown*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Vtown_New(void)
{
	//Allocate background structure
	Back_Vtown *this = (Back_Vtown*)Mem_Alloc(sizeof(Back_Vtown));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Vtown_DrawBG;
	this->back.free = Back_Vtown_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK8\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
