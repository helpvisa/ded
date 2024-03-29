/* A basic text editor written as an exercise in coding
a terminal-based editor in the C language. */
#include <stdio.h>

#define LIMIT 25000

//--------// command functions //
int read_input(char s[]);
//-------// string modification functions //
void copy_string(char from[], char to[]);
void strip_newline(char s[]); 
int squeeze(char s[], char c);
int line_squeeze(char s1[], char s2[]);void condense(char s[]);
void shift_lines_down(int from, int to);
void shift_lines_up(int from, int to);
//-------// file functions
void save_file(FILE *fptr);
void read_file(FILE *fptr);
//-------// misc functions
int count_chars(char s[]);
void print_lines();
void print_lines_numbered();
void print_help();
/* --> will have to include a function which checks all lines up to current
total line count in order to account for 'deleting' a line at the end of the
file and leaving blank lines behind.
--> we should also have a way to insert text in between lines without
overwriting the lines which follow (an append function triggered by 'a').
--> we should also alert if buffer has been modified but not saved when
an attempt is made to quit the progrma. */

enum Mode { // mode the text editor is currently operating within
  INSERT,
  APPEND,
  PROMPT
};
enum Bool { // whether the application should keep the core input loop alive
  TRUE,
  FALSE
};

//--------// global variable definitions //
FILE *f;
char *path;
char prompt[LIMIT];
char prompt_col[9] = "\033[35m";
char command = '.'; // the character which prompts command parsing
int mode = PROMPT;
int active = TRUE;
char lines[LIMIT][LIMIT];
char order[LIMIT];
int line_idx = 1;
int total_lines = 0;

int main(int argc, char *argv[]) {
  prompt[0] = '>'; // default prompt
  // process args
  printf("\033[33mWelcome to ded!\nType 'h' for a list of commands.\033[0m\n");
  if (argc > 1) {
    path = argv[1];
    read_file(f);
  } else {
    path = "untitled.txt";
    printf("\033[31mNo file specified. Editing untitled.txt\n"
      "Use 'f' to set filename.\033[0m\n");
  }

  while (active == TRUE) {
    int parsed;
    switch (mode) {
      case INSERT:
        fgets(order, LIMIT, stdin);
        strip_newline(order); // remove newline chars
        parsed = read_input(order); // check if a command was input
        if (!parsed) {
          copy_string(order, lines[line_idx]);
          if (line_idx > total_lines) {
            total_lines = line_idx;
          }
          line_idx++;
          mode = APPEND; // now we append each next line
        }
        break;
      case APPEND:
        fgets(order, LIMIT, stdin);
        strip_newline(order); // remove newline chars
        parsed = read_input(order); // check if a command was input
        if (!parsed) {
          shift_lines_up(line_idx, total_lines);
          total_lines++;
          copy_string(order, lines[line_idx]);
          line_idx++;
        }
        break;
      default: // we default to PROMPT mode if no other mode is set
        printf("%s%s\033[0m", prompt_col, prompt);
        fgets(order, LIMIT, stdin);
        strip_newline(order);
        read_input(order);
        break;
    }
  }

  printf("\n\033[33mGoodbye!\033[0m\n\n");

  return 0;
}

//--------// functions //
void copy_string(char from[], char to[]) { // copy one string to another
  int i;
  for (i = 0; from[i] != '\0'; i++) {
    to[i] = from[i];
  }
  to[i] = '\0'; // EOF null character
}

int read_input(char s[]) { // read the input string to parse and exec commands
  int command_parsed = 0;
  copy_string("\033[35m", prompt_col); // reset prompt col
  
  if (mode != PROMPT && s[0] == command && s[1] == '\0') {
    mode = PROMPT;
    command_parsed = 1;
    printf("\033[33mEntering prompt mode...\033[0m\n");
    return command_parsed;
  } else if (mode == PROMPT) {
    if (s[0] == '\0') {
      printf("\033[31m? Blank command.\033[0m\n");
      copy_string("\033[31m", prompt_col); // set prompt col to red
    } else if (s[0] != command) {
      int stop_parsing = 0; // if 1, do not execute next command
      for (int i = 0; s[i] != '\0' && stop_parsing == 0; i++) {
        switch (s[i]) {
          case 'i':
            mode = INSERT;
            printf("\033[33mEntering insert mode on line %d...\033[0m\n", line_idx);
            stop_parsing = 1;
            break;
          case 'a':
            shift_lines_up(line_idx+1, total_lines);
            total_lines++;
            line_idx++;
            mode = INSERT; // shifting lines up 'appends'; we insert on new line
            printf("\033[33mEntering append mode on new line %d...\033[0m\n", line_idx);
            stop_parsing = 1;
            break;
          case 'd':
            lines[line_idx][0] = '\0';
              if (line_idx <= total_lines) {
              shift_lines_down(line_idx,total_lines);
              total_lines--; // we have deleted one line
              printf("\033[33mCurrent line (%d) deleted.\033[0m\n", line_idx);
              if (line_idx > total_lines+1)
                line_idx = total_lines+1;
            } else printf("\033[33mCurrent line (%d) is empty.\033[0m\n", line_idx);
            break;
          case 'q':
            active = FALSE;
            stop_parsing = 1;
            break;
          case 'w':
            save_file(f);
            break;
          case 'r':
            read_file(f);
            break;
          case 'p':
            print_lines();
            break;
          case 'n':
            print_lines_numbered();
            break;
          case 'c':;
            int count = count_chars(lines[line_idx]);
            printf("\033[33m%d character%s on current line (%d).\033[0m\n",
              count, (count > 1 || count < 1) ? "s" : "", line_idx);
            printf("\033[32m%s\033[0m\n", lines[line_idx]);
            break;
          case 'l':
            printf("\033[33m%d total lines in document.\033[0m\n", total_lines);
            break;
          case '>':;
            int prompt_idx = 0;
            for (int j = i+1; s[j] != '\0'; j++) {
              prompt_idx = j-i-1;
              prompt[prompt_idx] = s[j];
            }
            prompt[prompt_idx+1] = '\0'; // null EOF char
            stop_parsing = 1;
            break;
          case 'f':;
            char new_name[LIMIT];
            int length = 0;
            int name_idx = 0;
            for (int j=i+1;
              s[j] != '\0' &&
              s[j] != '\n' &&
              s[j] != ' ' &&
              s[j] != '\t'; j++) {
                name_idx = j-i-1;
                new_name[name_idx] = s[j];
                length++;
            }
            if (length > 0) {
              new_name[name_idx+1] = '\0'; // null EOF char
              path = new_name;
              printf("\033[33mChanged filename to %s\033[0m\n", path);
            } else {
              printf("\033[31m"
                "No filename entered!\nEditing %s\033[0m\n", path);
              copy_string("\033[31m", prompt_col); // set prompt col to red
            }
            stop_parsing = 1;
            break;
          case 'g':;
            int goto_line = 0; // the line we target
            for (int j = i+1; s[j] >= '0' && s[j] <= '9'; j++) {
              goto_line *= 10;
              goto_line += s[j] - '0'; // convert char to real int
            }
            if (goto_line > -1 && goto_line < LIMIT) {
              line_idx = goto_line;
            } else {
              line_idx = 0;
            }
            printf("\033[33m"
              "Sitting on line %d...033[0m\n", line_idx);
            break;
          default:
            if (!(s[i] >= '0' && s[i] <= '9')) {
              printf("\033[31m"
                "? Command '%c' not recognized.\033[0m\n", s[i]);
              copy_string("\033[31m", prompt_col); // set prompt col to red
            }
            break;
        }
      }
    }
    command_parsed = 1;
  }
  
  return command_parsed; // 1 if successful
}

int count_chars(char s[]) { // count the chars in a given string
  int count;
  for (count = 0; s[count] != '\0'; count++) ;
  return count;
}

void strip_newline(char s[]) { // strip all newliness from the input string
  int i, j;

  for (i = j = 0; s[i] != '\0'; i++) {
    if (s[i] != '\n') {
      s[j++] = s[i];
    }
  }
  s[j] = '\0';
}

void shift_lines_down(int from, int to) { // shift down within selection
  for (int i = from; i <= to; i++) {
    copy_string(lines[i+1], lines[i]);
  }
}

void shift_lines_up(int from, int to) { // shift up within selection
  for (int i = to; i >= from; i--) {
    copy_string(lines[i], lines[i+1]);
  }
}

void save_file(FILE *fptr) { // save the open file to disk
  int total_characters = 0; // total number of characters saved to disk

  fptr = fopen(path, "w"); // open our file in write mode
  // write the text in memory to our file
  for (int i = 1; i <= total_lines; i++) {
    total_characters += count_chars(lines[i]) + 1; // add 1 for missing \n
    fprintf(fptr, "%s\n", lines[i]);
  }
  fclose(fptr);

  printf("\033[33m%d bytes saved to disk at %s.\033[0m\n",
    total_characters, path);
}

void read_file(FILE *fptr) { // open and read the specified file
  fptr = fopen(path, "r");
  if (fptr != NULL) {
    line_idx = 1; // we start reading from line 1
    while (fgets(lines[line_idx], LIMIT, fptr)) {
      strip_newline(lines[line_idx]);
      total_lines = line_idx;
      line_idx++;
    }
    printf("\033[33mEditing %s\033[0m\n", path);
    fclose(fptr);
  } else {
    printf("\033[31mFile %s does not exist!\033[0m\n", path);
    copy_string("\033[31m", prompt_col); // set prompt col to red
  }
}

void print_lines() { // print the document
  for (int i = 1; i <= total_lines; i++) {
    printf("\033[32m%s\033[0m\n", lines[i]);
  }
}

void print_lines_numbered() { // print the document with prepended numbers
  for (int i = 1; i <= total_lines; i++) {
    printf("\033[31m%5d\033[0m \033[32m%s\033[0m\n", i, lines[i]);
  }
}
