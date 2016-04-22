/* Printing directly the NCurses API makes for some very very very ugly looking code. This
 * is an intermediary API I created in order to shield the Scheduler from some of the
 * ugliness of NCurses. There is a lot of overloading and repeated calls; I just wanted to
 * give Scheduler.cpp all the tools it needs to look as pretty as possible.
 */


using namespace std;

const int CONSOLE_ROW = 7;
const int CONSOLE_ROW_MAX = 12;
const int MENU_ROW = 3;
const int FEED_ROW = 32;
const int FEED_ROW_MAX = 42;
const int CORE_ROW = 25;
const int CORE_ROW_MAX = 28;
const int STATS_ROW = 46;
const int PAUSED_ROW = 20;

const int COL_LOCATION = 0; //Mostly just so I can do a word search and change it in the future


int currentFeedRow = FEED_ROW;
int maxHeight;
int maxWidth;

//XXX on 0 menu 2, 4 console 6, 13 paused 15, 17 status 19, 22 core 24, 29 feed 31, 43 stats 45, 48 49 50

//func decs
void clear_console();
void console_bar(string str);
void wireframe(int line, string str);



//Calls a bunch of functions that configure NCurses
void curses_startup() {
	initscr(); //startup ncurses and initialize the stdscr
	
	cbreak(); //disable line buffering
	timeout(1); //set getch to non-blocking, allowing for synchronous loop breaking
	curs_set(0); //make the cursor invisible (makes everything look better)
	noecho(); //turn off echo user input by default
	getmaxyx( stdscr, maxHeight, maxWidth);
	
	console_bar("Successful initialization");
	
	//wireframing
	
	
	
	
	wireframe(0, "--------------------------------------------------");
	wireframe(1, "MENU");
	wireframe(2, "----");
	
	wireframe(4, "--------------------------------------------------");
	wireframe(5, "CONSOLE");
	wireframe(6, "-------");
	
	wireframe(13, "");
	wireframe(14, "");
	wireframe(15, "");
	
	wireframe(17, "--------------------------------------------------");
	wireframe(18, "STATUS OVERVIEW");
	wireframe(19, "---------------");
	
	wireframe(22, "--------------------------------------------------");
	wireframe(23, "CURRENT CORE THREAD");
	wireframe(24, "-------------------");
	
	wireframe(29, "--------------------------------------------------");
	wireframe(30, "LOG FEED");
	wireframe(31, "--------");

	wireframe(43, "--------------------------------------------------");
	wireframe(44, "STATISTICS");
	wireframe(45, "----------");	
	
	wireframe(52, "--------------------------------------------------");	
	
	
	refresh(); //refresh stdscr buffer
	
}


void wireframe(int line, string str) {
	mvprintw(line, COL_LOCATION, str.c_str());
}



///////////////////

void stats_bar(int line, string str, double num) {
	move(STATS_ROW + line, 0);
	clrtoeol();
	
	mvprintw(STATS_ROW + line, COL_LOCATION, str.c_str(), num);
	refresh();
}


///////////////////

void blocking_off() {
	timeout(1); //turn off input blocking (back to asynchronous)
	noecho();
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
	mvprintw(MENU_ROW, COL_LOCATION /2, str.c_str());
	refresh();
}

//console bar printer overloads//

void console_bar(int line, string str) {
	move(CONSOLE_ROW + line, 0);
	clrtoeol();
	mvprintw(CONSOLE_ROW + line, COL_LOCATION, str.c_str());
	refresh();
}

void console_bar(int line, string str, int num) {
	move(CONSOLE_ROW + line, 0);
	clrtoeol();
	mvprintw(CONSOLE_ROW + line, COL_LOCATION, str.c_str(), num);
	refresh();
}

void console_bar(int line, JobList *list) {
	move(CONSOLE_ROW + line, 0);
	clrtoeol();
	
	//if list is empty, just print "N/A"
	if (list->empty()) {
		wireframe(CONSOLE_ROW + line, "N/A");
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

void console_bar(string str, char name[]) {
	move(CONSOLE_ROW, 0);
	clrtoeol();
	
	mvprintw(CONSOLE_ROW, COL_LOCATION, str.c_str());
	addstr(name);
}

void console_bar(string str) {
	console_bar(0, str);
}

void console_bar(string str, int num) {
	console_bar(0, str, num);
}


///////////////////////////////////////

void feed_bar(string str, int num) {
	if (currentFeedRow == FEED_ROW_MAX) {
		currentFeedRow = FEED_ROW;
		move(FEED_ROW_MAX, 0);
		clrtoeol();
	}
	
	move(currentFeedRow, 0);
	clrtoeol();

	mvprintw(currentFeedRow, COL_LOCATION, str.c_str(), num);
	
	currentFeedRow++;
	
	mvprintw(currentFeedRow, COL_LOCATION, "==================================================");
}


///////////////////////////////////////

void core_bar(int line, string str, int num) {
	move(CORE_ROW + line, 0);
	clrtoeol();
	mvprintw(CORE_ROW + line, COL_LOCATION, str.c_str(), num);
	refresh();
}

void clear_core_bar() {
	for (int i = CORE_ROW; i < CORE_ROW_MAX; i++) {
		move(i, 0);
		clrtoeol();
	}
	
	mvprintw(CORE_ROW, 0, "N/A");
	refresh();
}
	
///////////////////////////////////////

void paused_bar(bool paused) {
	move(PAUSED_ROW, 0);
	clrtoeol();
	if (paused) {
		wireframe(PAUSED_ROW, "--Paused--");
	} else {
		wireframe(PAUSED_ROW, "--Running--");
	}
	refresh();
}

///////////////////////////////////////



bool get_y_n() {
	char yesno;
	cbreak();
	yesno = getch();
	nocbreak();
	echo();
	return (yesno == 'y');
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