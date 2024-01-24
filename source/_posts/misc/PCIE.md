---
title: Introducation to PCIE
date: 2024-01-23 22:04:18
categories: Misc
tags: PCIE
---

# 词汇表（Glossary）

- BAR: base address register.
- PCI configuration space.
- sba: system bus address.



[PCI](https://wiki.osdev.org/PCI)

[PCI configurtation space](https://en.wikipedia.org/wiki/PCI_configuration_space)

    PCI设备有一组被称为configuratin space的寄存器，并且PCI Express引入了Extended configuration space。configuration space寄存器被映射到memory locations。
    驱动设备和诊断软件必须要有权限访问configuration space，操作系统通常使用APIs来授权访问设备configuration space。

PCI configuration space有两种类型的Header（64 bytes）：

## Header Type 0x0

<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-rcf9{background-color:#F9F9F9;text-align:left;vertical-align:middle}
.tg .tg-tbl0{background-color:#F9F9F9;font-weight:bold;text-align:center;vertical-align:middle}
</style>
<table class="tg">
<thead>
  <tr>
    <th class="tg-tbl0">Register</th>
    <th class="tg-tbl0">Offset</th>
    <th class="tg-tbl0">Bits 31-24</th>
    <th class="tg-tbl0">Bits 23-16</th>
    <th class="tg-tbl0">Bits 15-8</th>
    <th class="tg-tbl0">Bits 7-0</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td class="tg-rcf9">0x0</td>
    <td class="tg-rcf9">0x0</td>
    <td class="tg-rcf9" colspan="2">Device ID</td>
    <td class="tg-rcf9" colspan="2">Vendor ID</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x1</td>
    <td class="tg-rcf9">0x4</td>
    <td class="tg-rcf9" colspan="2">Status</td>
    <td class="tg-rcf9" colspan="2">Command</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x2</td>
    <td class="tg-rcf9">0x8</td>
    <td class="tg-rcf9">Class code</td>
    <td class="tg-rcf9">Subclass</td>
    <td class="tg-rcf9">Prog IF</td>
    <td class="tg-rcf9">Revision ID</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x3</td>
    <td class="tg-rcf9">0xC</td>
    <td class="tg-rcf9">BIST</td>
    <td class="tg-rcf9">Header type</td>
    <td class="tg-rcf9">Latency Timer</td>
    <td class="tg-rcf9">Cache Line Size</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x4</td>
    <td class="tg-rcf9">0x10</td>
    <td class="tg-rcf9" colspan="4">Base address #0 (BAR0)</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x5</td>
    <td class="tg-rcf9">0x14</td>
    <td class="tg-rcf9" colspan="4">Base address #1 (BAR1)</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x6</td>
    <td class="tg-rcf9">0x18</td>
    <td class="tg-rcf9">Secondary Latency Timer</td>
    <td class="tg-rcf9">Subordinate Bus Number</td>
    <td class="tg-rcf9">Secondary Bus Number</td>
    <td class="tg-rcf9">Primary Bus Number</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x7</td>
    <td class="tg-rcf9">0x1C</td>
    <td class="tg-rcf9" colspan="2">Secondary Status</td>
    <td class="tg-rcf9">I/O Limit</td>
    <td class="tg-rcf9">I/O Base</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x8</td>
    <td class="tg-rcf9">0x20</td>
    <td class="tg-rcf9" colspan="2">Memory Limit</td>
    <td class="tg-rcf9" colspan="2">Memory Base</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x9</td>
    <td class="tg-rcf9">0x24</td>
    <td class="tg-rcf9" colspan="2">Prefetchable Memory Limit</td>
    <td class="tg-rcf9" colspan="2">Prefetchable Memory Base</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0xA</td>
    <td class="tg-rcf9">0x28</td>
    <td class="tg-rcf9" colspan="4">Prefetchable Base Upper 32 Bits</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0xB</td>
    <td class="tg-rcf9">0x2C</td>
    <td class="tg-rcf9" colspan="4">Prefetchable Limit Upper 32 Bits</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0xC</td>
    <td class="tg-rcf9">0x30</td>
    <td class="tg-rcf9" colspan="2">I/O Limit Upper 16 Bits</td>
    <td class="tg-rcf9" colspan="2">I/O Base Upper 16 Bits</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0xD</td>
    <td class="tg-rcf9">0x34</td>
    <td class="tg-rcf9" colspan="3">Reserved</td>
    <td class="tg-rcf9">Capability Pointer</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0xE</td>
    <td class="tg-rcf9">0x38</td>
    <td class="tg-rcf9" colspan="4">Expansion ROM base address</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0xF</td>
    <td class="tg-rcf9">0x3C</td>
    <td class="tg-rcf9" colspan="2">Bridge Control</td>
    <td class="tg-rcf9">Interrupt PIN</td>
    <td class="tg-rcf9">Interrupt Line</td>
  </tr>
</tbody>
</table>

## Header Type 0x1

<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-rcf9{background-color:#F9F9F9;text-align:left;vertical-align:middle}
.tg .tg-tbl0{background-color:#F9F9F9;font-weight:bold;text-align:center;vertical-align:middle}
</style>
<table class="tg">
<thead>
  <tr>
    <th class="tg-tbl0">Register</th>
    <th class="tg-tbl0">Offset</th>
    <th class="tg-tbl0">Bits 31-24</th>
    <th class="tg-tbl0">Bits 23-16</th>
    <th class="tg-tbl0">Bits 15-8</th>
    <th class="tg-tbl0">Bits 7-0</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td class="tg-rcf9">0x0</td>
    <td class="tg-rcf9">0x0</td>
    <td class="tg-rcf9" colspan="2">Device ID</td>
    <td class="tg-rcf9" colspan="2">Vendor ID</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x1</td>
    <td class="tg-rcf9">0x4</td>
    <td class="tg-rcf9" colspan="2">Status</td>
    <td class="tg-rcf9" colspan="2">Command</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x2</td>
    <td class="tg-rcf9">0x8</td>
    <td class="tg-rcf9">Class code</td>
    <td class="tg-rcf9">Subclass</td>
    <td class="tg-rcf9">Prog IF</td>
    <td class="tg-rcf9">Revision ID</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x3</td>
    <td class="tg-rcf9">0xC</td>
    <td class="tg-rcf9">BIST</td>
    <td class="tg-rcf9">Header type</td>
    <td class="tg-rcf9">Latency Timer</td>
    <td class="tg-rcf9">Cache Line Size</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x4</td>
    <td class="tg-rcf9">0x10</td>
    <td class="tg-rcf9" colspan="4">Base address #0 (BAR0)</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x5</td>
    <td class="tg-rcf9">0x14</td>
    <td class="tg-rcf9" colspan="4">Base address #1 (BAR1)</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x6</td>
    <td class="tg-rcf9">0x18</td>
    <td class="tg-rcf9">Secondary Latency Timer</td>
    <td class="tg-rcf9">Subordinate Bus Number</td>
    <td class="tg-rcf9">Secondary Bus Number</td>
    <td class="tg-rcf9">Primary Bus Number</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x7</td>
    <td class="tg-rcf9">0x1C</td>
    <td class="tg-rcf9" colspan="2">Secondary Status</td>
    <td class="tg-rcf9">I/O Limit</td>
    <td class="tg-rcf9">I/O Base</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x8</td>
    <td class="tg-rcf9">0x20</td>
    <td class="tg-rcf9" colspan="2">Memory Limit</td>
    <td class="tg-rcf9" colspan="2">Memory Base</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0x9</td>
    <td class="tg-rcf9">0x24</td>
    <td class="tg-rcf9" colspan="2">Prefetchable Memory Limit</td>
    <td class="tg-rcf9" colspan="2">Prefetchable Memory Base</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0xA</td>
    <td class="tg-rcf9">0x28</td>
    <td class="tg-rcf9" colspan="4">Prefetchable Base Upper 32 Bits</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0xB</td>
    <td class="tg-rcf9">0x2C</td>
    <td class="tg-rcf9" colspan="4">Prefetchable Limit Upper 32 Bits</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0xC</td>
    <td class="tg-rcf9">0x30</td>
    <td class="tg-rcf9" colspan="2">I/O Limit Upper 16 Bits</td>
    <td class="tg-rcf9" colspan="2">I/O Base Upper 16 Bits</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0xD</td>
    <td class="tg-rcf9">0x34</td>
    <td class="tg-rcf9" colspan="3">Reserved</td>
    <td class="tg-rcf9">Capability Pointer</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0xE</td>
    <td class="tg-rcf9">0x38</td>
    <td class="tg-rcf9" colspan="4">Expansion ROM base address</td>
  </tr>
  <tr>
    <td class="tg-rcf9">0xF</td>
    <td class="tg-rcf9">0x3C</td>
    <td class="tg-rcf9" colspan="2">Bridge Control</td>
    <td class="tg-rcf9">Interrupt PIN</td>
    <td class="tg-rcf9">Interrupt Line</td>
  </tr>
</tbody>
</table>

    BAR0
    Memory-mapped I/O (MMIO) registers
    BAR1
    Device memory windows.
    BAR2/3
    Complementary space of BAR1.
    BAR5
    I/O port.
    BAR6
    PCI ROM.

# Commands

lspci

[Interpreting the output of lspci](https://diego.assencio.com/?index=649b7a71b35fc7ad41e03b6d0e825f07)


# 参考

* [PCI Express I/O System](https://insujang.github.io/2017-04-03/pci-express-i/o-system/)
* [PCI Express](https://www.pcmag.com/encyclopedia/term/pci-express)
* [What is PCIe](https://www.trentonsystems.com/blog/what-is-pcie)
* [What is the Base Address Register (BAR) in PCIe?](https://stackoverflow.com/questions/30190050/what-is-the-base-address-register-bar-in-pcie)
* [PCIe bus addresses, lspci, and working out your PCIe bus topology](https://utcc.utoronto.ca/~cks/space/blog/linux/PCIeLspciBusAddresses)