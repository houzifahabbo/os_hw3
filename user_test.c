/** user_test.c
 *
 */
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#include <unistd.h>
int cat_test()
{
    int fd = open("/proc/mytaskinfo", O_RDWR);
    if (fd == -1)
    {
        printf("Couldn't open file\n");
        return -1;
    }
    char buf[10000];

    int r = read(fd, &buf, 10000);
    printf("cat test result:\n %.10000s\n", buf);
    close(fd);
    return 0;
}
int echo_test()
{
    int fd;
    char buf[256];

    fd = open("/proc/mytaskinfo", O_RDWR);
    if (fd == -1)
    {
        printf("Couldn't open file\n");
        return -1;
    }
    int w = write(fd, "R", 1);

    close(fd);
    fd = open("/proc/mytaskinfo", O_RDWR);
    if (fd == -1)
    {
        printf("Couldn't open file\n");
        return -1;
    }
    int r = read(fd, &buf, 256);
    printf("echo test result:\n%.256s\n", buf);
    close(fd);
    return 0;
}

int main()
{
    echo_test();
    cat_test();
    return 0;
}
