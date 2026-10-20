# Parts sourcing (all 41 parts, assuming hand soldering)

A list of where to buy every part, assuming all of them are hand soldered. Links are given **only
for pages whose existence has been confirmed**. A blank cell means "could not be confirmed at that
vendor", not that it does not exist.

There are 41 designators (v4). Grouped by identical value, that is **25 line items**.
**R5, D4 and C12 were removed in v2**, **C13 and R15 were added in v3**, and **in v4 (2026-08-14)
U1 (the M78AR05) was retired and U4/L1/C14/C15/C16 were added.** The D4 row is kept as a record of
stock already purchased.

> Note for readers outside Japan: this list is written around two Japanese distributors, Akizuki
> Denshi and Marutsu Elec. Marutsu is DigiKey's authorized distributor in Japan, so wherever a
> DigiKey part number appears, DigiKey or Mouser can be used directly instead.

---

## Conclusion first: Akizuki plus Marutsu covers everything

**Marutsu Elec is the only authorized DigiKey distributor in Japan**, so the
**6 million-plus DigiKey line items can be bought one at a time, at DigiKey prices**. Shipping is a
flat ¥240 nationwide (**free above ¥3,000**), shipped and paid for domestically.

In other words, **nearly everything in the DigiKey column below can be bought from Marutsu as-is**.
To avoid ordering from overseas, international shipping and customs, the whole list closes with two
vendors:

> **Akizuki Denshi (cheap parts in domestic stock) + Marutsu (overseas parts via DigiKey)**

Marutsu's part-number search takes this form:

```
https://www.marutsu.co.jp/GoodsListNavi.jsp?q=<part number>
```

Marutsu also [carries some of Akizuki Denshi's products](https://www.marutsu.co.jp/MakerCategoryList.jsp?narrow1Cond=%E7%A7%8B%E6%9C%88%E9%9B%BB%E5%AD%90%E9%80%9A%E5%95%86),
but the selection does not reach that of Akizuki itself.

---

## 1. Resistors (1608 / 0603, 13 parts, 5 line items)

| Value | Where it is used | Akizuki Denshi | Marutsu / Mouser / DigiKey |
|---|---|---|---|
| **100 kΩ** ×3 | R1, R2, R3 | [1/10 W, pack of 100 (130357)](https://akizukidenshi.com/catalog/g/g130357/) | `RC0603FR-07100KL` etc. |
| **22 kΩ** ×1 | R4 | ✗ (no small pack) | `RC0603FR-0722KL` etc. |
| **10 kΩ** ×7 | R6, R7, R10-R13, **R15** | [1/10 W, pack of 100 (130355)](https://akizukidenshi.com/catalog/g/g130355/) | `RC0603FR-0710KL` etc. |
| **120 Ω** ×1 | R8 (CAN termination) | △ [reel of 5000, ¥980 (130332)](https://akizukidenshi.com/catalog/g/g130332/) | `RC0603FR-07120RL` etc. |
| **1 kΩ** ×1 | R14 | △ [reel of 5000, ¥980 (114122)](https://akizukidenshi.com/catalog/g/g114122/) | `RC0603FR-071KL` etc. |

**Akizuki only stocks small packs of 1608 in 10 kΩ and 100 kΩ.** 22 kΩ, 120 Ω and 1 kΩ come only as
reels of 5000 (¥980), so for those three it is more realistic to buy singles from Marutsu (Yageo
RC0603 parts are ¥5–10 each).

If you really want to buy from Akizuki alone, the
[1608 chip resistor sample book, E24 (115657)](https://akizukidenshi.com/catalog/g/g115657/)
has 50 pieces of every E24 value for ¥8,900. That is expensive for five line items.

## 2. Capacitors (1608 / 0603, 10 parts, 3 line items)

| Value | Where it is used | Akizuki Denshi | Marutsu / Mouser / DigiKey |
|---|---|---|---|
| **100 nF / 50 V** ×5 | C2, C3, C4, C5, C10 | **[0.1 µF 100 V X7R (114559)](https://akizukidenshi.com/catalog/g/g114559/)** ← recommended | `CC0603KRX7R9BB104` etc. |
| | | [0.1 µF 50 V X8L (116143)](https://akizukidenshi.com/catalog/g/g116143/) also works | |
| **22 pF / 50 V** ×2 | C7, C8 (crystal load) | [22 pF 50 V CH, pack of 40 (P-11626)](https://www.akizukidenshi.com/catalog/g/gP-11626/) | `CC0603JRNPO9BN220` etc. |
| **10 µF** ×3 | C6 (V5), C11 (3V3), **C13 (G–S, needs 25 V)** | [10 µF 35 V X5R (113161)](https://akizukidenshi.com/catalog/g/g113161/) / [10 µF 16 V X5R (117202)](https://akizukidenshi.com/catalog/g/g117202/) | [GRM188R61E106KA73D (Mouser)](https://www.mouser.com/ProductDetail/Murata-Electronics/GRM188R61E106KA73D?qs=5aG0NVq1C4z8ebxPkQZf9A%3D%3D) / [same part (DigiKey)](https://www.digikey.com/en/products/detail/murata-electronics/GRM188R61E106KA73D/9867922) |

> **Do not buy Akizuki's [0.1 µF 50 V "F characteristic" (113374)](https://akizukidenshi.com/catalog/g/g113374/).**
> The JIS F characteristic is equivalent to Y5V and moves by **+22% / −82%** over temperature. It
> cannot be used for decoupling. Choose **X7R or X8L**. The 114559 above (100 V X7R) is the safest,
> with headroom on the voltage rating as well.

> 22 pF (P-11626) was shown as **out of stock** when this was researched. If you cannot get it, any
> 1608 22 pF with C0G / NP0 characteristics will do.

> **C13 must be rated 25 V or more (v3).** During a load dump |Vgs| reaches about 13 V, so the 16 V
> part (117202) is not acceptable. Use the 35 V part (113161) from Akizuki, or GRM188R61E106 (25 V)
> from overseas. Standardizing C6 and C11 on the same 35 V / 25 V part makes it a single line item.

## 3. Semiconductors and diodes (7 line items)

| Ref | Part | Akizuki Denshi | **Marutsu** | Mouser | DigiKey |
|---|---|---|---|---|---|
| **D1, D3** | Schottky 40 V/1 A **SOD-123** | ✗ (see below) | [search `1N5819HW-7-F`](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=1N5819HW-7-F) | [1N5819HW-7-F](https://www.mouser.com/ProductDetail/Diodes-Incorporated/1N5819HW-7-F?qs=NQ47qNm99eDyWTEd07miYA%3D%3D) | [1N5819HW-7-F](https://www.digikey.com/product-detail/en/1N5819HW-7-F/1N5819HW-FDICT-ND/815283) |
| **D2** | SMAJ16A (SMA / 400 W) | ✗ | **[SMAJ16A (Littelfuse)](https://www.marutsu.co.jp/pc/i/2079887/)** | search `SMAJ16A` | [SMAJ16A](https://www.digikey.com/en/products/detail/littelfuse-inc/SMAJ16A/762278) |
| ~~D4~~ | Zener 3.3 V (**retired in v2**, no need to buy) | △ [UDZV3.6B **3.6 V** (107435)](https://akizukidenshi.com/catalog/g/g107435/) | [search `BZT52C3V3`](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=BZT52C3V3) | [BZT52C3V3-HF](https://www.mouser.com/ProductDetail/Comchip-Technology/BZT52C3V3-HF?qs=sPbYRqrBIVmXM9SVNdMgcA%3D%3D) | [BZT52C3V3-7-F](https://www.digikey.com/en/products/detail/diodes-incorporated/BZT52C3V3-7-F/717854) |
| **D6** | Side-view green LED | **[LP812-010(T) (116854)](https://akizukidenshi.com/catalog/g/g116854/)** ← only here | ✗ | ✗ | ✗ |
| **F1** | PPTC 1 A hold, 1206 | ✗ | [search `1206L110/16WR`](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=1206L110) | search `1206L110` | [1206L110/16WR](https://www.digikey.com/en/products/detail/littelfuse-inc/1206L110-16WR/3997167) |
| **Q1** | MMBT3904 SOT-23 | [pack of 20, ¥100 (105969)](https://akizukidenshi.com/catalog/g/g105969/) | [search `MMBT3904`](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=MMBT3904) | [onsemi MMBT3904](https://www.mouser.com/ProductDetail/onsemi/MMBT3904?qs=UMEuL5FsraCg3xKjS4uBJQ%3D%3D) | [onsemi MMBT3904](https://www.digikey.com/en/products/detail/onsemi/MMBT3904/458868) |
| **Q2** | ZXMP6A17E6Q SOT-23-6 | ✗ | [search `ZXMP6A17E6QTA`](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=ZXMP6A17E6QTA) | [ZXMP6A17E6QTA](https://www.mouser.com/ProductDetail/Diodes-Incorporated/ZXMP6A17E6QTA?qs=JtunSoW0QGE6mLYIV79weA%3D%3D) | [ZXMP6A17E6QTA](https://www.digikey.com/en/products/detail/diodes-incorporated/ZXMP6A17E6QTA/4810942) |

## 4. ICs and modules (3 line items)

| Ref | Part | Akizuki Denshi | **Marutsu** | Mouser / DigiKey |
|---|---|---|---|---|
| ~~U1~~ | M78AR05-0.5 (**retired in v4**, no need to buy) | [¥580 (107179)](https://akizukidenshi.com/catalog/g/g107179/) | — | — |
| **U4** | **AP63205WU-7** (TSOT-26, 5 V/2 A buck) | ✗ | [search `AP63205WU-7`](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=AP63205WU-7) | [Mouser](https://www.mouser.com/ProductDetail/Diodes-Incorporated/AP63205WU-7?qs=u16ybLDytRZtkj8PzdWCOw%3D%3D) / [DigiKey](https://www.digikey.com/en/products/detail/diodes-incorporated/AP63205WU-7/9858424) |
| **L1** | 6.8 µH 4030, saturation ≥1.5 A | ✗ | [search `ASPI-4030S-6R8M`](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=ASPI-4030S-6R8M) | [ASPI-4030S-6R8M-T (Mouser)](https://www.mouser.com/ProductDetail/ABRACON/ASPI-4030S-6R8M-T?qs=BLmuIjeT3qE8jeo8oUT4Iw%3D%3D) |
| **C14** | 10 µF / **50 V** 3216 (1206) X7R | ✗ | search `GRM31CR71H106KA12` | search the same part number (Murata 1206 50 V) |
| **C15** | 22 µF / 16 V 2012 (0805) | ✗ | search `GRM21BR61C226ME44` | search the same part number |
| **C16** | 100 nF / 50 V 2012 (0805) | ✗ | search `0805 100nF 50V X7R` | any manufacturer |
| **U2** | MCP25625T-E/SS (SSOP-28) | **[¥400 (112663)](https://akizukidenshi.com/catalog/g/g112663/)** ← cheapest | [search `MCP25625`](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=MCP25625) | [Mouser](https://www.mouser.com/ProductDetail/Microchip-Technology/MCP25625T-E-SS?qs=YlpJWyuQ3PP8xO0AA8P3ZQ%3D%3D) / [DigiKey](https://www.digikey.com/en/products/detail/microchip-technology/MCP25625T-E-SS/4860100) |
| **U3** | Raspberry Pi Pico 2 | [Pico 2 (129604)](https://akizukidenshi.com/catalog/g/g129604/) / [**Pico 2 H** (headers fitted, 130982)](https://akizukidenshi.com/catalog/g/g130982/) | [search `Raspberry Pi Pico 2`](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=Raspberry+Pi+Pico+2) | — |

**U2 is cheapest at Akizuki for ¥400** (DigiKey is $3.38, about ¥500). It matches the schematic's
part number `MCP25625T-E/SS` exactly.

## 5. Connectors and others (4 line items)

| Ref | Part | Akizuki Denshi | **Marutsu** | Mouser / DigiKey |
|---|---|---|---|---|
| **J1** | TE 1-967658-1 (MQS 8 way RA) | ✗ | [search `1-967658-1`](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=1-967658-1) | △ [DigiKey Marketplace](https://www.digikey.be/en/products/detail/te-connectivity-amp-connectors/1-967658-1/10478177) or [TE direct](https://www.te.com/en/product-1-967658-1.html) |
| **J2** | DM3AT-SF-PEJM5 (microSD) | ✗ | **[Hirose DM3AT-SF-PEJM5](https://www.marutsu.co.jp/pc/i/2571574/)** ([another page](https://www.marutsu.co.jp/pc/i/238416/)) | [Mouser](https://www.mouser.com/ProductDetail/Hirose-Connector/DM3AT-SF-PEJM5?qs=LZSZKJVF+2WTDKp+R7IYAQ%3D%3D) / [DigiKey](https://www.digikey.com/en/products/detail/hirose-electric-co-ltd/DM3AT-SF-PEJM5/2533565) |
| **J3** | GT-502MGG-N (GPS) | **[¥3,180 (117980)](https://akizukidenshi.com/catalog/g/g117980/)** | [search `GT-502MGG-N`](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=GT-502MGG-N) | [DigiKey](https://www.digikey.jp/ja/products/detail/yic/GT-502MGG-N/16499054) |
| **Y1** | 16 MHz HC-49/S (THT) | [crystal 16 MHz (108671)](https://akizukidenshi.com/catalog/g/g108671/) | [search `16MHz HC-49`](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=16MHz+HC-49) | — |

**J1 (the MQS connector) is the hardest part to obtain.** On DigiKey it is handled through the
Marketplace (a third-party seller), so stock and lead time differ from normal stock items. It is
worth searching the part number at Marutsu and RS Components too.
**Always confirm stock before ordering.**

## 6. Needed but not in the BOM

**The design socket-mounts the Pico**, so the board needs female pin sockets.

| What is needed | Quantity | Akizuki Denshi |
|---|---|---|
| 1×20 female pin socket (2.54 mm) | 2 | Cut down a [breakable long pin socket, 1×42 (105779)](https://akizukidenshi.com/catalog/g/g105779/) |
| Male pin headers for the Pico | 2 | Not needed with [Pico 2 H (130982)](https://akizukidenshi.com/catalog/g/g130982/), which has them fitted |

---

## What not to buy

### SS14 for D1/D3

Akizuki does have [SS14 (113240)](https://akizukidenshi.com/catalog/g/g113240/), but that is
**SMA (DO-214AC)** while the board land is **SOD-123**. It physically will not fit.
Buy **1N5819HW-7-F** (Diodes Inc, 40 V/1 A, SOD-123).

### "F characteristic" 0.1 µF for C2-C5/C10/C12

As above, that is equivalent to Y5V. Choose **X7R or X8L**.

### 6 V or 8 V parts for F1

The `1206L110` family also comes in 6 V (1206L110SLYR) and 8 V (1206L110THYR). Those cannot be used
in a 12 V vehicle. Specify **1206L110/16WR**. Akizuki's PICOSMDC110S-2 is 6 V as well.

---

## Notes

### The 16 V rating of F1 is accepted

For a 1206 PPTC with 1.1 A hold, **16 V is the ceiling**. On top of that, F1 is **upstream of D2
(the TVS)** (J1.8 → F1 → D1 → D2), making it the only part that takes vehicle-side surges without
them being clamped. A 24 V/30 V part would need a larger size such as 1812 and therefore a board
change, so **by the user's decision it stays at 16 V** (2026-07-31).

### ~~Buying D4 from Akizuki means 3.6 V~~ (D4 retired in v2)

IG voltage sensing was removed entirely in v2, so D4 is no longer needed.
The following is kept as a v1 record: Akizuki's SOD-123 zeners come in 3.6 V and 5.1 V with no
3.3 V, which left zero margin against the ADC absolute maximum rating (VDD+0.3 = 3.6 V).

### Y1's load capacitance

C7/C8 are 22 pF, so choose a crystal with **a load capacitance CL of 16–20 pF**. Akizuki's 108671
is CL=20 pF and fits. Do not buy CL=12 pF or 32 pF parts.

### The hard parts to hand solder

| Part | Difficulty |
|---|---|
| **U2 MCP25625 (SSOP-28, 0.65 mm pitch)** | The hardest. Flux and drag soldering recommended |
| **J2 microSD connector** | Fine terminals, and the shell legs have a large thermal mass |
| **D6 side-view LED** | Positioning is critical because it sits on the board edge (y=61) |
| **J3 GPS pads** | Bare wires soldered directly to 5 pads |
| U3's socket | 40 holes. Just a lot of them; not difficult as of v2 |

**All GND pads were changed to thermal relief in v2** (spoke 0.4 mm / gap 0.3 mm). The direct pour
connection of v1 (where the GND pins alone sank an absurd amount of heat) is resolved. A normal iron
tip temperature works.

---

*As of 2026-07-31. Prices and stock change, so confirm on each page when ordering.*
