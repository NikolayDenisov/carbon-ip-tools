#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TONETMASK(PREFIX) (((PREFIX) == 0) ? (0) : (MAXINT << (32 - (PREFIX))))

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
    printf("MASK = %lu\n", mask);
    return EXIT_SUCCESS;
}

unsigned int prefix2int(int prefix)
{
    unsigned int mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
    return mask;
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

void int_to_ip(unsigned int num)
{
    char *ipstr = (char *)malloc(15);
    int a, b, c, d = 0;
    a = (num >> 24) & 255;
    b = (num >> 16) & 255;
    c = (num >> 8) & 255;
    d = num & 255;
    snprintf(ipstr, 15, "%d.%d.%d.%d", a, b, c, d);
    printf("%s\n", ipstr);
}

unsigned int optimize(struct network *addr, unsigned int len, int do_sort)
{
    unsigned int i, cur;
    unsigned int tmp_net;
    i = 0;   /*pointer to last valid position*/
    cur = 1; /*pointer to next addr to analize*/
    if (len <= 1)
    {
        /* empty or sigle element array is optimized by definition.*/
        return len;
    }

    while (cur < len)
    {
        /*check for expanded networks, they can never conflicts*/
        if (addr[cur].prefix >= 220)
        {
            i++;
            addr[i].first = addr[cur].first;
            addr[i].prefix = addr[cur].prefix;
            cur++;
            while ((addr[i].prefix >= 220) && (cur < len))
            {
                if (addr[cur].prefix != 200)
                {
                    i++;
                    if (cur != i)
                    {
                        addr[i].first = addr[cur].first;
                        addr[i].prefix = addr[cur].prefix;
                        addr[cur].prefix = 200;
                    }
                }
                cur++;
            }
        }
        else
        {
            /*If this test will fail we just skip addr[cur]*/
            if ((addr[cur].prefix <= 32) && ((addr[cur].first & TONETMASK(addr[i].prefix)) != addr[i].first))
            {
                tmp_net = TONETMASK(addr[i].prefix - 1);

                if ((addr[i].prefix == addr[cur].prefix) && ((addr[i].first & tmp_net) == (addr[cur].first & tmp_net)))
                {

                    if (i > 0)
                    {
                        addr[cur].prefix = addr[i].prefix - 1;
                        addr[cur].first &= tmp_net;
                        i--;
                    }
                    else
                    {
                        addr[i].prefix = addr[i].prefix - 1;
                        addr[i].first &= tmp_net;
                        cur++;
                    }
                }
                else
                {
                    i++;

                    addr[i].first = addr[cur].first;
                    addr[i].prefix = addr[cur].prefix;

                    cur++;
                }
            }
            else
            {
                cur++;
            }
        }
    }

    if (addr[i].prefix != 200)
    {
        return i + 1;
    }
    else
    {
        return i;
    }
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
    unsigned int range = 0;
    while (a > reverse)
    {
        range = (subnets[reverse].last - subnets[reverse].first) + 1;
        if (range == 1)
        {
            int_to_ip(subnets[reverse].first);
        }
        reverse++;
    }
    exit(EXIT_SUCCESS);
}
