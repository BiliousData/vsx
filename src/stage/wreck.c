#include "wreck.h"

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
} Back_Wreck;

//Week 1 background functions
void Back_Wreck_DrawBG(StageBack *back)
{
	Back_Wreck *this = (Back_Wreck*)back;
	
	fixed_t fx, fy;
	
	//Draw the shit
	fx = stage.camera.x;
	fy = stage.camera.y;

	RECT backr_src = {0, 0, 256, 173};
	RECT_FIXED backr_dst = {
		FIXED_DEC(-250,1) - fx,
		FIXED_DEC(-200,1) - fy,
		FIXED_DEC(450,1),
		FIXED_DEC(350,1)
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

void Back_Wreck_Free(StageBack *back)
{
	Back_Wreck *this = (Back_Wreck*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Wreck_New(void)
{
	//Allocate background structure
	Back_Wreck *this = (Back_Wreck*)Mem_Alloc(sizeof(Back_Wreck));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Wreck_DrawBG;
	this->back.free = Back_Wreck_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK8\\WRECK.ARC;1");
	Gfx_LoadTex(&this->tex_back, Archive_Find(arc_back, "back.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
