# 部品調達先（全41点・手はんだ前提）

全部品を手はんだする前提の購入先一覧。リンクは**実在を確認したページのみ**記載する。
空欄は「そのベンダーで確認できなかった」という意味で、無いとは限らない。

designator は41点（v4）。値が同じものをまとめると **25品目**。
**v2 で R5・D4・C12 削除**、**v3 で C13/R15 追加**、**v4（2026-08-14）で U1（M78AR05）を廃止し U4/L1/C14/C15/C16 を追加**した。
D4 の行は既に買った在庫の記録として残す。

---

## 先に結論: 秋月＋マルツの2社で全部揃う

**マルツエレックは DigiKey の国内唯一の正規代理店**で、DigiKey 取扱いの
**600万点以上を1個から、DigiKey と同価格で**買える。送料は全国一律 ¥240
（**¥3,000 以上で無料**）、国内発送・国内決済。

つまり **下表の DigiKey 欄にあるものは、ほぼそのままマルツで買える**。
海外発注・国際送料・通関を避けたいなら、

> **秋月電子（国内在庫の安い部品）＋ マルツ（DigiKey 経由の海外部品）**

の2社で完結する。マルツの型番検索はこの形式:

```
https://www.marutsu.co.jp/GoodsListNavi.jsp?q=＜型番＞
```

マルツは[秋月電子通商の製品も一部取り扱っている](https://www.marutsu.co.jp/MakerCategoryList.jsp?narrow1Cond=%E7%A7%8B%E6%9C%88%E9%9B%BB%E5%AD%90%E9%80%9A%E5%95%86)が、
品揃えは秋月本体に及ばない。

---

## 1. 抵抗（1608 / 0603、全13点・5品目）

| 値 | 使用箇所 | 秋月電子 | マルツ / Mouser / DigiKey |
|---|---|---|---|
| **100kΩ** ×3 | R1, R2, R3 | [1/10W 100個入（130357）](https://akizukidenshi.com/catalog/g/g130357/) | `RC0603FR-07100KL` 等 |
| **22kΩ** ×1 | R4 | ✗（小分けなし） | `RC0603FR-0722KL` 等 |
| **10kΩ** ×7 | R6, R7, R10-R13, **R15** | [1/10W 100個入（130355）](https://akizukidenshi.com/catalog/g/g130355/) | `RC0603FR-0710KL` 等 |
| **120Ω** ×1 | R8（CAN終端） | △ [5000個リール ¥980（130332）](https://akizukidenshi.com/catalog/g/g130332/) | `RC0603FR-07120RL` 等 |
| **1kΩ** ×1 | R14 | △ [5000個リール ¥980（114122）](https://akizukidenshi.com/catalog/g/g114122/) | `RC0603FR-071KL` 等 |

**秋月の1608小分けは 10kΩ と 100kΩ しか無い。** 22kΩ・120Ω・1kΩ は5000個リール（¥980）に
なるので、この3点はマルツで単品購入する方が現実的（Yageo RC0603 系が1本 ¥5〜10）。

どうしても秋月1社で済ませたいなら
[1608 チップ抵抗サンプルブック E24（115657）](https://akizukidenshi.com/catalog/g/g115657/)
が E24 全値 各50本入りで ¥8,900。5品目のためだけには高い。

## 2. コンデンサ（1608 / 0603、全10点・3品目）

| 値 | 使用箇所 | 秋月電子 | マルツ / Mouser / DigiKey |
|---|---|---|---|
| **100nF / 50V** ×5 | C2, C3, C4, C5, C10 | **[0.1µF 100V X7R（114559）](https://akizukidenshi.com/catalog/g/g114559/)** ← 推奨 | `CC0603KRX7R9BB104` 等 |
| | | [0.1µF 50V X8L（116143）](https://akizukidenshi.com/catalog/g/g116143/) も可 | |
| **22pF / 50V** ×2 | C7, C8（水晶負荷） | [22pF 50V CH 40個入（P-11626）](https://www.akizukidenshi.com/catalog/g/gP-11626/) | `CC0603JRNPO9BN220` 等 |
| **10µF** ×3 | C6（V5）, C11（3V3）, **C13（G–S間・要25V）** | [10µF 35V X5R（113161）](https://akizukidenshi.com/catalog/g/g113161/) / [10µF 16V X5R（117202）](https://akizukidenshi.com/catalog/g/g117202/) | [GRM188R61E106KA73D（Mouser）](https://www.mouser.com/ProductDetail/Murata-Electronics/GRM188R61E106KA73D?qs=5aG0NVq1C4z8ebxPkQZf9A%3D%3D) / [同（DigiKey）](https://www.digikey.com/en/products/detail/murata-electronics/GRM188R61E106KA73D/9867922) |

> **秋月の [0.1µF 50V「F特性」（113374）](https://akizukidenshi.com/catalog/g/g113374/) は買わないこと。**
> JIS の F 特性は Y5V 相当で、温度で **+22%／−82%** も動く。デカップリングには使えない。
> **X7R か X8L** を選ぶこと。上表の 114559（100V X7R）が耐圧にも余裕があって一番安全。

> 22pF（P-11626）は調査時点で**在庫切れ表示**だった。入手できない場合は
> C0G / NP0 特性の 1608 22pF なら何でもよい。

> **C13 は 25V 以上を選ぶこと（v3）。** ロードダンプ時に |Vgs| が最大約13V かかるため
> 16V 品（117202）は不可。秋月なら 35V 品（113161）、海外なら GRM188R61E106（25V）。
> C6・C11 も同じ 35V/25V 品で統一すれば1品目で済む。

## 3. 半導体・ダイオード（7品目）

| Ref | 部品 | 秋月電子 | **マルツ** | Mouser | DigiKey |
|---|---|---|---|---|---|
| **D1, D3** | ショットキー 40V/1A **SOD-123** | ✗（後述） | [`1N5819HW-7-F` で検索](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=1N5819HW-7-F) | [1N5819HW-7-F](https://www.mouser.com/ProductDetail/Diodes-Incorporated/1N5819HW-7-F?qs=NQ47qNm99eDyWTEd07miYA%3D%3D) | [1N5819HW-7-F](https://www.digikey.com/product-detail/en/1N5819HW-7-F/1N5819HW-FDICT-ND/815283) |
| **D2** | SMAJ16A（SMA / 400W） | ✗ | **[SMAJ16A（Littelfuse）](https://www.marutsu.co.jp/pc/i/2079887/)** | `SMAJ16A` で検索 | [SMAJ16A](https://www.digikey.com/en/products/detail/littelfuse-inc/SMAJ16A/762278) |
| ~~D4~~ | ツェナー 3.3V（**v2で廃止**・購入不要） | △ [UDZV3.6B **3.6V**（107435）](https://akizukidenshi.com/catalog/g/g107435/) | [`BZT52C3V3` で検索](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=BZT52C3V3) | [BZT52C3V3-HF](https://www.mouser.com/ProductDetail/Comchip-Technology/BZT52C3V3-HF?qs=sPbYRqrBIVmXM9SVNdMgcA%3D%3D) | [BZT52C3V3-7-F](https://www.digikey.com/en/products/detail/diodes-incorporated/BZT52C3V3-7-F/717854) |
| **D6** | サイドビュー緑LED | **[LP812-010(T)（116854）](https://akizukidenshi.com/catalog/g/g116854/)** ← ここだけ | ✗ | ✗ | ✗ |
| **F1** | PPTC 1A hold 1206 | ✗ | [`1206L110/16WR` で検索](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=1206L110) | `1206L110` で検索 | [1206L110/16WR](https://www.digikey.com/en/products/detail/littelfuse-inc/1206L110-16WR/3997167) |
| **Q1** | MMBT3904 SOT-23 | [20個入 ¥100（105969）](https://akizukidenshi.com/catalog/g/g105969/) | [`MMBT3904` で検索](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=MMBT3904) | [onsemi MMBT3904](https://www.mouser.com/ProductDetail/onsemi/MMBT3904?qs=UMEuL5FsraCg3xKjS4uBJQ%3D%3D) | [onsemi MMBT3904](https://www.digikey.com/en/products/detail/onsemi/MMBT3904/458868) |
| **Q2** | ZXMP6A17E6Q SOT-23-6 | ✗ | [`ZXMP6A17E6QTA` で検索](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=ZXMP6A17E6QTA) | [ZXMP6A17E6QTA](https://www.mouser.com/ProductDetail/Diodes-Incorporated/ZXMP6A17E6QTA?qs=JtunSoW0QGE6mLYIV79weA%3D%3D) | [ZXMP6A17E6QTA](https://www.digikey.com/en/products/detail/diodes-incorporated/ZXMP6A17E6QTA/4810942) |

## 4. IC・モジュール（3品目）

| Ref | 部品 | 秋月電子 | **マルツ** | Mouser / DigiKey |
|---|---|---|---|---|
| ~~U1~~ | M78AR05-0.5（**v4で廃止**・購入不要） | [¥580（107179）](https://akizukidenshi.com/catalog/g/g107179/) | — | — |
| **U4** | **AP63205WU-7**（TSOT-26、5V/2A バック） | ✗ | [`AP63205WU-7` で検索](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=AP63205WU-7) | [Mouser](https://www.mouser.com/ProductDetail/Diodes-Incorporated/AP63205WU-7?qs=u16ybLDytRZtkj8PzdWCOw%3D%3D) / [DigiKey](https://www.digikey.com/en/products/detail/diodes-incorporated/AP63205WU-7/9858424) |
| **L1** | 6.8µH 4030 飽和≥1.5A | ✗ | [`ASPI-4030S-6R8M` で検索](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=ASPI-4030S-6R8M) | [ASPI-4030S-6R8M-T（Mouser）](https://www.mouser.com/ProductDetail/ABRACON/ASPI-4030S-6R8M-T?qs=BLmuIjeT3qE8jeo8oUT4Iw%3D%3D) |
| **C14** | 10µF/**50V** 3216(1206) X7R | ✗ | `GRM31CR71H106KA12` で検索 | 同型番で検索（Murata 1206 50V）|
| **C15** | 22µF/16V 2012(0805) | ✗ | `GRM21BR61C226ME44` で検索 | 同型番で検索 |
| **C16** | 100nF/50V 2012(0805) | ✗ | `0805 100nF 50V X7R` で検索 | 任意メーカー可 |
| **U2** | MCP25625T-E/SS（SSOP-28） | **[¥400（112663）](https://akizukidenshi.com/catalog/g/g112663/)** ← 最安 | [`MCP25625` で検索](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=MCP25625) | [Mouser](https://www.mouser.com/ProductDetail/Microchip-Technology/MCP25625T-E-SS?qs=YlpJWyuQ3PP8xO0AA8P3ZQ%3D%3D) / [DigiKey](https://www.digikey.com/en/products/detail/microchip-technology/MCP25625T-E-SS/4860100) |
| **U3** | Raspberry Pi Pico 2 | [Pico 2（129604）](https://akizukidenshi.com/catalog/g/g129604/) / [**Pico 2 H**（ヘッダ実装済、130982）](https://akizukidenshi.com/catalog/g/g130982/) | [`Raspberry Pi Pico 2` で検索](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=Raspberry+Pi+Pico+2) | — |

**U2 は秋月が ¥400 で最安**（DigiKey は $3.38＝約¥500）。回路図の型番 `MCP25625T-E/SS`
と完全一致する。

## 5. コネクタ・その他（4品目）

| Ref | 部品 | 秋月電子 | **マルツ** | Mouser / DigiKey |
|---|---|---|---|---|
| **J1** | TE 1-967658-1（MQS 8極 RA） | ✗ | [`1-967658-1` で検索](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=1-967658-1) | △ [DigiKey Marketplace](https://www.digikey.be/en/products/detail/te-connectivity-amp-connectors/1-967658-1/10478177) ／ [TE 公式](https://www.te.com/en/product-1-967658-1.html) |
| **J2** | DM3AT-SF-PEJM5（microSD） | ✗ | **[ヒロセ DM3AT-SF-PEJM5](https://www.marutsu.co.jp/pc/i/2571574/)**（[別ページ](https://www.marutsu.co.jp/pc/i/238416/)） | [Mouser](https://www.mouser.com/ProductDetail/Hirose-Connector/DM3AT-SF-PEJM5?qs=LZSZKJVF+2WTDKp+R7IYAQ%3D%3D) / [DigiKey](https://www.digikey.com/en/products/detail/hirose-electric-co-ltd/DM3AT-SF-PEJM5/2533565) |
| **J3** | GT-502MGG-N（GPS） | **[¥3,180（117980）](https://akizukidenshi.com/catalog/g/g117980/)** | [`GT-502MGG-N` で検索](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=GT-502MGG-N) | [DigiKey](https://www.digikey.jp/ja/products/detail/yic/GT-502MGG-N/16499054) |
| **Y1** | 16MHz HC-49/S（THT） | [クリスタル 16MHz（108671）](https://akizukidenshi.com/catalog/g/g108671/) | [`16MHz HC-49` で検索](https://www.marutsu.co.jp/GoodsListNavi.jsp?q=16MHz+HC-49) | — |

**J1（MQS コネクタ）が一番入手しにくい。** DigiKey では Marketplace（第三者出品）扱いで、
在庫・納期が通常在庫品と異なる。マルツ・RS コンポーネンツでも型番検索する価値がある。
**発注前に必ず在庫を確認すること。**

## 6. BOM に無いが必要なもの

**Pico をソケット実装する設計**なので、基板側にメスのピンソケットが要る。

| 必要なもの | 数量 | 秋月電子 |
|---|---|---|
| 1×20 メスピンソケット（2.54mm） | 2本 | [分割ロングピンソケット 1×42（105779）](https://akizukidenshi.com/catalog/g/g105779/) を切って使う |
| Pico 側のオスピンヘッダ | 2本 | [Pico 2 H（130982）](https://akizukidenshi.com/catalog/g/g130982/) なら実装済みなので不要 |

---

## 買ってはいけないもの

### D1/D3 に SS14

秋月にも [SS14（113240）](https://akizukidenshi.com/catalog/g/g113240/) があるが、
これは **SMA（DO-214AC）**で、基板のランドは **SOD-123**。物理的に載らない。
**1N5819HW-7-F**（Diodes Inc、40V/1A、SOD-123）を買うこと。

### C2-C5/C10/C12 に「F特性」の 0.1µF

上記のとおり Y5V 相当。**X7R か X8L** を選ぶこと。

### F1 に 6V / 8V 品

`1206L110` 系には 6V（1206L110SLYR）と 8V（1206L110THYR）もある。
12V 車載では使えない。**1206L110/16WR** を指定すること。
秋月の PICOSMDC110S-2 も 6V。

---

## 補足

### F1 の耐圧 16V は許容済み

1206 で 1.1A hold の PPTC は **16V が上限**。しかも F1 は **D2（TVS）より前段**
（J1.8 → F1 → D1 → D2）にあり、車両側サージをクランプされずに直接受ける唯一の部品。
24V/30V 品は 1812 などサイズアップが必要で基板変更を伴うため、
**ユーザー判断で 16V のまま進める**（2026-07-31）。

### ~~D4 を秋月で買う場合は 3.6V になる~~（v2 で D4 廃止）

v2 で IG 電圧センシングごと削除したため D4 は不要になった。
以下は v1 の記録: 秋月の SOD-123 ツェナーは 3.6V と 5.1V で 3.3V が無く、
ADC 絶対最大定格（VDD+0.3 = 3.6V）に対し余裕ゼロの代用だった。

### Y1 の負荷容量

C7/C8 が 22pF なので、**負荷容量 CL = 16〜20pF** の水晶を選ぶこと。
秋月の 108671 は CL=20pF で適合する。CL=12pF や 32pF の品を買わないこと。

### 手はんだの難所

| 部品 | 難度 |
|---|---|
| **U2 MCP25625（SSOP-28、0.65mmピッチ）** | 最難関。フラックスと引きはんだ推奨 |
| **J2 microSD コネクタ** | 端子が細かく、シェル脚は熱容量が大きい |
| **D6 サイドビューLED** | 基板端（y=61）に置くため位置決めがシビア |
| **J3 GPS パッド** | 5パッドに先バラ線を直付け |
| U3 のソケット | 40穴。数が多いだけで v2 では難所ではない |

**v2 で GND パッドは全数サーマル接続に変更した**（スポーク 0.4mm / ギャップ 0.3mm）。
v1 のベタ直結（GND ピンだけ異様に熱を食う）は解消済み。通常のこて先温度で作業できる。

---

*2026-07-31 時点。価格・在庫は変動するため発注時に各ページで確認すること。*
