#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct network
{
    unsigned int first;
    unsigned int last;
    int prefix;
};

/*--------------------------------------*/
/* Compute netmask address given prefix */
/*--------------------------------------*/
int prefix2netmask(int prefix)
{
    unsigned long mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
    printf("%lu.%lu.%lu.%lu\n", mask >> 24, (mask >> 16) & 0xFF, (mask >> 8) & 0xFF, mask & 0xFF);
    return EXIT_SUCCESS;
}

unsigned int prefix2int(int prefix)
{
    unsigned int mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
    return mask;
}

unsigned int summarize_address_range(struct network *subnets, unsigned int len)
{
    unsigned int a, b;
    a = 0;
    b = 1;
    while (b < len)
    {
        printf("base.first = %u base.last = %u cur.first = %u cur.last = %u\n", subnets[a].first, subnets[a].last, subnets[b].first, subnets[b].last);
        if (subnets[a].first == subnets[b].first && subnets[a].last == subnets[b].last)
        {
            printf("IP уже есть в списке\n");
            b++;
            a++;
            continue;
        }
        if (subnets[a].first >= subnets[b].first && subnets[a].last <= subnets[b].last)
        {
            printf("a внутри b\n");
            subnets[a].first = subnets[b].first;
            subnets[a].last = subnets[b].last;
        }
        if (subnets[b].first >= subnets[a].first && subnets[b].last <= subnets[a].last)
        {
            printf("# b внутри a\n");
        }
        if (subnets[a].first < subnets[b].first && subnets[a].last < subnets[b].last && subnets[b].first <= (subnets[a].last + 1))
        {
            printf("bbbb  <- a/b overlap or immediately adjacent\n");
            subnets[a].first = subnets[a].first;
            subnets[a].last = subnets[b].last;
        }
        if (subnets[b].first < subnets[a].first && subnets[b].last < subnets[a].last && subnets[a].first <= (subnets[b].last + 1))
        {
            printf("bbbb    <- b/a overlap or immediately adjacent\n");
            subnets[a].first = subnets[b].first;
            subnets[a].last = subnets[a].last;
        }
        b++;
        a++;
    }
    return EXIT_SUCCESS;
}

void ip_to_int(char *cidr, unsigned int *first, unsigned int *last, int *prefixlen)
{
    /**
    *  IPv4-подсеть можно разбить на диапазон целых чисел.
    * Например, 192.168.1.0/31 это диапазон от 3232235776 до 3232235777
    * Так как 31 подсеть имеет 2 адреса 192.168.1.0 и 192.168.1.1
    * first_cidr_int = 3232235776
    * last_cidr_int = 3232235777
    * prefixlen = 31
     * */
    char *delim = strtok(cidr, "./");
    int octet = 0;
    int val;
    while (delim != NULL)
    {
        octet++;
        val = atoi(delim);
        switch (octet)
        {
        case 1:
            *first = val << 24;
            break;
        case 2:
            *first |= val << 16;
            break;
        case 3:
            *first |= val << 8;
            break;
        case 4:
            *first |= val << 0;
            break;
        case 5:
            *last = *first + pow(2, (32 - val)) - 1;
            *prefixlen = val;
            break;

        default:
            printf("Ошибка, указан неверный октет");
        }
        delim = strtok(NULL, "./");
    }
}

unsigned char *int_to_ip(unsigned int ip)
{
    char *ip_cidr = NULL;
    unsigned int a, b, c, d;
    a = ip;                              // 2**24
    b = (ip - a * 16777216);             // 2**16
    c = (ip - a * 16777216 - b * 65536); // 2**8
    d = (ip - a * 16777216 - b * 65536 - c * 256);
    printf("%u.%u.%u.%u", a, b, c, d);
    return ip_cidr;
}

int main()
{
    unsigned int first, last = 0;
    int prefix = 0;
    unsigned int size = 0;
    struct network *subnets;
    unsigned int index = 0;

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    unsigned long read;

    fp = fopen("/home/nick/projects/carbon-ip-tools/data.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        ip_to_int(line, &first, &last, &prefix);
        subnets[index].first = first;
        subnets[index].last = last;
        subnets[index].prefix = prefix;
        index++;
        // printf("first = %u last = %u prefix = %d\n", first, last, prefix);
    }
    fclose(fp);
    if (line)
        free(line);
    unsigned int a, b;
    a = 0;
    b = 1;
    unsigned int reverse;
    reverse = 0;
    while (b < index + 1)
    {
        printf("base.first = %u base.last = %u\n", subnets[a].first, subnets[a].last);
        if (subnets[a].first < subnets[b].first && subnets[a].last < subnets[b].last && subnets[b].first <= (subnets[a].last + 1))
        {
            subnets[a].last = subnets[b].last;
            subnets[b] = subnets[b + 1];
            b++;
            continue;
        }
        a++;
        subnets[a] = subnets[b];
        b++;
    }
    while (a > reverse)
    {
        reverse++;
    }
    exit(EXIT_SUCCESS);
}
