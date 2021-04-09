#ifndef __TRIGRAPHICS_EXT_H__
#define __TRIGRAPHICS_EXT_H__

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <psprtc.h>
#include <stdio.h>
#include <string.h>
#include "triImage.h"
#include "triTypes.h"
#include "triMemory.h"
#include "triGraphics.h"
#include "triVAlloc.h"
#include "triLog.h"

#define MIN(a, b) ((a)<(b)?(a):(b))

#define TRI_FLIP_NONE 0
#define TRI_FLIP_H    1
#define TRI_FLIP_V    2

void swap(float *a, float *b)
{
    float temp=*a;
    *a = *b;
    *b = temp;
}

void triDrawImageExt( triFloat x, triFloat y, triFloat width, triFloat height,	// rect pos and size on screen
				   triFloat u0, triFloat v0, triFloat u1, triFloat v1,		// area of texture to render
                   int flipFlag,    // controls texture flipping
				   triImage* img )
{
	#ifdef TRI_PARANOIA
	if (!triInitialized) return;
	#endif
	if (img==0 || img->width==0 || img->height==0 || width==0 || height==0) return;
	
	// Support negative width/height to do easy flipping
	if (width < 0)
	{
		//x += width;
		triFloat u = u0; u0 = u1; u1 = u;
		width = -width;
	}
	if (height < 0)
	{
		//y += height;
		triFloat v = v0; v0 = v1; v1 = v;
		height = -height;
	}
	
	if (img->format==IMG_FORMAT_T4)
	{
		sceGuClutMode(img->palformat, 0, 0xff, 0);
		sceGuClutLoad(2, img->palette);
	}
	else if (img->format==IMG_FORMAT_T8)
	{
		sceGuClutMode(img->palformat, 0, 0xff, 0);
		sceGuClutLoad(32, img->palette);
	}
	sceGuTexMode(img->format, 0, 0, img->swizzled);
	sceGuEnable(GU_TEXTURE_2D);

	triFloat cur_v = v0;
	triFloat cur_y = y;
	triFloat v_end = v1;
	triFloat y_end = y + height;
	triFloat vslice = 512.f;
	triFloat ystep = (height/(v1-v0) * vslice);
	triFloat vstep = ((v1-v0) > 0 ? vslice : -vslice);
	
	triFloat x_end = x + width;
	triFloat uslice = 64.f;
	//triFloat ustep = (u1-u0)/width * xslice;
	triFloat xstep = (width/(u1-u0) * uslice);
	triFloat ustep = ((u1-u0) > 0 ? uslice : -uslice);
	
	triChar* data = img->data;
	for ( ; cur_y < y_end; cur_y+=ystep, cur_v+=vstep )
	{
		triFloat cur_u = u0;
		triFloat cur_x = x;
		triFloat u_end = u1;

		// support large images (height > 512)
		int off = (vstep>0) ? ((int)cur_v) : ((int)(cur_v+vstep));
		data += ((off*img->stride*img->bits) >> 3);
		cur_v -= off;
		v_end -= off;
		sceGuTexImage(0, MIN(512,img->stride), MIN(512,img->tex_height), img->stride, data);

		triFloat poly_height = ((cur_y+ystep) > y_end) ? (y_end-cur_y) : ystep;
		triFloat source_height = vstep;
		// support negative vsteps
		if ((vstep > 0) && (cur_v+vstep > v_end))
		{
			source_height = (v_end-cur_v);
		}
		else
		if ((vstep < 0) && (cur_v+vstep < v_end))
		{
			source_height = (cur_v-v_end);
		}
		
		triChar* udata = data;
		// blit maximizing the use of the texture-cache
		for( ; cur_x < x_end; cur_x+=xstep, cur_u+=ustep )
		{
			// support large images (width > 512)
			if (cur_u>512.f || cur_u+ustep>512.f)
			{
				int off = (ustep>0) ? ((int)cur_u & ~31) : ((int)(cur_u+ustep) & ~31);
				udata += ((off*img->bits) >> 3);
				cur_u -= off;
				u_end -= off;
				sceGuTexImage(0, MIN(512,img->stride), MIN(512,img->tex_height), img->stride, udata);
			}
			triVertUV* vertices = (triVertUV*)sceGuGetMemory(2 * sizeof(triVertUV));

			triFloat poly_width = ((cur_x+xstep) > x_end) ? (x_end-cur_x) : xstep;
			triFloat source_width = ustep;
			// support negative usteps
			if ((ustep > 0) && (cur_u+ustep > u_end))
			{
				source_width = (u_end-cur_u);
			}
			else
			if ((ustep < 0) && (cur_u+ustep < u_end))
			{
				source_width = (cur_u-u_end);
			}

			vertices[0].u = cur_u;//0
			vertices[0].v = cur_v;//0
			vertices[0].x = cur_x;//100 
			vertices[0].y = cur_y;//100 
			vertices[0].z = 0;

			vertices[1].u = cur_u + source_width;//32 
			vertices[1].v = cur_v + source_height;// 32
			vertices[1].x = cur_x + poly_width;// 132
			vertices[1].y = cur_y + poly_height;//132
			vertices[1].z = 0;

            /* vertical flip */
            if(flipFlag & TRI_FLIP_V)
            {
                swap(&vertices[0].u,&vertices[1].u);
            }
      


			sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
		}
	}
}

void triDrawImageAnimationExt( triFloat x, triFloat y, int flipFlag, triImageAnimation* ani )
{
	if (ani->curFrame==0) return;
	triDrawImageExt( x + ani->curFrame->xOffs, y + ani->curFrame->yOffs, ani->curFrame->sw, ani->curFrame->sh, 
					ani->curFrame->sx, ani->curFrame->sy, ani->curFrame->sx+ani->curFrame->sw, ani->curFrame->sy+ani->curFrame->sh, flipFlag, ani->curFrame->image );
}


#endif