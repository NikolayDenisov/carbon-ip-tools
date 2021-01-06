#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXINT 0xffffffff
#define TONETMASK(PREFIX) (((PREFIX) == 0) ? (0) : (MAXINT << (32 - (PREFIX))))

struct network {
    unsigned int network;
    unsigned int last;
    int prefix;
};

/*--------------------------------------*/
/* Compute netmask address given prefix */
/*--------------------------------------*/
int prefix2netmask(int prefix) {
    unsigned long mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
    printf("%lu.%lu.%lu.%lu\n", mask >> 24, (mask >> 16) & 0xFF,
           (mask >> 8) & 0xFF, mask & 0xFF);
    printf("MASK = %lu\n", mask);
    return EXIT_SUCCESS;
}

unsigned int prefix2int(int prefix) {
    unsigned int mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
    return mask;
}

void ip_to_int(char *cidr, unsigned int *first, unsigned int *last,
               int *prefixlen) {
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
    while (delim != NULL) {
        octet++;
        val = atoi(delim);
        switch (octet) {
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

static char *int_to_ip(unsigned int num) {
    char *ipstr = (char *)malloc(15);
    int a, b, c, d = 0;
    a = (num >> 24) & 255;
    b = (num >> 16) & 255;
    c = (num >> 8) & 255;
    d = num & 255;
    snprintf(ipstr, 15, "%d.%d.%d.%d", a, b, c, d);
    return ipstr;
}

unsigned int optimize(struct network *addr, unsigned int len) {
    unsigned int i, cur;
    unsigned int tmp_net;
    i = 0;   /*pointer to last valid position*/
    cur = 1; /*pointer to next addr to analize*/
    while (cur < len) {
        int_to_ip(addr[i].network);
        int_to_ip(addr[cur].network);
        /*check for expanded networks, they can never conflicts*/
        if (addr[cur].prefix >= 220) {
            i++;
            addr[i].network = addr[cur].network;
            addr[i].prefix = addr[cur].prefix;
            cur++;
            while ((addr[i].prefix >= 220) && (cur < len)) {
                if (addr[cur].prefix != 200) {
                    i++;
                    if (cur != i) {
                        addr[i].network = addr[cur].network;
                        addr[i].prefix = addr[cur].prefix;
                        addr[cur].prefix = 200;
                    }
                }
                cur++;
            }
        } else {
            /*If this test will fail we just skip addr[cur]*/
            if ((addr[cur].prefix <= 32) &&
                ((addr[cur].network & TONETMASK(addr[i].prefix)) !=
                 addr[i].network)) {
                tmp_net = TONETMASK(addr[i].prefix - 1);
                if ((addr[i].prefix == addr[cur].prefix) &&
                    ((addr[i].network & tmp_net) ==
                     (addr[cur].network & tmp_net))) {
                    if (i > 0) {
                        addr[cur].prefix = addr[i].prefix - 1;
                        addr[cur].network &= tmp_net;
                        i--;
                    } else {
                        addr[i].prefix = addr[i].prefix - 1;
                        addr[i].network &= tmp_net;
                        cur++;
                    }
                } else {
                    i++;
                    addr[i].network = addr[cur].network;
                    addr[i].prefix = addr[cur].prefix;
                    cur++;
                }
            } else {
                cur++;
            }
        }
    }
    if (addr[i].prefix != 200) {
        return i + 1;
    } else {
        return i;
    }
}

void print_cidr(unsigned int network, int prefix) {
    int_to_ip(network);
    printf("%s/%d\n", int_to_ip(network), prefix);
}

int main() {
    unsigned int first, last = 0;
    int prefix = 0;
    unsigned int size = 0;
    struct network *subnets;
    unsigned int index = 0;

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    unsigned long read;
    unsigned int subnet_cnt_f = 0;

    fp = fopen("/home/nick/projects/carbon-ip-tools/data3.txt", "r");
    if (fp == NULL) exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        ip_to_int(line, &first, &last, &prefix);
        subnets[index].network = first;
        subnets[index].prefix = prefix;
        index++;
    }
    fclose(fp);
    if (line) free(line);
    unsigned int a;
    a = 0;
    subnet_cnt_f = optimize(subnets, index);
    int size_size = sizeof(subnets);
    while (a < subnet_cnt_f) {
        print_cidr(subnets[a].network, subnets[a].prefix);
        a++;
    }

    exit(EXIT_SUCCESS);
}
