import subprocess
import os

def get_numa_nodes():
    """获取NUMA节点信息"""
    result = subprocess.run(['numactl', '--hardware'], 
                          capture_output=True, text=True)
    return result.stdout

def bind_to_numa_node(node_id, command):
    """将命令绑定到指定的NUMA节点执行"""
    cmd = ['numactl', '--cpunodebind={}'.format(node_id),
           '--membind={}'.format(node_id)] + command
    return subprocess.run(cmd)

# 示例：获取NUMA信息
print(get_numa_nodes())

# 示例：在节点0上运行Python脚本
bind_to_numa_node(0, ['python', 'your_script.py'])

