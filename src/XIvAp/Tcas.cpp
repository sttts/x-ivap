/****************************************************************************
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
		code: KLMVA  XFMC Project
		textures: Max XFMC Project
		www.x-fmc.com/
****************************************************************************/
#include "xivap.h"
#include "helpers.h"
#include "Tcas.h"
#include "systemcallbacks.h"
#include <math.h>
#include "SoundGraphics.h"
//otherfiles affected: xivap.cpp,xivap.h,systemcallbacks.cpp,systemcallbacks.h,multiplayer.cpp
//t==time, d==distance a==alitude
const float SL_TABLE[7][6] = {
    // tTA  tRA    dTA   dRA   aTA   aRA
    {   20,   0,  0.30,    0,  850,    0  }, // SL2: < 1000 AGL
    {   25,  15,  0.33, 0.20,  850,  300  }, // SL3: < 2350 AGL
    {   30,  20,  0.48, 0.35,  850,  300  }, // SL4: < FL050
    {   40,  25,  0.75, 0.55,  850,  350  }, // SL5: < FL100
    {   45,  30,  1.00, 0.80,  850,  400  }, // SL6: < FL200
    {   48,  35,  1.30, 1.10,  850,  600  }, // SL7: < FL420
    {   48,  35,  1.30, 1.10, 1200,  700  }  // SL7: > FL420 = SL8
};

const char		RangeTab[]={3,5,10,20,40};

int Tcas::left;
int Tcas::right;
int Tcas::top;
int Tcas::bottom;
char Tcas::tcasmode;
TCASLIST Tcas::Tcaslist[maxplanes];
char Tcas::range;
#define Debug	0
#define TCASTEST	0
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::  This function converts decimal degrees to radians             :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double Tcas::deg2rad(double deg) {

	return (deg * PI / 180);

}

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::  This function converts radians to decimal degrees             :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double Tcas::rad2deg(double rad) {
	return (rad * 180 / PI);
}

float Tcas::TrueCourse (double lat2, double lon2, double lat1, double lon1)
{
	double bearing;
	float result;

	bearing = (rad2deg(atan2(sin(deg2rad(lon2) - deg2rad(lon1)) * cos(deg2rad(lat2)), cos(deg2rad(lat1)) * sin(deg2rad(lat2)) - sin(deg2rad(lat1)) * cos(deg2rad(lat2)) * cos(deg2rad(lon2) - deg2rad(lon1)))) + 360);
	result=(float) bearing;
	return (float) fmod (result,360); // (result%360);
}

/////////////////////////////////////////////////////////////////////////////////////
Tcas::~Tcas()
{
	pluginStop();
}
#if	TCASTEST
void Tcas::Testtargets(void)
{
	float range;
	for (int i=0;i<5;i++)
	{
//dump 5 targets on different distances
		range=i;
	Tcaslist[i].x_plot=sin(range)*i*2*2.2f;
	Tcaslist[i].y_plot=cos(range)*i*2*2.2f;
	Tcaslist[i].distance=0.4f;
	Tcaslist[i].altitude=3;
	Tcaslist[i].timeout=10;
	Tcaslist[i].show=true;
	Tcaslist[i].vspeed=0;

	}
}
#endif
//////////////////////////////////////
void Tcas::DumpPilotPos(const char* pilot,float lat,float lon,float alt,int tcascode)
{
#define TCASRANGE	40
#define LATENCY		10
#if Debug
char txt[100];
#endif
float course;
float dis=(float) deg2dist(lat,lon,xivap.GetLat(),xivap.GetLon());
char i=0;
if (tcascode==xpmpTransponderMode_Standby) return; //return in case transponder == off
while (i<maxplanes) {if (strcmp(Tcaslist[i].pilot,pilot)==0) //do we know this guy?
														{
															if (dis>TCASRANGE) {Tcaslist[i].timeout=0;return;}////dump this guy, out of visiblity

															course=TrueCourse(lat,lon,xivap.GetLat(),xivap.GetLon())-xivap.GetHeading();
															#if	Debug 
															sprintf(txt,"old guy %s x:%d y:%d alt:%5.0f distance %2.1f-course%3.1f  pos %d\r\n",pilot,Tcaslist[i].x_plot,Tcaslist[i].y_plot,alt,dis,course,i);
															
															XPLMDebugString(txt);
															#endif
															course=deg2rad(course);
															Tcaslist[i].x_plot=sin(course)*dis*2.2f;
															Tcaslist[i].y_plot=cos(course)*dis*2.2f;
															Tcaslist[i].distance=dis;
															Tcaslist[i].timeout=LATENCY;
															Tcaslist[i].altitude=(alt-xivap.elevationft())/100;
															Tcaslist[i].elevation=alt;
															#if	Debug 
															sprintf(txt,"Tcascode:%d\r\n",tcascode);
															XPLMDebugString(txt);
															#endif
															return;

														}
						
						i++;
						}
i=0;
//seems to be a new one: insert in free place
if (dis>TCASRANGE) return;////dump this guy, out of visiblity
while (i<maxplanes) {
					if (Tcaslist[i].timeout==0) {
												
												course=TrueCourse(lat,lon,xivap.GetLat(),xivap.GetLon())-xivap.GetHeading();
												strncpy(Tcaslist[i].pilot,pilot,12); //Kenny: max 12 chars
												#if	Debug
												sprintf(txt,"New Pilot %s lat:%f lon:%f alt:%5.01f distance %2.1f--course%3.1f-pos%d\r\n",pilot,lat,lon,alt,dis,course,i);
												XPLMDebugString(txt);
												#endif
												course=deg2rad(course);
												Tcaslist[i].x_plot=sin(course)*dis*2.2f;
												Tcaslist[i].y_plot=cos(course)*dis*2.2f;
												Tcaslist[i].distance=dis;
												Tcaslist[i].altitude=(alt-xivap.elevationft())/100;
												Tcaslist[i].elevation=alt;
												Tcaslist[i].lastaltitude=Tcaslist[i].elevation;
												Tcaslist[i].timeout=LATENCY;
												Tcaslist[i].show=false;
												Tcaslist[i].vspeed=0;
												Tcaslist[i].vstimer=12;
												Tcaslist[i].auraltimer=0;
												Tcaslist[i].displaymode=T_CYAN;
												return;
												}
					i++;
					}

}
////////////////////////////////////////////////////
bool Tcas::Resolution (int nr)
{
	const int Alttabel[6]={1000,2350,5000,10000,20000,42500};
	int i=0;

	while (i<6)   {
		if (xivap.Getgroundalt()<Alttabel[i]) break;
		i++;
	}
#if TCASTEST
	char txt[100];
	sprintf(txt,"dist:%f,alt%f-i:%4.1f-elevation:%3.1f speed:%3.1f\r\n",SL_TABLE[i][2],SL_TABLE[i][4],xivap.elevationft(),xivap.Getgroundalt(),xivap.GetSpeed());
		XPLMDebugString(txt);
if (xivap.GetSpeed()<50) return FALSE; //disable function in case no speed;

#endif
	switch (tcasmode)
	{
	case TA: if ((SL_TABLE[i][2]>Tcaslist[nr].distance) | (SL_TABLE[i][4]>abs(Tcaslist[nr].altitude*100))) 
			 {
				 if (Tcaslist[nr].auraltimer>0) Tcaslist[nr].auraltimer--; //wait in case a warning is already spoken
				 else {
					 Tcaslist[nr].auraltimer=AURALTIME_WARNING; //new timer aural warning
					 PlaySoundTcas(AURAL_traffic);
				 }
				 return TRUE;
				 
			 };
		break; //ta dMOD alarm
	case RA: if ((SL_TABLE[i][3]>Tcaslist[nr].distance) | (SL_TABLE[i][5]>abs(Tcaslist[nr].altitude))) 
			 {
				 if (Tcaslist[nr].auraltimer>0) Tcaslist[nr].auraltimer--; //wait in case a warning is already spoken
				 else {
					 Tcaslist[nr].auraltimer=AURALTIME_WARNING; //new timer aural warning
					 PlaySoundTcas(AURAL_traffic);
				 }
				 return TRUE;
				 
			 };
		break; //ra dMOD alarm
	default: break;
	}
	Tcaslist[nr].auraltimer=0;
	return FALSE;
}
/*
ABOVE - Expands the vertical scan to 9900 ft above and 2700 ft below the aircraft.
N - Normal position. TCAS vertical scan is +/- 2700 ft.
BELOW - Expands the vertical scan to 2700 ft above and 9900 ft below the aircraft.*/
void Tcas::TcasFlightLoopcallback()
{
#if TCASTEST
	Testtargets();
#endif
	char i;
	for (i=0;i<maxplanes;i++) {
		Tcaslist[i].show=false;
		if (Tcaslist[i].timeout!=0) {
			Tcaslist[i].timeout--; 
			Tcaslist[i].vstimer--;
			if (Tcaslist[i].vstimer==0) {
										Tcaslist[i].vstimer=12; //service 1x minute
										#if Debug
										char txt[100];
										sprintf(txt,"Elev:%d,LastElev%d\r\n",Tcaslist[i].lastaltitude,Tcaslist[i].elevation);
										XPLMDebugString(txt);
										#endif
										if ((Tcaslist[i].lastaltitude-Tcaslist[i].elevation)>100) Tcaslist[i].vspeed=-1; 
										else
										if ((Tcaslist[i].elevation-Tcaslist[i].lastaltitude)>100) Tcaslist[i].vspeed=1; 
										else Tcaslist[i].vspeed=0;
										Tcaslist[i].lastaltitude=Tcaslist[i].elevation; //save for next compare
										}

			if (Tcaslist[i].distance<RangeTab[range]) {

				switch(zone)
				{
				case	Norm:	if ((Tcaslist[i].altitude>-27) & (Tcaslist[i].altitude<+27)) Tcaslist[i].show=true;break;
				case	Above:	if ((Tcaslist[i].altitude>-27) & (Tcaslist[i].altitude<+99)) Tcaslist[i].show=true;break;
				case	Below:	if ((Tcaslist[i].altitude>-99) & (Tcaslist[i].altitude<+27)) Tcaslist[i].show=true;break;
				case	All: default:	Tcaslist[i].show=true;break;
				}
			} 
			if (Tcaslist[i].timeout==0) {Tcaslist[i].show=false;XPLMDebugString("removing somebody from active list\r\n");}
			switch (range)
			{
			case	NM3:	{Tcaslist[i].x_scale=Tcaslist[i].x_plot*13;Tcaslist[i].y_scale=Tcaslist[i].y_plot*13;break;}
			case	NM5:	{Tcaslist[i].x_scale=Tcaslist[i].x_plot*8;Tcaslist[i].y_scale=Tcaslist[i].y_plot*8;break;}break;
			case	NM10:	{Tcaslist[i].x_scale=Tcaslist[i].x_plot*4;Tcaslist[i].y_scale=Tcaslist[i].y_plot*4;break;}
			case	NM20:	{Tcaslist[i].x_scale=Tcaslist[i].x_plot*2;Tcaslist[i].y_scale=Tcaslist[i].y_plot*2;break;}
			case	NM40:default:{Tcaslist[i].x_scale=Tcaslist[i].x_plot;Tcaslist[i].y_scale=Tcaslist[i].y_plot;break;};
			}
			if ((Tcaslist[i].distance>=6)| (abs(Tcaslist[i].altitude)>12)) Tcaslist[i].displaymode=T_CYAN; //all traffic <>+/0 1200 and/or 6nm
			else if (Tcaslist[i].distance>3) Tcaslist[i].displaymode=T_WHITE; 
			else if (Tcaslist[i].distance>1) Tcaslist[i].displaymode=T_YELLOW; 
			else if (Resolution (i)) Tcaslist[i].displaymode=T_RED; else Tcaslist[i].displaymode=T_YELLOW;
		}
	}

}
///////////////////////////////////////////////////////////////////////////////
void	Tcas::TcasWinDrawCallback(XPLMWindowID inWindowID, void *inRefcon)
{
char txt[10];
if (xivap.uiWindow.visible()) {
									//XPLMDrawTranslucentDarkBox(left, top, right, bottom); //temp until opengl is finished
					
									sprintf(txt,"%dnm",RangeTab[range]);
									 XPLMDrawString(colWhite,left,top-7,txt, NULL, xplmFont_Basic);
																	
									switch (tcasmode)
									{
									case	STBY: {strcpy(txt,"STBY");XPLMDrawString(colRed, left+80,top-60,"TCAS OFF", NULL, xplmFont_Basic);break;}
									case	TA:		strcpy(txt,"TA");break;
									case	RA: default:	strcpy(txt,"RA");break;
									}
								    XPLMDrawString(colWhite, left,bottom+2,txt, NULL, xplmFont_Basic);
									
									switch (zone)
									{
									case Norm: strcpy(txt,"NRM");break;
									case All: strcpy(txt,"ALL");break;
									case Above: strcpy(txt,"ABV");break;
									case Below: default:strcpy(txt,"BLW");break;
									}
									XPLMDrawString(colWhite, AlignRight(txt),top-7,txt, NULL, xplmFont_Basic);
									if (sound) strcpy(txt,"SND ON"); else strcpy(txt,"SND OFF");
									XPLMDrawString(colWhite, AlignRight(txt),bottom+2,txt, NULL, xplmFont_Basic);
									if (tcasmode!=STBY) for (char (i)=0;i<maxplanes;i++) if (Tcaslist[i].show) {
																	if (Tcaslist[i].vspeed==0)  {
																		sprintf(txt,"%+d",Tcaslist[i].altitude); 
									
																	}
																	else  {
																		if (Tcaslist[i].vspeed>0) sprintf(txt,"%+d^",Tcaslist[i].altitude); 
																		else if (Tcaslist[i].vspeed<0) sprintf(txt,"%+dv",Tcaslist[i].altitude);
																		XPLMDrawString(colWhite, left+94+Tcaslist[i].x_scale+strlen(txt)*6,top-100+Tcaslist[i].y_scale-10, "!",NULL, xplmFont_Basic);
																		}
																	XPLMDrawString(colWhite, left+100+Tcaslist[i].x_scale,top-100+Tcaslist[i].y_scale-10, txt,NULL, xplmFont_Basic);
																	}
									
									
									
									}
}
int Tcas::TcasWinMouseCallback(XPLMWindowID inWindowID, int x, int y,
										XPLMMouseStatus inMouse, void *inRefcon)
{

		switch(inMouse) {
		case xplm_MouseDown:
			XPLMBringWindowToFront(window);
			if(checkClick(x, y))
				return 1;
			else
				return beginDrag(x, y);

			break;

		case xplm_MouseDrag:
			if(dragging) return continueDrag(x, y);
			else if(clicking) return continueClick(x, y);
			break;

		case xplm_MouseUp:
			if(dragging) return endDrag(x, y);
			else if(clicking) return endClick(x, y);
			break;
	}
	return 0;
}
//////////////////////////
void Tcas::Create()
{
	range=3;
	sound=true;
	zone=1;
	tcasmode=0;
	left = 50;
	top = 720;
	right = left + Size();
	bottom = top - Size();
	window = XPLMCreateWindow(left, top, right, bottom, 1, //start disabled
		TcasBoxWinDrawCallback, 
		TcasBoxKeyCallback, //provide a callback even not in use
		TcasBoxWinMouseCallback,
		NULL);

	for (char(i)=0;i<maxplanes;i++) {Tcaslist[i].timeout=0;Tcaslist[i].show=false;}
	XPLMRegisterFlightLoopCallback(TcasBoxFlightLoopCallback,		// Callback
								   1.0,					// Interval 1.0 = second per frame
								   NULL);		
								 	
}
///////////////////////////
int Tcas::continueDrag(int x, int y)
{
	int dX = mDownX - x;
	int dY = mDownY - y;
//	char text[100];
	//XPLMGetWindowGeometry(window, &left, &top, &right, &bottom);
	left -= dX;
	top -= dY;
	bottom = top - Size();
	right = left + Size();

	mDownX = x;
	mDownY = y;

	XPLMSetWindowGeometry(window, left, top, right, bottom);

	return 1;
}
int Tcas::beginDrag(int x, int y)
{
	mDownX = x;
	mDownY = y;
	dragging = true;

	return 1;
}

int Tcas::endDrag(int x, int y)
{
	dragging = false;

	return 1;
}

int Tcas::checkClick(int x, int y)
{
	clickButton = checkButton(x, y);
	if(clickButton == 0) {
		clicking = false;
		return 0;
	}
	clicking = true;
	return 1;
}

int Tcas::continueClick(int x, int y)
{
	if(clicking) return 1;
	return 0;
}

int Tcas::endClick(int x, int y)
{
	int button = checkButton(x, y);
	if(button == clickButton)
		buttonClicked(button);
	clickButton = 0;
	clicking = false;
	return 0;
}
void Tcas::buttonClicked(int button)
{
	switch (button)
	{
	case TopLeft: ChangeRange();break;
	case TopRight:Zone();break;
	case BottomLeft:TcasMode();break;
	case BottomRight:SwitchSound();break;
	default: break;
	}
}
int Tcas::checkButton(int x, int y)
{
#define CadreWidth	30
#define	CadreHight	10
	if (x<left+CadreWidth) {if (y>top-CadreHight) return TopLeft; else if (y<bottom+CadreHight) return BottomLeft;}
		else {if (x>right-CadreWidth) if (y>top-CadreHight) return TopRight; else if (y<bottom+CadreHight) return BottomRight;}
	//nothing found return 
	return 0;
}


void Tcas::PlaySoundTcas(char Ras)
{
	switch (Ras)
	{
	case	AURAL_none:  Playsound("cabin.wav");break; // =  1, ///< Traffic, Traffic           =  0, ///< don't say anything
	case	AURAL_traffic:      Playsound("tcas_traffic.wav");break; // =  1, ///< Traffic, Traffic
	case	AURAL_clear:        Playsound("tcas_clear.wav");break; //  =  3, ///< Clear of Conflict
	case	AURAL_traffic_silent:  Playsound("tcas.wav");break; //=  2, ///< State when traffic is near but no sound output. Traffic Traffic is not repeated
	case	AURAL_climb:           Playsound("tcas_climb.wav");break; //=  4, ///< Climb, Climb
	case	AURAL_descend:         Playsound("tcas_descend.wav");break; //=  5, ///< Descend, Descend
	case	AURAL_reversal_climb:  Playsound("tcas_climb_now.wav");break; //=  8, ///< Climb, Climb NOW; Climb, Climb NOW
	case	AURAL_reversal_descend:	Playsound("tcas_descend_now.wav");break; //=  9, ///< Descend, Descend NOW; Descend, Descend NOW
	case	AURAL_increase_climb:  Playsound("tcas_climb_incr.wav");break; //= 10, ///< Increase Climb, Increase Climb
	case	AURAL_increase_descend: Playsound("tcas_descend_incr.wav");break;//= 11, ///< Increase Descend, Increase Descend
	case	AURAL_crossing_climb:   Playsound("tcas_climb_x.wav");break;//= 14, ///< Climb, Crossing Climb (twice)
	case	AURAL_crossing_descend: Playsound("tcas_descend_x.wav");break;//= 15  ///< Descend, Crossing Descend (twice)
	default: break;
	}
}