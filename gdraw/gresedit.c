/* Copyright (C) 2008 by George Williams */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <ggadget.h>
#include "ggadgetP.h"
#include <gwidget.h>
#include <gresedit.h>
#include <string.h>
#include <ustring.h>

typedef struct greseditdlg {
    struct tofree {
	GGadgetCreateData *gcd;
	GTextInfo *lab;
	GGadgetCreateData mainbox[2], flagsbox, colbox, extrabox, ibox, fontbox,
	sabox;
	GGadgetCreateData *marray[11], *farray[5][6], *carray[12][9], *(*earray)[8];
	GGadgetCreateData *iarray[4], *fontarray[5], *saarray[5];
	char *fontname;
	char **extradefs;
	char bw[20], padding[20], rr[20];
	GResInfo *res;
	int startcid, fontcid, btcid;
    } *tofree;
    GWindow gw;
    GGadget *tabset;
    const char *def_res_file;
    void (*change_res_filename)(const char *);
    int done;
} GRE;

static GTextInfo bordertype[] = {
    { (unichar_t *) "None", NULL, 0, 0, (void *) (intpt) bt_none, NULL, 0, 0, 0, 0, 0, 0, 1},
    { (unichar_t *) "Box", NULL, 0, 0, (void *) (intpt) bt_box, NULL, 0, 0, 0, 0, 0, 0, 1},
    { (unichar_t *) "Raised", NULL, 0, 0, (void *) (intpt) bt_raised, NULL, 0, 0, 0, 0, 0, 0, 1},
    { (unichar_t *) "Lowered", NULL, 0, 0, (void *) (intpt) bt_lowered, NULL, 0, 0, 0, 0, 0, 0, 1},
    { (unichar_t *) "Engraved", NULL, 0, 0, (void *) (intpt) bt_engraved, NULL, 0, 0, 0, 0, 0, 0, 1},
    { (unichar_t *) "Embossed", NULL, 0, 0, (void *) (intpt) bt_embossed, NULL, 0, 0, 0, 0, 0, 0, 1},
    { (unichar_t *) "Double", NULL, 0, 0, (void *) (intpt) bt_double, NULL, 0, 0, 0, 0, 0, 0, 1},
    { NULL }
};

static GTextInfo bordershape[] = {
    { (unichar_t *) "Rect", NULL, 0, 0, (void *) (intpt) bs_rect, NULL, 0, 0, 0, 0, 0, 0, 1},
    { (unichar_t *) "Round Rect", NULL, 0, 0, (void *) (intpt) bs_roundrect, NULL, 0, 0, 0, 0, 0, 0, 1},
    { (unichar_t *) "Elipse", NULL, 0, 0, (void *) (intpt) bs_elipse, NULL, 0, 0, 0, 0, 0, 0, 1},
    { (unichar_t *) "Diamond", NULL, 0, 0, (void *) (intpt) bs_diamond, NULL, 0, 0, 0, 0, 0, 0, 1},
    { NULL }
};

static void GRE_RefreshAll(GRE *gre) {
    GDrawRequestExpose(gre->gw,NULL,false);
    GDrawRequestExpose(GTabSetGetSubwindow(gre->tabset,GTabSetGetSel(gre->tabset)),NULL,false);
}

static void GRE_Reflow(GRE *gre,GResInfo *res) {
    if ( res->examples!=NULL &&
	    ( res->examples->creator==GHBoxCreate ||
	      res->examples->creator==GVBoxCreate ||
	      res->examples->creator==GHVBoxCreate ))
	GHVBoxReflow(res->examples->ret);
    GRE_RefreshAll(gre);
}

static int GRE_InheritColChange(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_radiochanged ) {
	GRE *gre = GDrawGetUserData(GGadgetGetWindow(g));
	int cid = GGadgetGetCid(g), on = GGadgetIsChecked(g);
	GGadgetSetEnabled(GWidgetGetControl(gre->gw,cid+1),!on);
	g = GWidgetGetControl(gre->gw,cid+2);
	GGadgetSetEnabled(g,!on);
	if ( on ) {
	    int index = GTabSetGetSel(gre->tabset);
	    GResInfo *res = gre->tofree[index].res;
	    int offset = ((char *) GGadgetGetUserData(g)) - ((char *) (res->boxdata));
	    Color col = *((Color *) (((char *) (res->inherits_from->boxdata))+offset));
	    if ( col!= *(Color *) GGadgetGetUserData(g) ) {
		GColorButtonSetColor(g,col);
		*((Color *) GGadgetGetUserData(g)) = col;
		GRE_RefreshAll(gre);
	    }
	}
    }
return( true );
}

static int GRE_InheritListChange(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_radiochanged ) {
	GRE *gre = GDrawGetUserData(GGadgetGetWindow(g));
	int cid = GGadgetGetCid(g), on = GGadgetIsChecked(g);
	GGadgetSetEnabled(GWidgetGetControl(gre->gw,cid+1),!on);
	g = GWidgetGetControl(gre->gw,cid+2);
	GGadgetSetEnabled(g,!on);
	if ( on ) {
	    int index = GTabSetGetSel(gre->tabset);
	    GResInfo *res = gre->tofree[index].res;
	    int offset = ((char *) GGadgetGetUserData(g)) - ((char *) (res->boxdata));
	    int sel = *((uint8 *) (((char *) (res->inherits_from->boxdata))+offset));
	    if ( sel != *(uint8 *) GGadgetGetUserData(g) ) {
		GGadgetSelectOneListItem(g,sel);
		*((uint8 *) GGadgetGetUserData(g)) = sel;
		GRE_Reflow(gre,res);
	    }
	}
    }
return( true );
}

static int GRE_InheritTextChange(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_radiochanged ) {
	GRE *gre = GDrawGetUserData(GGadgetGetWindow(g));
	int cid = GGadgetGetCid(g), on = GGadgetIsChecked(g);
	GGadgetSetEnabled(GWidgetGetControl(gre->gw,cid+1),!on);
	g = GWidgetGetControl(gre->gw,cid+2);
	GGadgetSetEnabled(g,!on);
	if ( on ) {
	    int index = GTabSetGetSel(gre->tabset);
	    GResInfo *res = gre->tofree[index].res;
	    int offset = ((char *) GGadgetGetUserData(g)) - ((char *) (res->boxdata));
	    int val = *((uint8 *) (((char *) (res->inherits_from->boxdata))+offset));
	    if ( val != *(uint8 *) GGadgetGetUserData(g) ) {
		char buf[20];
		sprintf( buf, "%d", val );
		GGadgetSetTitle8(g,buf);
		*((uint8 *) GGadgetGetUserData(g)) = val;
		GRE_Reflow(gre,res);
	    }
	}
    }
return( true );
}

static int GRE_InheritFlagChange(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_radiochanged ) {
	GRE *gre = GDrawGetUserData(GGadgetGetWindow(g));
	int cid = GGadgetGetCid(g), on = GGadgetIsChecked(g);
	g = GWidgetGetControl(gre->gw,cid+1);
	GGadgetSetEnabled(g,!on);
	if ( on ) {
	    int index = GTabSetGetSel(gre->tabset);
	    GResInfo *res = gre->tofree[index].res;
	    int flag = (intpt) GGadgetGetUserData(g);
	    if ( (res->boxdata->flags&flag) != (res->inherits_from->boxdata->flags&flag)) {
		GGadgetSetChecked(g,
			(res->inherits_from->boxdata->flags&flag)?1:0);
		if ( res->inherits_from->boxdata->flags&flag )
		    res->boxdata->flags |= flag;
		else
		    res->boxdata->flags &= ~flag;
		GRE_Reflow(gre,res);
	    }
	}
    }
return( true );
}

static int GRE_FlagChanged(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_textchanged ) {
	GRE *gre = GDrawGetUserData(GGadgetGetWindow(g));
	int index = GTabSetGetSel(gre->tabset);
	GResInfo *res = gre->tofree[index].res;
	if ( GGadgetIsChecked(g))
	    res->boxdata->flags |= (int) (intpt) GGadgetIsChecked(g);
	else
	    res->boxdata->flags |= ~(int) (intpt) GGadgetIsChecked(g);
	GRE_Reflow(gre,res);
    }
return( true );
}

static int GRE_ListChanged(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_listselected ) {
	GRE *gre = GDrawGetUserData(GGadgetGetWindow(g));
	int index = GTabSetGetSel(gre->tabset);
	GResInfo *res = gre->tofree[index].res;
	*((uint8 *) GGadgetGetUserData(g)) = GGadgetGetFirstListSelectedItem(g);
	GRE_Reflow(gre,res);
    }
return( true );
}

static int GRE_BoolChanged(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_textchanged ) {
	*((int *) GGadgetGetUserData(g)) = GGadgetIsChecked(g);
    }
return( true );
}

static int GRE_ByteChanged(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_textchanged ) {
	GRE *gre = GDrawGetUserData(GGadgetGetWindow(g));
	char *txt = GGadgetGetTitle8(g), *end;
	int val = strtol(txt,&end,10);
	int index = GTabSetGetSel(gre->tabset);
	GResInfo *res = gre->tofree[index].res;
	if ( *end=='\0' && val>=0 && val<=255 ) {
	    *((uint8 *) GGadgetGetUserData(g)) = val;
	    GRE_Reflow(gre,res);
	}
    }
return( true );
}

static int GRE_IntChanged(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_textchanged ) {
	GRE *gre = GDrawGetUserData(GGadgetGetWindow(g));
	char *txt = GGadgetGetTitle8(g), *end;
	int val = strtol(txt,&end,10);
	int index = GTabSetGetSel(gre->tabset);
	GResInfo *res = gre->tofree[index].res;
	if ( *end=='\0' ) {
	    *((int *) GGadgetGetUserData(g)) = val;
	    GRE_Reflow(gre,res);
	}
    }
return( true );
}

static int GRE_DoubleChanged(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_textchanged ) {
	char *txt = GGadgetGetTitle8(g), *end;
	double val = strtod(txt,&end);
	if ( *end=='\0' )
	    *((double *) GGadgetGetUserData(g)) = val;
    }
return( true );
}

static int GRE_ColorChanged(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_textchanged ) {
	GRE *gre = GDrawGetUserData(GGadgetGetWindow(g));
	*((Color *) GGadgetGetUserData(g)) = GColorButtonGetColor(g);
	GRE_RefreshAll(gre);
    }
return( true );
}

static int GRE_FontChanged(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_textchanged ) {
	/*GRE *gre = GDrawGetUserData(GGadgetGetWindow(g));*/
    }
return( true );
}

static int GRE_StringChanged(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_textchanged ) {
	/*GRE *gre = GDrawGetUserData(GGadgetGetWindow(g));*/
    }
return( true );
}

static int GRE_ImageChanged(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_textchanged ) {
	/* GRE *gre = GDrawGetUserData(GGadgetGetWindow(g)); */
	GResImage **ri = GGadgetGetUserData(g);
	char *new;
	GImage *newi;
	new = gwwv_open_filename_with_path("Find Image",
		(*ri)==NULL?NULL: (*ri)->filename,
		"*.{png,jpeg,jpg,tiff,bmp,xbm}",NULL,
		_GGadget_GetImagePath());
	if ( new==NULL )
return( true );
	newi = GImageRead(new);
	if ( newi==NULL ) {
	    gwwv_post_error(_("Could not open image"),_("Could not open %s"), new );
	    free( new );
	}
	if ( *ri==NULL ) {
	    *ri = gcalloc(1,sizeof(GResImage));
	    (*ri)->filename = new;
	    (*ri)->image = newi;
	    ((GButton *) g)->image = newi;
	    GGadgetSetTitle8(g,"...");
	} else {
	    GImage hold;
	    free( (*ri)->filename );
	    (*ri)->filename = new;
	    /* Need to retain the GImage point, but update its */
	    /*  contents, and free the old stuff */
	    hold = *((*ri)->image);
	    *(*ri)->image = *newi;
	    *newi = hold;
	    GImageDestroy(newi);
	}
    }
return( true );
}

static void GRE_DoCancel(GRE *gre) {
    int i;

    for ( i=0; gre->tofree[i].res!=NULL; ++i ) {
	GResInfo *res = gre->tofree[i].res;
	struct resed *extras;
	GResImage **_ri, *ri;
	if ( res->boxdata!=NULL )
	    *res->boxdata = res->orig_state;
	if ( res->extras!=NULL ) {
	    for ( extras = res->extras; extras->name!=NULL; extras++ ) {
		switch ( extras->type ) {
		  case rt_bool:
		  case rt_int:
		  case rt_color:
		    *(int *) (extras->val) = extras->orig.ival;
		  break;
		  case rt_double:
		    *(int *) (extras->val) = extras->orig.dval;
		  break;
		  case rt_image:
		    _ri = extras->val;
		    ri = *_ri;
		    if ( extras->orig.sval==NULL ) {
			if ( ri!=NULL ) {
			    free(ri->filename);
			    GImageDestroy(ri->image);
			    free(ri);
			    *_ri = NULL;
			}
		    } else {
			if ( strcmp(extras->orig.sval,ri->filename)!=0 ) {
			    GImage *temp, hold;
			    temp = GImageRead(extras->orig.sval);
			    if ( temp!=NULL ) {
				hold = *(ri->image);
				*ri->image = *temp;
				*temp = hold;
				GImageDestroy(temp);
			        free(ri->filename);
			        ri->filename = copy(extras->orig.sval);
			    }
			}
		    }
		  break;
		  case rt_string:
		    /* These don't change dynamically */
		    /* Nothing to revert here */
		  break;
		}
	    }
	}
    }
    gre->done = true;
}

static int GRE_ChangePane(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_buttonactivate ) {
	GRE *gre = GDrawGetUserData(GGadgetGetWindow(g));
	GResInfo *new = GGadgetGetUserData(g);
	int i;
	for ( i=0; gre->tofree[i].res!=NULL && gre->tofree[i].res != new; ++i );
	if ( gre->tofree[i].res!=NULL )
	    GTabSetSetSel(gre->tabset,i);
    }
return( true );
}

static int GRE_Cancel(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_buttonactivate ) {
	GRE *gre = GDrawGetUserData(GGadgetGetWindow(g));
	GRE_DoCancel(gre);
    }
return( true );
}

static int TogglePrefs(GGadget *g, GEvent *e) {

    if ( e->type==et_controlevent && e->u.control.subtype == et_textchanged ) {
	*((int *) GGadgetGetUserData(g)) = GGadgetIsChecked(g);
    }
return( true );
}

static int GRE_Save(GGadget *g, GEvent *e) {
    static char *shapes[] = { "rect", "roundrect", "elipse", "diamond", NULL };
    static char *types[] = { "none", "box", "raised", "lowered", "engraved",
	    "embossed", "double", NULL };

    if ( e->type==et_controlevent && e->u.control.subtype == et_buttonactivate ) {
	GRE *gre = GDrawGetUserData(GGadgetGetWindow(g));
	char *filename;
	FILE *output;
	GGadgetCreateData gcd[2], *gcdp=NULL;
	GTextInfo lab[1];
	int update_prefs = true;
	int i,j;
	static char *flagnames[] = { "BorderInner", "BorderOuter", "ActiveInner",
		"ShadowOuter", "DoDepressedBackground", "DrawDefault",
		"GradientBG", NULL };
	static char *colornames[] = { "NormalForeground", "DisabledForeground", "NormalBackground",
		"DisabledBackground", "PressedBackground", "GradientStartCol", "BorderBrightest",
		"BorderBrighter", "BorderDarker", "BorderDarkest", "ActiveBorder",
		NULL };
	static char *intnames[] = { "BorderWidth", "Padding", "Radius", NULL };
	struct resed *extras;

	if ( gre->change_res_filename != NULL ) {
	    memset(gcd,0,sizeof(gcd));
	    memset(lab,0,sizeof(lab));
	    lab[0].text = (unichar_t *) _("Store this filename in preferences");
	    lab[0].text_is_1byte = true;
	    gcd[0].gd.label = &lab[0];
	    gcd[0].gd.flags = gg_visible|gg_enabled|gg_cb_on;
	    gcd[0].gd.handle_controlevent = TogglePrefs;
	    gcd[0].data = &update_prefs;
	    gcd[0].creator = GCheckBoxCreate;
	    gcdp = gcd;
	}
	
	filename = gwwv_save_filename_with_gadget(_("Save Resource file as..."),gre->def_res_file,NULL,gcdp);
	if ( filename==NULL )
return( true );
	output = fopen( filename,"w" );
	if ( output==NULL ) {
	    gwwv_post_error(_("Open failed"), _("Failed to open %s for output"), filename );
return( true );
	}
	for ( i=0; gre->tofree[i].res!=NULL; ++i ) {
	    GResInfo *res = gre->tofree[i].res;
	    int cid = gre->tofree[i].startcid;
	    if ( res->boxdata!=NULL ) {
		for ( j=0; flagnames[j]!=NULL; ++j ) {
		    if ( !GGadgetIsChecked( GWidgetGetControl(gre->gw,cid))) {
			fprintf( output, "%s.%s.Box.%s: %s\n",
				res->progname, res->resname, flagnames[j],
			        GGadgetIsChecked( GWidgetGetControl(gre->gw,cid+1))?"True" : "False" );
		    }
		    cid += 2;
		}
		for ( j=0; colornames[j]!=NULL; ++j ) {
		    if ( !GGadgetIsChecked( GWidgetGetControl(gre->gw,cid))) {
			fprintf( output, "%s.%s.Box.%s: #%06x\n",
				res->progname, res->resname, colornames[j],
			        GColorButtonGetColor( GWidgetGetControl(gre->gw,cid+2)) );
		    }
		    cid += 3;
		}
		if ( !GGadgetIsChecked( GWidgetGetControl(gre->gw,cid))) {
		    fprintf( output, "%s.%s.Box.BorderType: %s\n",
			    res->progname, res->resname,
			    types[ GGadgetGetFirstListSelectedItem(
				     GWidgetGetControl(gre->gw,cid+2)) ] );
		}
		cid += 3;
		if ( !GGadgetIsChecked( GWidgetGetControl(gre->gw,cid))) {
		    fprintf( output, "%s.%s.Box.BorderShape: %s\n",
			    res->progname, res->resname,
			    shapes[ GGadgetGetFirstListSelectedItem(
				     GWidgetGetControl(gre->gw,cid+2)) ] );
		}
		cid += 3;
		for ( j=0; intnames[j]!=NULL; ++j ) {
		    if ( !GGadgetIsChecked( GWidgetGetControl(gre->gw,cid))) {
			char *ival = GGadgetGetTitle8( GWidgetGetControl(gre->gw,cid+2));
			char *end;
			int val = strtol(ival,&end,10);
			if ( *end!='\0' || val<0 || val>255 )
			    gwwv_post_error(_("Bad Number"), _("Bad numeric value for %s.%s"),
				    res->resname, intnames[j]);
			fprintf( output, "%s.%s.Box.%s: %s\n",
				res->progname, res->resname, intnames[j], ival );
			free(ival);
		    }
		    cid += 3;
		}
	    }
	    if ( res->font!=NULL ) {
		char *ival = GGadgetGetTitle8( GWidgetGetControl(gre->gw,gre->tofree[i].fontcid));
		fprintf( output, "%s.%s.Font: %s\n",
			res->progname, res->resname, ival );
		free(ival);
	    }
	    if ( res->extras!=NULL ) for ( extras=res->extras; extras->name!=NULL; ++extras ) {
		GGadget *g = GWidgetGetControl(gre->gw,extras->cid);
		switch ( extras->type ) {
		  case rt_bool:
		    fprintf( output, "%s.%s.%s: %s\n",
			    res->progname, res->resname, extras->resname,
			    GGadgetIsChecked(g)?"True":"False");
		  break;
		  case rt_int: {
		    char *ival = GGadgetGetTitle8( g );
		    char *end;
		    (void) strtol(ival,&end,10);
		    if ( *end!='\0' )
			gwwv_post_error(_("Bad Number"), _("Bad numeric value for %s.%s"),
				res->resname, extras->name );
		    fprintf( output, "%s.%s.%s: %s\n",
			    res->progname, res->resname, extras->resname,
			    ival );
		    free(ival);
		  } break;
		  case rt_double: {
		    char *dval = GGadgetGetTitle8( g );
		    char *end;
		    (void) strtod(dval,&end);
		    if ( *end!='\0' )
			gwwv_post_error(_("Bad Number"), _("Bad numeric value for %s.%s"),
				res->resname, extras->name );
		    fprintf( output, "%s.%s.%s: %s\n",
			    res->progname, res->resname, extras->resname,
			    dval );
		    free(dval);
		  } break;
		  case rt_color:
		    fprintf( output, "%s.%s.%s: #%06x\n",
			    res->progname, res->resname, extras->resname,
			    GColorButtonGetColor(g) );
		  break;
		  case rt_image: {
		    GResImage *ri = *((GResImage **) (extras->val));
		    if ( ri!=NULL ) {	/* No image if ri==NULL */
			char **paths = _GGadget_GetImagePath();
			int i;
			for ( i=0; paths[i]!=NULL; ++i ) {
			    if ( strncmp(paths[i],ri->filename,strlen(paths[i]))==0 ) {
				fprintf( output, "%s.%s.%s: %s\n",
					res->progname, res->resname, extras->resname,
					ri->filename+strlen(paths[i]) );
			break;
			    }
			}
			if ( paths[i]==NULL )
			    fprintf( output, "%s.%s.%s: %s\n",
				    res->progname, res->resname, extras->resname,
				    ri->filename );
		    }
		  } break;
		  case rt_string: {
		    char *sval = GGadgetGetTitle8( g );
		    fprintf( output, "%s.%s.%s: %s\n",
			    res->progname, res->resname, extras->resname,
			    sval );
		    free(sval);
		  } break;
		}
	    }
	    fprintf( output, "\n" );
	}
	if ( ferror(output))
	    gwwv_post_error(_("Write failed"),_("An error occurred when writing the resource file"));
	fclose(output);
	if ( gre->change_res_filename != NULL && update_prefs )
	    (gre->change_res_filename)(filename);
	free(filename);
    }
return( true );
}

static int GRE_OK(GGadget *g, GEvent *e) {
    int i;
    struct resed *extras;

    if ( e->type==et_controlevent && e->u.control.subtype == et_buttonactivate ) {
	GRE *gre = GDrawGetUserData(GGadgetGetWindow(g));
	/* Handle fonts, strings and images */
	for ( i=0; gre->tofree[i].res!=NULL; ++i ) {
	    GResInfo *res = gre->tofree[i].res;
	    if ( res->font!=NULL ) {
		char *spec = GGadgetGetTitle8(GWidgetGetControl(gre->gw,gre->tofree[i].fontcid));
		res->font = GResource_font_cvt(spec,res->font);
		free(spec);
	    }
	    if ( res->extras!=NULL ) {
		for ( extras = res->extras; extras->name!=NULL; extras++ ) {
		    switch ( extras->type ) {
		      case rt_bool:
		      case rt_int:
		      case rt_color:
		      case rt_double:
			/* These should have been set as we went along */
		      break;
		      case rt_string:
		      case rt_image:
		      {
			char *spec = GGadgetGetTitle8(GWidgetGetControl(gre->gw,extras->cid));
			free( *(char **) (extras->val) );
			if ( *spec=='\0' ) { free( spec ); spec=NULL; }
			*(char **) (extras->val) = spec;
		      } break;
		    }
		}
	    }
	}
	gre->done = true;
    }
return( true );
}

static int gre_e_h(GWindow gw, GEvent *event) {
    if ( event->type==et_close ) {
	GRE *gre = GDrawGetUserData(gw);
	GRE_DoCancel(gre);
    } else if ( event->type == et_char ) {
return( false );
    }
return( true );
}

static void GResEditDlg(GResInfo *all,const char *def_res_file,void (*change_res_filename)(const char *)) {
    GResInfo *res, *parent;
    int cnt;
    GGadgetCreateData topgcd[4], topbox[3], *gcd, *barray[10], *tarray[3][2];
    GTextInfo *lab, toplab[4];
    GTabInfo *panes;
    struct tofree *tofree;
    struct resed *extras;
    int i,j,k,l,cid;
    static GBox small_blue_box;
    extern GBox _GGadget_button_box;
    GRE gre;
    GRect pos;
    GWindow gw;
    GWindowAttrs wattrs;

    memset(&gre,0,sizeof(gre));
    gre.def_res_file = def_res_file;
    gre.change_res_filename = change_res_filename;

    memset(&wattrs,0,sizeof(wattrs));
    wattrs.mask = wam_events|wam_cursor|wam_utf8_wtitle|wam_undercursor|wam_isdlg|wam_restrict;
    wattrs.event_masks = ~(1<<et_charup);
    wattrs.restrict_input_to_me = 1;
    wattrs.is_dlg = true;
    wattrs.undercursor = 1;
    wattrs.cursor = ct_pointer;
    wattrs.utf8_window_title = _("X Resource Editor");
    pos.x = pos.y = 0;
    pos.width = pos.height = 100;
    gre.gw = gw = GDrawCreateTopWindow(NULL,&pos,gre_e_h,&gre,&wattrs);

    if ( small_blue_box.main_foreground==0 ) {
	extern void _GButtonInit(void);
	_GButtonInit();
	small_blue_box = _GGadget_button_box;
	small_blue_box.border_type = bt_box;
	small_blue_box.border_shape = bs_rect;
	small_blue_box.border_width = 0;
	small_blue_box.flags = box_foreground_shadow_outer;
	small_blue_box.padding = 0;
	small_blue_box.main_foreground = 0x0000ff;
	small_blue_box.border_darker = small_blue_box.main_foreground;
	small_blue_box.border_darkest = small_blue_box.border_brighter =
		small_blue_box.border_brightest =
		small_blue_box.main_background = GDrawGetDefaultBackground(NULL);
    }

    for ( res=all, cnt=0; res!=NULL; res=res->next, ++cnt );

    panes = gcalloc(cnt+1,sizeof(GTabInfo));
    gre.tofree = tofree = gcalloc(cnt+1,sizeof(struct tofree));
    cid = 0;
    for ( res=all, i=0; res!=NULL; res=res->next, ++i ) {
	tofree[i].res = res;
	tofree[i].startcid = cid+1;

	cnt = 0;
	if ( res->extras!=NULL )
	    for ( extras=res->extras, cnt = 0; extras->name!=NULL; ++cnt, ++extras );
	tofree[i].earray = gcalloc(cnt+1,sizeof(GGadgetCreateData[8]));
	tofree[i].extradefs = gcalloc(cnt+1,sizeof(char *));
	cnt *= 2;
	if ( res->initialcomment!=NULL )
	    ++cnt;
	if ( res->boxdata!=NULL )
	    cnt += 3*16 + 8*2;
	if ( res->font )
	    cnt+=3;
	if ( res->inherits_from!=NULL )
	    cnt+=2;
	if ( res->seealso1!=NULL ) {
	    cnt+=2;
	    if ( res->seealso2!=NULL )
		++cnt;
	}

	tofree[i].gcd = gcd = gcalloc(cnt,sizeof(GGadgetCreateData));
	tofree[i].lab = lab = gcalloc(cnt,sizeof(GTextInfo));

	j=k=l=0;
	if ( res->initialcomment!=NULL ) {
	    lab[k].text = (unichar_t *) _(res->initialcomment);
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].marray[j++] = &gcd[k-1];
	}
	if ( res->examples!=NULL )
	    tofree[i].marray[j++] = res->examples;
	if ( res->inherits_from != NULL ) {
	    lab[k].text = (unichar_t *) _("Inherits from");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].iarray[0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _(res->inherits_from->name);
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_dontcopybox;
	    gcd[k].gd.box = &small_blue_box;
	    gcd[k].data = res->inherits_from;
	    gcd[k].gd.handle_controlevent = GRE_ChangePane;
	    gcd[k++].creator = GButtonCreate;
	    tofree[i].iarray[1] = &gcd[k-1];

	    tofree[i].iarray[2] = GCD_Glue; tofree[i].iarray[3] = NULL;
	    tofree[i].ibox.gd.flags = gg_visible|gg_enabled;
	    tofree[i].ibox.gd.u.boxelements = tofree[i].iarray;
	    tofree[i].ibox.creator = GHBoxCreate;
	    tofree[i].marray[j++] = &tofree[i].ibox;
	} else if ( res->boxdata!=NULL ) {
	    lab[k].text = (unichar_t *) _("Does not inherit from anything");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].marray[j++] = &gcd[k-1];
	}

	if ( res->boxdata!=NULL ) {
	    res->orig_state = *res->boxdata;

	    lab[k].text = (unichar_t *) _("Inherit");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritFlagChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].farray[l][0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Outline Inner Border");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    if ( res->boxdata->flags & box_foreground_border_inner )
		gcd[k].gd.flags |= gg_cb_on;
	    gcd[k].gd.handle_controlevent = GRE_FlagChanged;
	    gcd[k].data = (void *) (intpt) box_foreground_border_inner;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].farray[l][1] = &gcd[k-1];
	    if ( res->inherits_from==NULL )
		gcd[k-2].gd.flags &= ~gg_enabled;
	    else if ( (res->inherits_from->boxdata->flags&box_foreground_border_inner) == (res->boxdata->flags&box_foreground_border_inner) ) {
		gcd[k-2].gd.flags |= gg_cb_on;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

	    lab[k].text = (unichar_t *) _("Inherit");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritFlagChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].farray[l][2] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Outline Outer Border");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    if ( res->boxdata->flags & box_foreground_border_outer )
		gcd[k].gd.flags |= gg_cb_on;
	    gcd[k].gd.handle_controlevent = GRE_FlagChanged;
	    gcd[k].data = (void *) (intpt) box_foreground_border_outer;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].farray[l][3] = &gcd[k-1];
	    if ( res->inherits_from==NULL )
		gcd[k-2].gd.flags &= ~gg_enabled;
	    else if ( (res->inherits_from->boxdata->flags&box_foreground_border_outer) == (res->boxdata->flags&box_foreground_border_outer) ) {
		gcd[k-2].gd.flags |= gg_cb_on;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }
	    tofree[i].farray[l][4] = GCD_Glue;
	    tofree[i].farray[l++][5] = NULL;


	    lab[k].text = (unichar_t *) _("Inherit");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritFlagChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].farray[l][0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Show Active Border");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    if ( res->boxdata->flags & box_active_border_inner )
		gcd[k].gd.flags |= gg_cb_on;
	    gcd[k].gd.handle_controlevent = GRE_FlagChanged;
	    gcd[k].data = (void *) (intpt) box_active_border_inner;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].farray[l][1] = &gcd[k-1];
	    if ( res->inherits_from==NULL )
		gcd[k-2].gd.flags &= ~gg_enabled;
	    else if ( (res->inherits_from->boxdata->flags&box_active_border_inner) == (res->boxdata->flags&box_active_border_inner) ) {
		gcd[k-2].gd.flags |= gg_cb_on;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

	    lab[k].text = (unichar_t *) _("Inherit");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritFlagChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].farray[l][2] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Outer Shadow");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    if ( res->boxdata->flags & box_foreground_shadow_outer )
		gcd[k].gd.flags |= gg_cb_on;
	    gcd[k].gd.handle_controlevent = GRE_FlagChanged;
	    gcd[k].data = (void *) (intpt) box_foreground_shadow_outer;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].farray[l][3] = &gcd[k-1];
	    if ( res->inherits_from==NULL )
		gcd[k-2].gd.flags &= ~gg_enabled;
	    else if ( (res->inherits_from->boxdata->flags&box_foreground_shadow_outer) == (res->boxdata->flags&box_foreground_shadow_outer) ) {
		gcd[k-2].gd.flags |= gg_cb_on;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }
	    tofree[i].farray[l][4] = GCD_Glue;
	    tofree[i].farray[l++][5] = NULL;


	    lab[k].text = (unichar_t *) _("Inherit");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritFlagChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].farray[l][0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Depressed Background");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    if ( res->boxdata->flags & box_do_depressed_background )
		gcd[k].gd.flags |= gg_cb_on;
	    gcd[k].gd.handle_controlevent = GRE_FlagChanged;
	    gcd[k].data = (void *) (intpt) box_do_depressed_background;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].farray[l][1] = &gcd[k-1];
	    if ( res->inherits_from==NULL )
		gcd[k-2].gd.flags &= ~gg_enabled;
	    else if ( (res->inherits_from->boxdata->flags&box_do_depressed_background) == (res->boxdata->flags&box_do_depressed_background) ) {
		gcd[k-2].gd.flags |= gg_cb_on;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

	    lab[k].text = (unichar_t *) _("Inherit");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritFlagChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].farray[l][2] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Outline Default Button");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    if ( res->boxdata->flags & box_draw_default )
		gcd[k].gd.flags |= gg_cb_on;
	    gcd[k].gd.handle_controlevent = GRE_FlagChanged;
	    gcd[k].data = (void *) (intpt) box_draw_default;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].farray[l][3] = &gcd[k-1];
	    if ( res->inherits_from==NULL )
		gcd[k-2].gd.flags &= ~gg_enabled;
	    else if ( (res->inherits_from->boxdata->flags&box_draw_default) == (res->boxdata->flags&box_draw_default) ) {
		gcd[k-2].gd.flags |= gg_cb_on;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }
	    tofree[i].farray[l][4] = GCD_Glue;
	    tofree[i].farray[l++][5] = NULL;


	    lab[k].text = (unichar_t *) _("Inherit");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritFlagChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].farray[l][0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Background Gradient");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    if ( res->boxdata->flags & box_gradient_bg )
		gcd[k].gd.flags |= gg_cb_on;
	    gcd[k].gd.handle_controlevent = GRE_FlagChanged;
	    gcd[k].data = (void *) (intpt) box_gradient_bg;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].farray[l][1] = &gcd[k-1];
	    if ( res->inherits_from==NULL )
		gcd[k-2].gd.flags &= ~gg_enabled;
	    else if ( (res->inherits_from->boxdata->flags&box_gradient_bg) == (res->boxdata->flags&box_gradient_bg) ) {
		gcd[k-2].gd.flags |= gg_cb_on;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }
	    tofree[i].farray[l][2] = GCD_Glue;
	    tofree[i].farray[l][3] = GCD_Glue;
	    tofree[i].farray[l][4] = GCD_Glue;
	    tofree[i].farray[l++][5] = NULL;
	    tofree[i].farray[l][0] = NULL;

	    tofree[i].flagsbox.gd.flags = gg_enabled | gg_visible;
	    tofree[i].flagsbox.gd.u.boxelements = tofree[i].farray[0];
	    tofree[i].flagsbox.creator = GHVBoxCreate;
	    tofree[i].marray[j++] = &tofree[i].flagsbox;

	    l = 0;

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritColChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Normal Text Color:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][1] = &gcd[k-1];

	    gcd[k].gd.u.col = res->boxdata->main_foreground;
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ColorChanged;
	    gcd[k].data = &res->boxdata->main_foreground;
	    gcd[k++].creator = GColorButtonCreate;
	    tofree[i].carray[l][2] = &gcd[k-1];
	    tofree[i].carray[l][3] = GCD_Glue;
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->main_foreground == res->boxdata->main_foreground ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritColChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][4] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Disabled Text Color:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][5] = &gcd[k-1];

	    gcd[k].gd.u.col = res->boxdata->disabled_foreground;
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ColorChanged;
	    gcd[k].data = &res->boxdata->disabled_foreground;
	    gcd[k++].creator = GColorButtonCreate;
	    tofree[i].carray[l][6] = &gcd[k-1];
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->disabled_foreground == res->boxdata->disabled_foreground ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

	    tofree[i].carray[l][7] = GCD_Glue;
	    tofree[i].carray[l++][8] = NULL;

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritColChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Normal Background:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][1] = &gcd[k-1];

	    gcd[k].gd.u.col = res->boxdata->main_background;
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ColorChanged;
	    gcd[k].data = &res->boxdata->main_background;
	    gcd[k++].creator = GColorButtonCreate;
	    tofree[i].carray[l][2] = &gcd[k-1];
	    tofree[i].carray[l][3] = GCD_Glue;
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->main_background == res->boxdata->main_background ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritColChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][4] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Disabled Background:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][5] = &gcd[k-1];

	    gcd[k].gd.u.col = res->boxdata->disabled_background;
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ColorChanged;
	    gcd[k].data = &res->boxdata->disabled_background;
	    gcd[k++].creator = GColorButtonCreate;
	    tofree[i].carray[l][6] = &gcd[k-1];
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->disabled_background == res->boxdata->disabled_background ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

	    tofree[i].carray[l][7] = GCD_Glue;
	    tofree[i].carray[l++][8] = NULL;

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritColChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Depressed Background:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][1] = &gcd[k-1];

	    gcd[k].gd.u.col = res->boxdata->depressed_background;
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ColorChanged;
	    gcd[k].data = &res->boxdata->depressed_background;
	    gcd[k++].creator = GColorButtonCreate;
	    tofree[i].carray[l][2] = &gcd[k-1];
	    tofree[i].carray[l][3] = GCD_Glue;
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->depressed_background == res->boxdata->depressed_background ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritColChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][4] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Background Gradient:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][5] = &gcd[k-1];

	    gcd[k].gd.u.col = res->boxdata->gradient_bg_end;
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ColorChanged;
	    gcd[k].data = &res->boxdata->gradient_bg_end;
	    gcd[k++].creator = GColorButtonCreate;
	    tofree[i].carray[l][6] = &gcd[k-1];
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->gradient_bg_end == res->boxdata->gradient_bg_end ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

	    tofree[i].carray[l][7] = GCD_Glue;
	    tofree[i].carray[l++][8] = NULL;

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritColChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Brightest Border:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][1] = &gcd[k-1];

	    gcd[k].gd.u.col = res->boxdata->border_brightest;
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ColorChanged;
	    gcd[k].data = &res->boxdata->border_brightest;
	    gcd[k++].creator = GColorButtonCreate;
	    tofree[i].carray[l][2] = &gcd[k-1];
	    tofree[i].carray[l][3] = GCD_Glue;
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->border_brightest == res->boxdata->border_brightest ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritColChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][4] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Brighter Border:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][5] = &gcd[k-1];

	    gcd[k].gd.u.col = res->boxdata->border_brighter;
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ColorChanged;
	    gcd[k].data = &res->boxdata->border_brighter;
	    gcd[k++].creator = GColorButtonCreate;
	    tofree[i].carray[l][6] = &gcd[k-1];
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->border_brighter == res->boxdata->border_brighter ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

	    tofree[i].carray[l][7] = GCD_Glue;
	    tofree[i].carray[l++][8] = NULL;

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritColChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Darker Border:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][1] = &gcd[k-1];

	    gcd[k].gd.u.col = res->boxdata->border_darker;
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ColorChanged;
	    gcd[k].data = &res->boxdata->border_darker;
	    gcd[k++].creator = GColorButtonCreate;
	    tofree[i].carray[l][2] = &gcd[k-1];
	    tofree[i].carray[l][3] = GCD_Glue;
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->border_darker == res->boxdata->border_darker ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritColChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][4] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Darkest Border:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][5] = &gcd[k-1];

	    gcd[k].gd.u.col = res->boxdata->border_darkest;
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ColorChanged;
	    gcd[k].data = &res->boxdata->border_darkest;
	    gcd[k++].creator = GColorButtonCreate;
	    tofree[i].carray[l][6] = &gcd[k-1];
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->border_darkest == res->boxdata->border_darkest ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

	    tofree[i].carray[l][7] = GCD_Glue;
	    tofree[i].carray[l++][8] = NULL;

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritColChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Active Border:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][1] = &gcd[k-1];

	    gcd[k].gd.u.col = res->boxdata->active_border;
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ColorChanged;
	    gcd[k].data = &res->boxdata->active_border;
	    gcd[k++].creator = GColorButtonCreate;
	    tofree[i].carray[l][2] = &gcd[k-1];
	    tofree[i].carray[l][3] = GCD_Glue;
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->active_border == res->boxdata->active_border ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

	    tofree[i].carray[l][4] = GCD_Glue;
	    tofree[i].carray[l][5] = GCD_Glue;
	    tofree[i].carray[l][6] = GCD_Glue;
	    tofree[i].carray[l][7] = GCD_Glue;
	    tofree[i].carray[l++][8] = NULL;

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritListChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Border Type:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][1] = &gcd[k-1];

	    gcd[k].gd.u.list = bordertype;
	    gcd[k].gd.label = &bordertype[res->boxdata->border_type];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = tofree[i].btcid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ListChanged;
	    gcd[k].data = &res->boxdata->border_type;
	    gcd[k++].creator = GListButtonCreate;
	    tofree[i].carray[l][2] = &gcd[k-1];
	    tofree[i].carray[l][3] = GCD_ColSpan;
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->border_type == res->boxdata->border_type ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritListChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][4] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Border Shape:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][5] = &gcd[k-1];

	    gcd[k].gd.u.list = bordershape;
	    gcd[k].gd.label = &bordershape[res->boxdata->border_shape];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ListChanged;
	    gcd[k].data = &res->boxdata->border_shape;
	    gcd[k++].creator = GListButtonCreate;
	    tofree[i].carray[l][6] = &gcd[k-1];
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->border_shape == res->boxdata->border_shape ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

	    tofree[i].carray[l][7] = GCD_Glue;
	    tofree[i].carray[l++][8] = NULL;

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritTextChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Border Width:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][1] = &gcd[k-1];

	    sprintf( tofree[i].bw, "%d", res->boxdata->border_width );
	    lab[k].text = (unichar_t *) tofree[i].bw;
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.pos.width = 50;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ByteChanged;
	    gcd[k].data = &res->boxdata->border_width;
	    gcd[k++].creator = GNumericFieldCreate;
	    tofree[i].carray[l][2] = &gcd[k-1];
	    tofree[i].carray[l][3] = GCD_ColSpan;
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->border_width == res->boxdata->border_width ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritTextChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][4] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Padding:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][5] = &gcd[k-1];

	    sprintf( tofree[i].padding, "%d", res->boxdata->padding );
	    lab[k].text = (unichar_t *) tofree[i].padding;
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.pos.width = 50;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ByteChanged;
	    gcd[k].data = &res->boxdata->padding;
	    gcd[k++].creator = GNumericFieldCreate;
	    tofree[i].carray[l][6] = &gcd[k-1];
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->border_shape == res->boxdata->border_shape ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

	    tofree[i].carray[l][7] = GCD_Glue;
	    tofree[i].carray[l++][8] = NULL;

/* GT: "I." is an abreviation for "Inherits" */
	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritTextChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].carray[l][0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Radius:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].carray[l][1] = &gcd[k-1];

	    sprintf( tofree[i].rr, "%d", res->boxdata->rr_radius );
	    lab[k].text = (unichar_t *) tofree[i].rr;
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.pos.width = 50;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_ByteChanged;
	    gcd[k].data = &res->boxdata->rr_radius;
	    gcd[k++].creator = GNumericFieldCreate;
	    tofree[i].carray[l][2] = &gcd[k-1];
	    tofree[i].carray[l][3] = GCD_ColSpan;
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( res->inherits_from->boxdata->rr_radius == res->boxdata->rr_radius ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }

	    tofree[i].carray[l][4] = GCD_Glue;
	    tofree[i].carray[l][5] = GCD_Glue;
	    tofree[i].carray[l][6] = GCD_Glue;
	    tofree[i].carray[l][7] = GCD_Glue;
	    tofree[i].carray[l++][8] = NULL;
	    tofree[i].carray[l][0] = NULL;

	    tofree[i].colbox.gd.flags = gg_enabled | gg_visible;
	    tofree[i].colbox.gd.u.boxelements = tofree[i].carray[0];
	    tofree[i].colbox.creator = GHVBoxCreate;
	    tofree[i].marray[j++] = &tofree[i].colbox;
	}

	if ( res->font!=NULL ) {
	    int len;
	    FontRequest rq;
	    GDrawDecomposeFont(*res->font,&rq);
	    if ( rq.family_name!=NULL )
		len = 4*u_strlen(rq.family_name);
	    else
		len = strlen(rq.utf8_family_name);
	    len += 6 /* point size */ + 1 +
		    5 /* weight */ + 1 +
		    10 /* style */;
	    tofree[i].fontname = galloc(len);
	    if ( rq.family_name!=NULL ) {
		char *utf8_name = u2utf8_copy(rq.family_name);
		sprintf( tofree[i].fontname, "%d %s%dpt %s", rq.weight,
		    rq.style&1 ? "italic " : "",
		    rq.point_size,
		    utf8_name );
		free(utf8_name );
	    } else
		sprintf( tofree[i].fontname, "%d %s%dpt %s", rq.weight,
		    rq.style&1 ? "italic " : "",
		    rq.point_size,
		    rq.utf8_family_name );

	    lab[k].text = (unichar_t *) _("I.");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_utf8_popup;
	    gcd[k].gd.popup_msg = (unichar_t *) _("Inherits for same field in parent");
	    gcd[k].gd.cid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_InheritTextChange;
	    gcd[k++].creator = GCheckBoxCreate;
	    tofree[i].fontarray[0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _("Font:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = ++cid;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].fontarray[1] = &gcd[k-1];

	    lab[k].text = (unichar_t *) tofree[i].fontname;
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k].gd.cid = tofree[i].fontcid = ++cid;
	    gcd[k].gd.handle_controlevent = GRE_FontChanged;
	    gcd[k].data = &res->boxdata->active_border;
	    gcd[k++].creator = GTextFieldCreate;
	    tofree[i].fontarray[2] = &gcd[k-1];
	    if ( res->inherits_from==NULL )
		gcd[k-3].gd.flags &= ~gg_enabled;
	    else if ( *res->inherits_from->font == *res->font ) {
		gcd[k-3].gd.flags |= gg_cb_on;
		gcd[k-2].gd.flags &= ~gg_enabled;
		gcd[k-1].gd.flags &= ~gg_enabled;
	    }
	    tofree[i].fontarray[3] = GCD_Glue;
	    tofree[i].fontarray[4] = NULL;

	    tofree[i].fontbox.gd.flags = gg_enabled | gg_visible;
	    tofree[i].fontbox.gd.u.boxelements = tofree[i].fontarray;
	    tofree[i].fontbox.creator = GHBoxCreate;
	    tofree[i].marray[j++] = &tofree[i].fontbox;
	}
	if ( res->seealso1 != NULL ) {
	    lab[k].text = (unichar_t *) _("See also:");
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled;
	    gcd[k++].creator = GLabelCreate;
	    tofree[i].saarray[0] = &gcd[k-1];

	    lab[k].text = (unichar_t *) _(res->seealso1->name);
	    lab[k].text_is_1byte = true;
	    gcd[k].gd.label = &lab[k];
	    gcd[k].gd.flags = gg_visible|gg_enabled|gg_dontcopybox;
	    gcd[k].gd.box = &small_blue_box;
	    gcd[k].data = res->seealso1;
	    gcd[k].gd.handle_controlevent = GRE_ChangePane;
	    gcd[k++].creator = GButtonCreate;
	    tofree[i].saarray[1] = &gcd[k-1];

	    if ( res->seealso2!=NULL ) {
		lab[k].text = (unichar_t *) _(res->seealso2->name);
		lab[k].text_is_1byte = true;
		gcd[k].gd.label = &lab[k];
		gcd[k].gd.flags = gg_visible|gg_enabled|gg_dontcopybox;
		gcd[k].gd.box = &small_blue_box;
		gcd[k].data = res->seealso2;
		gcd[k].gd.handle_controlevent = GRE_ChangePane;
		gcd[k++].creator = GButtonCreate;
		tofree[i].saarray[2] = &gcd[k-1];
	    } else
		tofree[i].saarray[2] = GCD_Glue;

	    tofree[i].saarray[3] = GCD_Glue; tofree[i].saarray[4] = NULL;
	    tofree[i].sabox.gd.flags = gg_visible|gg_enabled;
	    tofree[i].sabox.gd.u.boxelements = tofree[i].saarray;
	    tofree[i].sabox.creator = GHBoxCreate;
	    tofree[i].marray[j++] = &tofree[i].sabox;
	}

	if ( res->extras!=NULL ) {
	    for ( l=0, extras = res->extras ; extras->name!=NULL; ++extras, ++l ) {
		int hl = l/2, base= (l&1) ? 3 : 0;
		switch ( extras->type ) {
		  case rt_bool:
		    extras->orig.ival = *(int *) (extras->val);
		    lab[k].text = (unichar_t *) _(extras->name);
		    lab[k].text_is_1byte = true;
		    gcd[k].gd.label = &lab[k];
		    gcd[k].gd.flags = gg_visible|gg_enabled;
		    if ( extras->orig.ival )
			gcd[k].gd.flags |= gg_cb_on;
		    gcd[k].gd.cid = extras->cid = ++cid;
		    gcd[k].data = extras->val;
		    gcd[k].gd.handle_controlevent = GRE_BoolChanged;
		    gcd[k++].creator = GCheckBoxCreate;
		    tofree[i].earray[hl][base] = &gcd[k-1];
		    tofree[i].earray[hl][base+1] = GCD_ColSpan;
		    tofree[i].earray[hl][base+2] = GCD_ColSpan;
		  break;
		  case rt_int:
		    extras->orig.ival = *(int *) (extras->val);
		    lab[k].text = (unichar_t *) _(extras->name);
		    lab[k].text_is_1byte = true;
		    gcd[k].gd.label = &lab[k];
		    gcd[k].gd.flags = gg_visible|gg_enabled;
		    gcd[k++].creator = GCheckBoxCreate;
		    tofree[i].earray[hl][base] = &gcd[k-1];

		    tofree[i].extradefs[l] = galloc(20);
		    sprintf( tofree[i].extradefs[l], "%d", extras->orig.ival );
		    lab[k].text = (unichar_t *) tofree[i].extradefs[l];
		    lab[k].text_is_1byte = true;
		    gcd[k].gd.pos.width = 60;
		    gcd[k].gd.label = &lab[k];
		    gcd[k].gd.flags = gg_visible|gg_enabled;
		    gcd[k].gd.cid = extras->cid = ++cid;
		    gcd[k].data = extras->val;
		    gcd[k].gd.handle_controlevent = GRE_IntChanged;
		    gcd[k++].creator = GNumericFieldCreate;
		    tofree[i].earray[hl][base+1] = &gcd[k-1];
		    tofree[i].earray[hl][base+2] = GCD_ColSpan;
		  break;
		  case rt_double:
		    extras->orig.dval = *(double *) (extras->val);
		    lab[k].text = (unichar_t *) _(extras->name);
		    lab[k].text_is_1byte = true;
		    gcd[k].gd.label = &lab[k];
		    gcd[k].gd.flags = gg_visible|gg_enabled;
		    gcd[k++].creator = GCheckBoxCreate;
		    tofree[i].earray[hl][base] = &gcd[k-1];

		    tofree[i].extradefs[l] = galloc(40);
		    sprintf( tofree[i].extradefs[l], "%g", extras->orig.dval );
		    lab[k].text = (unichar_t *) tofree[i].extradefs[l];
		    lab[k].text_is_1byte = true;
		    gcd[k].gd.label = &lab[k];
		    gcd[k].gd.flags = gg_visible|gg_enabled;
		    gcd[k].data = extras->val;
		    gcd[k].gd.cid = extras->cid = ++cid;
		    gcd[k].gd.handle_controlevent = GRE_DoubleChanged;
		    gcd[k++].creator = GTextFieldCreate;
		    tofree[i].earray[hl][base+1] = &gcd[k-1];
		    tofree[i].earray[hl][base+2] = GCD_ColSpan;
		  break;
		  case rt_color:
		    extras->orig.ival = *(int *) (extras->val);
		    lab[k].text = (unichar_t *) _(extras->name);
		    lab[k].text_is_1byte = true;
		    gcd[k].gd.label = &lab[k];
		    gcd[k].gd.flags = gg_visible|gg_enabled;
		    gcd[k++].creator = GCheckBoxCreate;
		    tofree[i].earray[hl][base] = &gcd[k-1];
		    gcd[k].gd.u.col = extras->orig.ival;
		    gcd[k].gd.flags = gg_visible|gg_enabled;
		    gcd[k].gd.cid = extras->cid = ++cid;
		    gcd[k].data = extras->val;
		    gcd[k].gd.handle_controlevent = GRE_ColorChanged;
		    gcd[k++].creator = GColorButtonCreate;
		    tofree[i].earray[hl][base+1] = &gcd[k-1];
		    tofree[i].earray[hl][base+2] = GCD_ColSpan;
		  break;
		  case rt_string:
		    extras->orig.sval = *(char **) (extras->val);
		    lab[k].text = (unichar_t *) _(extras->name);
		    lab[k].text_is_1byte = true;
		    gcd[k].gd.label = &lab[k];
		    gcd[k].gd.flags = gg_visible|gg_enabled;
		    gcd[k++].creator = GLabelCreate;
		    tofree[i].earray[hl][base] = &gcd[k-1];

		    if ( extras->orig.sval != NULL ) {
			lab[k].text = (unichar_t *) extras->orig.sval;
			lab[k].text_is_1byte = true;
			gcd[k].gd.label = &lab[k];
		    }
		    gcd[k].gd.flags = gg_visible|gg_enabled;
		    gcd[k].data = extras->val;
		    gcd[k].gd.cid = extras->cid = ++cid;
		    gcd[k].gd.handle_controlevent = GRE_StringChanged;
		    gcd[k++].creator = GTextFieldCreate;
		    tofree[i].earray[hl][base+1] = &gcd[k-1];
		    tofree[i].earray[hl][base+2] = GCD_ColSpan;
		  break;
		  case rt_image: {
		    GResImage *ri = *(GResImage **) (extras->val);
		    extras->orig.sval = copy( ri==NULL ? NULL : ri->filename );
		    lab[k].text = (unichar_t *) _(extras->name);
		    lab[k].text_is_1byte = true;
		    gcd[k].gd.label = &lab[k];
		    gcd[k].gd.flags = gg_visible|gg_enabled;
		    gcd[k++].creator = GLabelCreate;
		    tofree[i].earray[hl][base] = &gcd[k-1];

		    if ( ri != NULL ) {
			lab[k].text = (unichar_t *) "...";
			lab[k].image = ri->image;
		    } else
			lab[k].text = (unichar_t *) "? ...";
		    lab[k].text_is_1byte = true;
		    gcd[k].gd.label = &lab[k];
		    gcd[k].gd.flags = gg_visible|gg_enabled;
		    gcd[k].data = extras->val;
		    gcd[k].gd.cid = extras->cid = ++cid;
		    gcd[k].gd.handle_controlevent = GRE_ImageChanged;
		    gcd[k++].creator = GButtonCreate;
		    tofree[i].earray[hl][base+1] = &gcd[k-1];
		    tofree[i].earray[hl][base+2] = GCD_ColSpan;
		  } break;
		}
		if ( base==3 ) {
		    tofree[i].earray[hl][base+3] = GCD_Glue;
		    tofree[i].earray[hl][base+4] = NULL;
		}
	    }
	    if ( l&1 ) {
		int hl = l/2;
		tofree[i].earray[hl][3] = GCD_Glue;
		tofree[i].earray[hl][4] = GCD_Glue;
		tofree[i].earray[hl][5] = GCD_Glue;
		tofree[i].earray[hl][6] = GCD_Glue;
		tofree[i].earray[hl][7] = NULL;
	    }
	    tofree[i].earray[(l+2)/2][0] = NULL;
	    tofree[i].extrabox.gd.flags = gg_visible|gg_enabled;
	    tofree[i].extrabox.gd.u.boxelements = tofree[i].earray[0];
	    tofree[i].extrabox.creator = GHVBoxCreate;
	    tofree[i].marray[j++] = &tofree[i].extrabox;
	}
	tofree[i].marray[j++] = GCD_Glue;
	tofree[i].marray[j++] = NULL;
	tofree[i].mainbox[0].gd.flags = gg_visible|gg_enabled;
	tofree[i].mainbox[0].gd.u.boxelements = tofree[i].marray;
	tofree[i].mainbox[0].creator = GVBoxCreate;
	panes[i].text = (unichar_t *) _(res->name);
	panes[i].text_is_1byte = true;
	panes[i].gcd = &tofree[i].mainbox[0];
	for ( parent=res; parent!=NULL; parent=parent->inherits_from, ++panes[i].nesting );
	if ( k>cnt )
	    GDrawIError( "ResEdit Miscounted, expect a crash" );
    }

    memset(topgcd,0,sizeof(topgcd));
    memset(topbox,0,sizeof(topbox));
    memset(toplab,0,sizeof(toplab));

    topgcd[0].gd.flags = gg_visible|gg_enabled|gg_tabset_vert|gg_tabset_scroll;
    topgcd[0].gd.u.tabs = panes;
    topgcd[0].creator = GTabSetCreate;

    toplab[1].text = (unichar_t *) _("_OK");
    toplab[1].text_is_1byte = true;
    toplab[1].text_in_resource = true;
    topgcd[1].gd.label = &toplab[1];
    topgcd[1].gd.flags = gg_visible|gg_enabled | gg_but_default;
    topgcd[1].gd.handle_controlevent = GRE_OK;
    topgcd[1].creator = GButtonCreate;

    toplab[2].text = (unichar_t *) _("_Save As...");
    toplab[2].text_is_1byte = true;
    toplab[2].text_in_resource = true;
    topgcd[2].gd.label = &toplab[2];
    topgcd[2].gd.flags = gg_visible|gg_enabled;
    topgcd[2].gd.handle_controlevent = GRE_Save;
    topgcd[2].creator = GButtonCreate;

    toplab[3].text = (unichar_t *) _("_Cancel");
    toplab[3].text_is_1byte = true;
    toplab[3].text_in_resource = true;
    topgcd[3].gd.label = &toplab[3];
    topgcd[3].gd.flags = gg_visible|gg_enabled | gg_but_cancel;
    topgcd[3].gd.handle_controlevent = GRE_Cancel;
    topgcd[3].creator = GButtonCreate;

    barray[0] = GCD_Glue; barray[1] = &topgcd[1]; barray[2] = GCD_Glue;
    barray[3] = GCD_Glue; barray[4] = &topgcd[2]; barray[5] = GCD_Glue;
    barray[6] = GCD_Glue; barray[7] = &topgcd[3]; barray[8] = GCD_Glue;
    barray[9] = NULL;

    topbox[2].gd.flags = gg_visible | gg_enabled;
    topbox[2].gd.u.boxelements = barray;
    topbox[2].creator = GHBoxCreate;

    tarray[0][0] = &topgcd[0]; tarray[0][1] = NULL;
    tarray[1][0] = &topbox[2]; tarray[1][1] = NULL;
    tarray[2][0] = NULL;

    topbox[0].gd.pos.x = topbox[0].gd.pos.y = 2;
    topbox[0].gd.flags = gg_visible | gg_enabled;
    topbox[0].gd.u.boxelements = tarray[0];
    topbox[0].creator = GHVGroupCreate;

    GGadgetsCreate(gw,topbox);
    gre.tabset = topgcd[0].ret;

    GHVBoxSetExpandableRow(topbox[0].ret,0);
    GHVBoxSetExpandableCol(topbox[2].ret,gb_expandgluesame);

    for ( res=all, i=0; res!=NULL; res=res->next, ++i ) {
	GHVBoxSetExpandableRow(tofree[i].mainbox[0].ret,gb_expandglue);
	if ( res->examples!=NULL &&
		( res->examples->creator==GHBoxCreate ||
		  res->examples->creator==GVBoxCreate ||
		  res->examples->creator==GHVBoxCreate )) {
	    GHVBoxSetExpandableCol(res->examples->ret,gb_expandglue);
	    GHVBoxSetExpandableRow(res->examples->ret,gb_expandglue);
	}
	if ( tofree[i].ibox.ret!=NULL )
	    GHVBoxSetExpandableCol(tofree[i].ibox.ret,gb_expandglue);
	if ( tofree[i].flagsbox.ret!=NULL )
	    GHVBoxSetExpandableCol(tofree[i].flagsbox.ret,gb_expandglue);
	if ( tofree[i].colbox.ret!=NULL )
	    GHVBoxSetExpandableCol(tofree[i].colbox.ret,gb_expandglue);
	if ( tofree[i].extrabox.ret!=NULL )
	    GHVBoxSetExpandableCol(tofree[i].extrabox.ret,gb_expandglue);
	if ( tofree[i].sabox.ret!=NULL )
	    GHVBoxSetExpandableCol(tofree[i].sabox.ret,gb_expandglue);
	if ( tofree[i].fontbox.ret!=NULL )
	    GHVBoxSetExpandableCol(tofree[i].fontbox.ret,2);
	if ( res->boxdata!=NULL ) {
	    GGadgetSelectOneListItem(GWidgetGetControl(gw,tofree[i].btcid),res->boxdata->border_type);
	    GGadgetSelectOneListItem(GWidgetGetControl(gw,tofree[i].btcid+3),res->boxdata->border_shape);
	}
    }

    GHVBoxFitWindow(topbox[0].ret);
    GDrawSetVisible(gw,true);
    
    while ( !gre.done )
	GDrawProcessOneEvent(NULL);
    GDrawDestroyWindow(gw);

    for ( res=all, i=0; res!=NULL; res=res->next, ++i ) {
	free(tofree[i].gcd);
	free(tofree[i].lab);
	free(tofree[i].earray);
	free(tofree[i].fontname);
	if ( res->extras!=NULL ) {
	    for ( l=0, extras = res->extras ; extras->name!=NULL; ++extras, ++l ) {
		free( tofree[i].extradefs[l]);
		if ( extras->type == rt_image )
		    free( extras->orig.sval );
	    }
	}
	free(tofree[i].extradefs);
    }
    free( tofree );
    free( panes );
}

void GResEdit(GResInfo *additional,const char *def_res_file,void (*change_res_filename)(const char *)) {
    GResInfo *re_end;
    static int initted = false;

    if ( !initted ) {
	initted = true;
	for ( re_end = _GGadgetRIHead(); re_end->next!=NULL; re_end = re_end->next );
	re_end->next = _GButtonRIHead();
	for ( re_end = _GButtonRIHead(); re_end->next!=NULL; re_end = re_end->next );
	re_end->next = _GRadioRIHead();
	for ( re_end = _GRadioRIHead(); re_end->next!=NULL; re_end = re_end->next );
	re_end->next = _GTextFieldRIHead();
    }
    if ( additional!=NULL ) {
	for ( re_end=additional; re_end->next!=NULL; re_end = re_end->next );
	re_end->next = _GGadgetRIHead();
    } else {
	additional = _GGadgetRIHead();
	re_end = NULL;
    }
    GResEditDlg(additional,def_res_file,change_res_filename);
    if ( re_end!=NULL )
	re_end->next = NULL;
}
    