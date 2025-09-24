#!/bin/bash

# 该脚本会自动将 hostname 提交到延迟最小、最空闲的队列
# 处理后的命令为 bsub -q <queue> -m <host> <args>

# 用法提示
# 例如：$0 myhostname -Is bash
if [ $# -lt 2 ]; then
    echo "用法: $0 <hostname> <args>"
    exit 1
fi

HOSTNAME=$1
# JOB_SCRIPT=$2

# # 检查作业脚本是否存在
# if [ ! -f "$JOB_SCRIPT" ]; then
#     echo "错误：找不到作业脚本 $JOB_SCRIPT"
#     exit 1
# fi

echo "🔍 正在查找包含主机 [$HOSTNAME] 的队列..."

declare -A matching_queues

# 获取所有队列名
queues=$(bqueues | awk 'NR>1 {print $1}')

for q in $queues; do
    # 获取队列的 HOSTS 字段
    hosts_line=$(bqueues -l "$q" | awk '/HOSTS:/,/^$/')
    host_array=($hosts_line)

    for entry in "${host_array[@]}"; do
        # 如果是主机组（以@开头或以/结尾）
        if [[ $entry == @* || $entry == */ ]]; then
            # 去掉前面的@符号和后面的斜杠
            group=${entry#@}
            group=${entry%/}
            if bmgroup "$group" 2>/dev/null | grep -qw "$HOSTNAME"; then
                echo "✅ 队列 [$q] 包含主机组 [$group]，其中包括主机 [$HOSTNAME]"
                matching_queues["$q"]=1
            fi
        elif [[ "$entry" == "$HOSTNAME" ]]; then
            echo "✅ 队列 [$q] 直接包含主机 [$HOSTNAME]"
            matching_queues["$q"]=1
        fi
    done
done

if [ ${#matching_queues[@]} -eq 0 ]; then
    echo "❌ 没有找到包含主机 [$HOSTNAME] 的队列。"
    exit 1
fi

echo "✅ 找到以下队列包含主机 [$HOSTNAME]："
for q in "${!matching_queues[@]}"; do
    echo " - $q"
done

# 获取这些队列的等待任务数，选择最空闲的一个
best_queue=""
min_pend=999999

sorted_queues=$(for q in "${!matching_queues[@]}"; do
    pend=$(bqueues "$q" | awk 'NR==2 {print $9}')
    echo "$pend $q"
    # if [[ "$pend" =~ ^[0-9]+$ ]] && [ "$pend" -lt "$min_pend" ]; then
    #     min_pend=$pend
    #     best_queue=$q
    # fi
done | sort -n | awk '{print $2}')

# if [ -z "$best_queue" ]; then
#     echo "⚠️ 无法确定最空闲队列，默认使用第一个：${!matching_queues[@]}"
#     best_queue=${!matching_queues[@]}
# fi


# 依次尝试提交任务
for q in $sorted_queues; do
    echo "🔎 尝试队列 [$q]..."
    test_output=$(timeout 10 bsub -q "$q" -n 1 -Is /bin/true 2>&1)
    if [ $? -eq 124 ]; then
        echo "⏳ 队列 [$q] 测试超时，跳过。"
        continue
    fi
    if echo "$test_output" | grep -q "User cannot use the queue"; then
        echo "⛔ 无权限使用队列 [$q]，跳过。"
        continue
    fi

    echo "🚀 提交任务到队列 [$q]..."
    CMD="bsub -q $q -m $HOSTNAME ${@:2}"
    # exec将当前终端传递给bsub
    # exec $CMD
    $CMD
    exit 0
done

echo "❌ 所有队列都无法使用或提交失败。"
exit 1

# echo "🚀 正在将任务提交到最空闲的队列 [$best_queue]..."
# # bsub -q "$best_queue" "$JOB_SCRIPT"
# bsub -q "$best_queue" -m "$HOSTNAME" "$argv[2-]"
