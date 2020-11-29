#include <linux/list.h>

struct rotation_lock{
	int degree;
	int range;
	int rw_type;
	pid_t pid;
	struct list_head list;
};

void exit_rotlock(void);
