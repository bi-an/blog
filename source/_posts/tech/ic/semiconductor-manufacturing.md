---
title: 半导体制造：从沙石到芯片封装
date: 2026-07-12 19:40:18
categories: ic
tags:
  - ic
  - analog-circuit
---

**Sand / 沙子 (SiO₂)**
↓ High-temperature reduction with carbon in electric arc furnace / 矿热炉高温**碳**强力还原
**Metallurgical Grade Silicon (MGS) / 冶金级硅**
↓ Chlorination with HCl and fractional distillation / 通入 HCl 气体并反复精馏提纯
**Trichlorosilane Gas (SiHCl₃) / 三氯氢硅气体**
↓ Modified Siemens Process (CVD) / 改良西门子法（化学气相沉积）
**Electronic Grade Silicon (EGS) / 电子级多晶硅（硅原料）**
↓ Czochralski (CZ) crystal pulling method / 柴可拉斯基法（单晶旋转提拉）
**Silicon Ingot (Silicon Crystal) / 单晶硅棒（硅晶体）**
↓ Diamond wire sawing and CMP / 钻石线锯切片与化学机械抛光
**Wafer (Bare Wafer) / 晶圆（裸晶圆）**
↓ Photolithography, Etching, Ion Implantation / 光刻、刻蚀、离子注入（反复几十次）
**Processed Wafer (Patterned Wafer) / 满布电路的晶圆（有图形晶圆）**
↓ Wafer testing and Dicing / 晶圆测试与芯片切割
**Die (Bare Die) / 管芯（裸片/晶粒）**
↓ Wire bonding and Packaging / 金属打线连接与引脚封装保护
**Chip (IC / Integrated Circuit) / 芯片（集成电路）**

**💡 同一个 Wafer 上的各个 Die 电路一样吗？**

- **量产时（绝大多数情况）：完全一样。** 像盖章一样复制同一套设计图纸（如批量生产某款特定 CPU 或内存），以实现规模化量产。
- **研发时（极少数情况）：可能不一样（MPW，多项目晶圆）。** 俗称"拼车"或"班车"，几家公司或实验室为了平摊昂贵的模具费，把各自不同的芯片设计拼在同一块 Wafer 上制造，此时的 Die 各不相同。

**💡 晶圆非有效区域**

- **划片槽（Scribe Line）：** 切割 Die 的必需间隙，与省料无关。槽内刻测试图形（Test Structures）做工艺监控，反正划片时会变成粉末，不占良品面积。
- **边缘残缺区（Edge Dice）：** Wafer 按网格划片切成方形 Die，但 Wafer 是圆的，最外圈凑不出完整 Die；边缘缺陷率也高，本就无法出货。降级刻对齐标记、测试图案等，榨干辅助价值后报废。
