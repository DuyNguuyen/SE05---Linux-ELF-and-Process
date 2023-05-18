typedef struct process{
    int pid, ppid;                    
    unsigned long long start;
    double total_time, cpu_usage, mem_usage;
    char* tty;
    char* uname;
    char comm[100] , state;
    int num_children;
} process; 