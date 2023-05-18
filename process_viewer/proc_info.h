#include <unistd.h>
#include <ftw.h>
#include <pwd.h>
#include <stdio.h>

char* get_user(uid_t uid);
void get_command(char comm[], char buff[], int length, int* first_open, int* last_close);
double cpu_caculate(double total_time, double start);
double mem_caculate(int pid);
int get_slibrary(int pid, int level);
int print_process(process proc, int level);
void print_tree(process proc_array[], int num_process,  process proc, int level);