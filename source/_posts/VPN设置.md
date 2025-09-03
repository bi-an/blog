---
title: VPN设置
date: 2025-01-14 20:44:40
tags:
---

## Linux 配置 clash 代理

‌1. 创建并配置服务文件‌

编辑 /etc/systemd/system/clash_meta.service，内容示例如下（需根据实际路径调整）：

```ini
[Unit]
Description=Clash Meta Service
After=network.target

[Service]
ExecStart=/usr/local/bin/clash_meta -d /path/to/config
Restart=always

[Install]
WantedBy=multi-user.target
```

关键参数说明：

- ExecStart：指定 clash_meta 的启动命令和配置目录。
- Restart=always：确保服务崩溃后自动重启‌

‌2. 启用开机自启‌
执行以下命令使配置生效：

```bash
sudo systemctl daemon-reload  # 重新加载配置
sudo systemctl enable clash_meta  # 设置开机自启
sudo systemctl start clash_meta   # 立即启动服务
```

命令的作用分析‌

- sudo systemctl daemon-reload‌

重新加载 systemd 的配置文件（如 .service 文件），仅对本次修改生效，‌不影响开机自启‌。
（例如：修改了 /etc/systemd/system/clash_meta.service 后需执行此命令使配置生效）

- ‌sudo systemctl enable clash_meta‌

‌关键命令‌：将 clash_meta 服务添加到开机自启列表，‌下次开机会自动启动‌。
（实际效果：在 /etc/systemd/system/multi-user.target.wants/ 下创建服务符号链接）

- ‌sudo systemctl start clash_meta‌
立即启动服务，但仅对当前会话有效，‌不直接影响开机行为‌。

验证是否成功：

```bash
systemctl is-enabled clash_meta  # 输出应为 "enabled"
systemctl status clash_meta      # 检查服务状态
```

‌3. 验证自启动机制‌
‌检查服务状态‌：

```bash
systemctl list-unit-files | grep clash_meta  # 确认状态为 "enabled"
```

‌查看日志‌：

```bash
journalctl -u clash_meta -f  # 实时跟踪日志
```

‌常见问题‌

- ‌路径错误‌：确保 ExecStart 中的路径正确（如 /usr/local/bin/clash_meta 是否存在）‌
- ‌权限不足‌：服务文件需以 root 权限创建，或通过 User= 指定运行用户‌
- ‌依赖未满足‌：若依赖网络，需在 [Unit] 中添加 After=network.target


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
# 方法1
http_proxy=http://127.0.0.1:7890 https_proxy=http://127.0.0.1:7890 curl https://www.google.com
# 方法2
curl -I https://www.google.com -x http://127.0.0.1:7890
# 方法3：走socks5代理
curl -I --socks5 127.0.0.1:7890 https://www.google.com
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
