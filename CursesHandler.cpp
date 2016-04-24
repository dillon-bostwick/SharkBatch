#include <string>
#include <iostream>
#include <stdlib.h>
#include <ncurses.h>
#include "cursesHandler.h"
#include "Job.h"

using namespace std;

//Calls a bunch of functions that configure NCurses
CursesHandler::CursesHandler() {
	initscr(); //startup ncurses and initialize the stdscr
	
	cbreak(); //disable line buffering
	timeout(1); //set getch to non-blocking, allowing for synchronous loop breaking
	curs_set(0); //make the cursor invisible (makes everything look better)
	keypad(stdscr, TRUE);
	echo(); 
	
	do {
		getmaxyx(stdscr, consoleHeight, consoleWidth);
		mvprintw(0, 0, "INCREASE TERMINAL SIZE");
		refresh();
	} while (consoleHeight < MIN_HEIGHT || consoleWidth < MIN_WIDTH);
	
	noecho(); //turn off echo user input by default

	console_bar("Successful initialization");
	
	currentFeedRow = FEED_ROW;

	refresh(); //refresh stdscr buffer
	
}

CursesHandler::~CursesHandler() {
	printw("\n"); //puts the command line cursor beneath the UI if the program quits
	curs_set(1); //make cursor visible again
	endwin(); //return window to normal command line state
}


void CursesHandler::wireframe(int numQueues) {
	mvprintw(0, COL_LOCATION, "------------------------------------------------------------------------------------");
	mvprintw(1, COL_LOCATION, "MENU");
	mvprintw(2, COL_LOCATION, "----");
	
	mvprintw(4, COL_LOCATION, "------------------------------------------------------------------------------------");
	mvprintw(5, COL_LOCATION, "CONSOLE");
	mvprintw(6, COL_LOCATION, "-------");
	
	mvprintw(13, COL_LOCATION, "------------------------------------------------------------------------------------");
	mvprintw(14, COL_LOCATION, "STATUS OVERVIEW");
	mvprintw(15, COL_LOCATION, "---------------");
	
	mvprintw(18, COL_LOCATION, "Priority:");
	
	for (int i = 0; i < numQueues; i++) {
		mvprintw(18, 15 + i * 4, "%d |", i);
	}
	
	mvprintw(18, 15 + numQueues * 4, "W");
	
	mvprintw(22, COL_LOCATION, "------------------------------------------------------------------------------------");
	mvprintw(23, COL_LOCATION, "CURRENT CORE THREAD");
	mvprintw(24, COL_LOCATION, "-------------------");
	
	mvprintw(29, COL_LOCATION, "------------------------------------------------------------------------------------");
	mvprintw(30, COL_LOCATION, "LOG");
	mvprintw(31, COL_LOCATION, "---");

	mvprintw(43, COL_LOCATION, "------------------------------------------------------------------------------------");
	mvprintw(44, COL_LOCATION, "STATISTICS");
	mvprintw(45, COL_LOCATION, "----------");	
	
	mvprintw(52, COL_LOCATION, "------------------------------------------------------------------------------------");		
}




//////////////////////////////////////////////////////////////////////////////////////////

void CursesHandler::CursesHandler::keep_cursor_in_menu(int num) {
	move(MENU_ROW, 44 + num * 3);
}

///////////////////

void CursesHandler::CursesHandler::stats_bar(int line, string str, double num) {
	move(STATS_ROW + line, 0);
	clrtoeol();
	
	mvprintw(STATS_ROW + line, COL_LOCATION, str.c_str(), num);
	refresh();
}


///////////////////

void CursesHandler::CursesHandler::blocking_off() {
	timeout(1); //turn off input blocking (back to asynchronous)
	noecho();
	cbreak();
}

void CursesHandler::CursesHandler::blocking_on() {
	nodelay(stdscr, false); //turn on input blocking
	echo();
	nocbreak();
}

int CursesHandler::get_int_input() {
	char input[10];
	getstr(input);
	return atoi(input);
}
	

void CursesHandler::CursesHandler::menu_bar(string str) {
	move(MENU_ROW, 0);
	clrtoeol();
	mvprintw(MENU_ROW, COL_LOCATION /2, str.c_str());
	refresh();
}

//console bar printer overloads//

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

void CursesHandler::console_bar(int line, JobList *list) {
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
		
		//stop printing after 5 elements
		if (i == 10) {
			printw("......(%d more jobs)", list->size() - 10);
			break;
		} else if (i != list->size() - 1) {
			printw(", ");
		}
	}

	refresh();
}

void CursesHandler::console_bar(string str, char name[]) {
	move(CONSOLE_ROW, 0);
	clrtoeol();
	
	mvprintw(CONSOLE_ROW, COL_LOCATION, str.c_str());
	addstr(name);
}

void CursesHandler::console_bar(string str) {
	console_bar(0, str);
}

void CursesHandler::console_bar(string str, int num) {
	console_bar(0, str, num);
}


///////////////////////////////////////

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
	
	mvprintw(currentFeedRow, COL_LOCATION, "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
	
	refresh();
}

///////////////////////////////////////

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


///////////////////////////////////////

void CursesHandler::core_bar(int line, string str, int num) {
	move(CORE_ROW + line, 0);
	clrtoeol();
	mvprintw(CORE_ROW + line, COL_LOCATION, str.c_str(), num);
	refresh();
}

void CursesHandler::clear_core_bar() {
	for (int i = CORE_ROW; i < CORE_ROW_MAX; i++) {
		move(i, 0);
		clrtoeol();
	}
	
	mvprintw(CORE_ROW, 0, "N/A");
	refresh();
}
	
///////////////////////////////////////

void CursesHandler::paused_bar(bool paused) {
	move(PAUSED_ROW, 0);
	clrtoeol();
	if (paused) {
		mvprintw(PAUSED_ROW, COL_LOCATION, "--Paused--");
	} else {
		mvprintw(PAUSED_ROW, COL_LOCATION, "--Running--");
	}
	refresh();
}

///////////////////////////////////////



bool CursesHandler::get_y_n() {
	char yesno;
	cbreak();
	yesno = getch();
	nocbreak();
	echo();
	return (yesno == 'y');
}

void CursesHandler::clear_console() {
	for (int i = CONSOLE_ROW; i < CONSOLE_ROW_MAX; i++) {
		move(i, 0);
		clrtoeol();
	}
	refresh();
}

void CursesHandler::main_menu() {
	menu_bar("p = toggle pause. a = add job. f = add jobs from file. l = lookup. k = kill. e = end");
}