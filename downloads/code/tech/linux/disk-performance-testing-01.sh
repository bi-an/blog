#!/bin/bash

# 测试参数
DEVICE="./testfile"   # 修改为你要测试的文件或设备路径
RUNTIME=30                       # 每个测试运行时间（秒）
# BLOCK_SIZES=("4k" "16k" "64k" "256k" "1M" "4M" "16M" "32M" "64M" "128M")  # 测试块大小列表
BLOCK_SIZES=("128M" "64M" "32M" "16M" "4M" "1M" "256k" "64k" "16k" "4k")  # 测试块大小列表
LOG_FILE="fio_bs_output.log"        # 原始输出日志文件
PERFORMANCE_LOG="fio_bs_performance.log"  # 性能结果日志文件

# 输出表头
printf "%-8s | %-10s | %-8s | %-10s | %-10s\n" "RW" "BlockSize" "IOPS" "BW(MiB/s)" "AvgLat(ms)" | tee "$PERFORMANCE_LOG"
echo "-------------------------------------------------------------" | tee -a "$PERFORMANCE_LOG"

echo "" > "$LOG_FILE"  # 清空日志文件

# 循环测试不同块大小
for RW in read write; do
  for BS in "${BLOCK_SIZES[@]}"; do
    OUTPUT=$(fio --name=bs_test \
                 --filename="$DEVICE" \
                 --rw=$RW \
                 --bs=$BS \
                 --size=1G \
                 --time_based \
                 --runtime=$RUNTIME \
                 --numjobs=1 \
                 --direct=1 \
                 --ioengine=psync \
                 --group_reporting)
    echo "----------------------------------------------" >> "$LOG_FILE"
    echo "$OUTPUT" >> "$LOG_FILE"
    echo "" >> "$LOG_FILE"

    # 提取关键指标
    read IOPS BW BWUNIT LAT LAT_UNIT <<< $(echo "$OUTPUT" | awk '
        /IOPS=/ {match($0, /IOPS= *([0-9.]+)/, iops)}
        /BW=/ {
            match($0, /BW= *([0-9.]+)([KMG]iB)\/s/, bwinfo)
            bwval=bwinfo[1]; bwunit=bwinfo[2]
        }
        /clat \(/ {match($0, /avg= *([0-9.]+),/, lat); match($0, /\(([^)]+)\)/, lat_unit)}
        END {print iops[1], bwval, bwunit, lat[1], lat_unit[1]}
    ')

    # 延迟单位换算
    if [ "$LAT_UNIT" = "usec" ]; then
        LAT_MS=$(awk "BEGIN {printf \"%.2f\", $LAT/1000}")
    elif [ "$LAT_UNIT" = "msec" ]; then
        LAT_MS=$LAT
    else
        LAT_MS="Unknown"
    fi

    # 带宽单位换算为 MiB/s
    case "$BWUNIT" in
        "KiB") BW_MIB=$(awk "BEGIN {printf \"%.2f\", $BW/1024}") ;;
        "MiB") BW_MIB=$BW ;;
        "GiB") BW_MIB=$(awk "BEGIN {printf \"%.2f\", $BW*1024}") ;;
        *) BW_MIB="Unknown" ;;
    esac

    # 输出结果行
    printf "%-8s | %-10s | %-8s | %-10s | %-10s\n" "$RW" "$BS" "$IOPS" "$BW_MIB" "$LAT_MS" | tee -a "$PERFORMANCE_LOG"
  done
done

# 删除 fio 创建的测试文件
rm -f "$DEVICE"
echo "测试完成，结果已保存到 $LOG_FILE 和 $PERFORMANCE_LOG"
