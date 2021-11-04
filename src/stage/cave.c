#include "cave.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"

//Week 1 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back; //Entire Stage
} Back_Cave;

//Week 1 background functions
void Back_Cave_DrawBG(StageBack *back)
{
	Back_Cave *this = (Back_Cave*)back;
	
	fixed_t fx, fy;
	
	//Draw the shit
	fx = stage.camera.x;
	fy = stage.camera.y;

	RECT backr_src = {0, 0, 256, 173};
	RECT_FIXED backr_dst = {
		FIXED_DEC(-250,1) - fx,
		FIXED_DEC(-200,1) - fy,
		FIXED_DEC(550,1),
		FIXED_DEC(270,1)
	};
	RECT backf_src = {0, 59, 1, 1};
	RECT backf_dst = {
		0,
		0,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
	};
	
	Stage_DrawTex(&this->tex_back, &backr_src, &backr_dst, stage.camera.bzoom);
	Gfx_DrawTex(&this->tex_back, &backf_src, &backf_dst);
}

void Back_Cave_Free(StageBack *back)
{
	Back_Cave *this = (Back_Cave*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Cave_New(void)
{
	//Allocate background structure
	Back_Cave *this = (Back_Cave*)Mem_Alloc(sizeof(Back_Cave));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Cave_DrawBG;
	this->back.free = Back_Cave_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK8\\CAVE.ARC;1");
	Gfx_LoadTex(&this->tex_back, Archive_Find(arc_back, "back.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
