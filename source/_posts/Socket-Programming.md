---
title: Socket Programming
date: 2023-11-27 11:05:29
tags: tcp/ip
---

https://zhuanlan.zhihu.com/p/392820963
端口列表是 /etc/services，/etc/protocols是协议列表

## Tools

### lsof

Options:

    -n inhibits the conversion of network numbers to host names for network files.

    -P inhibits the conversion of port numbers to port names for network files.

    -i specifies the Internet addresses

        multiple addresses (up to a limit of 100) may be specified with multiple options.

        An Internet address is specified in the form (Items in square brackets are optional.):

        [46][protocol][@hostname|hostaddr][:service|port]

        where:
        46 specifies the IP version, IPv4 or IPv6
            that applies to the following address.
            '6' may be be specified only if the UNIX
            dialect supports IPv6.  If neither '4' nor
            '6' is specified, the following address
            applies to all IP versions.
        protocol is a protocol name - TCP, UDP
        hostname is an Internet host name.  Unless a
            specific IP version is specified, open
            network files associated with host names
            of all versions will be selected.
        hostaddr is a numeric Internet IPv4 address in
            dot form; or an IPv6 numeric address in
            colon form, enclosed in brackets, if the
            UNIX dialect supports IPv6.  When an IP
            version is selected, only its numeric
            addresses may be specified.
        service is an /etc/services name - e.g., smtp -
            or a list of them.
        port is a port number, or a list of them.

    -s list file size

    -s p:s  exclude(^)|select protocol (p = TCP|UDP) states by name(s).

Examples:

```bash
lsof -i:<port>
lsof -i -P -n | grep LISTEN
lsof -nP -iTCP -sTCP:LISTEN
```

### netstat
