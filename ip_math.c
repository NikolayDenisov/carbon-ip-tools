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

#define MASK1 0xff000000
#define MASK2 0xff0000
#define MASK3 0xff00
#define MASK4 0xff

static char *dotted[] = {
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",   "8",   "9",   "10",
    "11",  "12",  "13",  "14",  "15",  "16",  "17",  "18",  "19",  "20",  "21",
    "22",  "23",  "24",  "25",  "26",  "27",  "28",  "29",  "30",  "31",  "32",
    "33",  "34",  "35",  "36",  "37",  "38",  "39",  "40",  "41",  "42",  "43",
    "44",  "45",  "46",  "47",  "48",  "49",  "50",  "51",  "52",  "53",  "54",
    "55",  "56",  "57",  "58",  "59",  "60",  "61",  "62",  "63",  "64",  "65",
    "66",  "67",  "68",  "69",  "70",  "71",  "72",  "73",  "74",  "75",  "76",
    "77",  "78",  "79",  "80",  "81",  "82",  "83",  "84",  "85",  "86",  "87",
    "88",  "89",  "90",  "91",  "92",  "93",  "94",  "95",  "96",  "97",  "98",
    "99",  "100", "101", "102", "103", "104", "105", "106", "107", "108", "109",
    "110", "111", "112", "113", "114", "115", "116", "117", "118", "119", "120",
    "121", "122", "123", "124", "125", "126", "127", "128", "129", "130", "131",
    "132", "133", "134", "135", "136", "137", "138", "139", "140", "141", "142",
    "143", "144", "145", "146", "147", "148", "149", "150", "151", "152", "153",
    "154", "155", "156", "157", "158", "159", "160", "161", "162", "163", "164",
    "165", "166", "167", "168", "169", "170", "171", "172", "173", "174", "175",
    "176", "177", "178", "179", "180", "181", "182", "183", "184", "185", "186",
    "187", "188", "189", "190", "191", "192", "193", "194", "195", "196", "197",
    "198", "199", "200", "201", "202", "203", "204", "205", "206", "207", "208",
    "209", "210", "211", "212", "213", "214", "215", "216", "217", "218", "219",
    "220", "221", "222", "223", "224", "225", "226", "227", "228", "229", "230",
    "231", "232", "233", "234", "235", "236", "237", "238", "239", "240", "241",
    "242", "243", "244", "245", "246", "247", "248", "249", "250", "251", "252",
    "253", "254", "255"};

#define INT2IP(x) \
    ((x) >> 24) & 0xFF, ((x) >> 16) & 0xFF, ((x) >> 8) & 0xFF, ((x) >> 0) & 0xFF

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

#define MAXCIDR 23

/*Special prefix values.
Important: INVALID_PREFIX must be < EXPANDED_PREFIX.
EXPANDED_PREFIX + 32 must be < 255
*/
#define INVALID_PREFIX 200
#define EXPANDED_PREFIX 220

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

unsigned int print_address2(unsigned int net, unsigned char pref, char *buf,
                            unsigned int pos) {
    if (pref != INVALID_PREFIX) {
        pos += STRCPY(buf + pos, dotted[net >> 24]);
        buf[pos++] = '.';
        pos += STRCPY(buf + pos, dotted[(net & MASK2) >> 16]);
        buf[pos++] = '.';
        pos += STRCPY(buf + pos, dotted[(net & MASK3) >> 8]);
        buf[pos++] = '.';
        pos += STRCPY(buf + pos, dotted[net & MASK4]);
        buf[pos++] = '/';
        pos += STRCPY(buf + pos, dotted[pref]);
        buf[pos++] = '\n';
    }
    return pos;
}
void print_addresses(STREAM f, struct network *addr, int size,
                     struct network *expanded, int level) {
    int i = 0, pos = 0, len;
    char buf[BUFFER];

    if (level > 256) {
        /*Sanity ckeck */
        fprintf(
            stderr,
            "Array too nested: internal error, aborting. Please report this "
            "issue together with input files to depetrini@libero.it\n");
        exit(1);
    }

    while (i < size) {
        if (addr[i].prefix < EXPANDED_PREFIX) {
            pos = print_address2(addr[i].network, addr[i].prefix, buf, pos);
            if (pos > BUFFER - MAXCIDR - 2) {
                len = WRITE(f, buf, pos);
                if (len < pos) {
                    fprintf(stderr,
                            "Written only %d bytes instead of %d, aborting\n",
                            len, pos);
                    exit(1);
                }
                pos = 0;
            }
        } else {
            if (expanded == NULL) {
                /*Sanity ckeck */
                fprintf(
                    stderr,
                    "Expanded is NULL. Please report this issue together with "
                    "input files and paramethers to depetrini@libero.it\n");
                exit(1);
            }
            len = WRITE(f, buf, pos);
            if (len < pos) {
                fprintf(stderr,
                        "Written only %d bytes instead of %d, aborting\n", len,
                        pos);
                exit(1);
            }

            pos = 0;
            print_addresses(f, &(expanded[addr[i].network]),
                            addr[i].prefix - EXPANDED_PREFIX, expanded,
                            level + 1);
        }

        i++;
    }

    /* flush the buffer */
    if (pos > 0) {
        len = WRITE(f, buf, pos);
        if (len < pos) {
            fprintf(stderr, "Written only %d bytes instead of %d, aborting\n",
                    len, pos);
            exit(1);
        }
    }
}

int main() {
    unsigned int len1 = 0;
    unsigned int size1 = 0;

    struct network *subnets = NULL;
    len1 = get_entries(STDIN, &subnets, &size1);
    unsigned int a, subnet_cnt_f;
    a = 0;
    subnet_cnt_f = optimize(subnets, len1);
    print_addresses(STDOUT, subnets, subnet_cnt_f, NULL, 0);
    free(subnets);
    exit(EXIT_SUCCESS);
}
