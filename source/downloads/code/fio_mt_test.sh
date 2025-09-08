#!/bin/bash

DEVICE="./mt_testfile"   # 测试文件路径
RUNTIME=30            # 每个测试运行时间（秒）
THREADS=(1 2 4 8 16 32 64 72 120)  # 测试线程数列表
BLOCK_SIZE="64k"      # 块大小，可根据需要修改
LOG_FILE="fio_thread_output.log"
PERFORMANCE_LOG="fio_thread_performance.log"

# 输出表头
printf "%-8s | %-8s | %-10s | %-10s | %-10s\n" "RW" "Threads" "IOPS" "BW(MiB/s)" "AvgLat(ms)" | tee "$PERFORMANCE_LOG"
echo "---------------------------------------------------------------" | tee -a "$PERFORMANCE_LOG"

echo "" > "$LOG_FILE"  # 清空日志文件

for RW in read write; do
  for THREAD in "${THREADS[@]}"; do
    OUTPUT=$(fio --name=thread_test \
                 --filename="$DEVICE" \
                 --rw=$RW \
                 --bs=$BLOCK_SIZE \
                 --size=5G \
                 --time_based \
                 --runtime=$RUNTIME \
                 --numjobs=$THREAD \
                 --direct=1 \
                 --ioengine=psync \
                 --group_reporting)
    echo "---------------------------------------------------------------" >> "$LOG_FILE"
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
  printf "%-8s | %-8s | %-10s | %-10s | %-10s\n" "$RW" "$THREAD" "$IOPS" "$BW_MIB" "$LAT_MS" | tee -a "$PERFORMANCE_LOG"
  done
done

rm -f "$DEVICE"
echo "测试完成，结果已保存到 $LOG_FILE 和 $PERFORMANCE_LOG"