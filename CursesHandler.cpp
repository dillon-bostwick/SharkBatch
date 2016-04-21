

/* Printing directly the NCurses API makes for some very very very ugly looking code. This
 * is an intermediary API I created in order to shield the Scheduler from some of the
 * ugliness of NCurses. There is a lot of overloading and repeated calls; I just wanted to
 * give Scheduler.cpp all the tools it needs to look as pretty as possible.
 */


using namespace std;

const int CONSOLE_ROW = 10;
const int CONSOLE_ROW_MAX = 16;
const int MENU_ROW = 0;
const int FEED_ROW = 20;
int currentFeedRow = FEED_ROW;

//func decs
void clear_console();


///////////////////

void blocking_off() {
	timeout(1); //turn off input blocking (back to asynchronous)
	cbreak();
}

void blocking_on() {
	nodelay(stdscr, false); //turn on input blocking
	echo();
	nocbreak();
}

int getIntInput() {
	char input[10];
	getstr(input);
	return atoi(input);
}
	

void menu_bar(string str) {
	move(MENU_ROW, 0);
	clrtoeol();
	mvprintw(MENU_ROW, 0, str.c_str());
	refresh();
}

//console bar printer overloads//

void console_bar(int line, string str) {
	move(CONSOLE_ROW + line, 0);
	clrtoeol();
	mvprintw(CONSOLE_ROW + line, 0, str.c_str());
	refresh();
}

void console_bar(int line, string str, int num) {
	move(CONSOLE_ROW + line, 0);
	clrtoeol();
	mvprintw(CONSOLE_ROW + line, 0, str.c_str(), num);
	refresh();
}

void console_bar(int line, JobList *list) {
	move(CONSOLE_ROW + line, 0);
	clrtoeol();
	
	//if list is empty, just print "N/A"
	if (list->empty()) {
		mvprintw(CONSOLE_ROW + line, 0, "N/A");
		refresh();
		return;
	}
	
	//iterate and print until second to last element with commas
	for (unsigned i = 0; i < list->size() - 1; i++) {
		mvprintw(CONSOLE_ROW + line, i * 2, "%d", list->at(i)->get_pid());
		mvprintw(CONSOLE_ROW + line, (i * 2) + 1, ",");
	}
	
	//print the last element without a comma
	mvprintw(CONSOLE_ROW + line, list->size() * 3, "%d", list->at(list->size() - 1)->get_pid());
	
	refresh();
}

void console_bar(string str) {
	console_bar(0, str);
}

void console_bar(string str, int num) {
	console_bar(0, str, num);
}


///////////////////////////////////////

void feed_bar(string str, int num) {
	mvprintw(currentFeedRow, 0, str.c_str(), num);
	currentFeedRow++;
}
	



bool get_y_n() {
	char yesno;
	cbreak();
	yesno = getch();
	nocbreak();
	echo();
	return (yesno == 'y');
}

void paused_bar(bool paused) {
	move(6, 0);
	clrtoeol();
	if (paused) {
		mvprintw(6, 0, "PAUSED");
	} else {
		mvprintw(6, 0, "RUNNING");
	}
	refresh();
}

//Calls a bunch of functions that configure NCurses
void curses_startup() {
	initscr(); //startup ncurses and initialize the stdscr
	cbreak(); //disable line buffering
	timeout(1); //set getch to non-blocking, allowing for synchronous loop breaking
	curs_set(0); //make the cursor invisible (makes everything look better)
	noecho(); //turn off echo user input by default
	refresh(); //refresh stdscr buffer
}

void clear_console() {
	for (int i = CONSOLE_ROW; i < CONSOLE_ROW_MAX; i++) {
		move(i, 0);
		clrtoeol();
	}
	refresh();
}

void main_menu() {
	menu_bar("p = toggle pause. a = add job. f = add jobs from file. l = lookup. k = kill. e = end");
}