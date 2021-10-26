#include "simp.h"

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
} Back_Simp;

//Week 1 background functions
void Back_Simp_DrawBG(StageBack *back)
{
	Back_Simp *this = (Back_Simp*)back;
	
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

void Back_Simp_Free(StageBack *back)
{
	Back_Simp *this = (Back_Simp*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Simp_New(void)
{
	//Allocate background structure
	Back_Simp *this = (Back_Simp*)Mem_Alloc(sizeof(Back_Simp));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Simp_DrawBG;
	this->back.free = Back_Simp_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK8\\SIMP.ARC;1");
	Gfx_LoadTex(&this->tex_back, Archive_Find(arc_back, "back.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
