---
title: Shell Configuration
categories: Linux
tags: shell
date: 2023-11-13 14:27:59
---

## Color Scheme

Refer to [the link](https://askubuntu.com/questions/466198/how-do-i-change-the-color-for-directories-with-ls-in-the-console).

Add these three lines to ~/.bashrc

```bash
$ vi ~/.bashrc
export LS_OPTIONS='--color=auto'
# dircolors - color set for ls
eval "$(dircolors -b)"
alias ls='ls $LS_OPTIONS'
```

