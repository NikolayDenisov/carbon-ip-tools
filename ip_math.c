#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXINT 0xffffffff
#define TONETMASK(PREFIX) (((PREFIX) == 0) ? (0) : (MAXINT << (32 - (PREFIX))))

#define STREAM FILE *
#define STDIN (stdin)
#define STDOUT (stdout)
#define STDERR (stderr)
#define MAXLINE 1023
#define BUFFER 8192 * 8

#define READ(f, b, n) fread(b, 1, n, f)
#define WRITE(f, b, n) fwrite(b, 1, n, f)
#define OPEN(name) fopen(name, "r")
#define CLOSE(f) fclose(f)
#define N_EOF(f, ret) feof(f)

/* Allocate BUCKET array entries per time */
#define BUCKET 100000

struct network {
    unsigned int network;
    unsigned int last;
    int prefix;
};

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
    printf("%s/%d\n", int_to_ip(network), prefix);
}

static int STRCPY(char *dst, char *src) {
    int i = 0;

    for (; src[i] != '\0'; i++) dst[i] = src[i];

    return i;
}

static int parse_line(char *line, struct network *res) {
    unsigned int first, last = 0;
    int prefix = 0;
    ip_to_int(line, &first, &last, &prefix);
    res->network = first;
    res->prefix = prefix;
    return 1;
}

int get_entries(STREAM f, struct network **subnets, unsigned int *size) {
    unsigned int i;
    int start = 0, end = 0, n, stop, quit;
    char buf[BUFFER];
    char line[MAXLINE + 1];
    char error[MAXLINE + 50];
    i = *size = 0;
    quit = 0;
    while (!quit) {
        /*buffered read */
        stop = 0;
        n = 0;
        /*Read one line*/
        while (!stop) {
            while ((end > start) && (buf[start] != '\n') &&
                   (buf[start] != '\r')) {
                line[n] = buf[start];
                n++;
                if (n != MAXLINE) {
                    start++;
                }
            }
            if (end > start) {
                line[n] = '\0';
                start++;
                /* DOS line separator */
                if ((end > start) && (buf[start] == '\n')) start++;
                stop = 1;
            } else {
                start = 0;
                end = READ(f, buf, BUFFER);
                if (end <= 0) {
                    quit = 1;
                    line[n] = '\0';
                    stop = 1;
                }
            }
        }
        /* end buffered read */
        if (*size <= i) {
            *size += BUCKET;
            *subnets = (struct network *)realloc(
                *subnets, sizeof(struct network) * (*size));
        }
        parse_line(line, &((*subnets)[i]));
        i++;
    }
    return i;
}

int main() {
    unsigned int len1 = 0;
    unsigned int size1 = 0;

    struct network *subnets = NULL;
    len1 = get_entries(STDIN, &subnets, &size1);
    unsigned int a, subnet_cnt_f;
    a = 0;
    subnet_cnt_f = optimize(subnets, len1 - 1);
    while (a < subnet_cnt_f) {
        print_cidr(subnets[a].network, subnets[a].prefix);
        a++;
    }
    free(subnets);
    exit(EXIT_SUCCESS);
}
