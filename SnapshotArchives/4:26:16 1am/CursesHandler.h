/*
 * CursesHandler
 * by Dillon Bostwick
 * 
 * A UI manager for the SharkBatch Scheduler to communicate with the NCurses API
 * 
 * Available "bars" for printing:
 * - Menu bar (1 line)
 * - Console bar (7 lines)
 * - Status bar including a "pause bar" and "mode bar"
 * - Core bar (3 lines)
 * - Feed bar (Print a new line and the 10 most recent prints will be visible to user)
 * - Statistics bar (7 lines)
 * 
 * Most functions are overloaded sufficiently to allow the client to print integers like
 * C style printf(), and specify rows, etc.
 *
 * Input functions allow a variety of 
 *
 * Printing directly the NCurses API makes for some very very very ugly looking code. This
 * is an intermediary API I created in order to shield the Scheduler from some of the
 * ugliness of NCurses. CursesHandler does not create a new window, but instead converts
 * the terminal to an NCurses window via the standard NCurses window object "stdscr."
 * (Hence why it seemed apt to call it a CursesHandler and not a Window class, also
 * because NCurses already has a WINDOW object).
 */
 
#ifndef CURSESHANDLER_H_
#define CURSESHANDLER_H_
 
 #include "Job.h"
 
 class CursesHandler {
 	public:
 		 CursesHandler(); //Initialization of the NCurses environment
 		~CursesHandler(); //Returns the NCurses environment to a standard terminal window
 		
 		void wireframe(int numQueues); //Creates a UI skeleton for the SharkBatch program
 		
 		//Input///////////////////////////////////////////////////////////////////////////
 		
		int get_int_input(); 	//return integer from input (blocking must be on)
		bool get_y_n();			//return whether user pressed y (blocking must be on)
		void blocking_off();	//asynchronous I/O: getch returns ERR if no key pressed
		void blocking_on();	    //Blocking: an input function waits indefinitely for a key
		void main_menu(); 	 	//print the main menu items to the menu bar
		void keep_cursor_in_menu(int num);//Ensures input echoing is displayed in menu bar
		
		//Output//////////////////////////////////////////////////////////////////////////
		
		//Menu bar
		void menu_bar(std::string str);
		
		//Console bar
		void console_bar  	      (std::string str, char name[]); //C string compatibility 
		void console_bar		  (std::string str, int num);
		void console_bar	      (std::string str);
		void console_bar(int line, std::string str);
		void console_bar(int line, std::string str, int num);
		void console_bar(int line, const Job::JobList *list); //JobList compatibility

		void clear_console(); //refresh (otherwise some lines might linger sometimes)
		
		//Status bar
		void status_bar			 (int row, std::string str);
		void status_bar			 (int row, std::string str, int num);
		void status_bar(int line, int row, std::string str, int num);
		void clear_status_bar();
		
		void mode_bar(bool varyQuanta, bool chainWeighting); //display flags enabled
		void paused_bar(bool paused); //displays running or paused

		//Core bar
		void core_bar(int line, std::string str, int num);
		void clear_core_bar();

		//Feed bar
		void feed_bar(std::string str, int num);

		//Statistics bar
		void stats_bar(int line, std::string str, double num);


 	private:
		//Constants///////////////////////////////////////////////////////////////////////
		
		//Parameters for wireframing the UI
 		static const int CONSOLE_ROW 	 = 7;
		static const int CONSOLE_ROW_MAX = 13;
		static const int MENU_ROW		 = 3;
		static const int FEED_ROW 		 = 32;
		static const int FEED_ROW_MAX 	 = 43;
		static const int CORE_ROW 		 = 25;
		static const int CORE_ROW_MAX 	 = 27;
		static const int STATS_ROW 		 = 46;
		static const int MODE_ROW		 = 17;
		static const int PAUSED_ROW 	 = 18;
		static const int STATUS_ROW 	 = 20;
		static const int STATUS_ROW_MAX  = 22;
		
		
		//Minimum necessary size of terminal in order to wireframe appropriately
		static const int MIN_WIDTH  = 87;
		static const int MIN_HEIGHT = 53;

		//Right now there is no left margin padding but this makes it easy to change
		static const int COL_LOCATION = 0;

		//Used by functions///////////////////////////////////////////////////////////////
		int currentFeedRow; //used by feed row when iterating new lines
		int consoleHeight;
		int consoleWidth;
 };

#endif //CURSESHANDLER_H_
