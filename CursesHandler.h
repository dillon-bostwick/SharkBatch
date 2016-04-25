/*
 * Printing directly the NCurses API makes for some very very very ugly looking code. This
 * is an intermediary API I created in order to shield the Scheduler from some of the
 * ugliness of NCurses. There is a lot of overloading and repeated calls; I just wanted to
 * give Scheduler.cpp all the tools it needs to look as pretty as possible.
 */
 
#ifndef CURSESHANDLER_H_
#define CURSESHANDLER_H_
 
 #include "Job.h"
 
 
 
 class CursesHandler {
 	public:
 		CursesHandler();
 		~CursesHandler();
 		
 		//Input
 		
		int get_int_input();
		bool get_y_n();
		void blocking_off();
		void blocking_on();
		void keep_cursor_in_menu(int num);
		void wireframe(int numQueues);


		//Output
		
		void menu_bar(std::string str);
		void main_menu();

		void console_bar(int line, std::string str);
		void console_bar(int line, std::string str, int num);
		void console_bar(int line, const Job::JobList *list);
		void console_bar(std::string str, char name[]);
		void console_bar(std::string str);
		void console_bar(std::string str, int num);
		void clear_console();
		
		void status_bar( int row, std::string str);
		void status_bar(int row, std::string str, int num);
		void status_bar(int line, int row, std::string str, int num);
		void clear_status_bar();
		
		void mode_bar(bool varyQuanta, bool chainWeighting);
		
		void paused_bar(bool paused);

		void core_bar(int line, std::string str, int num);
		void clear_core_bar();

		void feed_bar(std::string str, int num);

		void stats_bar(int line, std::string str, double num);


 	private:
		//Parameters for wireframing the UI
 		static const int CONSOLE_ROW = 7;
		static const int CONSOLE_ROW_MAX = 13;
		static const int MENU_ROW = 3;
		static const int FEED_ROW = 32;
		static const int FEED_ROW_MAX = 43;
		static const int CORE_ROW = 25;
		static const int CORE_ROW_MAX = 27;
		static const int STATS_ROW = 46;
		static const int MODE_ROW = 17;
		static const int PAUSED_ROW = 18;
		static const int STATUS_ROW = 20;
		static const int STATUS_ROW_MAX = 22;

		static const int MIN_WIDTH = 87;
		static const int MIN_HEIGHT = 53;

		static const int COL_LOCATION = 0;

		//Used by functions
		int currentFeedRow;
		int consoleHeight;
		int consoleWidth;

 };

#endif //CURSESHANDLER_H_
