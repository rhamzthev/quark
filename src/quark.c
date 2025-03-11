#include <ncurses.h>

int main() {
    // Initialize ncurses
    initscr();
    // Disable echoing of input characters
    noecho();
    // Hide the cursor
    curs_set(0);

    // Get the dimensions of the window
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    // Print a message at the center of the window
    mvprintw(yMax / 2, xMax / 2 - 5, "Hello, ncurses!");
    // Refresh the screen to show the changes
    refresh();
    // Wait for user input
    getch();

    // Clean up and exit
    endwin();
    return 0;
}