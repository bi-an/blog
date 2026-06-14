#!/bin/bash

# 检查是否提供了主机名参数
if [ -z "$1" ]; then
    echo "用法: $0 <hostname>"
    exit 1
fi

HOSTNAME=$1

echo "正在查找包含主机 [$HOSTNAME] 或其主机组的队列..."

# 获取所有队列名
queues=$(bqueues | awk 'NR>1 {print $1}')

for q in $queues; do
    # 获取队列的 HOSTS 字段
    hosts_line=$(bqueues -l "$q" | awk '/HOSTS:/,/^$/')
    # 将字符串按空格拆分为数组
    host_array=($hosts_line)

    # 提取所有主机或主机组
    for entry in "${host_array[@]}"; do
        # 如果是主机组（以@开头或以/结尾）
        if [[ $entry == @* || $entry == */ ]]; then
            # if [[ $entry == @* ]]; then
            #     # 去掉前面的@符号
            #     entry=${entry:1}
            # fi
            # 去掉前面的@符号和后面的斜杠
            group=${entry#@}
            group=${entry%/}
            # 获取主机组中的主机列表
            if bmgroup $group 2>/dev/null | grep -qw "$HOSTNAME"; then
                echo "✅ 队列 [$q] 包含主机组 [$group]，其中包括主机 [$HOSTNAME]"
            fi
        else
            # 直接匹配主机名
            if [[ "$entry" == "$HOSTNAME" ]]; then
                echo "✅ 队列 [$q] 直接包含主机 [$HOSTNAME]"
            fi
        fi
    done
done
