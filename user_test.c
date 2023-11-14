/*
 * The test is not supposed to print the whole result because of some error that I couldn't figure out.
 * We have to run each test separately.
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int cat_test()
{
    int fd = open("/proc/mytaskinfo", O_RDWR);
    if (fd == -1)
    {
        perror("Couldn't open file");
        return -1;
    }

    char buf[1024];

    int r = read(fd, buf, sizeof(buf) - 1);

    buf[r] = '\0'; // Null-terminate the string

    printf("return value: %d\n buf: %.1024s\n", r, buf);
    close(fd);
    return 0;
}
int echo_test()
{
    int fd;
    char buf[1024];
    char *task_state_array[] = {
        "R",
        "S",
        "D",
        "T",
        "t",
        "X",
        "Z",
        "P",
        "I",
    };
    for (int i = 0; i < 9; i++)
    {
        buf[0] = '\0';

        fd = open("/proc/mytaskinfo", O_RDWR);
        if (fd == -1)
        {
            printf("Couldn't open file\n");
            return -1;
        }
        int w = write(fd, task_state_array[i], 1);

        close(fd);
        fd = open("/proc/mytaskinfo", O_RDWR);
        if (fd == -1)
        {
            printf("Couldn't open file\n");
            return -1;
        }
        int r = read(fd, &buf, 1024);
        printf("echo test result for %s:\n%.1024s\n", task_state_array[i], buf);
        close(fd);
    }
    return 0;
}

int main()
{
    cat_test();
    echo_test();
    return 0;
}
