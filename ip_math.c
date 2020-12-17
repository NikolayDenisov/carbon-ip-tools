#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct network
{
    unsigned int first;
    int prefix;
};

// _private_networks = [
//     IPv4Network('0.0.0.0/8'),
//     IPv4Network('10.0.0.0/8'),
//     IPv4Network('127.0.0.0/8'),
//     IPv4Network('169.254.0.0/16'),
//     IPv4Network('172.16.0.0/12'),
//     IPv4Network('192.0.0.0/29'),
//     IPv4Network('192.0.0.170/31'),
//     IPv4Network('192.0.2.0/24'),
//     IPv4Network('192.168.0.0/16'),
//     IPv4Network('198.18.0.0/15'),
//     IPv4Network('198.51.100.0/24'),
//     IPv4Network('203.0.113.0/24'),
//     IPv4Network('240.0.0.0/4'),
//     IPv4Network('255.255.255.255/32'),
//     ]

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
    unsigned int i, cur;
    unsigned int tmp_net;
    int j = 0;
    i = 0;   /*pointer to last valid position*/
    cur = 1; /*pointer to next addr to analyze*/
    if ((subnets[cur].prefix <= 32) && ((subnets[cur].first & prefix2int(subnets[i].prefix)) != subnets[i].first))
    {
        tmp_net = prefix2int(subnets[i].prefix - 1);
        printf("tmp_net = %u\n", tmp_net);
    }
    if ((subnets[i].prefix == subnets[cur].prefix) && ((subnets[i].first & tmp_net) == (subnets[cur].first & tmp_net)))
    {
        if (i > 0)
        {
            subnets[cur].prefix = subnets[i].prefix - 1;
            subnets[cur].first &= tmp_net;
            i--;
        }
        else
        {
            subnets[i].prefix = subnets[i].prefix - 1;
            subnets[i].first &= tmp_net;
            cur++;
        }
    }
    else
    {
        i++;
        subnets[i].first = subnets[cur].first;
        subnets[i].prefix = subnets[cur].prefix;
        cur++;
    }
    return i;
}

// int ip2int2(const char* ip)
// {
//     char* s = strdup(ip);
//     char* t = strtok(s, ".");
//     int num = 0;
//     while (t) {
//         num = num*256 + atoi(t);
//         t = strtok(NULL, ".");
//     }
//     free(s);
//     return num;

void ip_to_int(char *cidr, unsigned int *first_cidr_int, int *prefixlen)
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
            *first_cidr_int = val << 24;
            break;
        case 2:
            *first_cidr_int |= val << 16;
            break;
        case 3:
            *first_cidr_int |= val << 8;
            break;
        case 4:
            *first_cidr_int |= val << 0;
            break;
        case 5:
            // val = netmask(/32)
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
static int STRCPY(char *dst, char *src)
{
    int i = 0;

    for (; src[i] != '\0'; i++)
        dst[i] = src[i];

    return i;
}
static char *dotted[] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
    "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
    "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
    "30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
    "40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
    "50", "51", "52", "53", "54", "55", "56", "57", "58", "59",
    "60", "61", "62", "63", "64", "65", "66", "67", "68", "69",
    "70", "71", "72", "73", "74", "75", "76", "77", "78", "79",
    "80", "81", "82", "83", "84", "85", "86", "87", "88", "89",
    "90", "91", "92", "93", "94", "95", "96", "97", "98", "99",
    "100", "101", "102", "103", "104", "105", "106", "107", "108", "109",
    "110", "111", "112", "113", "114", "115", "116", "117", "118", "119",
    "120", "121", "122", "123", "124", "125", "126", "127", "128", "129",
    "130", "131", "132", "133", "134", "135", "136", "137", "138", "139",
    "140", "141", "142", "143", "144", "145", "146", "147", "148", "149",
    "150", "151", "152", "153", "154", "155", "156", "157", "158", "159",
    "160", "161", "162", "163", "164", "165", "166", "167", "168", "169",
    "170", "171", "172", "173", "174", "175", "176", "177", "178", "179",
    "180", "181", "182", "183", "184", "185", "186", "187", "188", "189",
    "190", "191", "192", "193", "194", "195", "196", "197", "198", "199",
    "200", "201", "202", "203", "204", "205", "206", "207", "208", "209",
    "210", "211", "212", "213", "214", "215", "216", "217", "218", "219",
    "220", "221", "222", "223", "224", "225", "226", "227", "228", "229",
    "230", "231", "232", "233", "234", "235", "236", "237", "238", "239",
    "240", "241", "242", "243", "244", "245", "246", "247", "248", "249",
    "250", "251", "252", "253", "254", "255"};

void print_address(unsigned int subnet, int prefix)
{
    char buf[23 + 2];
    int pos = 0, len;
    // pos += STRCPY(buf + pos, dotted[net >> 24]);
    // buf[pos++] = '.';
    // pos += STRCPY(buf + pos, dotted[(net & 0xff000000) >> 16]);
    // buf[pos++] = '.';
    // pos += STRCPY(buf + pos, dotted[(net & 0xff00) >> 8]);
    // buf[pos++] = '.';
    // pos += STRCPY(buf + pos, dotted[net & 0xff]);
    // buf[pos++] = '/';
    // pos += STRCPY(buf + pos, dotted[pref]);
    // buf[pos++] = '\n';

    // printf("value of a_static: %s\n", buf);
}

char *my_itoa(unsigned int num, char *str)
{
    if (str == NULL)
    {
        return NULL;
    }
    sprintf(str, "%d", num);
    return str;
}

void print_addresses(struct network *subnets, unsigned int size)
{
    int i = 0, pos = 0, len;
    char buf[8192 * 8];
    char prefix[2];
    while (i < size)
    {
        printf("%u/%d\n", subnets[i].first, subnets[i].prefix);
        print_address(subnets[i].first, subnets[i].prefix);
        i++;
    }
}

int main()
{
    unsigned int first = 0;
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
        ip_to_int(line, &first, &prefix);
        subnets[index].first = first;
        subnets[index].prefix = prefix;
        index++;
    }
    fclose(fp);
    if (line)
        free(line);
    printf("index = %u\n", index);
    summarize_address_range(subnets, index);
    printf("SIZE = %u\n", size);
    print_addresses(subnets, index);
    exit(EXIT_SUCCESS);
}