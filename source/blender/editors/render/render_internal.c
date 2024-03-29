/*
 * $Id$
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2008 Blender Foundation.
 * All rights reserved.
 *
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/editors/render/render_internal.c
 *  \ingroup edrend
 */


#include <math.h>
#include <string.h>
#include <stddef.h>

#include "MEM_guardedalloc.h"

#include "BLI_blenlib.h"
#include "BLI_math.h"
#include "BLI_threads.h"
#include "BLI_rand.h"
#include "BLI_utildefines.h"

#include "DNA_scene_types.h"

#include "BKE_blender.h"
#include "BKE_context.h"
#include "BKE_global.h"
#include "BKE_image.h"
#include "BKE_library.h"
#include "BKE_main.h"
#include "BKE_node.h"
#include "BKE_multires.h"
#include "BKE_report.h"
#include "BKE_sequencer.h"
#include "BKE_screen.h"
#include "BKE_scene.h"
#include "BKE_colormanagement.h"

#include "WM_api.h"
#include "WM_types.h"

#include "ED_screen.h"
#include "ED_object.h"

#include "RE_pipeline.h"
#include "IMB_imbuf.h"
#include "IMB_imbuf_types.h"

#include "RNA_access.h"
#include "RNA_define.h"

#include "wm_window.h"

#include "render_intern.h"

/* Render Callbacks */

/* called inside thread! */
void image_buffer_rect_update(Scene *scene, RenderResult *rr, ImBuf *ibuf, volatile rcti *renrect)
{
	float x1, y1, *rectf= NULL;
	int ymin, ymax, xmin, xmax;
	int rymin, rxmin, do_color_management;
	char *rectc;

	/* if renrect argument, we only refresh scanlines */
	if(renrect) {
		/* if ymax==recty, rendering of layer is ready, we should not draw, other things happen... */
		if(rr->renlay==NULL || renrect->ymax>=rr->recty)
			return;

		/* xmin here is first subrect x coord, xmax defines subrect width */
		xmin = renrect->xmin + rr->crop;
		xmax = renrect->xmax - xmin + rr->crop;
		if(xmax<2)
			return;

		ymin= renrect->ymin + rr->crop;
		ymax= renrect->ymax - ymin + rr->crop;
		if(ymax<2)
			return;
		renrect->ymin= renrect->ymax;

	}
	else {
		xmin = ymin = rr->crop;
		xmax = rr->rectx - 2*rr->crop;
		ymax = rr->recty - 2*rr->crop;
	}

	/* xmin ymin is in tile coords. transform to ibuf */
	rxmin= rr->tilerect.xmin + xmin;
	if(rxmin >= ibuf->x) return;
	rymin= rr->tilerect.ymin + ymin;
	if(rymin >= ibuf->y) return;

	if(rxmin + xmax > ibuf->x)
		xmax= ibuf->x - rxmin;
	if(rymin + ymax > ibuf->y)
		ymax= ibuf->y - rymin;

	if(xmax < 1 || ymax < 1) return;

	/* find current float rect for display, first case is after composit... still weak */
	if(rr->rectf)
		rectf= rr->rectf;
	else {
		if(rr->rect32)
			return;
		else {
			if(rr->renlay==NULL || rr->renlay->rectf==NULL) return;
			rectf= rr->renlay->rectf;
		}
	}
	if(rectf==NULL) return;

	if(ibuf->rect==NULL)
		imb_addrectImBuf(ibuf);
	
	rectf+= 4*(rr->rectx*ymin + xmin);
	rectc= (char *)(ibuf->rect + ibuf->x*rymin + rxmin);

	do_color_management = (scene && (scene->r.color_mgt_flag & R_COLOR_MANAGEMENT));
	
	/* XXX make nice consistent functions for this */
	for(y1= 0; y1<ymax; y1++) {
		float *rf= rectf;
		float srgb[3];
		char *rc= rectc;
		const float dither = ibuf->dither / 255.0f;

		/* XXX temp. because crop offset */
		if(rectc >= (char *)(ibuf->rect)) {
			for(x1= 0; x1<xmax; x1++, rf += 4, rc+=4) {
				/* color management */
				if(do_color_management) {
					srgb[0]= linearrgb_to_srgb(rf[0]);
					srgb[1]= linearrgb_to_srgb(rf[1]);
					srgb[2]= linearrgb_to_srgb(rf[2]);
				}
				else {
					copy_v3_v3(srgb, rf);
				}

				/* dither */
				if(dither != 0.0f) {
					const float d = (BLI_frand()-0.5f)*dither;

					srgb[0] += d;
					srgb[1] += d;
					srgb[2] += d;
				}

				/* write */
				rc[0]= FTOCHAR(srgb[0]);
				rc[1]= FTOCHAR(srgb[1]);
				rc[2]= FTOCHAR(srgb[2]);
				rc[3]= FTOCHAR(rf[3]);
			}
		}

		rectf += 4*rr->rectx;
		rectc += 4*ibuf->x;
	}
}

/* ****************************** render invoking ***************** */

/* set callbacks, exported to sequence render too.
 Only call in foreground (UI) renders. */

/* executes blocking render */
static int screen_render_exec(bContext *C, wmOperator *op)
{
	Scene *scene= CTX_data_scene(C);
	Render *re= RE_NewRender(scene->id.name);
	Image *ima;
	View3D *v3d= CTX_wm_view3d(C);
	Main *mainp= CTX_data_main(C);
	unsigned int lay= (v3d)? v3d->lay: scene->lay;
	const short is_animation= RNA_boolean_get(op->ptr, "animation");
	const short is_write_still= RNA_boolean_get(op->ptr, "write_still");
	struct Object *camera_override= v3d ? V3D_CAMERA_LOCAL(v3d) : NULL;

	if(!is_animation && is_write_still && BKE_imtype_is_movie(scene->r.imtype)) {
		BKE_report(op->reports, RPT_ERROR, "Can't write a single file with an animation format selected.");
		return OPERATOR_CANCELLED;
	}

	G.afbreek= 0;
	RE_test_break_cb(re, NULL, (int (*)(void *)) blender_test_break);

	ima= BKE_image_verify_viewer(IMA_TYPE_R_RESULT, "Render Result");
	BKE_image_signal(ima, NULL, IMA_SIGNAL_FREE);
	BKE_image_backup_render(scene, ima);

	/* cleanup sequencer caches before starting user triggered render.
	   otherwise, invalidated cache entries can make their way into
	   the output rendering. We can't put that into RE_BlenderFrame,
	   since sequence rendering can call that recursively... (peter) */
	seq_stripelem_cache_cleanup();

	RE_SetReports(re, op->reports);

	if(is_animation)
		RE_BlenderAnim(re, mainp, scene, camera_override, lay, scene->r.sfra, scene->r.efra, scene->r.frame_step);
	else
		RE_BlenderFrame(re, mainp, scene, NULL, camera_override, lay, scene->r.cfra, is_write_still);

	RE_SetReports(re, NULL);

	// no redraw needed, we leave state as we entered it
	ED_update_for_newframe(mainp, scene, CTX_wm_screen(C), 1);

	WM_event_add_notifier(C, NC_SCENE|ND_RENDER_RESULT, scene);

	return OPERATOR_FINISHED;
}

typedef struct RenderJob {
	Main *main;
	Scene *scene;
	Render *re;
	wmWindow *win;
	SceneRenderLayer *srl;
	struct Object *camera_override;
	int lay;
	short anim, write_still;
	Image *image;
	ImageUser iuser;
	short *stop;
	short *do_update;
	float *progress;
	ReportList *reports;
} RenderJob;

static void render_freejob(void *rjv)
{
	RenderJob *rj= rjv;

	MEM_freeN(rj);
}

/* str is IMA_MAX_RENDER_TEXT in size */
static void make_renderinfo_string(RenderStats *rs, Scene *scene, char *str)
{
	char info_time_str[32];	// used to be extern to header_info.c
	uintptr_t mem_in_use, mmap_in_use, peak_memory;
	float megs_used_memory, mmap_used_memory, megs_peak_memory;
	char *spos= str;

	mem_in_use= MEM_get_memory_in_use();
	mmap_in_use= MEM_get_mapped_memory_in_use();
	peak_memory = MEM_get_peak_memory();

	megs_used_memory= (mem_in_use-mmap_in_use)/(1024.0*1024.0);
	mmap_used_memory= (mmap_in_use)/(1024.0*1024.0);
	megs_peak_memory = (peak_memory)/(1024.0*1024.0);

	if(scene->lay & 0xFF000000)
		spos+= sprintf(spos, "Localview | ");
	else if(scene->r.scemode & R_SINGLE_LAYER)
		spos+= sprintf(spos, "Single Layer | ");

	if(rs->statstr) {
		spos+= sprintf(spos, "%s ", rs->statstr);
	}
	else {
		spos+= sprintf(spos, "Fra:%d  Ve:%d Fa:%d ", (scene->r.cfra), rs->totvert, rs->totface);
		if(rs->tothalo) spos+= sprintf(spos, "Ha:%d ", rs->tothalo);
		if(rs->totstrand) spos+= sprintf(spos, "St:%d ", rs->totstrand);
		spos+= sprintf(spos, "La:%d Mem:%.2fM (%.2fM, peak %.2fM) ", rs->totlamp, megs_used_memory, mmap_used_memory, megs_peak_memory);

		if(rs->curfield)
			spos+= sprintf(spos, "Field %d ", rs->curfield);
		if(rs->curblur)
			spos+= sprintf(spos, "Blur %d ", rs->curblur);
	}

	BLI_timestr(rs->lastframetime, info_time_str);
	spos+= sprintf(spos, "Time:%s ", info_time_str);

	if(rs->curfsa)
		spos+= sprintf(spos, "| Full Sample %d ", rs->curfsa);
	
	if(rs->infostr && rs->infostr[0])
		spos+= sprintf(spos, "| %s ", rs->infostr);

	/* very weak... but 512 characters is quite safe */
	if(spos >= str+IMA_MAX_RENDER_TEXT)
		if (G.f & G_DEBUG)
			printf("WARNING! renderwin text beyond limit \n");

}

static void image_renderinfo_cb(void *rjv, RenderStats *rs)
{
	RenderJob *rj= rjv;
	RenderResult *rr;

	rr= RE_AcquireResultRead(rj->re);

	if(rr) {
		/* malloc OK here, stats_draw is not in tile threads */
		if(rr->text==NULL)
			rr->text= MEM_callocN(IMA_MAX_RENDER_TEXT, "rendertext");

		make_renderinfo_string(rs, rj->scene, rr->text);
	}

	RE_ReleaseResult(rj->re);

	/* make jobs timer to send notifier */
	*(rj->do_update)= 1;

}

static void render_progress_update(void *rjv, float progress)
{
	RenderJob *rj= rjv;
	
	if(rj->progress)
		*rj->progress = progress;
}

static void image_rect_update(void *rjv, RenderResult *rr, volatile rcti *renrect)
{
	RenderJob *rj= rjv;
	Image *ima= rj->image;
	ImBuf *ibuf;
	void *lock;

	/* only update if we are displaying the slot being rendered */
	if(ima->render_slot != ima->last_render_slot)
		return;

	ibuf= BKE_image_acquire_ibuf(ima, &rj->iuser, &lock);
	if(ibuf) {
		image_buffer_rect_update(rj->scene, rr, ibuf, renrect);

		/* make jobs timer to send notifier */
		*(rj->do_update)= 1;
	}
	BKE_image_release_ibuf(ima, lock);
}

static void render_startjob(void *rjv, short *stop, short *do_update, float *progress)
{
	RenderJob *rj= rjv;

	rj->stop= stop;
	rj->do_update= do_update;
	rj->progress= progress;

	RE_SetReports(rj->re, rj->reports);

	if(rj->anim)
		RE_BlenderAnim(rj->re, rj->main, rj->scene, rj->camera_override, rj->lay, rj->scene->r.sfra, rj->scene->r.efra, rj->scene->r.frame_step);
	else
		RE_BlenderFrame(rj->re, rj->main, rj->scene, rj->srl, rj->camera_override, rj->lay, rj->scene->r.cfra, rj->write_still);

	RE_SetReports(rj->re, NULL);
}

static void render_endjob(void *rjv)
{
	RenderJob *rj= rjv;	

	/* this render may be used again by the sequencer without the active 'Render' where the callbacks
	 * would be re-assigned. assign dummy callbacks to avoid referencing freed renderjobs bug [#24508] */
	RE_InitRenderCB(rj->re);

	if(rj->main != G.main)
		free_main(rj->main);

	/* else the frame will not update for the original value */
	if(!(rj->scene->r.scemode & R_NO_FRAME_UPDATE))
		ED_update_for_newframe(G.main, rj->scene, rj->win->screen, 1);
	
	/* XXX above function sets all tags in nodes */
	ntreeClearTags(rj->scene->nodetree);
	
	/* potentially set by caller */
	rj->scene->r.scemode &= ~R_NO_FRAME_UPDATE;
	
	if(rj->srl) {
		NodeTagIDChanged(rj->scene->nodetree, &rj->scene->id);
		WM_main_add_notifier(NC_NODE|NA_EDITED, rj->scene);
	}
	
	/* XXX render stability hack */
	G.rendering = 0;
	WM_main_add_notifier(NC_WINDOW, NULL);
}

/* called by render, check job 'stop' value or the global */
static int render_breakjob(void *rjv)
{
	RenderJob *rj= rjv;

	if(G.afbreek)
		return 1;
	if(rj->stop && *(rj->stop))
		return 1;
	return 0;
}

/* runs in thread, no cursor setting here works. careful with notifiers too (malloc conflicts) */
/* maybe need a way to get job send notifer? */
static void render_drawlock(void *UNUSED(rjv), int lock)
{
	BKE_spacedata_draw_locks(lock);
	
}

/* catch esc */
static int screen_render_modal(bContext *C, wmOperator *UNUSED(op), wmEvent *event)
{
	/* no running blender, remove handler and pass through */
	if(0==WM_jobs_test(CTX_wm_manager(C), CTX_data_scene(C))) {
		return OPERATOR_FINISHED|OPERATOR_PASS_THROUGH;
	}

	/* running render */
	switch (event->type) {
		case ESCKEY:
			return OPERATOR_RUNNING_MODAL;
			break;
	}
	return OPERATOR_PASS_THROUGH;
}

/* using context, starts job */
static int screen_render_invoke(bContext *C, wmOperator *op, wmEvent *event)
{
	/* new render clears all callbacks */
	Main *mainp;
	Scene *scene= CTX_data_scene(C);
	SceneRenderLayer *srl=NULL;
	bScreen *screen= CTX_wm_screen(C);
	View3D *v3d= CTX_wm_view3d(C);
	Render *re;
	wmJob *steve;
	RenderJob *rj;
	Image *ima;
	int jobflag;
	const short is_animation= RNA_boolean_get(op->ptr, "animation");
	const short is_write_still= RNA_boolean_get(op->ptr, "write_still");
	struct Object *camera_override= v3d ? V3D_CAMERA_LOCAL(v3d) : NULL;
	
	/* only one render job at a time */
	if(WM_jobs_test(CTX_wm_manager(C), scene))
		return OPERATOR_CANCELLED;

	if(!RE_is_rendering_allowed(scene, camera_override, op->reports)) {
		return OPERATOR_CANCELLED;
	}

	if(!is_animation && is_write_still && BKE_imtype_is_movie(scene->r.imtype)) {
		BKE_report(op->reports, RPT_ERROR, "Can't write a single file with an animation format selected.");
		return OPERATOR_CANCELLED;
	}	
	
	/* stop all running jobs, currently previews frustrate Render */
	WM_jobs_stop_all(CTX_wm_manager(C));

	/* get main */
	if(G.rt == 101) {
		/* thread-safety experiment, copy main from the undo buffer */
		mainp= BKE_undo_get_main(&scene);
	}
	else
		mainp= CTX_data_main(C);

	/* cancel animation playback */
	if (screen->animtimer)
		ED_screen_animation_play(C, 0, 0);
	
	/* handle UI stuff */
	WM_cursor_wait(1);

	/* flush multires changes (for sculpt) */
	multires_force_render_update(CTX_data_active_object(C));

	/* cleanup sequencer caches before starting user triggered render.
	   otherwise, invalidated cache entries can make their way into
	   the output rendering. We can't put that into RE_BlenderFrame,
	   since sequence rendering can call that recursively... (peter) */
	seq_stripelem_cache_cleanup();

	/* get editmode results */
	ED_object_exit_editmode(C, 0);	/* 0 = does not exit editmode */

	// store spare
	// get view3d layer, local layer, make this nice api call to render
	// store spare

	/* ensure at least 1 area shows result */
	render_view_open(C, event->x, event->y);

	jobflag= WM_JOB_EXCL_RENDER|WM_JOB_PRIORITY|WM_JOB_PROGRESS;
	
	/* single layer re-render */
	if(RNA_property_is_set(op->ptr, "layer")) {
		SceneRenderLayer *rl;
		Scene *scn;
		char scene_name[MAX_ID_NAME-2], rl_name[RE_MAXNAME];

		RNA_string_get(op->ptr, "layer", rl_name);
		RNA_string_get(op->ptr, "scene", scene_name);

		scn = (Scene *)BLI_findstring(&mainp->scene, scene_name, offsetof(ID, name) + 2);
		rl = (SceneRenderLayer *)BLI_findstring(&scene->r.layers, rl_name, offsetof(SceneRenderLayer, name));
		
		if (scn && rl) {
			/* camera switch wont have updated */
			scn->r.cfra= scene->r.cfra;
			scene_camera_switch_update(scn);

			scene = scn;
			srl = rl;
		}
		jobflag |= WM_JOB_SUSPEND;
	}

	/* job custom data */
	rj= MEM_callocN(sizeof(RenderJob), "render job");
	rj->main= mainp;
	rj->scene= scene;
	rj->win= CTX_wm_window(C);
	rj->srl = srl;
	rj->camera_override = camera_override;
	rj->lay = (v3d)? v3d->lay: scene->lay;
	rj->anim= is_animation;
	rj->write_still= is_write_still && !is_animation;
	rj->iuser.scene= scene;
	rj->iuser.ok= 1;
	rj->reports= op->reports;

	/* setup job */
	steve= WM_jobs_get(CTX_wm_manager(C), CTX_wm_window(C), scene, "Render", jobflag);
	WM_jobs_customdata(steve, rj, render_freejob);
	WM_jobs_timer(steve, 0.2, NC_SCENE|ND_RENDER_RESULT, 0);
	WM_jobs_callbacks(steve, render_startjob, NULL, NULL, render_endjob);

	/* get a render result image, and make sure it is empty */
	ima= BKE_image_verify_viewer(IMA_TYPE_R_RESULT, "Render Result");
	BKE_image_signal(ima, NULL, IMA_SIGNAL_FREE);
	BKE_image_backup_render(rj->scene, ima);
	rj->image= ima;

	/* setup new render */
	re= RE_NewRender(scene->id.name);
	RE_test_break_cb(re, rj, render_breakjob);
	RE_draw_lock_cb(re, rj, render_drawlock);
	RE_display_draw_cb(re, rj, image_rect_update);
	RE_stats_draw_cb(re, rj, image_renderinfo_cb);
	RE_progress_cb(re, rj, render_progress_update);

	rj->re= re;
	G.afbreek= 0;

	WM_jobs_start(CTX_wm_manager(C), steve);

	WM_cursor_wait(0);
	WM_event_add_notifier(C, NC_SCENE|ND_RENDER_RESULT, scene);

	/* we set G.rendering here already instead of only in the job, this ensure
	   main loop or other scene updates are disabled in time, since they may
	   have started before the job thread */
	G.rendering = 1;

	/* add modal handler for ESC */
	WM_event_add_modal_handler(C, op);

	return OPERATOR_RUNNING_MODAL;
}

/* contextual render, using current scene, view3d? */
void RENDER_OT_render(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Render";
	ot->description= "Render active scene";
	ot->idname= "RENDER_OT_render";

	/* api callbacks */
	ot->invoke= screen_render_invoke;
	ot->modal= screen_render_modal;
	ot->exec= screen_render_exec;

	/*ot->poll= ED_operator_screenactive;*/ /* this isnt needed, causes failer in background mode */

	RNA_def_boolean(ot->srna, "animation", 0, "Animation", "Render files from the animation range of this scene");
	RNA_def_boolean(ot->srna, "write_still", 0, "Write Image", "Save rendered the image to the output path (used only when animation is disabled)");
	RNA_def_string(ot->srna, "layer", "", RE_MAXNAME, "Render Layer", "Single render layer to re-render");
	RNA_def_string(ot->srna, "scene", "", MAX_ID_NAME-2, "Scene", "Re-render single layer in this scene");
}

