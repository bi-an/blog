import schemdraw
import schemdraw.elements as elm

with schemdraw.Drawing() as d:
    d.config(fontsize=12, color='black', bgcolor='white')

    # PFET（左侧）：source=上(IN)，drain=下(OUT)，gate=左(CLK̄)
    MP = d.add(elm.PFet().right().reverse())

    # NFET（右侧）：drain=上(IN)，source=下(OUT)，gate=右(CLK)
    # 以 drain 为入笔锚点，使 drain 与 MP.source 等高
    MN = d.add(
        elm.NFet().right()
        .at((MP.source[0] + 2.8, MP.source[1]))
        .anchor('drain')
    )

    # IN 横线（顶部）：MP.source → MN.drain
    d.add(elm.Line().at(MP.source).to(MN.drain))
    in_x = (MP.source[0] + MN.drain[0]) / 2
    in_y = float(MP.source[1])
    d.add(elm.Dot().at((in_x, in_y)))
    d.add(elm.Line().up().at((in_x, in_y)).length(0.8).label('IN', loc='top'))

    # OUT 横线（底部）：MP.drain → MN.source
    d.add(elm.Line().at(MP.drain).to(MN.source))
    out_x = (MP.drain[0] + MN.source[0]) / 2
    out_y = float(MP.drain[1])
    d.add(elm.Dot().at((out_x, out_y)))
    d.add(elm.Line().down().at((out_x, out_y)).length(0.8).label('OUT', loc='bot'))

    # 栅极标签
    d.add(elm.Line().left().at(MP.gate).length(1.2)
          .label(r'$\overline{CLK}$', loc='left'))
    d.add(elm.Line().right().at(MN.gate).length(1.2)
          .label('CLK', loc='right'))

    d.save('05-seq-tg-01.svg')
