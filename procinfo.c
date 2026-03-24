#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE 512
#define MAX_CPUS 256
#define MAX_TEXT 256

typedef struct CpuEntry
{
    char model[MAX_TEXT];
    int phys_bits;
    int virt_bits;
    int physical_id;
    int core_id;
    int has_physical_id;
    int has_core_id;
} CpuEntry;

static void trim_newline(char *s)
{
    int n = (int)strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r'))
    {
        s[n - 1] = '\0';
        --n;
    }
}

static void trim_spaces(char *s)
{
    int start = 0;
    int end = (int)strlen(s);

    while (s[start] == ' ' || s[start] == '\t')
        ++start;

    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\n' || s[end - 1] == '\r'))
        --end;

    if (start > 0)
    {
        int i;
        for (i = 0; start + i < end; ++i)
            s[i] = s[start + i];
        s[i] = '\0';
    }
    else
    {
        s[end] = '\0';
    }
}

static int get_value_after_colon(const char *line, char *out, int out_size)
{
    const char *p = strchr(line, ':');
    int i = 0;

    if (p == NULL)
        return 0;

    ++p;
    while (*p == ' ' || *p == '\t')
        ++p;

    while (*p != '\0' && *p != '\n' && i < out_size - 1)
    {
        out[i++] = *p++;
    }
    out[i] = '\0';
    trim_spaces(out);
    return 1;
}

static int get_arch(char *arch, int size)
{
    FILE *fp = popen("arch", "r");
    if (fp == NULL)
        return 0;

    if (fgets(arch, size, fp) == NULL)
    {
        pclose(fp);
        return 0;
    }

    trim_newline(arch);

    if (pclose(fp) == -1)
        return 0;

    return 1;
}

static int parse_address_sizes_x86(const char *value, int *phys_bits, int *virt_bits)
{
    int p, v;
    if (sscanf(value, "%d bits physical, %d bits virtual", &p, &v) == 2)
    {
        *phys_bits = p;
        *virt_bits = v;
        return 1;
    }
    return 0;
}

static int parse_mmu_riscv(const char *value, int *phys_bits, int *virt_bits)
{
    if (strcmp(value, "sv39") == 0)
    {
        *phys_bits = 56;
        *virt_bits = 39;
        return 1;
    }
    if (strcmp(value, "sv48") == 0)
    {
        *phys_bits = 56;
        *virt_bits = 48;
        return 1;
    }
    if (strcmp(value, "sv57") == 0)
    {
        *phys_bits = 56;
        *virt_bits = 57;
        return 1;
    }
    return 0;
}

static int cpu_equal_core(const CpuEntry *a, const CpuEntry *b)
{
    if (a->has_physical_id && a->has_core_id && b->has_physical_id && b->has_core_id)
    {
        return a->physical_id == b->physical_id && a->core_id == b->core_id;
    }
    return 0;
}

static int read_cpuinfo(CpuEntry cpus[], int *count, char *global_model, int *global_phys, int *global_virt)
{
    FILE *fp = fopen("/proc/cpuinfo", "r");
    char line[MAX_LINE];

    CpuEntry current;
    int in_block = 0;
    int n = 0;

    global_model[0] = '\0';
    *global_phys = -1;
    *global_virt = -1;

    if (fp == NULL)
        return 0;

    memset(&current, 0, sizeof(current));
    current.phys_bits = -1;
    current.virt_bits = -1;
    current.physical_id = -1;
    current.core_id = -1;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        char value[MAX_TEXT];

        if (line[0] == '\n')
        {
            if (in_block)
            {
                if (n >= MAX_CPUS)
                {
                    fclose(fp);
                    return 0;
                }
                cpus[n++] = current;

                memset(&current, 0, sizeof(current));
                current.phys_bits = -1;
                current.virt_bits = -1;
                current.physical_id = -1;
                current.core_id = -1;
                in_block = 0;
            }
            continue;
        }

        in_block = 1;

        if (strncmp(line, "model name", 10) == 0)
        {
            if (get_value_after_colon(line, value, sizeof(value)))
            {
                strcpy(current.model, value);
                if (global_model[0] == '\0')
                    strcpy(global_model, value);
            }
        }
        else if (strncmp(line, "uarch", 5) == 0)
        {
            if (current.model[0] == '\0' && get_value_after_colon(line, value, sizeof(value)))
            {
                strcpy(current.model, value);
                if (global_model[0] == '\0')
                    strcpy(global_model, value);
            }
        }
        else if (strncmp(line, "isa", 3) == 0)
        {
            if (current.model[0] == '\0' && get_value_after_colon(line, value, sizeof(value)))
            {
                strcpy(current.model, value);
                if (global_model[0] == '\0')
                    strcpy(global_model, value);
            }
        }
        else if (strncmp(line, "address sizes", 13) == 0)
        {
            if (get_value_after_colon(line, value, sizeof(value)) &&
                parse_address_sizes_x86(value, &current.phys_bits, &current.virt_bits))
            {
                *global_phys = current.phys_bits;
                *global_virt = current.virt_bits;
            }
        }
        else if (strncmp(line, "mmu", 3) == 0)
        {
            if (get_value_after_colon(line, value, sizeof(value)) &&
                parse_mmu_riscv(value, &current.phys_bits, &current.virt_bits))
            {
                *global_phys = current.phys_bits;
                *global_virt = current.virt_bits;
            }
        }
        else if (strncmp(line, "physical id", 11) == 0)
        {
            if (get_value_after_colon(line, value, sizeof(value)))
            {
                current.physical_id = atoi(value);
                current.has_physical_id = 1;
            }
        }
        else if (strncmp(line, "core id", 7) == 0)
        {
            if (get_value_after_colon(line, value, sizeof(value)))
            {
                current.core_id = atoi(value);
                current.has_core_id = 1;
            }
        }
    }

    if (in_block)
    {
        if (n >= MAX_CPUS)
        {
            fclose(fp);
            return 0;
        }
        cpus[n++] = current;
    }

    fclose(fp);
    *count = n;
    return n > 0;
}

int main(void)
{
    char arch[MAX_TEXT];
    char model[MAX_TEXT];
    CpuEntry cpus[MAX_CPUS];
    int cpu_count;
    int global_phys;
    int global_virt;
    int threads;
    int cores;
    int i, j;

    if (!get_arch(arch, sizeof(arch)))
    {
        fprintf(stderr, "error: cannot get architecture\n");
        return 1;
    }

    if (!read_cpuinfo(cpus, &cpu_count, model, &global_phys, &global_virt))
    {
        fprintf(stderr, "error: cannot read /proc/cpuinfo\n");
        return 1;
    }

    if (model[0] == '\0')
    {
        fprintf(stderr, "error: cannot determine cpu model\n");
        return 1;
    }

    if (global_phys < 0 || global_virt < 0)
    {
        fprintf(stderr, "error: cannot determine address sizes\n");
        return 1;
    }

    threads = cpu_count;
    cores = 0;

    for (i = 0; i < cpu_count; ++i)
    {
        int unique = 1;

        if (cpus[i].has_physical_id && cpus[i].has_core_id)
        {
            for (j = 0; j < i; ++j)
            {
                if (cpu_equal_core(&cpus[i], &cpus[j]))
                {
                    unique = 0;
                    break;
                }
            }
            if (unique)
                ++cores;
        }
    }

    if (cores == 0)
        cores = threads;

    printf("%s\n", arch);
    printf("%s\n", model);
    printf("cores: %d, threads: %d\n", cores, threads);

    for (i = 0; i < cpu_count; ++i)
    {
        const char *name = cpus[i].model[0] ? cpus[i].model : model;
        int phys = cpus[i].phys_bits >= 0 ? cpus[i].phys_bits : global_phys;
        int virt = cpus[i].virt_bits >= 0 ? cpus[i].virt_bits : global_virt;

        printf("%s, %d, %d\n", name, phys, virt);
    }

    return 0;
}
