// mini_fzf_fast.c - Linux + WSL only
// Build: gcc mini_fzf_fast.c -lncurses -o mini_fzf_fast

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#define MAX_QUERY 256

// case-insensitive strstr
static char *strcasestr_c(const char *haystack, const char *needle) {
    if (!*needle) return (char*)haystack;
    for (; *haystack; haystack++) {
        const char *h = haystack, *n = needle;
        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) { h++; n++; }
        if (!*n) return (char*)haystack;
    }
    return NULL;
}

// basename helper
static const char *basename_c(const char *path) {
    const char *s = strrchr(path, '/');
    return s ? s + 1 : path;
}

// load files & dirs
static char **load_files(const char *dir, int *count) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "find %s -type f -o -type d 2>/dev/null", dir);
    FILE *fp = popen(cmd, "r");
    if (!fp) { perror("find"); exit(1); }

    char **list = NULL;
    char line[1024];
    int cap = 0, n = 0;
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        if (n >= cap) { cap = cap ? cap*2 : 512; list = realloc(list, cap*sizeof(char*)); }
        list[n++] = strdup(line);
    }
    pclose(fp);
    *count = n;
    return list;
}

// filter by basename or full path
static int filter_results(char **all, int n_all, char **out, const char *q) {
    if (!*q) return 0; // show nothing until query typed
    int n = 0;
    for (int i=0;i<n_all;i++) {
        if (strcasestr_c(all[i], q) || strcasestr_c(basename_c(all[i]), q))
            out[n++] = all[i];
    }
    return n;
}

// choose color
static int file_color(const char *path) {
    struct stat st;
    if (stat(path,&st)==-1) return 1;
    if (S_ISDIR(st.st_mode)) return 2;
    if (st.st_mode & S_IXUSR) return 3;
    return 1;
}

int main(int argc, char **argv) {
    const char *start = (argc>1)?argv[1]:".";
    int n_all;
    char **all_files = load_files(start,&n_all);

    initscr(); cbreak(); noecho(); keypad(stdscr,TRUE); curs_set(1);
    if (has_colors()) { start_color(); init_pair(1,COLOR_WHITE,COLOR_BLACK);
                        init_pair(2,COLOR_BLUE,COLOR_BLACK); init_pair(3,COLOR_GREEN,COLOR_BLACK); }

    char query[MAX_QUERY]="";
    int pos=0, sel=0;
    char **matches=malloc(n_all*sizeof(char*));
    int n_match = filter_results(all_files,n_all,matches,query);

    while(1){
        int rows,cols; getmaxyx(stdscr,rows,cols);
        erase();
        attron(A_BOLD); mvprintw(0,0,"> %s",query); attroff(A_BOLD);

        int max_show = rows-3;
        int start_idx = (sel>=max_show)? sel-max_show+1 : 0;

        for(int i=0;i<max_show && start_idx+i<n_match;i++){
            int idx=start_idx+i;
            const char *line=matches[idx];
            if(idx==sel) attron(A_REVERSE);
            if(has_colors()) attron(COLOR_PAIR(file_color(line)));
            mvprintw(i+1,0,"%.*s",cols-1,line);
            if(has_colors()) attroff(COLOR_PAIR(file_color(line)));
            if(idx==sel) attroff(A_REVERSE);
        }

        mvprintw(rows-1,0,"[%d/%d matches | up/down=move | Enter=select | ESC/Alt+Q=quit]",n_match,n_all);
        move(0,2+pos); refresh();

        int ch=getch();
        if(ch==27 || ch==17) break;
        else if(ch=='\n' && n_match>0){ endwin(); printf("%s\n",matches[sel]); return 0; }
        else if(ch==KEY_UP && sel>0) sel--;
        else if(ch==KEY_DOWN && sel+1<n_match) sel++;
        else if((ch==KEY_BACKSPACE||ch==127||ch==8)&&pos>0){ query[--pos]=0; sel=0; n_match=filter_results(all_files,n_all,matches,query);}
        else if(isprint(ch)&&pos<MAX_QUERY-1){ query[pos++]=ch; query[pos]=0; sel=0; n_match=filter_results(all_files,n_all,matches,query);}
    }
    endwin(); return 0;
}
