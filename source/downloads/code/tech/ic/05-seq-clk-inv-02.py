import schemdraw
import schemdraw.elements as elm

# 钟控反相器实现二：时钟管在中间
# 栈序（上→下）：MP_in(IN) → MP_clk(CLK̄) → [OUT] → MN_clk(CLK) → MN_in(IN)

with schemdraw.Drawing() as d:
    d.config(fontsize=12, color='black', bgcolor='white')

    MP_in = d.add(elm.PFet().right().reverse())
    d.add(elm.Line().down().at(MP_in.drain).length(0.4))
    MP_clk = d.add(elm.PFet().right().reverse().anchor('source'))
    d.add(elm.Line().down().at(MP_clk.drain).length(0.4))
    MN_clk = d.add(elm.NFet().right().reverse().anchor('drain'))
    d.add(elm.Line().down().at(MN_clk.source).length(0.4))
    MN_in = d.add(elm.NFet().right().reverse().anchor('drain'))

    d.add(elm.Line().up().at(MP_in.source).length(0.8))
    d.add(elm.Label().label('VDD'))
    d.add(elm.Ground().at(MN_in.source))

    # OUT 节点
    d.add(elm.Dot().at(MP_clk.drain))
    d.add(elm.Line().right().at(MP_clk.drain).length(1).label('OUT', loc='end'))
    out_y = float(MP_clk.drain[1])   # OUT 所在的 y 坐标，IN 也将对齐到此高度

    # CLK 管栅极（中层，较短连线居左）
    d.add(elm.Line().left().at(MP_clk.gate).length(1.0)
          .label(r'$\overline{CLK}$', loc='left'))
    d.add(elm.Line().left().at(MN_clk.gate).length(1.0)
          .label('CLK', loc='left'))

    # IN 管栅极：顶（MP_in.gate）和底（MN_in.gate）需绕过中层 CLK 管
    # 向左延伸至 in_far_x，用垂直线连接两栅，
    # 在与 OUT 同高处（out_y）打圆点并引出 IN 标签
    gate_x = float(MP_in.gate[0])
    in_far_x = gate_x - 2.2

    d.add(elm.Line().left().at(MP_in.gate)
          .to((in_far_x, float(MP_in.gate[1]))))
    d.add(elm.Line().down().at((in_far_x, float(MP_in.gate[1])))
          .to((in_far_x, float(MN_in.gate[1]))))
    d.add(elm.Line().right().at((in_far_x, float(MN_in.gate[1])))
          .to(MN_in.gate))

    # IN 引出点与 OUT 同高
    d.add(elm.Dot().at((in_far_x, out_y)))
    d.add(elm.Line().left().at((in_far_x, out_y)).length(0.5)
          .label('IN', loc='left'))

    d.save('05-seq-clk-inv-02.svg')
