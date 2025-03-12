#include "editor.h"
#include <ncurses.h>
#include <math.h>

static int scroll_y = 0;

void initEditor(void)
{
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    mousemask(BUTTON1_PRESSED, NULL);
    mouseinterval(0);
}

void cleanupEditor(void)
{
    endwin();
}

void displayBuffer(Buffer *buffer, int cursor_x, int cursor_y)
{
    clear();

    // Adjust scroll position to keep cursor in view
    if (cursor_y < scroll_y + SCROLL_MARGIN)
    {
        scroll_y = MAX(0, cursor_y - SCROLL_MARGIN);
    }
    if (cursor_y >= scroll_y + LINES - SCROLL_MARGIN - 1)
    {
        scroll_y = cursor_y - LINES + SCROLL_MARGIN + 2;
    }

    // Display buffer content
    int screen_y = 0;
    int file_y = 0;
    int x = 0;

    for (int i = 0; i < buffer->size && screen_y < LINES - 1; i++)
    {
        if (file_y >= scroll_y)
        {
            if (buffer->content[i] == '\n')
            {
                screen_y++;
                x = 0;
            }
            else
            {
                mvaddch(screen_y, x, buffer->content[i]);
                x++;
            }
        }
        if (buffer->content[i] == '\n')
        {
            file_y++;
            if (file_y < scroll_y)
            {
                x = 0;
            }
        }
    }

    // Status line
    int pos_str_length = 11 + (int)log10(cursor_y + 1) + (int)log10(cursor_x + 1);
    mvprintw(LINES - 1, COLS - pos_str_length, "Ln %d, Col %d", cursor_y + 1, cursor_x + 1);

    // Move cursor to correct screen position
    move(cursor_y - scroll_y, cursor_x);
    refresh();
}

void handleInput(Buffer *buffer, int ch, int *cursor_x, int *cursor_y)
{
    MEVENT event;

    if (ch == KEY_MOUSE)
    {
        if (getmouse(&event) == OK)
        {
            if (event.bstate & BUTTON1_PRESSED)
            {
                // First check if the clicked line exists
                int clicked_y = event.y;
                int clicked_x = event.x;

                // Count lines and find line length
                int current_y = 0;
                int line_start = 0;
                int line_length = 0;

                // Find the clicked line
                for (int i = 0; i < buffer->size && current_y <= clicked_y; i++)
                {
                    if (current_y == clicked_y)
                    {
                        if (buffer->content[i] == '\n')
                        {
                            break;
                        }
                        line_length++;
                    }
                    else if (buffer->content[i] == '\n')
                    {
                        current_y++;
                        line_start = i + 1;
                        line_length = 0;
                    }
                }

                // Only update position if line exists
                if (current_y == clicked_y)
                {
                    *cursor_y = clicked_y;
                    // Limit x position to end of line
                    *cursor_x = (clicked_x < line_length) ? clicked_x : line_length;
                }
                return;
            }
        }
    }

    int current_pos = 0;
    for (int i = 0; i < *cursor_y; i++)
    {
        while (current_pos < buffer->size && buffer->content[current_pos] != '\n')
        {
            current_pos++;
        }
        if (current_pos < buffer->size)
        {
            current_pos++; // Skip the newline
        }
    }
    current_pos += *cursor_x;

    if (ch == 17)
    { // Ctrl+Q
        return;
    }
    else if (ch == '\t')
    { // Tab key
        for (int i = 0; i < 4; i++)
        { // 4 spaces for a tab
            insertChar(buffer, current_pos + i, ' ');
        }
        *cursor_x += 4;
    }
    else if (ch == KEY_HOME)
    { // Home key
        *cursor_x = 0;
    }
    else if (ch == KEY_END)
    { // End key
        // Find end of current line
        int end_pos = current_pos;
        while (end_pos < buffer->size && buffer->content[end_pos] != '\n')
        {
            end_pos++;
        }
        *cursor_x = end_pos - (current_pos - *cursor_x);
    }
    else if (ch == 535)
    { // Ctrl+Home (may vary by terminal)
        *cursor_x = 0;
        *cursor_y = 0;
    }
    else if (ch == 530)
    { // Ctrl+End (may vary by terminal)
        // Find last line and its length
        int total_lines = 0;
        int last_line_length = 0;
        for (int i = 0; i < buffer->size; i++)
        {
            if (buffer->content[i] == '\n')
            {
                total_lines++;
                last_line_length = 0;
            }
            else if (i == buffer->size - 1)
            {
                last_line_length++;
            }
            else
            {
                last_line_length++;
            }
        }
        *cursor_y = total_lines;
        *cursor_x = last_line_length;
    }
    else if (ch == KEY_UP)
    {
        if (*cursor_y > 0)
        {
            (*cursor_y)--;
            // Adjust cursor_x if needed
            int line_length = 0;
            int line_start = current_pos;
            while (line_start > 0 && buffer->content[line_start - 1] != '\n')
            {
                line_start--;
            }
            while (line_start + line_length < buffer->size &&
                   buffer->content[line_start + line_length] != '\n')
            {
                line_length++;
            }
            if (*cursor_x > line_length)
            {
                *cursor_x = line_length;
            }
        }
    }
    else if (ch == KEY_DOWN)
    {
        int count_newlines = 0;
        for (int i = 0; i < buffer->size; i++)
        {
            if (buffer->content[i] == '\n')
            {
                count_newlines++;
            }
        }
        if (*cursor_y < count_newlines)
        {
            (*cursor_y)++;
            // Adjust cursor_x if needed
            int line_length = 0;
            int line_start = current_pos;
            while (line_start < buffer->size && buffer->content[line_start] != '\n')
            {
                line_start++;
            }
            if (line_start < buffer->size)
            {
                line_start++; // Skip the newline
                while (line_start + line_length < buffer->size &&
                       buffer->content[line_start + line_length] != '\n')
                {
                    line_length++;
                }
                if (*cursor_x > line_length)
                {
                    *cursor_x = line_length;
                }
            }
        }
    }
    else if (ch == KEY_LEFT)
    {
        if (*cursor_x > 0)
        {
            (*cursor_x)--;
        }
        else if (*cursor_y > 0)
        {
            (*cursor_y)--;
            int line_length = 0;
            int temp_pos = current_pos - 1;
            while (temp_pos >= 0 && buffer->content[temp_pos] != '\n')
            {
                temp_pos--;
                line_length++;
            }
            *cursor_x = line_length;
        }
    }
    else if (ch == KEY_RIGHT)
    {
        if (current_pos < buffer->size && buffer->content[current_pos] != '\n')
        {
            (*cursor_x)++;
        }
        else if (current_pos < buffer->size && buffer->content[current_pos] == '\n')
        {
            (*cursor_y)++;
            *cursor_x = 0;
        }
    }
    else if (ch == KEY_BACKSPACE || ch == 127)
    {
        if (current_pos > 0)
        {
            if (current_pos <= buffer->size && buffer->content[current_pos - 1] == '\n')
            {
                deleteChar(buffer, current_pos - 1);
                (*cursor_y)--;
                // Find the length of the previous line
                int prev_line_length = 0;
                int temp_pos = current_pos - 2;
                while (temp_pos >= 0 && buffer->content[temp_pos] != '\n')
                {
                    temp_pos--;
                    prev_line_length++;
                }
                *cursor_x = prev_line_length;
            }
            else
            {
                deleteChar(buffer, current_pos - 1);
                (*cursor_x)--;
            }
        }
    }
    else if (ch == KEY_DC)
    { // Delete key
        if (current_pos < buffer->size)
        {
            deleteChar(buffer, current_pos);
        }
    }
    else if (ch == '\n' || ch == KEY_ENTER)
    {
        insertChar(buffer, current_pos, '\n');
        (*cursor_y)++;
        *cursor_x = 0;
    }
    else if (ch >= 32 && ch <= 126)
    { // Printable ASCII
        insertChar(buffer, current_pos, ch);
        (*cursor_x)++;
    }
    else if (ch == KEY_PPAGE)
    { // Page Up
        int page_size = LINES - 2;
        for (int i = 0; i < page_size && *cursor_y > 0; i++)
        {
            (*cursor_y)--;
            // Adjust cursor_x similar to UP key
            int line_length = 0;
            int line_start = current_pos;
            while (line_start > 0 && buffer->content[line_start - 1] != '\n')
            {
                line_start--;
            }
            while (line_start + line_length < buffer->size &&
                   buffer->content[line_start + line_length] != '\n')
            {
                line_length++;
            }
            if (*cursor_x > line_length)
            {
                *cursor_x = line_length;
            }
        }
    }
    else if (ch == KEY_NPAGE)
    { // Page Down
        int page_size = LINES - 2;
        int count_newlines = 0;
        for (int i = 0; i < buffer->size; i++)
        {
            if (buffer->content[i] == '\n')
            {
                count_newlines++;
            }
        }
        for (int i = 0; i < page_size && *cursor_y < count_newlines; i++)
        {
            (*cursor_y)++;
            // Adjust cursor_x similar to DOWN key
            int line_length = 0;
            int line_start = current_pos;
            while (line_start < buffer->size && buffer->content[line_start] != '\n')
            {
                line_start++;
            }
            if (line_start < buffer->size)
            {
                line_start++;
                while (line_start + line_length < buffer->size &&
                       buffer->content[line_start + line_length] != '\n')
                {
                    line_length++;
                }
                if (*cursor_x > line_length)
                {
                    *cursor_x = line_length;
                }
            }
        }
    }
}