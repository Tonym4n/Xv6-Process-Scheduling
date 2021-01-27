#include "types.h"
#include "user.h"

#define P_LOOP_CNT 0x01000000
#define C_LOOP_CNT 0x03000000

#define NUM_CHILD_PROC 6
#define MAX_PRIORITY 3


void do_parent(void)
{
    volatile int cnt = 0;
    volatile int tmp = 0;

    while(cnt < P_LOOP_CNT)
    {
        tmp += cnt;
        cnt ++;
    }
}


void do_child(void)
{
    volatile int cnt = 0;
    volatile int tmp = 0;

    while(cnt < C_LOOP_CNT)
    {
        tmp += cnt;
        cnt ++;
    }

    exit();
}

void example_test_code()
{
    int pid = 0;

    pid = fork();
    if (pid < 0)
    {
        printf(1, "fork() failed!\n");
        exit();
    }
    else if (pid == 0) // child
    {
        //sleep(100);
        do_child();
    }
    else // parent
    {
        do_parent();
        if (wait() < 0)
        {
            printf(1, "wait() failed!\n");
        }
    }
	
	printf(1, "\n");
}

void my_test_code(char sched_policy, char rg_policy)
{
    set_priority(getpid(), MAX_PRIORITY);       //parent has highest priority

    int cpid[NUM_CHILD_PROC];
    
    for(int i = 0; i < NUM_CHILD_PROC; i++)     //create all children
    {
        cpid[i] = fork();
        if(cpid[i] == 0)
        {
            do_child();
            exit();
        }
    }

    if(sched_policy == '1')
        for(int i = 0; i < NUM_CHILD_PROC; i++)     //parent assigns every 2 child procs w/ same priority
            set_priority(cpid[i], (i/2) % MAX_PRIORITY + 1);

    printf(1, "------------\n");
    for(int i = 0; i < NUM_CHILD_PROC; i++)         //display all child pids and priorities
        printf(1, "Parent: child (pid=%d priority=%d) created!\n", cpid[i], get_priority(cpid[i]));
    printf(1, "------------\n");

    enable_sched_trace(1);

    if(rg_policy == '1')
    {
        int num_proc_top_two_priority = 0;
        for(int i = 0; i < NUM_CHILD_PROC; i++)     //get num of child procs with top 2 priorities
            if(get_priority(cpid[i]) >= MAX_PRIORITY - 1)
                num_proc_top_two_priority++;
            
        while(wait() > 0)                           //wait for all children to finish
        {
            num_proc_top_two_priority--;
            if(num_proc_top_two_priority == 0)      //create a rg_proc after all procs with top 2 priorites exit
            {
                int rg_cpid = fork();
                if(rg_cpid == 0)
                {
                    do_child();
                    exit();
                }
            }
        }
    }
    else
        while(wait() > 0);

    enable_sched_trace(0);

    printf(1, "\n");
}

int
main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf(1, "Usage: %s scheduler_type [with_rg_proc]\n"
                  "\tscheduler_type:\n"
                  "\t\t0: Use the default xv6 scheduler\n"
                  "\t\t1: Use the priority-based scheduler\n"
                  "\twith_rg_proc (used with priority based scheduler):\n"
                  "\t\t0: Without runtime-generated process\n"
                  "\t\t1: With runtime-generated process\n", argv[0]);
        exit();
    }

    char sched_policy = argv[1][0];
    char rg_policy = argv[2][0];

    if(sched_policy == '0' && rg_policy == '0')
    {
        set_sched(0);
        printf(1, "============\n"
                  "Using the default xv6 based scheduler, without runtime generated process\n");
    }
    else if(sched_policy == '0' && rg_policy == '1')
    {
        set_sched(0);
        printf(1, "============\n"
                  "Using the default xv6 based scheduler, with runtime generated process\n");
    }
    else if(sched_policy == '1' && rg_policy == '0')
    {
        set_sched(1);
        printf(1, "============\n"
                  "Using the priority-based scheduler, without runtime generated process\n");
    }
    else if(sched_policy == '1' && rg_policy == '1')
    {
        set_sched(1);
        printf(1, "============\n"
                  "Using the priority-based scheduler, with runtime generated process\n");
    }
    else
        exit();
    
    /* ---------------- start: add your test code ------------------- */
    
    my_test_code(sched_policy, rg_policy);

    /* ---------------- end: add your test code ------------------- */
    
    exit();
}
