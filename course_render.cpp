/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tuxracer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#include "textures.h"
#include "course_render.h"
#include "course.h"
#include "ogl.h"
#include "quadtree.h"
#include "particles.h"
#include "env.h"
#include "game_ctrl.h"

static const bool clip_course = true;

// --------------------------------------------------------------------	
//							render course
// --------------------------------------------------------------------	
void RenderCourse () {
    set_gl_options (COURSE);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    set_material (colWhite, colBlack, 1.0);

    CControl *ctrl = Players.GetCtrl (g_game.player_id);
    UpdateQuadtree (ctrl->viewpos, param.course_detail_level);
    RenderQuadtree ();
}

// --------------------------------------------------------------------
//				DrawTrees
// --------------------------------------------------------------------
void DrawTrees() {
    TCollidable	*treeLocs;
    int       	numTrees;
    TItem    	*itemLocs;
    int       	numItems;
    double  	treeRadius;
    double  	treeHeight;
    int       	i;
    TVector3  	normal;
    double  	fwd_clip_limit, bwd_clip_limit;
    double	fwd_tree_detail_limit;
    int		tree_type = -1;
    double  	itemRadius;
    double  	itemHeight;
    int       	item_type = -1;
	TObjectType	*object_types = Course.ObjTypes;
	CControl *ctrl = Players.GetCtrl (g_game.player_id);

	set_gl_options (TREES); 

    fwd_clip_limit = param.forward_clip_distance;
    bwd_clip_limit = param.backward_clip_distance;
    fwd_tree_detail_limit = param.tree_detail_distance;

    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    set_material (colWhite, colBlack, 1.0);

    const GLfloat tex[] = {
	0,0,  1,0,  1,1,  0,1
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

//	-------------- trees ------------------------
    treeLocs = Course.CollArr;
    numTrees = Course.numColl;

    for (i = 0; i< numTrees; i++) {
	if (clip_course) {
		if (ctrl->viewpos.z - treeLocs[i].pt.z > fwd_clip_limit) continue;
		if (treeLocs[i].pt.z - ctrl->viewpos.z > bwd_clip_limit) continue;
	}

	if (treeLocs[i].tree_type != tree_type) {
		tree_type = treeLocs[i].tree_type;
		glBindTexture (GL_TEXTURE_2D, object_types[tree_type].texid);
	}

	glPushMatrix();
        	glTranslatef (treeLocs[i].pt.x, treeLocs[i].pt.y, treeLocs[i].pt.z);
		if (param.perf_level > 1) glRotatef (1, 0, 1, 0);

        	treeRadius = treeLocs[i].diam / 2.0;
        	treeHeight = treeLocs[i].height;
		normal = MakeVector (0, 0, 1);

		// slower but better method of setting the normals
/*		normal = SubtractVectors (ctrl->viewpos, treeLocs[i].pt);
		NormVector (&normal); */
		glNormal3f (normal.x, normal.y, normal.z);

		const GLfloat vtx1[] = {
			-treeRadius, 0.0, 0.0,
			+treeRadius, 0.0, 0.0,
			+treeRadius, treeHeight, 0.0,
			-treeRadius, treeHeight, 0.0
		};
		const GLfloat vtx2[] = {
			0.0, 0.0, -treeRadius,
			0.0, 0.0, +treeRadius,
			0.0, treeHeight, +treeRadius,
			0.0, treeHeight, -treeRadius
		};

		glVertexPointer(3, GL_FLOAT, 0, vtx1);
		glTexCoordPointer(2, GL_FLOAT, 0, tex);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

//		if  (!clip_course || ctrl->viewpos.z - treeLocs[i].pt.z < fwd_tree_detail_limit) {
			glVertexPointer(3, GL_FLOAT, 0, vtx2);
			glTexCoordPointer(2, GL_FLOAT, 0, tex);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//		}
	glPopMatrix();
    }
	
//  items -----------------------------
    itemLocs = Course.NocollArr;
    numItems = Course.numNocoll;

    for (i = 0; i< numItems; i++) {
		if (itemLocs[i].collectable == 0 || itemLocs[i].drawable == false) continue;
		if (clip_course) {
		    if (ctrl->viewpos.z - itemLocs[i].pt.z > fwd_clip_limit) continue;
		    if (itemLocs[i].pt.z - ctrl->viewpos.z > bwd_clip_limit) continue;
		}
	
		if (itemLocs[i].item_type != item_type) {
		    item_type = itemLocs[i].item_type;
		    glBindTexture (GL_TEXTURE_2D, object_types[item_type].texid);
		}

		itemRadius = itemLocs[i].diam / 2;
		itemHeight = itemLocs[i].height;

		if (object_types[item_type].use_normal) {
			normal = object_types[item_type].normal;
		} else {
//			normal = MakeVector (0, 0, 1);
			normal = SubtractVectors (ctrl->viewpos, itemLocs[i].pt);
			NormVector (&normal);
		}

		glNormal3f (normal.x, normal.y, normal.z);
		normal.y = 0.0;
		NormVector (&normal);

		const GLfloat vtx[] = {
			itemLocs[i].pt.x-itemRadius*normal.z, itemLocs[i].pt.y, itemLocs[i].pt.z+itemRadius*normal.x,
			itemLocs[i].pt.x+itemRadius*normal.z, itemLocs[i].pt.y, itemLocs[i].pt.z-itemRadius*normal.x,
			itemLocs[i].pt.x+itemRadius*normal.z, itemLocs[i].pt.y+itemHeight, itemLocs[i].pt.z-itemRadius*normal.x,
			itemLocs[i].pt.x-itemRadius*normal.z, itemLocs[i].pt.y+itemHeight, itemLocs[i].pt.z+itemRadius*normal.x
		};

		glVertexPointer(3, GL_FLOAT, 0, vtx);
		glTexCoordPointer(2, GL_FLOAT, 0, tex);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    } 

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
} 


