import schemdraw
import schemdraw.elements as elm

# 钟控反相器实现一：时钟管在外侧
# 栈序（上→下）：MP_clk(CLK̄) → MP_in(IN) → [OUT] → MN_in(IN) → MN_clk(CLK)

with schemdraw.Drawing() as d:
    d.config(fontsize=12, color='black', bgcolor='white')

    MP_clk = d.add(elm.PFet().right().reverse())
    d.add(elm.Line().down().at(MP_clk.drain).length(0.4))
    MP_in = d.add(elm.PFet().right().reverse().anchor('source'))
    d.add(elm.Line().down().at(MP_in.drain).length(0.4))
    MN_in = d.add(elm.NFet().right().reverse().anchor('drain'))
    d.add(elm.Line().down().at(MN_in.source).length(0.4))
    MN_clk = d.add(elm.NFet().right().reverse().anchor('drain'))

    d.add(elm.Line().up().at(MP_clk.source).length(0.8))
    d.add(elm.Label().label('VDD'))
    d.add(elm.Ground().at(MN_clk.source))

    # OUT 节点
    d.add(elm.Dot().at(MP_in.drain))
    d.add(elm.Line().right().at(MP_in.drain).length(1).label('OUT', loc='end'))
    out_y = float(MP_in.drain[1])   # OUT 所在的 y 坐标

    # 时钟管栅极
    d.add(elm.Line().left().at(MP_clk.gate).length(1)
          .label(r'$\overline{CLK}$', loc='left'))
    d.add(elm.Line().left().at(MN_clk.gate).length(1)
          .label('CLK', loc='left'))

    # IN 栅极：竖线连接 MP_in.gate → MN_in.gate，
    # 在与 OUT 同高处（out_y）打圆点并引出 IN 标签，使 IN 与 OUT 同水平
    d.add(elm.Line().at(MP_in.gate).to(MN_in.gate))
    gate_x = float(MP_in.gate[0])
    d.add(elm.Dot().at((gate_x, out_y)))
    d.add(elm.Line().left().at((gate_x, out_y)).length(1.5)
          .label('IN', loc='left'))

    d.save('05-seq-clk-inv-01.svg')
