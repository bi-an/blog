---
title: Virtual memory
date: 2023-11-07 15:36:21
categories: Programming
tags:
---

## Terms

resident set size (RSS)

## Comands

## File

/proc/self/statm

### top

#### Fields

| TIME+<sup>[1]</sup>                                            | TIME                                      |
|----------------------------------------------------------------|-------------------------------------------|
| `5432:01` means "5432 minutes and 1 second"                    | `90,32` means "90 hours and 32 minutes"   |
| `25:15.20` means "25 minutes, 15 seconds and 20% of 1 second"  | `25:15` means "25 minutes and 15 seconds" |

### ps

#### Files

`TIME`: the cumulated CPU time in [DD-]hh:mm:ss format (time=TIME)

| Field  | value & means                                                  |
|--------|----------------------------------------------------------------|
| `TIME` | `1-18:09:38` means "1 day, 18 hours, 9 minutes and 38 seconds" |


## Reference

[1]: [what-does-time-cpu-time-hundredth-in-top-mean](https://superuser.com/questions/1148884/what-does-time-cpu-time-hundredth-in-top-mean/)
[what-does-virtual-memory-size-in-top-mean](https://serverfault.com/questions/138427/what-does-virtual-memory-size-in-top-mean)
