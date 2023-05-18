void swap_process(process* a, process* b);
int compare_pid(const process* a, const process* b);
int compare_ppid(const process* a, const process* b);
int compare_start(const process* a, const process* b);
int compare_total(const process* a, const process* b);
int compare_cpu(const process* a, const process* b);
int compare_mem(const process* a, const process* b);
void merge(process arr[], int l, int m, int r, int (*comparator)(const process*, const process*));
void merge_sort(process arr[], int l, int r, int (*comparator)(const process*, const process*));