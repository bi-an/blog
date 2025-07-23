---
title: VPN设置
date: 2025-01-14 20:44:40
tags:
---

## VMWare 的 Linux 虚拟机共享 Windows 主机的 VPN

方案 2：NAT 模式 + Windows 网络共享
另一种方式是保持 VMware 使用 NAT 模式，并通过 Windows 主机的网络共享功能，将 VPN 流量共享给虚拟机。

步骤：
在 Windows 上启用网络共享：

确保 Clash 在 Windows 主机上运行，并已连接到 VPN。
打开 控制面板 > 网络和共享中心 > 更改适配器设置。
右键点击你的 VPN 连接（通常是一个以 Ethernet 开头的适配器），选择 属性。
在 共享 标签页，勾选 允许其他网络用户通过此计算机的 Internet 连接连接。
选择虚拟机使用的网络适配器（例如 VMware 的 VMnet8 适配器），然后点击确定。
配置虚拟机网络为 NAT：

在 VMware 中，确保虚拟机的网络适配器设置为 NAT（这通常是默认设置）。
在 NAT 模式下，虚拟机会通过 Windows 主机访问网络，但流量会被路由到共享的网络适配器。
验证虚拟机是否可以通过 VPN 访问网络：

启动虚拟机，测试网络连接。
如果虚拟机连接成功并通过 VPN 访问互联网，那么配置就完成了。

## WSL1 通过主机的 clash 客户端访问网络

### 方法一：配置环境变量（适用于普通网络请求）

适合 curl、wget、apt、pip、git 等命令行工具走代理。

```bash
echo 'export http_proxy=http://127.0.0.1:7890' >> ~/.bashrc
echo 'export https_proxy=http://127.0.0.1:7890' >> ~/.bashrc
echo 'export all_proxy=socks5://127.0.0.1:7890' >> ~/.bashrc
source ~/.bashrc
```

验证代理是否有效：

```bash
curl cip.cc
```

这个网站会返回你的公网 IP。

```bash
curl https://www.google.com
```

应该会返回网页。

**临时开启：**

如果你只想暂时走代理一次，不想改 .bashrc，你可以直接这样运行：

```bash
http_proxy=http://127.0.0.1:7890 https_proxy=http://127.0.0.1:7890 curl https://www.google.com
```


### 方法二：使用 proxychains 全局转发（适用于所有程序，包括二进制）

安装 proxychains，让任意程序走 Clash 的代理。

1. 安装 proxychains

```bash
sudo apt update
sudo apt install proxychains -y
```

2. 编辑配置 /etc/proxychains.conf

```bash
sudo vim /etc/proxychains.conf
```

在文件末尾加一行或修改为：

```nginx
socks5  127.0.0.1 7890
```

注意：不要改成 http，要用 socks5。

使用 proxychains（只代理你想代理的程序）：

```bash
proxychains curl https://www.google.com
```

优点：

- 精准控制哪些程序走代理。
- 不影响系统其他设置。

缺点：

- 每次都要手动加 proxychains。


### 方法三：为单独的程序配置代理

1. 配置 git 使用代理

执行以下命令设置全局代理（只设置一次即可）：

```bash
git config --global http.proxy http://127.0.0.1:7890
git config --global https.proxy http://127.0.0.1:7890
```

若你之后想关闭 git 代理，可运行：

```bash
git config --global --unset http.proxy
git config --global --unset https.proxy
```

2. 配置 pip 使用代理

在主目录创建 pip 配置文件：

```bash
mkdir -p ~/.config/pip
vim ~/.config/pip/pip.conf
```

添加内容：

```ini
[global]
proxy = http://127.0.0.1:7890
```

3. 配置 apt 使用代理

编辑 apt 的配置文件：

```bash
sudo mkdir -p /etc/apt/apt.conf.d
sudo nano /etc/apt/apt.conf.d/99proxy
```

粘贴以下内容（假设 HTTP 代理端口是 7890）：

```bash
Acquire::http::Proxy "http://127.0.0.1:7890";
Acquire::https::Proxy "http://127.0.0.1:7890";
```


### 方法四：配置 iptables + redsocks（实现真正的全局转发）

适合高级用户，WSL2 使用 TUN 模式实现系统级全局代理。这比较复杂，建议在需要所有流量强制走代理时使用。TODO
