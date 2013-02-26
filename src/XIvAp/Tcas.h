#ifndef TCAS_H
#define TCAS_H

#define T_CYAN	0
#define T_WHITE	1
#define T_RED	2
#define T_YELLOW	3
#define	AURALTIME_WARNING	10 //in seconds
#define	AURALTIME_THREAT	3 //in seconds
/** Enumeration of aural warnings */

enum AURAL_code {
  AURAL_none             =  0, ///< don't say anything

  AURAL_traffic          =  1, ///< Traffic, Traffic
  AURAL_traffic_silent   =  2, ///< State when traffic is near but no sound output. Traffic Traffic is not repeated
  AURAL_clear            =  3, ///< Clear of Conflict
  AURAL_climb            =  4, ///< Climb, Climb
  AURAL_descend          =  5, ///< Descend, Descend
  AURAL_donot_climb      =  6, ///< Do NOT Climb, Do NOT Climb
  AURAL_donot_descend    =  7, ///< Do NOT Descend, Do NOT Descend
  AURAL_reversal_climb   =  8, ///< Climb, Climb NOW; Climb, Climb NOW
  AURAL_reversal_descend =  9, ///< Descend, Descend NOW; Descend, Descend NOW
  AURAL_increase_climb   = 10, ///< Increase Climb, Increase Climb
  AURAL_increase_descend = 11, ///< Increase Descend, Increase Descend
  AURAL_maintain_rate    = 12, ///< Maintain Vertical Speed, Maintain
  AURAL_weakened         = 13, ///< Adjust Vertical Speed, Adjust
  AURAL_crossing_climb   = 14, ///< Clinb, Crossing Climb (twice)
  AURAL_crossing_descend = 15  ///< Descend, Crossing Descend (twice)
};


typedef	struct
{
	char	pilot[14];	
	float	distance;
	int		x_plot;
	int		y_plot;
	int		x_scale;
	int		y_scale;
	char	timeout;
	int		altitude;
	bool	show;
	int		lastaltitude;
	int		elevation;
	char	vspeed;		//0=stable,+1 climbing with more then 500f/min, -1 desc with more then -500f/min
	char	displaymode;
	char	vstimer;
	int		auraltimer;
}TCASLIST;

class Tcas{
private: XPLMWindowID window;
#define	 TopLeft	1
#define	 TopRight	2
#define	 BottomLeft 3
#define	 BottomRight	4

#define	Norm	0
#define	All		1
#define	Above	2
#define	Below	3
#define	STBY	0
#define	TA		1
#define RA		2
		 void Create();
		 bool dragging,clicking;
		 int beginDrag(int x, int y);
		 int continueDrag(int x, int y);
		 int endDrag(int x, int y);
		 int mDownX, mDownY;
		 int clickButton;
		 bool sound;

		 int AlignRight(char* txt)
			{
				return (right-(int) strlen(txt)*6);
		 };
		 int checkClick(int x, int y);
		 int continueClick(int x, int y);
		 int endClick(int x, int y);
		 int checkButton(int x, int y);
		 char zone;
		 void buttonClicked(int button);
		 void SwitchSound() { if (sound) sound=false; else sound=true;};
		 void Zone() {if (zone>=Below) zone=Norm; else zone++;}
		 void ChangeRange(){range++;if (range>4) range=0;}; 
		 void TcasMode(){tcasmode++;if (tcasmode>=3) tcasmode=0;};
		 void	Testtargets();
		 double deg2rad(double deg);
		 double rad2deg(double rad);
		 float TrueCourse (double lat2, double lon2, double lat1, double lon1);

public:
	~Tcas();
#define TcasSize	200
#define	maxplanes	10

	enum RangeScale{NM3,NM5,NM10,NM20,NM40};
	static   int left, top, right, bottom;
	
	int Size()
	{
		return	TcasSize;
	}
	void TcasWinDrawCallback(XPLMWindowID inWindowID, void *inRefcon);
	int TcasWinMouseCallback(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus inMouse, void *inRefcon);
	void TcasKeyCallback(XPLMWindowID inWindowID, char inKey, XPLMKeyFlags inFlags, char inVirtualKey,
	void *inRefcon, int losingFocus);
	void pluginStart() { Create(); };
	void pluginStop() { if(window) XPLMDestroyWindow(window); window = NULL; };
	void DumpPilotPos(const char *,float ,float,float,int );
	void TcasFlightLoopcallback(void);
	//void show() { XPLMSetWindowIsVisible(window, 1); };
	//void hide() { XPLMSetWindowIsVisible(window, 0); };
	static TCASLIST Tcaslist[];
	static char tcasmode;
	static char range;
	void PlaySoundTcas(char);
	bool Resolution (int);

};
#endif