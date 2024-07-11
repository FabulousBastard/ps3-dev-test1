
/* 
   TINY3D sample / (c) 2010 Hermes  <www.elotrolado.net>

*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#include <io/pad.h>

#include <sysmodule/sysmodule.h>
#include <pngdec/pngdec.h>

#include <tiny3d.h>

#include "dorian_png_bin.h"


//__asm volatile("trap");


pngData dorianTexture; // PNG container of texture
u32 dorianTexture_offset; // offset for texture (used to pass the texture)

// draw one background color in virtual 2D coordinates

void DrawBackground2D(u32 rgba)
{
    tiny3d_SetPolygon(TINY3D_QUADS);

    tiny3d_VertexPos(0  , 0  , 65535);
    tiny3d_VertexColor(rgba);

    tiny3d_VertexPos(847, 0  , 65535);

    tiny3d_VertexPos(847, 511, 65535);

    tiny3d_VertexPos(0  , 511, 65535);
    tiny3d_End();
}

void DrawSprites2D(float x, float y, float layer, float dx, float dy, u32 rgba)
{
    tiny3d_SetPolygon(TINY3D_QUADS);

    tiny3d_VertexPos(x     , y     , layer);
    tiny3d_VertexColor(rgba);
    tiny3d_VertexTexture(0.0f, 0.0f);

    tiny3d_VertexPos(x + dx, y     , layer);
    tiny3d_VertexTexture(0.99f, 0.0f);

    tiny3d_VertexPos(x + dx, y + dy, layer);
    tiny3d_VertexTexture(0.99f, 0.99f);

    tiny3d_VertexPos(x     , y + dy, layer);
    tiny3d_VertexTexture(0.0f, 0.99f);

    tiny3d_End();
}

/*
void DrawSpritesRot2D(float x, float y, float layer, float dx, float dy, u32 rgba, float angle)
{
    dx /= 2.0f; dy /= 2.0f;

    MATRIX matrix;
    
    // rotate and translate the sprite
    matrix = MatrixRotationZ(angle);
    matrix = MatrixMultiply(matrix, MatrixTranslation(x + dx, y + dy, 0.0f));
    
    // fix ModelView Matrix
    tiny3d_SetMatrixModelView(&matrix);
   
    tiny3d_SetPolygon(TINY3D_QUADS);

    tiny3d_VertexPos(-dx, -dy, layer);
    tiny3d_VertexColor(rgba);
    tiny3d_VertexTexture(0.0f , 0.0f);

    tiny3d_VertexPos(dx , -dy, layer);
    tiny3d_VertexTexture(0.99f, 0.0f);

    tiny3d_VertexPos(dx , dy , layer);
    tiny3d_VertexTexture(0.99f, 0.99f);

    tiny3d_VertexPos(-dx, dy , layer);
    tiny3d_VertexTexture(0.0f , 0.99f);

    tiny3d_End();

    tiny3d_SetMatrixModelView(NULL); // set matrix identity

}
*/

struct item{

    float x, y;
    float dx, dy;
    int frame;
    u32 color;

} item;

void drawScene()
{
	int i = 0;

   

    tiny3d_Project2D(); // change to 2D context (remember you it works with 848 x 512 as virtual coordinates)

    // fix Perspective Projection Matrix

    DrawBackground2D(0x9191a9ff) ; // (r, g, b, a) 0xff0000ff ##!!! 

       
    // Load sprite texture
    tiny3d_SetTexture(0, dorianTexture_offset, dorianTexture.width,
        dorianTexture.height, dorianTexture.pitch,
        TINY3D_TEX_FORMAT_A8R8G8B8, TEXTURE_LINEAR);

         
        // draw sprite
    DrawSprites2D(item.x, item.y, (float) i, 473, 500, item.color);
        
    

}

void Load_PNG()
{

    // load PNG from memory

    pngLoadFromBuffer(dorian_png_bin, dorian_png_bin_size, &dorianTexture);

}

void LoadTexture()
{

    u32 * texture_mem = tiny3d_AllocTexture(64*1024*1024); // alloc 64MB ((64*1024*1024) 64 Mib??? )* of space for textures (this pointer can be global)
    
    u32 * texture_pointer; // use to asign texture space without changes texture_mem

    if(!texture_mem) return; // fail!

    texture_pointer = texture_mem;


    Load_PNG();

    // copy texture datas from PNG to the RSX memory allocated for textures

        dorianTexture_offset = 0;
       
        if(dorianTexture.bmp_out) {

            memcpy(texture_pointer, dorianTexture.bmp_out, dorianTexture.pitch * dorianTexture.height);
            
            free(dorianTexture.bmp_out); // free the PNG because i don't need this datas

            dorianTexture_offset = tiny3d_TextureOffset(texture_pointer);      // get the offset (RSX use offset instead address)

            texture_pointer += ((dorianTexture.pitch * dorianTexture.height + 15) & ~15) / 4; // aligned to 16 bytes (it is u32) and update the pointer
         }
}


void exiting()
{

    sysModuleUnload(SYSMODULE_PNGDEC);
  
}

s32 main()
{
	padInfo padinfo;
	padData paddata;
	int i;
	
	tiny3d_Init(1024*1024);

	ioPadInit(7);
    
    sysModuleLoad(SYSMODULE_PNGDEC);

    atexit(exiting); // Tiny3D register the event 3 and do exit() call when you exit  to the menu

	// Load texture

    LoadTexture();

    /* data for the ghost */

    item.x     = (1280 / 2) - 473 ;
    item.y     = 0.0f;
    item.dx    = 1.5f;
    item.dy    = 1.5f;
    item.frame = 0;
    item.color = 0xffffffff;

   
	
	// Ok, everything is setup. Now for the main loop.
	while(1) {

        /* DRAWING STARTS HERE */

        // clear the screen, buffer Z and initializes environment to 2D

        tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);

        // Enable alpha Test
        tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);

        // Enable alpha blending.
        tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
            TINY3D_BLEND_FUNC_DST_RGB_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_DST_ALPHA_ZERO,
            TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);
      

		// Check the pads.
		ioPadGetInfo(&padinfo);

		for(i = 0; i < MAX_PADS; i++){

			if(padinfo.status[i]){
				ioPadGetData(i, &paddata);
				
				if(paddata.BTN_CROSS){
					return 0;
				}
			}
			
		}

        drawScene(); // Draw

        /* DRAWING FINISH HERE */

        tiny3d_Flip();
		
	}
	
	return 0;
}

