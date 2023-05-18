#include "proc_struct.h"
#include "proc_info.h"

char* get_user(uid_t uid)
{
    struct passwd *pws;
    pws = getpwuid(uid);
    return pws->pw_name;
}

void get_command(char comm[], char buff[], int length, int* first_open, int* last_close){
    
    for (int i = 0; i < length; i++) {
        if (buff[i] == '(' && *first_open == -1) {
            *first_open = i;
        }
        if (buff[i] == ')') {
            *last_close = i;
        }
    }

    for (int i = *first_open + 1; i < *last_close; i++){
        comm[i - *first_open - 1] = buff[i];
    }
}

double uptime(){
    FILE * uptimefile;
    char uptime_chr[28] = "";
    double uptime = 0;

    if((uptimefile = fopen("/proc/uptime", "r")) == NULL){
        return 0;
    }

    fgets(uptime_chr, 12, uptimefile);
    fclose(uptimefile);

    uptime = strtol(uptime_chr, NULL, 10);

    return uptime;
}

double cpu_caculate(double total_time, double start){
    long Hertz =  sysconf(_SC_CLK_TCK);
    double seconds = uptime() - (start / Hertz);
    double cpu_usage = 100.0 * ((total_time / Hertz) / seconds);           
    
    return cpu_usage;
}

double mem_caculate(int pid){
    char statm_path[100] = "";
    unsigned long long mem;
    double mem_usage;
    
    sprintf(statm_path, "/proc/%d/statm", pid);
    FILE *memf = fopen(statm_path, "r");
    if (memf != NULL){
        fscanf(memf, "%*llu %llu", &mem);               
        fclose(memf);

        long total_memory = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE);
        mem_usage = (double)(mem * 4.0 * 1000.0 * 100.0) /  (double) total_memory;
        
        return (mem_usage > 0)?mem_usage:0;
    }

    return -1;
}

int get_slibrary(int pid, int level){
    int counter = 0;
    char maps_path[100] = "";
    sprintf(maps_path, "/proc/%d/maps", pid);
    FILE* maps_file = fopen(maps_path, "r");
    if (maps_file != NULL){
        
        char line[1024] = "";
        while (fgets(line, sizeof(line), maps_file) && counter < 4){
            unsigned long long start, end, offset;
            char permissions[5];
            unsigned int dev_major, dev_minor, inode;
            char pathname[1024];
            int num_fields = sscanf(line, "%*lx-%*lx %4s %*llx %*x:%*x %*u %s", permissions, pathname);
            
            if (num_fields == 2 && permissions[0] == 'r' && permissions[2] == 'x' && pathname[0] == '/') {               
                char* filename;
                int pLength = strlen(pathname);
                for (int i = pLength; i--; i > -1){
                    if (pathname[i] == '/'){
                        filename = pathname + i + 1;
                        break;
                    }
                }               

                char str[512];
                int len;
                int width = get_prompt_width();
                
                sprintf(str, "%-135s %s", "", filename);
                
                if(level > 0){
                    len = strlen(str) + level + 3;
                    if (len > width){                   
                        filename[width - 137 - level - 3] = 0;
                    }                    
                } else{
                    len = strlen(str) + 1;
                    if (len > width){                   
                        filename[width - 137] = 0;
                    }
                }
                
                if (counter == 1){
                    printf("%s", filename);
                }
                
                if (counter == 2){
                    printf("\n%-135s %s", "", filename);
                } 
                
                if (counter == 3) {            
                    printf("\n%-135s %s", "", filename);
                }
                counter++;
            }
        }
        
        fclose(maps_file);
    } else {
        printf("Error open maps");
    }
    
    printf("\n");
    return counter;
}

int print_process(process proc, int level){   
    if(level > 0){
        char *new_s = (char *) malloc(strlen(proc.uname) + level + 3);
        memset(new_s , ' ', level);
        memset(new_s + level, '|', 1);
        memset(new_s + level + 1, '-', 1);
        strcpy(new_s + level + 2, proc.uname);
        printf("%-16s", new_s);
        
        free(new_s);
        new_s = NULL;
    } else {
        printf("%-16s", proc.uname);
    }
    
    printf("%-8d", proc.pid);
    printf("%-8d", proc.ppid);
    printf("%-8c",proc.state);
    printf("%-8.2f", proc.cpu_usage);
    printf("%-8.2f", proc.mem_usage);
    printf("%-16s", proc.tty);
    printf("%-16.2f", (double) proc.start * 1.0 /sysconf(_SC_CLK_TCK));
    printf("%-16.2f", proc.total_time/sysconf(_SC_CLK_TCK));
    printf("%-32s", proc.comm);
    int total_line = get_slibrary(proc.pid, level);
    
    total_line--;
    
    if (total_line <= 0){
        return 1;
    }
    
    return total_line;
}


void print_tree(process proc_array[], int num_process, process proc, int level) {
    int num_line = print_process(proc, level);
    
    for (int i = 0; i < num_process; i++) {
        if (proc_array[i].ppid == proc.pid) {
            print_tree(proc_array, num_process, proc_array[i], level + 2);
        }
    }
}