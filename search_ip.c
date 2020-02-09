#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define true 1
#define false 0
#define MASK_MAX_LEN 32

void str_to_netaddr( char *ipstr ) {

	long int prefix = 32;
	char *prefixstr;
	printf( "IN %s\n", ipstr );

	if ( (prefixstr = strchr(ipstr, '/')) ) {
		*prefixstr = '\0';
		prefixstr++;
		prefix = strtol( prefixstr, (char **) NULL, 10 );
	}
	printf( "PREFIX %ld\n", prefix );
	uint32_t mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
	printf("%u.%u.%u.%u\n", mask >> 24, (mask >> 16) & 0xFF, (mask >> 8) & 0xFF, mask & 0xFF);


}

int IPToUInt(char* ip) 
{
	int a, b, c, d;
	int addr = 0;
	if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) != 4)
		return 0;
	addr = a << 24;
	addr |= b << 16;
	addr |= c << 8;
	addr |= d;
	return addr;
}

int netmask_from_prefix(int prefix) {
	uint32_t mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
	printf("%u.%u.%u.%u\n", mask >> 24, (mask >> 16) & 0xFF, (mask >> 8) & 0xFF, mask & 0xFF);
	return 0;
}

int IsIPInRange(char* ip, char* network, char* mask)
{
	int ip_addr = IPToUInt(ip);
	int network_addr = IPToUInt(network);
	int mask_addr = IPToUInt(mask);

	int net_lower = (network_addr & mask_addr);
	int net_upper = (net_lower | (~mask_addr));

	if (ip_addr >= net_lower &&
			ip_addr <= net_upper)
		return true;
	return false;
}

uint32_t getNetworkMask(uint8_t mask) {
	uint32_t networkMask = 0x00000000;
	for (uint8_t bitPos = MASK_MAX_LEN - 1; mask != 0; --mask, --bitPos) {
		networkMask |= (1 << bitPos);
	}
	return networkMask;
}


int main(int argc, char **argv) {
	char* ip = argv[1];
	FILE * fp;
	char * ipstr = NULL;
	size_t len = 0;
	ssize_t read;

	char* find_ip = argv[1];

	fp = fopen("data.txt", "r");
	if (fp == NULL)
		exit(EXIT_FAILURE);
	long int prefix = 32;
	char *prefixstr;
	while ((read = getline(&ipstr, &len, fp)) != -1) {
        if ( (prefixstr = strchr(ipstr, '/')) ) {
            *prefixstr = '\0';
            prefixstr++;
            prefix = strtol( prefixstr, (char **) NULL, 10 );
        }
        uint32_t mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
        char *subnet = (char*)malloc(16 * sizeof(char));
        sprintf(subnet, "%u.%u.%u.%u", mask >> 24, (mask >> 16) & 0xFF, (mask >> 8) & 0xFF, mask & 0xFF);
		if (IsIPInRange(ip, ipstr, subnet)){
		    printf("IP адрес: %s найден в списке %s/%ld\n", find_ip,  ipstr, prefix);
		}
	}
	fclose(fp);
	if (ipstr)
		free(ipstr);
	exit(EXIT_SUCCESS);

	return 0;
}
