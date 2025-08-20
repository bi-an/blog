import pandas as pd
import matplotlib.pyplot as plt

# 读取 CSV
df = pd.read_csv('memory_power.csv')

# 找到动态功耗峰值周期
peak_cycle = df['DynamicPower'].idxmax() + 1
peak_value = df['DynamicPower'].max()

plt.figure(figsize=(12,6))

# 绘制条形背景（ASCII风格效果）
for i, val in enumerate(df['DynamicPower']):
    bar_len = int(val / peak_value * 50)  # 50字符最大长度
    plt.text(i+1, 0, '#' * bar_len, fontsize=8, color='grey', va='bottom')

# 绘制曲线
plt.plot(df['Cycle'], df['DynamicPower'], label='Dynamic Power', marker='o', color='blue')
plt.plot(df['Cycle'], df['StaticPower'], label='Static Power', linestyle='--', color='orange')
plt.plot(df['Cycle'], df['TotalPower'], label='Total Power', linestyle='-.', color='green')

# 高亮峰值
plt.scatter(peak_cycle, peak_value, color='red', s=100, label='Peak Dynamic Power')

plt.title('Memory Power Simulation with Peak Highlight & ASCII-style Bars')
plt.xlabel('Cycle')
plt.ylabel('Power Units')
plt.grid(True, linestyle=':')
plt.legend()
plt.tight_layout()
plt.show()
