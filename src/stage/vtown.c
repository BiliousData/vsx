#include "vtown.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"

//Week 1 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //Stage and back
	Gfx_Tex tex_back1; //Sky
} Back_Vtown;

//Week 1 background functions
void Back_Vtown_DrawBG(StageBack *back)
{
	Back_Vtown *this = (Back_Vtown*)back;
	
	fixed_t fx, fy;
	
	//Draw stage
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT stage_src = {0, 0, 256, 81};
	RECT_FIXED stage_dst = {
		FIXED_DEC(-255,1) - FIXED_DEC(SCREEN_WIDEOADD,2) - fx,
		FIXED_DEC(30,1) - fy,
		FIXED_DEC(450,1) + FIXED_DEC(SCREEN_WIDEOADD,1),
		FIXED_DEC(115,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &stage_src, &stage_dst, stage.camera.bzoom);
	
	//Draw back
	fx = stage.camera.x * 2 / 3;
	fy = stage.camera.y * 2 / 3;
	
	RECT backl_src = {0, 59, 256, 197};
	RECT_FIXED backl_dst = {
		FIXED_DEC(-250,1) - fx,
		FIXED_DEC(-230,1) - fy,
		FIXED_DEC(450,1),
		FIXED_DEC(350,1)
	};

	fx = stage.camera.x * 1 / 3;
	fy = stage.camera.y * 1 / 3;

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
	
	Stage_DrawTex(&this->tex_back0, &backl_src, &backl_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_back1, &backr_src, &backr_dst, stage.camera.bzoom);
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
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
