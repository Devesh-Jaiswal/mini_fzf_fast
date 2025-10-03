// mini_fzf_fast_opt.c
// Build: gcc mini_fzf_fast_opt.c -lncurses -o mini_fzf_fast_opt

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#define MAX_QUERY 256
#define MAX_MATCHES 500

typedef struct {
    char *path;
    char *path_lc;   // lowercase for fast case-insensitive search
    int color;       // 1=white,2=blue(dir),3=green(exec)
} FileEntry;

// lowercase helper
static char *str_tolower(const char *s) {
    char *res = strdup(s);
    for (char *p = res; *p; p++) *p = tolower(*p);
    return res;
}

// load files & dirs with cached color & lowercase
static FileEntry *load_files(const char *dir, int *count) {
    char cmd[1024];
    snprintf(cmd,sizeof(cmd),"find %s -type f -o -type d 2>/dev/null",dir);
    FILE *fp = popen(cmd,"r");
    if(!fp){ perror("find"); exit(1); }

    FileEntry *files = NULL;
    int cap=0,n=0;
    char line[1024];
    while(fgets(line,sizeof(line),fp)){
        line[strcspn(line,"\n")]=0;
        if(n>=cap){ cap = cap ? cap*2 : 512; files = realloc(files,cap*sizeof(FileEntry)); }
        files[n].path = strdup(line);
        files[n].path_lc = str_tolower(line);

        struct stat st;
        if(stat(line,&st)==-1) files[n].color=1;
        else if(S_ISDIR(st.st_mode)) files[n].color=2;
        else if(st.st_mode & S_IXUSR) files[n].color=3;
        else files[n].color=1;

        n++;
    }
    pclose(fp);
    *count=n;
    return files;
}

// filter by basename or full path (pre-lowercased)
static int filter_results(FileEntry *all,int n_all, FileEntry **out, const char *q) {
    if(!*q) return 0; // don't show until query typed
    char *q_lc = str_tolower(q);
    int n=0;
    for(int i=0;i<n_all;i++){
        const char *base = strrchr(all[i].path,'/');
        base = base ? base+1 : all[i].path;
        if(strstr(all[i].path_lc,q_lc) || strstr(str_tolower(base),q_lc))
            out[n++] = &all[i];
        if(n>=MAX_MATCHES) break; // limit for performance
    }
    free(q_lc);
    return n;
}

int main(int argc,char **argv){
    const char *start = (argc>1)?argv[1]:".";
    int n_all;
    FileEntry *all_files = load_files(start,&n_all);

    initscr(); cbreak(); noecho(); keypad(stdscr,TRUE); curs_set(1);
    if(has_colors()){ start_color(); init_pair(1,COLOR_WHITE,COLOR_BLACK);
                      init_pair(2,COLOR_BLUE,COLOR_BLACK); init_pair(3,COLOR_GREEN,COLOR_BLACK); }

    char query[MAX_QUERY]="";
    int pos=0, sel=0;
    FileEntry *matches[MAX_MATCHES];
    int n_match = filter_results(all_files,n_all,matches,query);

    while(1){
        int rows,cols; getmaxyx(stdscr,rows,cols);
        erase();
        attron(A_BOLD); mvprintw(0,0,"> %s",query); attroff(A_BOLD);

        int max_show = rows-3;
        int start_idx = (sel>=max_show)? sel-max_show+1 : 0;

        for(int i=0;i<max_show && start_idx+i<n_match;i++){
            int idx = start_idx+i;
            if(idx==sel) attron(A_REVERSE);
            if(has_colors()) attron(COLOR_PAIR(matches[idx]->color));
            mvprintw(i+1,0,"%.*s",cols-1,matches[idx]->path);
            if(has_colors()) attroff(COLOR_PAIR(matches[idx]->color));
            if(idx==sel) attroff(A_REVERSE);
        }

        mvprintw(rows-1,0,"[%d/%d matches | up/down=move | Enter=select | ESC/Alt+Q=quit]",n_match,n_all);
        move(0,2+pos); refresh();

        int ch = getch();
        if(ch==27 || ch==17) break;
        else if(ch=='\n' && n_match>0){ endwin(); printf("%s\n",matches[sel]->path); return 0; }
        else if(ch==KEY_UP && sel>0) sel--;
        else if(ch==KEY_DOWN && sel+1<n_match) sel++;
        else if((ch==KEY_BACKSPACE||ch==127||ch==8)&&pos>0){ query[--pos]=0; sel=0; n_match=filter_results(all_files,n_all,matches,query);}
        else if(isprint(ch)&&pos<MAX_QUERY-1){ query[pos++]=ch; query[pos]=0; sel=0; n_match=filter_results(all_files,n_all,matches,query);}
    }
    endwin();
    return 0;
}
