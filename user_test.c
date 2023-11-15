#include <stdio.h>
#include <stdlib.h>

int test()
{
    char *task_state_array[] = {
        "cat /proc/mytaskinfo",
        "echo \"R\" > /proc/mytaskinfo & cat /proc/mytaskinfo",
        "echo \"S\" > /proc/mytaskinfo & cat /proc/mytaskinfo",
        "echo \"D\" > /proc/mytaskinfo & cat /proc/mytaskinfo",
        "echo \"T\" > /proc/mytaskinfo & cat /proc/mytaskinfo",
        "echo \"t\" > /proc/mytaskinfo & cat /proc/mytaskinfo",
        "echo \"X\" > /proc/mytaskinfo & cat /proc/mytaskinfo",
        "echo \"Z\" > /proc/mytaskinfo & cat /proc/mytaskinfo",
        "echo \"P\" > /proc/mytaskinfo & cat /proc/mytaskinfo",
        "echo \"I\" > /proc/mytaskinfo & cat /proc/mytaskinfo",
    };
    for (int i = 0; i < 10; i++)
    {
        printf("currently running command: %s\n\n", task_state_array[i]);
        system(task_state_array[i]);
    }
    return 0;
}
int main()
{
    test();
    return 0;
}
