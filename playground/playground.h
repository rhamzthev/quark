#ifndef PLAYGROUND_H
#define PLAYGROUND_H

#include <ncurses.h>

// Constants
#define FILETREE_WIDTH 20
#define SCROLLBAR_WIDTH 1
#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1000
#define PATH_MAX 4096

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Data structures
typedef struct {
    char **lines;
    int line_count;
    int scroll_position;
} FileContent;

typedef struct TreeNode {
    char *name;
    int is_directory;
    int is_expanded;
    struct TreeNode **children;
    struct TreeNode *parent;  // Add this line
    int child_count;
    int child_capacity;
} TreeNode;

typedef struct {
    TreeNode *root;
    int selected_index;
} FileTree;

// Function declarations
// Screen handling
void init_screen(void);
void draw_layout(FileContent *content, FileTree *tree);
void draw_scrollbar(int total_lines, int visible_lines, int scroll_position);

// File operations
FileContent* load_file(const char *filename);
void display_file_content(FileContent *content);

// Tree operations
TreeNode* create_node(const char *name, int is_directory);
void add_child(TreeNode *parent, TreeNode *child);
TreeNode* build_tree(const char *path);
void draw_tree_node(TreeNode *node, int x, int y, int *current_y, int depth);

// Helper function declarations to add to playground.h
TreeNode* find_node_at_y(TreeNode *root, int target_y, int *current_y, int depth);
char* get_node_path(TreeNode *node);

// Memory management functions
void free_file_content(FileContent *content);
void free_file_tree(FileTree *tree);
void free_tree_node(TreeNode *node);  // Add this line

#endif // PLAYGROUND_H