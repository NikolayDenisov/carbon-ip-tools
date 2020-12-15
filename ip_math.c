#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

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

// void summarize_address_range(unsigned long first, unsigned long last)
// {
//     /**
//      * Example:
//      * summarize_address_range("192.168.1.0", "192.168.1.1")
//      * return "192.168.1.0/31"
//     */
// }

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
// }

void ip_to_int(char *cidr, unsigned int *start_cidr_range, unsigned int *end_cidr_range)
{
    /**
     * Convert IPv4 to int
     * Example:
     * IN: 192.168.1.1
     * OUT: 3232235777
     * TO-BIN: 11000000 10101000 00000001 00000001
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
            *start_cidr_range = val << 24;
            break;
        case 2:
            *start_cidr_range |= val << 16;
            break;
        case 3:
            *start_cidr_range |= val << 8;
            break;
        case 4:
            *start_cidr_range |= val << 0;
            break;
        case 5:
            // val = netmask(/32)
            *end_cidr_range = *start_cidr_range + pow(2, (32 - val)) - 1;
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
    char *line = NULL;
    int len = 0;
    int lineSize = 0;
    unsigned int start_cidr_range = 0;
    unsigned int stop_cidr_range = 0;
    while (lineSize = getline(&line, &len, stdin) != -1)
    {
        ip_to_int(line, &start_cidr_range, &stop_cidr_range);
        printf("Array %u/%u\n", start_cidr_range, stop_cidr_range);
    }
    free(line);
    exit(EXIT_SUCCESS);
}