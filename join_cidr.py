#!/usr/bin/python3

import math


# function dot2dec(ip) {
#   var octet = ip.split(".");
#   return (octet[0] << 24) | (octet[1] << 16) | (octet[2] << 8) | octet[3];
# }
# function num2dot(num) {
#   var d = num%256;
#   for (var i = 3; i > 0; i--) {
#   num = Math.floor(num/256);
#   d = num%256 + '.' + d;}
#   return d;
# }
#
# function dec2bin(ip) {
#     return ip.split(".").map(function(num) { return ("00000000"+parseInt(num,10).toString(2)).slice(-8); }).join(".");
# }
# $(function() {
#   var res1 = $("#res1");
#   var adr1 = "111.163.224.0";
#   var adr2 = "111.163.239.255"
#   res1.append("<hr>"+dot2dec(adr1)+" - "+adr1);
#   res1.append("<hr>"+dot2dec(adr2)+" - "+adr2);
#   var XOR=num2dot(dot2dec(adr1)^dot2dec(adr2)),
#       AND=num2dot((dot2dec(adr1)&dot2dec(adr2))),
#      MASK=dec2bin(XOR).replace(/\./g,"").indexOf("1");
#   res1.append("<hr>XOR:"+XOR);
#   res1.append("<hr>AND:"+AND);
#   res1.append("<hr>MASK:"+MASK+"<hr>");
#   });


def dot2dec(ip):
    octet = ip.split('.')
    return (int(octet[0]) << 24) | (int(octet[1]) << 16) | (int(octet[2]) << 8) | int(octet[3])


def ip2bin(ip):
    b = ""
    inQuads = ip.split(".")
    outQuads = 4
    for q in inQuads:
        if q != "":
            b += dec2bin(int(q), 8)
            outQuads -= 1
    while outQuads > 0:
        b += "00000000"
        outQuads -= 1
    return b


def dec2bin(n, d=None):
    s = ""
    while n > 0:
        if n & 1:
            s = "1" + s
        else:
            s = "0" + s
        n >>= 1
    if d is not None:
        while len(s) < d:
            s = "0" + s
    if s == "": s = "0"
    return s


def num2dot(num):
    d = num % 256
    for i in range(0, 3):
        num = math.floor(num / 256)
        d = str(num % 256) + '.' + str(d)
    return d


def addressInNetwork(ip, net):
    if not '/' in net:
        net = net + '/32'
    ipaddr = int(''.join(['%02x' % int(x) for x in ip.split('.')]), 16)
    print("ipaddr", ipaddr)
    netstr, bits = net.split('/')
    print("netstr", netstr)
    netaddr = int(''.join(['%02x' % int(x) for x in netstr.split('.')]), 16)
    print("netaddr", netaddr)
    mask = (0xffffffff << (32 - int(bits))) & 0xffffffff
    print("mask", mask)
    return (ipaddr & mask) == (netaddr & mask)


if __name__ == '__main__':
    adr1 = "111.163.224.0"
    adr2 = "212.163.239.255"
    print("dot2dec", dot2dec(adr1))
    print("dot2dec", dot2dec(adr2))
    if addressInNetwork(adr2, adr1):
        print("YES")
        exit(1)
    XOR = num2dot(dot2dec(adr1) ^ dot2dec(adr2))
    print("XOR", XOR)
    AND = num2dot((dot2dec(adr1) & dot2dec(adr2)))
    print("AND", AND)
    MASK = 32 - dec2bin(dot2dec(XOR)).count('1')
    print("MASK", MASK)
