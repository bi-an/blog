import schemdraw
import schemdraw.elements as elm

# D 锁存器：两个背靠背钟控反相器（2×4 管）
#   CI1（CLK=HIGH 时导通）：D → Q̄
#   CI2（CLK=LOW  时导通）：Q̄ → D（正反馈保持状态）

with schemdraw.Drawing() as d:
    d.config(fontsize=12, color='black', bgcolor='white')

    # ── CI1 ──────────────────────────────────────────────
    MP_clk1 = d.add(elm.PFet().right().reverse())
    d.add(elm.Line().down().at(MP_clk1.drain).length(0.4))
    MP_in1 = d.add(elm.PFet().right().reverse().anchor('source'))
    d.add(elm.Line().down().at(MP_in1.drain).length(0.4))
    MN_in1 = d.add(elm.NFet().right().reverse().anchor('drain'))
    d.add(elm.Line().down().at(MN_in1.source).length(0.4))
    MN_clk1 = d.add(elm.NFet().right().reverse().anchor('drain'))

    d.add(elm.Line().up().at(MP_clk1.source).length(0.8).label('VDD', loc='top'))
    d.add(elm.Ground().at(MN_clk1.source))

    # CI1 输入门（D 节点）
    d.add(elm.Line().at(MP_in1.gate).to(MN_in1.gate))
    d_mid = 0.5 * (MP_in1.gate + MN_in1.gate)
    d.add(elm.Dot().at(d_mid))
    d.add(elm.Line().left().at(d_mid).length(1.2).label('D', loc='left'))

    # CI1 时钟管标签
    d.add(elm.Line().left().at(MP_clk1.gate).length(1.2)
          .label(r'$\overline{CLK}$', loc='left'))
    d.add(elm.Line().left().at(MN_clk1.gate).length(1.2)
          .label('CLK', loc='left'))

    # CI1 输出节点（Q̄）
    d.add(elm.Dot().at(MP_in1.drain))

    # ── CI2 ──────────────────────────────────────────────
    x2 = float(MP_clk1.source[0]) + 4.5
    y2 = float(MP_clk1.source[1])

    MP_clk2 = d.add(elm.PFet().right().reverse().at((x2, y2)))
    d.add(elm.Line().down().at(MP_clk2.drain).length(0.4))
    MP_in2 = d.add(elm.PFet().right().reverse().anchor('source'))
    d.add(elm.Line().down().at(MP_in2.drain).length(0.4))
    MN_in2 = d.add(elm.NFet().right().reverse().anchor('drain'))
    d.add(elm.Line().down().at(MN_in2.source).length(0.4))
    MN_clk2 = d.add(elm.NFet().right().reverse().anchor('drain'))

    d.add(elm.Line().up().at(MP_clk2.source).length(0.8).label('VDD', loc='top'))
    d.add(elm.Ground().at(MN_clk2.source))

    # CI2 输入门（Q̄ 节点）
    d.add(elm.Line().at(MP_in2.gate).to(MN_in2.gate))
    qbar_gate = 0.5 * (MP_in2.gate + MN_in2.gate)
    d.add(elm.Dot().at(qbar_gate))

    # CI2 时钟管标签（CLK/CLK̄ 互换）
    d.add(elm.Line().left().at(MP_clk2.gate).length(1.2)
          .label('CLK', loc='left'))
    d.add(elm.Line().left().at(MN_clk2.gate).length(1.2)
          .label(r'$\overline{CLK}$', loc='left'))

    # CI2 输出节点（反馈至 D）
    d.add(elm.Dot().at(MP_in2.drain))

    # ── CI1 输出 → CI2 输入（Q̄ 连线）──────────────────
    ci1_out = MP_in1.drain
    qbar_gate_x = float(qbar_gate[0])
    qbar_gate_y = float(qbar_gate[1])
    ci1_out_y = float(ci1_out[1])

    # 水平段
    d.add(elm.Line().right().at(ci1_out)
          .to((qbar_gate_x, ci1_out_y)))
    # 垂直修正段（如不对齐）
    if abs(ci1_out_y - qbar_gate_y) > 0.05:
        d.add(elm.Line().at((qbar_gate_x, ci1_out_y)).to(qbar_gate))

    # Q̄ 标签
    qbar_label_x = (float(ci1_out[0]) + qbar_gate_x) / 2
    d.add(elm.Label().at((qbar_label_x, ci1_out_y))
          .label(r'$\overline{Q}$', loc='top'))

    # ── CI2 输出 → D 节点（正反馈连线）────────────────
    ci2_out = MP_in2.drain
    fb_x = float(ci2_out[0]) + 1.5
    fb_y = float(MN_clk2.source[1]) - 1.5

    d.add(elm.Line().right().at(ci2_out).length(1.5))
    d.add(elm.Line().down().at((fb_x, float(ci2_out[1]))).to((fb_x, fb_y)))
    d.add(elm.Line().left().at((fb_x, fb_y)).to((float(d_mid[0]), fb_y)))
    d.add(elm.Line().up().at((float(d_mid[0]), fb_y)).to(d_mid))

    d.save('05-seq-dlatch-01.svg')
