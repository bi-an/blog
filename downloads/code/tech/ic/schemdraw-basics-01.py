# 用 schemdraw 绘制 CMOS 反相器：PFet 上拉 + NFet 下拉，标注 VDD / GND / IN / OUT，并导出 SVG。
import schemdraw
import schemdraw.elements as elm

with schemdraw.Drawing() as d:
    d.config(fontsize=12, color='black', bgcolor='white')

    # PFet: placement at source; θ=0 → vertical channel; reverse → gate on left
    MP = d.add(elm.PFet().right().reverse())
    d.add(elm.Line().down().at(MP.drain).length(0.4))
    # NFet: align drain to current pen for series stack
    MN = d.add(elm.NFet().right().reverse().anchor('drain'))

    d.add(elm.Line().up().at(MP.source).length(0.6))
    d.add(elm.Label().label('VDD'))
    d.add(elm.Ground().at(MN.source))

    d.add(elm.Line().at(MP.gate).to(MN.gate))
    mid = 0.5 * (MP.gate + MN.gate)
    d.add(elm.Dot().at(mid))
    d.add(elm.Line().left().at(mid).length(1).label('IN', loc='left'))

    d.add(elm.Dot().at(MP.drain))
    d.add(elm.Line().right().at(MP.drain).length(1).label('OUT', loc='end'))

    d.save('schemdraw-basics-01.svg')
