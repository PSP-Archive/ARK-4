#ifndef OSK_H
#define OSK_H

// OSK class created from the OSL OSK sources

#define OSK_RESULT_UNCHANGED	PSP_UTILITY_OSK_RESULT_UNCHANGED
#define OSK_RESULT_CANCELLED	PSP_UTILITY_OSK_RESULT_CANCELLED
#define OSK_RESULT_CHANGED		PSP_UTILITY_OSK_RESULT_CHANGED
//<-- STAS END -->

/**OSK cancel*/
#define OSK_CANCEL    PSP_UTILITY_OSK_RESULT_CANCELLED		/**<-- STAS: for backward compatibility */
/**OSK changed*/
#define OSK_CHANGED    PSP_UTILITY_OSK_RESULT_CHANGED
/**OSK unchanged*/
#define OSK_UNCHANGED    PSP_UTILITY_OSK_RESULT_UNCHANGED

class OSK{

	private:
		static SceUtilityOskParams* oskParams;
		static unsigned short *intext;
		static unsigned short *desc;
		
		static SceUtilityOskParams* initOskEx(int nData, int language);
		static int initOskDataEx(SceUtilityOskParams* oskParams, int idx,
                     unsigned short *desc, unsigned short *intext, int textLimit, int linesNumber);
		static int activateOskEx(SceUtilityOskParams* oskParams, int waitcycle);
		static int oskIsActiveEx(SceUtilityOskParams* oskParams);
		static void deActivateOskEx(SceUtilityOskParams* oskParams);
		static int oskGetResultEx(SceUtilityOskParams* oskParams, int idx);
		static unsigned short* oskOutTextEx(SceUtilityOskParams* oskParams, int idx);
		static void endOskEx(SceUtilityOskParams* oskParams);
	
	public:
		OSK();
		~OSK();

		/** Initializes the OSK
			\param *descStr
				Text shown as a description (bottom right corner)
			\param initialStr
				Initial text in the OSK
			\param textLimit
				Maximum number of chars
			\param linesNumber
				Number of lines
			\param language
				Language
					JAPANESE			0
					ENGLISH				1
					FRENCH				2
					SPANISH				3
					GERMAN				4
					ITALIAN				5
					DUTCH				6
					PORTUGUESE			7
					RUSSIAN				8
					KOREAN				9
					CHINESE_TRADITIONAL	10
					CHINESE_SIMPLIFIED	11
				If you pass to the function -1 then the language set in the firmware is used
		*/
		void init(const char *descStr, const char *initialStr, int textLimit, int linesNumber=1, int language=1);
		
		/** Draws the osk while its still active. Exits once the osk is inactive
		*/
		void loop();
		
		/** Draws the OSK
		After drawing it you should check if the user closed it. Remember to call oslEndOsk.
		\code
		if (oslOskIsActive()){
			oslDrawOsk();
			if (oslGetOskStatus() == PSP_UTILITY_DIALOG_NONE){
				//The user closed the OSK
				oslEndOsk();
			}
		}
		\endcode
		*/
		void draw();
		
		/** Checks if the OSK is active.
		*/		
		int isActive();

		/**Returns the current OSK status
		*/		
		int getStatus();

		/**Returns the OSK result (OSK_CHANGED, OSK_UNCHANGED or OSK_CANCEL)
		*/		
		int getResult();

		/**Get the text inserted in the OSK
		*/		
		void getText(char* text);
		void getText(unsigned short* text);

		/**Ends the OSK
		*/		
		void end();
};


/** @} */ // end of osk
#endif
