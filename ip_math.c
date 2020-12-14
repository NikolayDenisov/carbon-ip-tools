#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

void int2bin(int number, int *reverse)
{
    int count_bit = 0;
    int number_original = number;
    while (number)
    {
        count_bit++;
        number >>= 1;
    }
    int direct[count_bit];
    int j, i;
    for (i = 0; number_original > 0; i++)
    {
        direct[i] = number_original % 2;
        number_original = number_original / 2;
    }
    for (j = 0; j < count_bit; j++)
    {
        reverse[j] = direct[i - (j + 1)];
    }
}

/*--------------------------------------*/
/* Compute netmask address given prefix */
/*--------------------------------------*/
int prefix2netmask(int prefix)
{
    unsigned long mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
    printf("%lu.%lu.%lu.%lu\n", mask >> 24, (mask >> 16) & 0xFF, (mask >> 8) & 0xFF, mask & 0xFF);
    return EXIT_SUCCESS;
}

void summarize_address_range(unsigned long first, unsigned long last)
{
    /**
     * Example:
     * summarize_address_range("192.168.1.0", "192.168.1.1")
     * return "192.168.1.0/31"
    */
}

void ip_to_int(char *ip_address)
{
    /**
     * Convert IPv4 to int
     * Example:
     * IN: 192.168.1.1
     * OUT: 3232235777
     * TO-BIN: 11000000 10101000 00000001 00000001
     * */
    char *delim = strtok(ip_address, ".");
    int octet = 0;
    int prefix;
    char buff[32];
    while (delim != NULL)
    {
        printf("DELIM %s\n", delim);
        delim = atoi(delim);
        octet++;
        if (octet == 5)
        {
            prefix = atoi(delim);
            prefix2netmask(prefix);
            delim = strtok(NULL, "./");
            continue;
        }
        int *p;
        int max_octet_len = 8;
        int binary[max_octet_len];
        int val = atoi(delim);
        int2bin(192, &binary[0]);
        for (int i = 0; i < max_octet_len; i++)
        {
            printf("value of arr[%d] is %d\n", i, binary[i]);
        }
        delim = strtok(NULL, "./");
    }
}

int main()
{
    char *line = NULL;
    int len = 0;
    int lineSize = 0;
    while (lineSize = getline(&line, &len, stdin) != -1)
    {
        ip_to_int(line);
    }
    free(line);
    exit(EXIT_SUCCESS);
}