#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MAXINT 0xffffffff
static unsigned int prefix_table[] = {
    0x0,        0x80000000, 0xc0000000, 0xe0000000, 0xf0000000, 0xf8000000,
    0xfc000000, 0xfe000000, 0xff000000, 0xff800000, 0xffc00000, 0xffe00000,
    0xfff00000, 0xfff80000, 0xfffc0000, 0xfffe0000, 0xffff0000, 0xffff8000,
    0xffffc000, 0xffffe000, 0xfffff000, 0xfffff800, 0xfffffc00, 0xfffffe00,
    0xffffff00, 0xffffff80, 0xffffffc0, 0xffffffe0, 0xfffffff0, 0xfffffff8,
    0xfffffffc, 0xfffffffe, 0xffffffff};

#define TONETMASK(PREFIX) prefix_table[PREFIX]

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
#ifdef CHAR_PREFIX
    unsigned char prefix;
#else
    unsigned int prefix;
#endif
};

void ip2int(const char *s, unsigned int *n) {
    unsigned int i, j, t;
    t = 0;
    for (i = 0, j = 0;; i++) {
        if (s[i] == '.') {
            j = (j << 8) + t;
            t = 0;
        } else if (s[i] == '\0') {
            j = (j << 8) + t;
            t = 0;
            break;  //终止条件
        } else {
            t = t * 10 + (s[i] - '0');
        }
    }
    *n = j;
    printf("J: %d ", j);
    return;
}

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
    *network = ((atoi(a) << 24) + (atoi(b) << 16) + (atoi(c) << 8) +
    atoi(d));
    *prefix = atoi(netmask);
}

static char *int_to_ip(unsigned int num) {
    char *ipstr = (char *)malloc(16);
    int a, b, c, d = 0;
    a = (num >> 24) & 255;
    b = (num >> 16) & 255;
    c = (num >> 8) & 255;
    d = num & 255;
    snprintf(ipstr, 16, "%d.%d.%d.%d", a, b, c, d);
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

static unsigned char collect_octet(char *line, int current_position,
                                   int *end_position) {
    unsigned int number = 0;

    *end_position = current_position;
    while (line[current_position] >= '0' && line[current_position] <= '9') {
        number *= 10;
        number += line[current_position] - 48;
        current_position++;
    }

    /*update return pointer only if it's a valid octect*/
    if (number < 256) {
        *end_position = current_position;
    }

    return number;
}

static int parse_line(char *line, struct network *res) {
    int i, n;
    int end = -1, stop = 0;

    i = 0;
    n = 24;
    res->network = 0;
    res->prefix = 220;

    while ((n >= 0) && !stop) {
        res->network |= collect_octet(line, i, &end) << n;

        if ((end == i) && (end > 0)) {
            if (line[end - 1] == '.') {
                n += 8;
            } else {
                break;
            }
        }
        switch (line[end]) {
            case '.':
                if (n > 0) {
                    i = end + 1;
                } else {
                    stop = 1;
                }
                break;
            case '/':
                if (n == 0) {
                    i = end + 1;
                    res->prefix = collect_octet(line, i, &end);
                    if ((line[end] != '\0') && (line[end] != ' ')) {
                        res->prefix = 220;
                    }
                }

                stop = 1;
                break;
            case ' ':
                /*no break here*/
                fprintf(stderr,
                        "WARNING: not considering characters after space in "
                        "line %s",
                        line);
            case '*':
                if ((line[end] == '*') && (line[end + 1] != '\0')) {
                    res->prefix = 220;
                    n = 32;
                }
            case '\0':
                /*				line[end]='\n';
                                                line[end+1]='\0';
                                        case '\n':
                */
                if (n < 24) {
                    res->prefix = 32 - n;
                }
                stop = 1;
                break;
            default:
                stop = 1;
                break;
        }
        n -= 8;
    }
    if (res->prefix > 32) {
        return 0;
    }

    /* final sanity check, very important: library functions will not work if
     * this costraint is not enforced */
    if ((res->network & TONETMASK(res->prefix)) != res->network) {
        return 0;
    }

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
