/*
 * CursesHandler.cpp
 * by Dillon Bostwick
 * see CursesHandler.h for details
 */

#include <string>
#include <iostream>
#include <stdlib.h>
#include <ncurses.h>
#include "CursesHandler.h"
#include "Job.h"

using namespace std;

//Calls a variety of NCurses methods to initialize the SharkBatch I/O environment
CursesHandler::CursesHandler() {
	initscr(); //startup ncurses and initialize the stdscr (terminal window object)
	cbreak(); //disables line buffering
	timeout(1); //set getch to non-blocking, allowing for "asynchronous" loop breaking
	curs_set(0); //make the cursor invisible (makes everything look better)
	echo(); //echo user input to the current location of the cursor
	
	//Force the user to expand the terminal size until it is at MIN_WIDTH x MIN_HEIGHT
	do {
		getmaxyx(stdscr, consoleHeight, consoleWidth);
		mvprintw(0, 0, "INCREASE TERMINAL SIZE (must be at least %d x %d)", 
		         MIN_WIDTH, MIN_HEIGHT);
		refresh();
	} while (consoleHeight < MIN_HEIGHT || consoleWidth < MIN_WIDTH);
	
	noecho(); //echo prints user input tot he current location of the cursor. We want to
			  //start with noecho() by default for the main menu but can turn it on later
	
	currentFeedRow = FEED_ROW;

	refresh(); //Syncs the buffer with the stdscr window
}

//The destructor returns the terminal to it's original state (if SharkBatch terminates and
//there are some messy text wrapping issues in your terminal, it's probably because this
//destructor was not called at the right time)
CursesHandler::~CursesHandler() {
	printw("\n"); //puts the command line cursor beneath where we were working
	curs_set(1); //make cursor visible again
	endwin(); //terminates NCureses mode and the stdscr object
	cerr << "Sucessfully exited\n";
}

//SharkBatch calls wireframe to create the UI skeleton of the entire program. Here, each
//"bar" is given a nice border and title in the appropriate place. For row numbers, the
//constant bar row specifiers are not used to make it easier to make adjustments to the UI
//in the future.
void CursesHandler::wireframe(int numQueues) {
	mvprintw(0, COL_LOCATION, "----------------------------------------------------------"
							  "--------------------------");
	mvprintw(1, COL_LOCATION, "MENU");
	mvprintw(2, COL_LOCATION, "----");
	
	mvprintw(4, COL_LOCATION, "----------------------------------------------------------"
							  "--------------------------");
	mvprintw(5, COL_LOCATION, "CONSOLE");
	mvprintw(6, COL_LOCATION, "-------");
	
	mvprintw(14, COL_LOCATION, "---------------------------------------------------------"
							   "---------------------------");
	mvprintw(15, COL_LOCATION, "STATUS OVERVIEW");
	mvprintw(16, COL_LOCATION, "---------------");
	
	mvprintw(19, COL_LOCATION, "Priority:");
	
	//print the priority numbers based on the passed int numQueues
	for (int i = 0; i < numQueues; i++) {
		mvprintw(19, 15 + i * 4, "%d |", i);
	}
	
	mvprintw(19, 15 + numQueues * 4, "W");
	
	mvprintw(22, COL_LOCATION, "---------------------------------------------------------"
							   "---------------------------");
	mvprintw(23, COL_LOCATION, "CURRENT CORE THREAD");
	mvprintw(24, COL_LOCATION, "-------------------");
	
	mvprintw(29, COL_LOCATION, "---------------------------------------------------------"
							   "---------------------------");
	mvprintw(30, COL_LOCATION, "LOG");
	mvprintw(31, COL_LOCATION, "---");

	mvprintw(44, COL_LOCATION, "---------------------------------------------------------"
							   "---------------------------");
	mvprintw(45, COL_LOCATION, "STATISTICS");
	mvprintw(46, COL_LOCATION, "----------");	

	mvprintw(53, COL_LOCATION, "---------------------------------------------------------"
							   "---------------------------");	
}



//////////////////////////////////////////////////////////////////////////////////////////

//Ensures input echoing is displayed in menu bar
void CursesHandler::CursesHandler::keep_cursor_in_menu(int num) {
	move(MENU_ROW, 44 + num * 3);
}

//sets NCurses to take "asynchronous" I/O. If off, getch returns ERR if no key has been
//pressed, allowing it to be checked after each iteration of in an infinite loop
void CursesHandler::CursesHandler::blocking_off() {
	timeout(1); //turn off input blocking (back to asynchronous)
	noecho();
	cbreak(); //returns characters one at a time
}

//If blocking is on, an input function will pause and wait until the user does something.
//I also want input to be echoed and the user presses enter to submit
void CursesHandler::CursesHandler::blocking_on() {
	nodelay(stdscr, false); //turn on input blocking
	echo();
	nocbreak(); //waits for enter before a string of characters or integers is returned
}

//returns an integer from the user (make sure blocking is on)
int CursesHandler::get_int_input() {
	char input[10];
	getstr(input);
	return atoi(input);
}

//returns whether or not the user pressed the 'y' character (make sure blocking is on)
bool CursesHandler::get_y_n() {
	char yesno;
	cbreak();
	yesno = getch();
	nocbreak();
	echo();
	return (yesno == 'y');
}

//print the main menu to the menu bar
void CursesHandler::main_menu() {
	menu_bar("p = toggle pause. a = add job. f = add jobs from file. l = lookup. "
	"k = kill. e = end");
}



//Menu bar////////////////////////////////////////////////////////////////////////////////

//Always takes a string.
void CursesHandler::CursesHandler::menu_bar(string str) {
	move(MENU_ROW, 0);
	clrtoeol(); //these lines clear the bar from its current state
	mvprintw(MENU_ROW, COL_LOCATION, str.c_str()); //convert the std::string to a C string
	refresh(); //Sync the buffer
}



//Console bar/////////////////////////////////////////////////////////////////////////////

//When printing to the console, you can optionally specify a line number to begin, and an
//integer at the end if you are printing a printf() style %d (if you pass a str with a
//printf() style break, mvprintw can interpret that str with the break)

void CursesHandler::CursesHandler::console_bar(int line, string str) {
	move(CONSOLE_ROW + line, 0);
	clrtoeol();
	mvprintw(CONSOLE_ROW + line, COL_LOCATION, str.c_str());
	refresh();
}

void CursesHandler::console_bar(int line, string str, int num) {
	move(CONSOLE_ROW + line, 0);
	clrtoeol();
	mvprintw(CONSOLE_ROW + line, COL_LOCATION, str.c_str(), num);
	refresh();
}

//Passed a vector of jobs, the console will print inline the list of PIDs up to 10 PIDs
void CursesHandler::console_bar(int line, const Job::JobList *list) {
	move(CONSOLE_ROW + line, 0);
	clrtoeol();
	
	//if list is empty, just print "N/A"
	if (list->empty()) {
		mvprintw(CONSOLE_ROW + line, COL_LOCATION, "N/A");
		refresh();
		return;
	}
	
	//iterate and print until second to last element with commas
	//NOTE: WHEN PRINTING INTEGERS, THE ROW IS THE LEFTMOST DIGIT, REGARDLESS OF # DIGITS
	for (unsigned i = 0; i < list->size(); i++) {
		printw("%d", list->at(i)->get_pid());
		
		//stop printing after 10 elements
		if (i == 10) {
			printw("......(%d more jobs)", list->size() - 10);
			break;
		} else if (i != list->size() - 1) {
			printw(", ");
		}
	}

	refresh();
}

//Compatibility with a C style string -- always prints the string str first and then the
//char name[] immediately afterwards
void CursesHandler::console_bar(string str, char name[]) {
	move(CONSOLE_ROW, 0);
	clrtoeol();
	
	mvprintw(CONSOLE_ROW, COL_LOCATION, str.c_str());
	addstr(name); //prints a C style string to the current location of the cursor
}

void CursesHandler::console_bar(string str) {
	console_bar(0, str);
}

void CursesHandler::console_bar(string str, int num) {
	console_bar(0, str, num);
}

//Wipe all console lines away -- useful if some lower level lines linger and were not
//removed when an inline print function is called
void CursesHandler::clear_console() {
	for (int i = CONSOLE_ROW; i < CONSOLE_ROW_MAX; i++) {
		move(i, 0);
		clrtoeol();
	}
	refresh();
}



//Status bar//////////////////////////////////////////////////////////////////////////////

void CursesHandler::status_bar( int row, string str) {
	mvprintw(STATUS_ROW, row, str.c_str());
	refresh();
}

void CursesHandler::status_bar(int row, string str, int num) {
	mvprintw(STATUS_ROW, row, str.c_str(), num);
	refresh();
}

void CursesHandler::status_bar(int line, int row, string str, int num) {
	mvprintw(STATUS_ROW + line, row, str.c_str(), num);
	refresh();
}

void CursesHandler::clear_status_bar() {
	for (int i = STATUS_ROW; i < STATUS_ROW_MAX; i++) {
		move(i, 0);
		clrtoeol();
	}
	refresh();
}



//Paused and mode/////////////////////////////////////////////////////////////////////////

//The paused bar and mode bar are technically within the status bar area, but they have
//their own functions. Mode bar simply specifies whether quanta mode and/or weighting
//mode are selected, and the paused bar specifies whether the program is currently
//paused or running

void CursesHandler::paused_bar(bool paused) {
	move(PAUSED_ROW, 0);
	clrtoeol();
	if (paused) {
		mvprintw(PAUSED_ROW, COL_LOCATION, "~~Paused~~");
	} else {
		mvprintw(PAUSED_ROW, COL_LOCATION, "~~Running~~");
	}
	refresh();
}

void CursesHandler::mode_bar(bool varyQuanta, bool chainWeighting) {
	move(MODE_ROW, 0);
	clrtoeol();
	
	if (varyQuanta) {
		mvprintw(MODE_ROW, COL_LOCATION, "~~Quanta Mode~~");
	}
	if (chainWeighting) {
		mvprintw(MODE_ROW, COL_LOCATION + 17, "~~Weighting Mode~~");
	}
	
	refresh();
}



//Core bar////////////////////////////////////////////////////////////////////////////////

//Print lines to the core bar, must specify a line when printing
void CursesHandler::core_bar(int line, string str, int num) {
	move(CORE_ROW + line, 0);
	clrtoeol();
	mvprintw(CORE_ROW + line, COL_LOCATION, str.c_str(), num);
	refresh();
}

//When the core bar clears, it always says "N/A"
void CursesHandler::clear_core_bar() {
	for (int i = CORE_ROW; i < CORE_ROW_MAX; i++) {
		move(i, 0);
		clrtoeol();
	}
	
	mvprintw(CORE_ROW, 0, "N/A");
	refresh();
}



//Feed bar////////////////////////////////////////////////////////////////////////////////

//The feed bar prints each line in place and iterates until the end of the bar, in which
//case it wraps back to the top. The "^^^" bar is always beneath the most recently printed
//line. In the future I hope use an array to cycle through where the most recent line
//is always at the top (e.g. a Facebook news feed)
void CursesHandler::feed_bar(string str, int num) {
	if (currentFeedRow == FEED_ROW_MAX) {
		currentFeedRow = FEED_ROW;
		move(FEED_ROW_MAX, 0);
		clrtoeol();
	}
	
	move(currentFeedRow, 0);
	clrtoeol();

	mvprintw(currentFeedRow, COL_LOCATION, str.c_str(), num);
	
	currentFeedRow++;
	
	mvprintw(currentFeedRow, COL_LOCATION, "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
										   "^^^^^^^^^^");
	refresh();
}



//Statitistcs bar/////////////////////////////////////////////////////////////////////////

void CursesHandler::CursesHandler::stats_bar(int line, string str, double num) {
	move(STATS_ROW + line, 0);
	clrtoeol();
	
	mvprintw(STATS_ROW + line, COL_LOCATION, str.c_str(), num);
	refresh();
}


