---
title: Linux权限管理
date: 2025-08-16 20:37:50
categories: 操作系统
tags:
---

Unix权限涉及三个部分：**用户**、**进程**、**文件**。
权限分为常用权限、SELinux权限。
manpage: `man 2 stat` 中搜索 "mode" 可以看到几种常用权限的详情。

## 用户和分组

用户用user ID区分。多个用户可以划入同一个用户分组，一个用户可以同时属于不同的分组。这就是UID（User ID）和GID（Group ID）。

现在我将使用我的凭据登录到 shell 并运行：

```bash
grep $LOGNAME /etc/passwd
```

> rotem: x:1000:1000:rotem,,,:/home/rotem:/bin/bash

您可以看到我的日志名 (rotem)、均为 1000的UID和GID，以及其他详细信息，例如我登录的 shell。

## 进程UID

每个进程都有一个所有者，并且每个进程都属于一个组。

进程有3种UID：`real user ID`、`effective user ID`、`saved user ID`。其中还有一个`set-user-ID`的概念，这个概念和`effective user ID`紧密关联。

### real user ID (RUID）[^5]

在我们的 shell 中，我们现在将运行的每个进程都将继承我的用户帐户的权限，并将使用相同的 UID 和 GID 运行。

当您 fork 一个新进程时，该进程会继承父进程的 RUID。通常父级的 RUID 是你的 shell 并且它有当前登录用户的 UID。所以新进程有当前登录用户的 UID 的 RUID。通常这不会改变，只有 root 可以改变它。

举个例子，想想 init 进程派生了你的登录 shell。在 fork 期间，shell 将具有 root 的 RUID（因为 shell 的父级是 init）。但是，init 进程使用/etc/passwd 将 shell 的 RUID 改成你的 UID. 因此，此后登录 shell 的 RUID 将是您的 UID，而不是 root。所以，我们可以说 RUID 是进程所有者的。 

让我们运行一个简单的命令来检查它：

```bash
sleep 10 & ps aux | grep 'sleep'
```

然后根据`ps`命令打印出的PID，查进程 UID 和 GID：

```bash
stat -c "%u %g" /proc/上一步查询到的PID/
```

> 1000 1000

然而，系统判断一个进程对一个文件是否有权限时，要验证的ID是effective user ID，而不是real user ID。

### effective user ID (EUID)

Linux通常都不建议用户使用root权限去进行一般的处理，但是普通用户在做很多很多services相关的操作的时候，可能需要一个特殊的权限。为 了满足这样的要求，许多services相关的executable有一个标志，这就是set-user-id bit。当这个ser-user-id bit=ON的时候，这个executable被用exec启动之后的进程的effective user id就是这个executable的owner id，而并非parent process real user id。如果set-user-id bit=OFF的时候，这个被exec起来的进程的effective user id应该是等于进程的user id的。
我们以ping命令为例。

使用`which`命令搜索二进制位置，然后运行`ls -la`：

> -rwsr-xr-x  1 root root   64424 Mar 10  2017  ping

可以看到文件的所有者和组是root. 这是因为该`ping`命令需要打开一个套接字，而 Linux 内核root为此需要特权。

但是，如果没有root特权，我如何使用`ping`？

注意文件权限的所有者部分中的“`s`”字母而不是“`x`”。

这是特定二进制可执行文件（如`ping`和`sudo`）的特殊权限位，称为`set-user-ID`。

这是`EUID`和`EGID`发挥作用。

将会发生的情况是，当执行setuid二进制文件时`ping`，该进程将其有效用户 ID ( `EUID`) 从默认值`RUID`更改为此特殊二进制可执行文件的所有者，在本例中为 root。

这一切都是通过这个文件有这个简单的事实来完成的`set-user-ID`。

内核通过查看进程的 来决定该进程是否具有特权`EUID`。因为现在`EUID`指向root，操作不会被内核拒绝。

注意：在最新的 Linux 版本中，ping命令的输出看起来会有所不同，因为它们采用了Linux Capabilities方法而不是这种setuid方法（对于不熟悉的人）请阅读[此处](TODO)。

#### set-user-ID 和 set-group-ID
参考[链接1](https://www.gnu.org/software/coreutils/manual/html_node/Directory-Setuid-and-Setgid.html) 。

Unix包含另一个权限位，即该权限`set-user-ID`位。如果为可执行文件设置了该位，那么只要所有者以外的用户执行该文件，该用户就可以访问所有者的其他任何文件，从而获得所有者的所有文件读/写/执行特权！

这会导致运行该文件的任何人或进程都可以访问系统资源，就好像他们是该文件的所有者一样。

```bash
ls -l testfile
```

> -rw-rw-r-- 1 rotem rotem 0 Nov  8 17:12 testfile

为文件添加set-user-ID权限：

```bash
sudo chmod u+s testfile
ls -l testfile
```

> -rwSrw-r-- 1 rotem rotem 0 Nov  8 17:12 testfile

说明：大写`S`表示，有set-user-ID权限，但是没有执行权限。

#### set-group-id
参考：ManPage `man 2 stat`。

> The set-group-ID bit (S_ISGID) has several special uses.
> For a directory, it indicates that BSD semantics is to be used for that directory: files created there inherit their group ID from the directory, not from the effective group ID of the creating process, and directories created there will also get the S_ISGID bit set.

如果目录具有设置组 ID（S_ISGID）：
- 其下创建的文件将从其父目录的继承组 ID，而不能从创建其的进程中继承有效组 ID；
- 其下创建的子目录将继承设置组 ID 位（S_ISGID）。

> For a file that does not have the group execution bit (S_IXGRP) set, the set-group-ID bit indicates mandatory file/record locking.

如果一个文件有“设置组 ID（set-group-ID）”但是没有“组执行权限（S_IXGRP）”，也就说具有如下权限：

```bash
% ls -l a.txt
-rw-r-Sr-- 1 username groupname 0 Feb 23 19:34 a.txt
```

那么，此时 set-group-ID 位喻示强制文件/记录锁。

TODO：强制文件/记录锁是什么？
[Mandatory File Locking](https://www.kernel.org/doc/Documentation/filesystems/mandatory-locking.txt)
[Why remove group execute for mandatory file lock?](https://superuser.com/questions/968565/why-remove-group-execute-for-mandatory-file-lock)

#### 粘滞位：
> The sticky bit (S_ISVTX) on a directory means that a file in that directory can be renamed or deleted only by the owner of the file, by the owner of the directory, and by a privileged process.

大写`T`表示，有restricted deletion flag or sticky bit（粘滞位），但是没有执行权限，`t`权限只对目录有效，作用是保护目录项不能被其他用户删除。目录要同时具有`x`和`s`才能保证粘滞位有效。

### saved user ID (SUID)

为什么要设置一个saved set-user-id呢？它的意义是，它相当于是一个buffer， 在exec启动进程之后，它会从effective user id位拷贝信息到自己。

- 对于非root用户，可以在未来使用`setuid()`来将effective user id设置成为real user id和saved set-user-id中的任何一个。但是非root用户是不允许用`setuid()`把effective user id设置成为任何第三个user id。
- 对于root来说，就没有那么大的意义了。因为root调用setuid()的时候，将会设置所有的这三个user id位。所以可以综合说，这三个位的设置为为了让unprivilege user可以获得两种不同的permission而设置的。

APUE的例子是，普通用户去执行一个tip进程，set-user-id bit=ON，执行起来的时候，进程可以有uucp (executable owner)的权限来写lock文件，也有当前普通用户的权限来写数据文件。在两种文件操作之间，使用setuid()来切换effective user id。但是正是因为setuid()的限制，该进程无法获得更多的第三种用户的权限。

saved set-user-id是无法取出来的，是kernel来控制的。注意saved set-user-id是进程中的id，而set-user-id bit则是文件上的权限。


## 文件权限

### 符号形式的权限（Symbolic permissions）

符号形式表示文件权限有5种：`rwxst`。

- `r`：可读。对文件来说，意味着能执行 `vim`查看、`cat`等。对目录来说，意味着能执行`ls`查看其下的文件列表。

- `w`：可写。对文件来说，意味着能执行`vim`等工具编辑并保存。对目录来说，意味着能够创建、删除文件或新的目录。

- `x`：可执行（文件） / 可搜索（目录）。对文件来说，意味着可以执行。对目录来说，意味着能够`cd`进入该目录。

> 《UNIX环境高级编程》P80中如此描述：
> 
> 读权限允许我们读目录，获得该目录中所有文件名的列表。当一个目录是我们要访问文件的路径名的一个组成部分时，对该目录的执行权限使我们可以通过该目录（也就是搜索该目录，寻找一个特定的文件名）。

> `ls 没有可搜索权限的目录`可以看到文件列表、文件类型，但是不能看到其他信息，比如文件权限、所有者、大小、修改时间等，因为这些信息保存在inode中，必须先`cd`进入该目录，才能读取这些信息。同样，`ls -R` 不能显示没有执行权限的目录下的子目录下的文件，因为这也必须先`cd`该目录，然后执行`ls`显示子目录的文件。

 ```bash
ls -lR mydir/
 ```

> mydir/:
> ls: cannot access 'mydir/dir2': Permission denied
> ls: cannot access 'mydir/file2': Permission denied
> total 0
> d????????? ? ? ? ?            ? dir2
> -????????? ? ? ? ?            ? file2
> ls: cannot open directory 'mydir/dir2': Permission denied

给目录递归恢复权限：

```bash
chmod -R u+X mydir/
ls -lR mydir/
```

> mydir/:
> total 4
> drwxrwxr-x 3 rotem rotem 4096 Nov  9 10:02 dir2
> -rwxrw-r-- 1 rotem rotem    0 Nov  9 10:03 file2
>
> mydir/dir2:
> total 8
> -rwxrw-r-- 1 rotem rotem   10 Nov  8 22:55 1.txt
> -rwxrw-r-- 1 rotem rotem    0 Nov  8 22:55 2.txt
> drwxrwxr-x 2 rotem rotem 4096 Nov  9 10:05 dir3
>
> mydir/dir2/dir3:
> total 0
> -rwxrw-r-- 1 rotem rotem 0 Nov  9 10:05 3.txt

- `s`：即`set-user-ID`权限，如果可执行文件有`s`权限属性，那么任意进程执行该文件时，将自动获得该文件所有者相同的所有权限。如果文件没有`x`权限，却有`s`权限，那么`ls -l`命令将该文件显示为大写的`S`。文件只有同时具备`s`权限和`x`权限，才有意义，因为一个文件要应用`set-user-ID`属性，首先要保证其可执行。

例如`ping`文件：

```bash
ls -l /bin/ping
```

> -rwsr-xr-x 1 root root 44168 May  8  2014 /bin/ping

由于设置了`s`权限，所以任何文件都能以root用户的身份运行，也就被内核允许打开套接字。

- `t`：即`restricted deletion flag or sticky bit`，称为“粘滞位”或“限制删除标记”。仅仅对目录有效，对文件无效。在一个目录上设了`t`权限位后，（如/home，权限为`1777`)任何的用户都能够在这个目录下创建文档，但只能删除自己创建的文档(root除外)，这就对任何用户能写的目录下的用户文档 启到了保护的作用。如果目录/文件没有`x`权限，却有`s`权限，则`ls -l`命令将目录/文件显示为大写的`T`。目录只有同时具备`t`权限和`x`权限，才有意义，因为一个目录如果本来就不允许增删目录项（`x`权限），删除其他用户的文件更无须提了。

例如：/tmp和 /var/tmp目录供所有用户暂时存取文件，亦即每位用户皆拥有完整的权限进入该目录，去浏览、删除和移动文件。

### 数字形式的权限（Numeric permissions）

可以用4位八进制数（0-7）表示这些文件权限，由4、2、1相加得到，0表示所有权限都没有。

这4位的含义如下：

- 第 1 bit：4表示set-user-ID，2表示set-group-ID，1表示restricted deletion or sticky属性（粘滞位）；
- 第 2 bit：表示文件所有者的权限：可读（4）、可写（2）、可执行（1）；
- 第 3 bit：表示文件所属组的权限：可读（4）、可写（2）、可执行（1）；
- 第 4 bit：表示其他用户的权限：可读（4）、可写（2）、可执行（1）。

以下是`chmod`的man page的说明[^2]：

> A numeric mode is from one to  four  octal  digits  (0-7),  derived  by adding up the bits with values 4, 2, and 1.  Omitted digits are assumed to be leading zeros.  The first digit selects the set user ID  (4)  and set group ID (2) and restricted deletion or sticky (1) attributes.  The second digit selects permissions for the user who owns the  file:  read (4),  write  (2),  and  execute  (1); the third selects permissions for other users in the file's group, with the same values; and  the  fourth for other users not in the file's group, with the same values.

## 相关命令

### `ls`命令

`ls -l`命令用于查看文件权限。

### `chmod` 命令

`chmod`命令用于改变文件权限。

基本用法：

```bash
chmod [OPTION]... MODE[,MODE]... FILE...
chmod [OPTION]... OCTAL-MODE FILE...
chmod [OPTION]... --reference=RFILE FILE...
```

**大写`X`参数** [^1]：

例如`chmod u+X filename`或`chmod u-X filename`。

在`chmod`的man page中介绍如下：

> The  letters `rwxXst` select file mode bits for the affected users: read (`r`), write (`w`), execute (or search for directories) (`x`), **execute/search only if the file is a directory or
>   already has execute permission for some user (`X`)**, set user or group ID on execution (`s`), restricted deletion flag or sticky bit (`t`) [^2].

加黑体的话很费解，但是又十分准确，解释如下：

1. 对所有目录赋予执行权限，这意味着可以执行`cd`。

2. 对所有文件，如果原来文件的`ugo`（user / group / others）任意一个原先有执行权限，那么动作与小写`-x`参数相同；如果原先没有，那么忽略。例如：

   ```bash
   ls -l
   ```

   结果：

   > -rw-rw-r-- 1 rotem rotem 0 Nov  9 10:55 file1
   > -rw-rw-r-x 1 rotem rotem 0 Nov  9 10:56 file2

   可以看到，原来file1的`ugo`都不具备执行权限，file2的others具备执行权限。

   - 对file1执行`X`动作，将无效：

   ```bash
   sudo chmod u+X file1
   ls -l file1
   ```

   > -rw-rw-r-- 1 rotem rotem 0 Nov  9 10:55 file1

   - 但是对file2执行`X`，能够生效：

   ```bash
   sudo chmod u+X file2
   ls -l file2
   ```

   > -rwxrw-r-x 1 rotem rotem 0 Nov  9 10:56 file2

   题外话：` chmod -R u-X mydir`该命令无法递归执行，因为当执行完顶层目录的权限更改之后，已经没有权限`cd`顶层目录了，其他执行全部被停止。
- getfacl/setfacl
- umask

## SELinux权限
SELinux提供更为严格的访问控制。
可以用`ls -Z`查看文件的SELinux权限（安全上下文）。这部分将来另用一篇博客说明。暂时可以参考文献[^3]。


## 参考文献

[^1]: https://www.franzoni.eu/chmod-and-the-capital-x/
[^2]: chmod 的 man page
[^3]:https://www.jianshu.com/p/73621cc7c222
[^4]:https://en.wikipedia.org/wiki/User_identifier#Saved_user_ID
[^5]:https://stackoverflow.com/questions/32455684/difference-between-real-user-id-effective-user-id-and-saved-user-id
[^6]:https://cloud.tencent.com/developer/article/1722142
