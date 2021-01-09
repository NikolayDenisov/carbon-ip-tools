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

void ip_to_int(char *cidr, unsigned int *network, int *prefix) {
    /**
     *  IPv4-подсеть можно разбить на диапазон целых чисел.
     * Например, 192.168.1.0/31 это диапазон от 3232235776 до 3232235777
     * Так как 31 подсеть имеет 2 адреса 192.168.1.0 и 192.168.1.1
     * first_cidr_int = 3232235776
     * last_cidr_int = 3232235777
     * prefixlen = 31
     * */
    int i, n = 0;
    int j = 0;
    char a[4] = "\0";
    char b[4] = "\0";
    char c[4] = "\0";
    char d[4] = "\0";
    char netmask[3];
    for (i = 0; cidr[i] != '\0'; i++) {
        if (cidr[i] == 46) {
            n++;
            j = 0;
            continue;
        } else if (cidr[i] == 47) {
            n = 4;
            j = 0;
            continue;
        }
        if (n == 0) {
            a[j] = cidr[i];
            j++;
        } else if (n == 1) {
            b[j] = cidr[i];
            j++;
        } else if (n == 2) {
            c[j] = cidr[i];
            j++;
        } else if (n == 3) {
            d[j] = cidr[i];
            j++;
        } else if (n == 4) {
            netmask[j] = cidr[i];
            j++;
        }
    }
    printf("L = %s a = %s, b = %s, c = %s, d = %s\n", cidr, a, b, c, d);
    printf("ATOI L = %s a = %d, b = %d, c = %d, d = %d\n", cidr, atoi(a), atoi(b), atoi(c), atoi(d));
    *network = ((atoi(a) << 24) + (atoi(b) << 16) + (atoi(c) << 8) + atoi(d));
    printf("NETWORK = %u")
    *prefix = atoi(netmask);
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
    unsigned int net = 0;
    if (*line == '\0') {
        return 0;
    }
    ip_to_int(line, &net, &prefix);
    res->network = net;
    res->prefix = prefix;
    return 1;
}

int get_entries(STREAM f, struct network **addr, unsigned int *size) {
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
                } else {
                    fprintf(stderr,
                            "Error, line too long, taking first %d chars\n",
                            MAXLINE);
                    buf[start] = '\n';
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
                    if (!N_EOF(f, end)) {
                        fprintf(stderr, "Error reading file, aborting\n");
                        exit(1);
                    }
                    quit = 1;
                    line[n] = '\0';
                    stop = 1;
                }
            }
        }
        /* end buffered read */

        if (*size <= i) {
            *size += BUCKET;
            *addr = (struct network *)realloc(*addr,
                                              sizeof(struct network) * (*size));
            if (addr == NULL) {
                fprintf(stderr, "Error allocating %lu bytes\n",
                        (unsigned long)sizeof(struct network) * (*size));
                exit(1);
            }
        }

        if (!parse_line(line, &((*addr)[i]))) {
            int line_len;
            n = STRCPY(error, "Invalid line ");
            line_len = STRCPY(error + n, line);
            n += line_len;
            n += STRCPY(error + n, "\n");
            if (line_len > 0) {
                WRITE(STDERR, error, n);
            }
        } else {
            i++;
        }
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
    subnet_cnt_f = optimize(subnets, len1);
    while (a < subnet_cnt_f) {
        print_cidr(subnets[a].network, subnets[a].prefix);
        a++;
    }
    free(subnets);
    exit(EXIT_SUCCESS);
}
