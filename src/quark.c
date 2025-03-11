#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define INITIAL_BUFFER_SIZE 1000

typedef struct
{
    char *content;
    int size;
    int capacity;
} Buffer;

void initBuffer(Buffer *buffer)
{
    buffer->content = malloc(INITIAL_BUFFER_SIZE);
    if (buffer->content == NULL)
    {
        endwin();
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    buffer->content[0] = '\0';
    buffer->size = 0;
    buffer->capacity = INITIAL_BUFFER_SIZE;
}

void loadFile(Buffer *buffer, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        endwin();
        fprintf(stderr, "Error: Could not open file '%s'\n", filename);
        exit(1);
    }

    // Read file content into buffer
    int ch;
    while ((ch = fgetc(file)) != EOF)
    {
        if (buffer->size + 1 >= buffer->capacity)
        {
            buffer->capacity *= 2;
            char *new_content = realloc(buffer->content, buffer->capacity);
            if (new_content == NULL)
            {
                endwin();
                fprintf(stderr, "Memory reallocation failed\n");
                fclose(file);
                exit(1);
            }
            buffer->content = new_content;
        }
        buffer->content[buffer->size++] = ch;
    }

    fclose(file);
}

void insertChar(Buffer *buffer, int pos, char ch)
{
    if (buffer->size + 1 >= buffer->capacity)
    {
        buffer->capacity *= 2;
        buffer->content = realloc(buffer->content, buffer->capacity);
        if (buffer->content == NULL)
        {
            endwin();
            fprintf(stderr, "Memory reallocation failed\n");
            exit(1);
        }
    }

    memmove(&buffer->content[pos + 1], &buffer->content[pos], buffer->size - pos + 1);
    buffer->content[pos] = ch;
    buffer->size++;
}

void deleteChar(Buffer *buffer, int pos)
{
    if (pos < buffer->size)
    {
        memmove(&buffer->content[pos], &buffer->content[pos + 1], buffer->size - pos);
        buffer->size--;
    }
}

void displayBuffer(Buffer *buffer, int cursor_x, int cursor_y)
{
    clear();

    // Display buffer content with line numbers
    int y = 0, x = 0;
    int line_number = 1;
    int line_number_width = 4; // Width for line numbers (e.g., "1  ", "12 ", etc.)

    // Print first line number
    mvprintw(y, 0, "%*d ", line_number_width - 1, line_number);
    x = line_number_width + 2; // Account for line number and separator

    for (int i = 0; i < buffer->size; i++)
    {
        if (buffer->content[i] == '\n')
        {
            y++;
            line_number++;
            x = 0;
            // Print line number at the start of each line
            mvprintw(y, 0, "%*d ", line_number_width - 1, line_number);
            x = line_number_width + 2;
        }
        else
        {
            mvaddch(y, x, buffer->content[i]);
            x++;
        }
    }

    // Status line
    int pos_str_length = 11 + (int)log10(cursor_y + 1) + (int)log10(cursor_x + 1);
    mvprintw(LINES - 1, COLS - pos_str_length, "Ln %d, Col %d", cursor_y + 1, cursor_x + 1);

    // Move cursor to current position (accounting for line number width)
    move(cursor_y, cursor_x + line_number_width + 2);
    refresh();
}

void handleInput(Buffer *buffer, int ch, int *cursor_x, int *cursor_y)
{
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
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    Buffer buffer;
    initBuffer(&buffer);
    loadFile(&buffer, argv[1]);

    int ch;
    int cursor_x = 0, cursor_y = 0;

    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    while (1)
    {
        displayBuffer(&buffer, cursor_x, cursor_y);

        ch = getch();

        if (ch == 17)
        { // Ctrl+Q
            break;
        }

        handleInput(&buffer, ch, &cursor_x, &cursor_y);
    }

    endwin();
    free(buffer.content);
    return 0;
}