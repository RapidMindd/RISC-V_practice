#include <stdio.h>

int main(void)
{
    FILE *meminfo = fopen("/proc/meminfo", "r");

    char line[256];
    while (fgets(line, sizeof(line), meminfo))
    {
        int ram;
        if (sscanf(line, "MemTotal: %d kB", &ram) == 1)
        {
            printf("%d\n", ram);
            fclose(meminfo);
            return 0;
        }
    }

    fclose(meminfo);
    return 1;
}
