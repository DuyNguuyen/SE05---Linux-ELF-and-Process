#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/time.h>
#include "proc_struct.h"
#include "misc.h"
#include "proc_info.h"
#include "proc_sort.h"

#define MAX_LINES 20

int main(int argc, char* argv[]) {  
    int wait_timer = 10;
    int mode = 0;
    int order = 1;
    char buffer[1024] = "";
    DIR *proc_dir;
    int start_position = 0;
    int max_line = get_prompt_heigth() - 5;
    
    if (argc > 1 && is_numeric(argv[1])){
        wait_timer = atoi(argv[1]);
    }
    
    if (argc > 2 && is_numeric(argv[2])){
        mode = atoi(argv[2]);
    }
    
    if (argc > 3 && is_numeric(argv[3])){
        order = atoi(argv[3]);
    }    
    
    while (1){
        proc_dir = opendir("/proc");
         
        if (proc_dir == NULL) {
            return 1;
        }
        
        int array_size = 10;
        process *proc_array = (process*) malloc(array_size * sizeof(process));
        int num_process = 0;
        
        system("clear");
        
        struct dirent* entry;
        
        while ((entry = readdir(proc_dir)) != NULL) {  
            if (is_numeric(entry->d_name)) {    
                char stat_path[512] = "";
                sprintf(stat_path, "/proc/%s/stat", entry->d_name);

                FILE *f = fopen(stat_path, "r");
                if (f != NULL){
                    struct stat sb;
                    int pid, ppid, tty_nr;
                    int first_open = -1;
                    int last_close = -1;
                    char comm[100] = "", state;                    
                    unsigned long long utime, stime, start;
                    double total_time, cpu_usage, mem_usage;
                    
                    fgets(buffer, sizeof(buffer), f);
                    fclose(f);
                    f = NULL;

                    stat(stat_path, &sb);    //Call stat to access uid        
                    char* uname = get_user(sb.st_uid);
                    sscanf(buffer, "%d", &pid);  //Scan for pid                  
                    get_command(comm, buffer, 1024, &first_open, &last_close);       //Scan for command             
                    sscanf(buffer + last_close + 2, "%c %d %*d %*d %d %*d %*u %*lu %*lu %*lu %*lu %llu %llu %*ld %*ld %*ld %*ld %*ld %*ld %llu", 
                                &state, &ppid, &tty_nr, &utime, &stime, &start);    //Scan necessary data                      
                    total_time = (double) (utime + stime);
                    cpu_usage = cpu_caculate(total_time, (double) start);
                    mem_usage = mem_caculate(pid);
                    
                    if (num_process == array_size) {
                        array_size += 10;
                        proc_array = (process*) realloc(proc_array, array_size * sizeof(process));
                    }
                                 
                    proc_array[num_process].uname = uname;
                    proc_array[num_process].pid = pid;
                    proc_array[num_process].ppid = ppid;
                    strcpy(proc_array[num_process].comm, comm);
                    proc_array[num_process].state = state;
                    proc_array[num_process].start = start;
                    proc_array[num_process].total_time = total_time;
                    proc_array[num_process].cpu_usage = cpu_usage;
                    proc_array[num_process].mem_usage = mem_usage;
                    proc_array[num_process].tty = ttyname(tty_nr);
                    proc_array[num_process].num_children = 0;
                    
                    num_process++;
                }
            }    
        }
        
        for (int i = 0; i < num_process; i++) {
            int ppid = proc_array[i].ppid;
            for (int j = 0; j < num_process; j++) {
                if (proc_array[j].pid == ppid) {
                    proc_array[j].num_children++;
                    break;
                }
            }
        }
        
        printf("USER\t\tPID\tPPID\tSTATE\t%%CPU\t%%MEM\tTTY\t\tSTART\t\tTIME\t\tCOMMAND\t\t\t\tLIBRARY\n");
        switch (mode){
            case 1:
            {
                merge_sort(proc_array, 0, num_process - 1, compare_pid);
                break;
            }
            case 2:
            {
                merge_sort(proc_array, 0, num_process - 1, compare_ppid);
                break;
            }
            case 3:
            {
                merge_sort(proc_array, 0, num_process - 1, compare_start);
                break;
            }
            case 4:
            {
                merge_sort(proc_array, 0, num_process - 1, compare_total);
                break;
            }
            case 5:
            {
                merge_sort(proc_array, 0, num_process - 1, compare_cpu);
                break;
            }
            case 6:
            {
                merge_sort(proc_array, 0, num_process - 1, compare_mem);
                break;
            }
            default:
            {
                order = 2;
                break;
            }
        }
        
        if (order == 0)
        {
            for (int i = 0; i < num_process/2; i++){
                swap_process(&proc_array[i], &proc_array[num_process - i - 1]);
            }
        }

        struct termios old_tio, new_tio;
        tcgetattr(STDIN_FILENO, &old_tio);
        memcpy(&new_tio, &old_tio, sizeof(struct termios));
        new_tio.c_lflag &= ~ICANON;
        new_tio.c_lflag &= ~ECHO;
        new_tio.c_cc[VMIN] = 0;
        new_tio.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

        int total_line = 0;
        
        if (order == 2){
            for (int i = 0; i < num_process; i++){
                if (proc_array[i].ppid == 0){
                    print_tree(proc_array, num_process, proc_array[i], 0);
                }  
            }
        } else {
            for (int i = start_position; i < num_process; i++){
                if (total_line > max_line){
                    break;
                }
                total_line += print_process(proc_array[i], 0);
            }
        }
        
        
        fd_set fds;
        struct timeval timeout;        
        timeout.tv_sec = wait_timer;
        timeout.tv_usec = 0;        
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);  
        
        char buffer[1024];
        int ret;
        char ch;

        ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout);

        if (ret == 0) {
            printf("Timeout occurred!\n");            
        } else {
            char ch = getchar();
            if (ch == '\x1b') {
            // escape sequence
                if (getchar() == '[') {
                    char arrow = getchar();
                    if (arrow == 'A') {
                        start_position = (start_position >= 2)?(start_position - 2):start_position;
                    } else if (arrow == 'B') {
                        start_position += 2;
                    }
                }
            }
        }
        
        tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
        closedir(proc_dir);
        free(proc_array);
        proc_dir = NULL;
        proc_array = NULL;
    }
    return 0;
}//end main()