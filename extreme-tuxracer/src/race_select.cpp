/* 
 * Extreme Tux Racer
 * Copyleft 2007-2008 The ETR Team <http://www.extremetuxracer.com/>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "race_select.h"

#include "ppgltk/button.h"
#include "ppgltk/ssbutton.h"
#include "ppgltk/listbox.h"
#include "ppgltk/textarea.h"
#include "ppgltk/font.h"

#include "ppgltk/audio/audio.h"

#include "course_load.h"
#include "textures.h"
#include "joystick.h"

#include "stuff.h"
#include "game_mgr.h"

#include <iostream>



static std::list<CourseData> *courseList;
static std::list<CourseData>::iterator curElem;
static std::list<CourseData>::iterator lastCompletedRace;

RaceSelect::RaceSelect()
{
    pp::Vec2d pos(0,0);

	m_modelList = ModelHndl->l_models;
	std::list<model_t>::iterator modelit=m_modelList.begin();

	courseList = &openCourseList;
		
    // Unless we're coming back from a race, initialize the race data to 
    //   defaults.
    
    if ( GameMode::prevmode != GAME_OVER ) {
		// Initialize the race data 
		curElem = courseList->begin();
		gameMgr->setCurrentRace(curElem);
		
	    gameMgr->getCurrentRace().mirrored = false;
	    gameMgr->getCurrentRace().condition = RACE_CONDITIONS_SUNNY;
	    gameMgr->getCurrentRace().windy = false;
	    gameMgr->getCurrentRace().snowing = false;
        gameMgr->getCurrentRace().snowtype = -1;
        gameMgr->getCurrentRace().windtype = -1;
    }

	mp_titleLbl = new pp::Label(pos, "heading", _("Select a race"));
	mp_titleLbl->alignment.center();
	mp_titleLbl->alignment.middle();	
	
    mp_backBtn = new pp::Button(pos,
			      pp::Vec2d(150, 40), 
			      "button_label", 
			      _("Back") );
    mp_backBtn->setHilitFontBinding( "button_label_hilit" );
    mp_backBtn->signalClicked.Connect(pp::CreateSlot(this,&RaceSelect::back));

	
    mp_startBtn = new pp::Button(pos,
			       pp::Vec2d(150, 40),
			       "button_label",
			       _("Race!") );
    mp_startBtn->setHilitFontBinding( "button_label_hilit" );
    mp_startBtn->setDisabledFontBinding( "button_label_disabled" );
	mp_startBtn->signalClicked.Connect(pp::CreateSlot(this,&RaceSelect::start));

    
    mp_nameLbl = new pp::Label( pos,"event_and_cup_label",_("Player name:"));
	mp_nameEnt = new pp::Entry(pos,pp::Vec2d(176,32),"event_and_cup_label",players[0].name.c_str());
	mp_nameEnt->setMaxChars(13);


	mp_modelEnt = new pp::Listbox<model_t>( pos, pp::Vec2d(139, 32), "listbox_item", m_modelList);
    mp_modelEnt->signalChange.Connect(pp::CreateSlot(this,&RaceSelect::listboxModelChange));

    #define MODELID_TUX 0
    #define MODELID_SAMUEL 1
    #define MODELID_TRIXI 2
    #define MODELID_SPEEDY 3
    // model icon
	mp_modelSSBtn = new pp::SSButton( pos,
				      pp::Vec2d(89, 99),
				      4 );
    mp_modelSSBtn->setStateImage(MODELID_TUX,"modelpreviews_button",
				  pp::Vec2d( 0.0, 0.0 ),
				  pp::Vec2d( 0.25, 1.0 ),
				  pp::Color::white );
	mp_modelSSBtn->setStateImage(MODELID_SAMUEL,"modelpreviews_button",
				  pp::Vec2d( 0.25, 0.0 ),
				  pp::Vec2d( 0.50, 1.0 ),
				  pp::Color::white );

	mp_modelSSBtn->setStateImage(MODELID_TRIXI,"modelpreviews_button",
				  pp::Vec2d( 0.50, 0.0 ),
				  pp::Vec2d( 0.75, 1.0 ),
				  pp::Color::white );
                  
    mp_modelSSBtn->setStateImage(MODELID_SPEEDY,"modelpreviews_button",
				  pp::Vec2d( 0.75, 0.0 ),
				  pp::Vec2d( 1.0, 1.0 ),
				  pp::Color::white );

    
    
    modelit = mp_modelEnt->getCurrentItem();    
	mp_modelSSBtn->setState((*modelit).id);
    mp_modelSSBtn->signalClicked.Connect(pp::CreateSlot(this,&RaceSelect::buttonModelChange));
  
	mp_raceListbox = new pp::Listbox<CourseData>(pos,
				   pp::Vec2d(460, 44),
				   "listbox_item",
				   *courseList);
    mp_raceListbox->setCurrentItem( curElem );
	mp_raceListbox->signalChange.Connect(pp::CreateSlot(this,&RaceSelect::listboxItemChange));
    // Create text area 
     
    mp_descTa = new pp::Textarea( pos,
			       pp::Vec2d(312, 107),
			       "race_description",
			       "" );
	mp_descTa->setText( (*curElem).description.c_str() );
// Create preview

	mp_previewSSBtn = new pp::SSButton( pos, pp::Vec2d(132,99), 1);
	handlePreview( (*curElem).course.c_str() );
	mp_previewSSBtn->signalClicked.Connect(pp::CreateSlot(this,&RaceSelect::start));
	

    // Create state buttons - only if practicing or if cup_complete
    
	// mirror 
	mp_mirrorSSBtn = new pp::SSButton( pos,
					pp::Vec2d(32, 32),
					2 );
	mp_mirrorSSBtn->setStateImage(0,"mirror_button",
				  pp::Vec2d( 0.0/64.0, 32.0/64.0 ),
				  pp::Vec2d( 32.0/64.0, 64.0/64.0 ),
				  pp::Color::white );

	mp_mirrorSSBtn->setStateImage(1,"mirror_button",
				  pp::Vec2d( 32.0/64.0, 32.0/64.0 ),
				  pp::Vec2d( 64.0/64.0, 64.0/64.0 ),
				  pp::Color::white );

	mp_mirrorSSBtn->setState( (int)gameMgr->getCurrentRace().mirrored );
	// conditions
	mp_conditionsSSBtn = new pp::SSButton( pos,
					    pp::Vec2d(32, 32),
					    4 );
	mp_conditionsSSBtn->setStateImage(0,"conditions_button",
				  pp::Vec2d( 0.0/64.0, 32.0/64.0 ),
				  pp::Vec2d( 32.0/64.0, 64.0/64.0 ),
				  pp::Color::white );

	mp_conditionsSSBtn->setStateImage(1,"conditions_button",
				  pp::Vec2d( 32.0/64.0, 0.0/64.0 ),
				  pp::Vec2d( 64.0/64.0, 32.0/64.0 ),
				  pp::Color::white );

	mp_conditionsSSBtn->setStateImage(2,"conditions_button",
				  pp::Vec2d( 32.0/64.0, 32.0/64.0 ),
				  pp::Vec2d( 64.0/64.0, 64.0/64.0 ),
				  pp::Color::white );	

	mp_conditionsSSBtn->setStateImage(3,"conditions_button",
				  pp::Vec2d( 0.0/64.0, 0.0/64.0 ),
				  pp::Vec2d( 32.0/64.0, 32.0/64.0 ),
				  pp::Color::white );	

	mp_conditionsSSBtn->setState( (int)gameMgr->getCurrentRace().condition );
	// wind
	mp_windSSBtn = new pp::SSButton( pos,
				      pp::Vec2d(32, 32),
				      7 );
	mp_windSSBtn->setStateImage(0,"wind_button",
				  pp::Vec2d( 0.0/64.0, 96.0/128.0 ),
				  pp::Vec2d( 32.0/64.0, 128.0/128.0 ),
				  pp::Color::white );

	mp_windSSBtn->setStateImage(1,"wind_button",
				  pp::Vec2d( 32.0/64.0, 96.0/128.0 ),
				  pp::Vec2d( 64.0/64.0, 128.0/128.0 ),
				  pp::Color::white );
                  
    mp_windSSBtn->setStateImage(2,"wind_button",
				  pp::Vec2d( 0.0/64.0, 64.0/128.0 ),
				  pp::Vec2d( 32.0/64.0, 96.0/128.0 ),
				  pp::Color::white );

    mp_windSSBtn->setStateImage(3,"wind_button",
				  pp::Vec2d( 32.0/64.0, 64.0/128.0 ),
				  pp::Vec2d( 64.0/64.0, 96.0/128.0 ),
				  pp::Color::white );

    mp_windSSBtn->setStateImage(4,"wind_button",
				  pp::Vec2d( 0.0/64.0, 32.0/128.0 ),
				  pp::Vec2d( 32.0/64.0, 64.0/128.0 ),
				  pp::Color::white );

    mp_windSSBtn->setStateImage(5,"wind_button",
				  pp::Vec2d( 32.0/64.0, 32.0/128.0 ),
				  pp::Vec2d( 64.0/64.0, 64.0/128.0 ),
				  pp::Color::white );
                  
    mp_windSSBtn->setStateImage(6,"wind_button",
				  pp::Vec2d( 0.0/64.0, 0.0/128.0 ),
				  pp::Vec2d( 32.0/64.0, 32.0/128.0 ),
				  pp::Color::white );

	mp_windSSBtn->setState( gameMgr->getCurrentRace().windtype + 1 );

	// snow 
	mp_snowSSBtn = new pp::SSButton( pos,
				      pp::Vec2d(32, 32),
				      4 );
	mp_snowSSBtn->setStateImage(0,"snow_button",
				  pp::Vec2d( 0.0/64.0, 32.0/64.0 ),
				  pp::Vec2d( 32.0/64.0, 64.0/64.0 ),
				  pp::Color::white );

	mp_snowSSBtn->setStateImage(1,"snow_button",
				  pp::Vec2d( 32.0/64.0, 32.0/64.0 ),
				  pp::Vec2d( 64.0/64.0, 64.0/64.0 ),
				  pp::Color::white );
                  
    mp_snowSSBtn->setStateImage(2,"snow_button",
				  pp::Vec2d( 0.0/64.0, 0.0/64.0 ),
				  pp::Vec2d( 32.0/64.0, 32.0/64.0 ),
				  pp::Color::white );

    mp_snowSSBtn->setStateImage(3,"snow_button",
				  pp::Vec2d( 32.0/64.0, 0.0/64.0 ),
				  pp::Vec2d( 64.0/64.0, 32.0/64.0 ),
				  pp::Color::white );

	mp_snowSSBtn->setState( gameMgr->getCurrentRace().snowtype + 1 );
	
    updateButtonEnabledStates();

    play_music( "start_screen" );



}

RaceSelect::~RaceSelect()
{
	delete mp_backBtn;
	delete mp_startBtn;
	delete mp_raceListbox;
	delete mp_previewSSBtn;
	delete mp_conditionsSSBtn;
	delete mp_snowSSBtn;
	delete mp_modelSSBtn;
	delete mp_windSSBtn;
	delete mp_mirrorSSBtn;
	delete mp_descTa;	
	delete mp_titleLbl;
	delete mp_nameLbl;
	delete mp_nameEnt;
	delete mp_modelEnt;
}

void
RaceSelect::loop(float timeStep)
{
	update_audio();

    set_gl_options( GUI );

    clear_rendering_context();

    UIMgr.setupDisplay();

	// draw snow and set windy to the winSSBtn state
	drawSnow(timeStep, mp_windSSBtn->getState());

    theme.drawMenuDecorations();

    setWidgetPositionsAndDrawDecorations();

    UIMgr.draw();

    reshape( getparam_x_resolution(), getparam_y_resolution() );

    winsys_swap_buffers();
}

void
RaceSelect::setWidgetPositionsAndDrawDecorations()
{
    int w = getparam_x_resolution();
    int h = getparam_y_resolution();
    int box_width, box_height, box_max_y;
    int x_org, y_org;
//    GLuint texobj;

    // set the dimensions of the box in which all widgets should fit
    box_width = 460;
    box_height = 310;
    box_max_y = h - 128;

    x_org = w/2 - box_width/2;
    y_org = h/2 - box_height/2;
    if ( y_org + box_height > box_max_y ) {
		y_org = box_max_y - box_height;
    }

    mp_backBtn->setPosition(pp::Vec2d( x_org + 131 - mp_backBtn->getWidth()/2.0,
		      42 ) );

    mp_startBtn->setPosition(pp::Vec2d( x_org + 343 - mp_startBtn->getWidth()/2.0,
		      42 ) );

	mp_nameLbl->setPosition(pp::Vec2d( x_org,
		      y_org-40) );
	mp_nameEnt->setPosition(pp::Vec2d( x_org+120,
		      y_org-48) );

	mp_modelEnt->setPosition(pp::Vec2d( x_org+120,
		      y_org) );
    mp_modelSSBtn->setPosition(pp::Vec2d( x_org+320,
		      y_org-60) );

	mp_raceListbox->setPosition(pp::Vec2d( x_org,y_org + 221 ) );

	mp_descTa->setPosition(pp::Vec2d( x_org,y_org + 66 ) );
    
	mp_conditionsSSBtn->setPosition(
	    pp::Vec2d( x_org + box_width - 4*36 + 4,
			  y_org + 181 ) );

	mp_windSSBtn->setPosition(
	    pp::Vec2d( x_org + box_width - 3*36 + 4 ,
			  y_org + 181 ) );

	mp_snowSSBtn->setPosition(
	    pp::Vec2d( x_org + box_width - 2*36 + 4,
			  y_org + 181 ) );

	mp_mirrorSSBtn->setPosition(
	    pp::Vec2d( x_org + box_width - 1*36 + 4,
			  y_org + 181 ) );
    
	mp_titleLbl->setPosition(pp::Vec2d(
				x_org + box_width/2.0,
				y_org + 310));
	
    // Draw text indicating race requirements (if race not completed), 
    //   or results in race if race completed.
    drawStatusMsg( x_org, y_org );

    // Draw preview

	mp_previewSSBtn->setPosition(pp::Vec2d( x_org+box_width-136, y_org+70 ));
	
	//std::list<CourseData>::iterator elem;
	//elem = mp_raceListbox->getCurrentItem();

    glDisable( GL_TEXTURE_2D );

    glColor4f( 1.0, 1.0, 1.0, 1.0 );
    glBegin( GL_QUADS );
    {
	glVertex2f( x_org+box_width-140, y_org+66 );
	glVertex2f( x_org+box_width, y_org+66 );
	glVertex2f( x_org+box_width, y_org+66+107 );
	glVertex2f( x_org+box_width-140, y_org+66+107 );
    }
    glEnd();

    glEnable( GL_TEXTURE_2D );

/* Old preview system

	//current_course = (*elem).course;
	//if ( !get_texture_binding( current_course, &texobj ) ) {
    if ( !get_texture_binding( (*elem).course.c_str(), &texobj ) ) {
	if ( !get_texture_binding( "no_preview", &texobj ) ) {
	    texobj = 0;
	}
    }

    glBindTexture( GL_TEXTURE_2D, texobj );

    glBegin( GL_QUADS );
    {
	glTexCoord2d( 0, 0);
	glVertex2f( x_org+box_width-136, y_org+70 );

	glTexCoord2d( 1, 0);
	glVertex2f( x_org+box_width-4, y_org+70 );

	glTexCoord2d( 1, 1);
	glVertex2f( x_org+box_width-4, y_org+70+99 );

	glTexCoord2d( 0, 1);
	glVertex2f( x_org+box_width-136, y_org+70+99 );
	    }
    glEnd();
*/
}


void
RaceSelect::updateRaceData()
{
	players[0].name = mp_nameEnt->getContent();
  	gameMgr->getCurrentRace().course = (*curElem).course;
	gameMgr->getCurrentRace().name = (*curElem).name;
	gameMgr->getCurrentRace().description = (*curElem).description;
	gameMgr->getCurrentRace().mirrored = (bool) mp_mirrorSSBtn->getState();
	gameMgr->getCurrentRace().condition = (race_conditions_t) mp_conditionsSSBtn->getState();
	
    int snowtype = (int) mp_snowSSBtn->getState();
    gameMgr->getCurrentRace().snowing = (snowtype > 0);
    gameMgr->getCurrentRace().snowtype = gameMgr->getCurrentRace().snowing ? snowtype-1:-1;
    int windtype = (int) mp_windSSBtn->getState();
    gameMgr->getCurrentRace().windy = (windtype > 0);
    gameMgr->getCurrentRace().windtype = gameMgr->getCurrentRace().windy ? windtype-1:-1;
}

void
RaceSelect::drawStatusMsg( int x_org, int y_org )
{
	
	PlayerCourseData data;
	
	pp::Font *font = pp::Font::get("race_requirements");
	pp::Font *labelfont = pp::Font::get("race_requirements_label");
	
	if(players[0].getOpenCourseData((*curElem).name,data)){
		
		char buff[BUFF_LEN];
		int minutes;
		int seconds;
		int hundredths;

		pp::Vec2d pos(x_org,y_org + 184);
		
		getTimeComponents( data.time, minutes, seconds, hundredths );

		const char* string = _("Time:");
		labelfont->draw(string, pos);
		pos.x+=labelfont->advance(string) + 5;
		
		sprintf( buff, _("%02d:%02d.%02d"),minutes, seconds, hundredths);
		font->draw(buff, pos);
		
		pos.x+=75;
		string = _("Herring:");
		labelfont->draw(string, pos);
		pos.x+=labelfont->advance(string) + 5;

		sprintf( buff, "%03d", data.herring ); 
		font->draw(buff, pos);
		
		pos.x+=40;	
		string = _("Score:");
		labelfont->draw(string, pos);
		pos.x+=labelfont->advance(string) + 5;
	
		sprintf( buff, "%06d", data.score );
		font->draw(buff, pos);
	}
		
	const char* string = _("Contributed by:");
	pp::Vec2d pos(x_org,y_org+50);
	labelfont->draw(string,pos);
	
	pos.x += labelfont->advance(string) + 5;
		
	if(!(*curElem).contributed.empty()){
		font->draw((*curElem).contributed.c_str(),pos);		
	}else{
		font->draw(_("Unknown"),pos);
	}	
}


void
RaceSelect::updateButtonEnabledStates()
{
	mp_startBtn->setSensitive( true );
	mp_previewSSBtn->setSensitive( true );
}

void
RaceSelect::back()
{
	set_game_mode( GAME_TYPE_SELECT );
    UIMgr.setDirty();
}

void
RaceSelect::start()
{
    mp_startBtn->setHighlight( true );
	updateRaceData();	
	loop( 0 );
    set_game_mode( LOADING );
	std::list<model_t>::iterator modelit = mp_modelEnt->getCurrentItem();
	ModelHndl->load_model((*modelit).id);	
}

void
RaceSelect::handlePreview(const char* preview)
{
	GLuint texobj;
	if ( get_texture_binding( preview, &texobj ) ) 
		mp_previewSSBtn->setStateImage(0, preview, pp::Vec2d(0.0, 0.0), pp::Vec2d(1.0, 1.0), pp::Color::white);	
	else
		mp_previewSSBtn->setStateImage(0, "no_preview", pp::Vec2d(0.0, 0.0), pp::Vec2d(1.0, 1.0), pp::Color::white);
	
}

void
RaceSelect::listboxItemChange()
{
	curElem=mp_raceListbox->getCurrentItem();
	gameMgr->setCurrentRace(curElem);	
	updateRaceData();
	mp_descTa->setText( (*curElem).description.c_str() );
	handlePreview( (*curElem).course.c_str() );
	updateButtonEnabledStates();
	UIMgr.setDirty();
}

void
RaceSelect::listboxModelChange()
{
    std::list<model_t>::iterator modelit = mp_modelEnt->getCurrentItem();
    mp_modelSSBtn->setState((*modelit).id);
}

void
RaceSelect::buttonModelChange()
{
    std::list<model_t>::iterator modelit = mp_modelEnt->getCurrentItem();
    modelit++;
    if(modelit!=mp_modelEnt->getItemList().end()) {
        mp_modelEnt->setCurrentItem(modelit);
    } else {
        mp_modelEnt->setCurrentItem(mp_modelEnt->getItemList().begin());
    }
}

bool
RaceSelect::keyPressEvent(SDLKey key)
{
	switch (key){
		case SDLK_UP:
		case SDLK_LEFT:
			mp_raceListbox->gotoPrevItem();
	    	return true;
		case SDLK_RIGHT:
		case SDLK_DOWN:
			mp_raceListbox->gotoNextItem();
	    	return true;
		case SDLK_RETURN: 
	    	mp_startBtn->simulateMouseClick();
			UIMgr.setDirty();
			return true;
		case SDLK_ESCAPE: 
	    	mp_backBtn->simulateMouseClick();
			UIMgr.setDirty();
			return true;
/* We cant use only letters since that makes them imposible to type in the name input
		case 'c': 
	    	mp_conditionsSSBtn->simulateMouseClick();
	    	return true;
		case 'w': 
	    	mp_windSSBtn->simulateMouseClick();
	    	return true;
		case 'm':
	    	mp_mirrorSSBtn->simulateMouseClick();
	    	return true;
	    */
		default:
			return false;
	}	
}
