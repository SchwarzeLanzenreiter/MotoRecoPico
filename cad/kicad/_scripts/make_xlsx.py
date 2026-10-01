# -*- coding: utf-8 -*-
"""Build the parts-sourcing workbook from docs/parts-sourcing.md's verified links."""
import os
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter

OUT = r"E:\Claude\MotoRecoPico\docs\MotoRecoPico-部品購入先.xlsx"
FONT = 'Arial'

AKI = 'https://akizukidenshi.com/catalog/g/g{}/'
MARU_Q = 'https://www.marutsu.co.jp/GoodsListNavi.jsp?q={}'

# 品目, 型番/値, パッケージ, 数量, 使用箇所, 秋月(text,url), マルツ, Mouser, DigiKey, 備考
N = (None, None)
ROWS = [
    ('抵抗', '100kΩ 1/10W', '1608 (0603)', 3, 'R1, R2, R3',
     ('100個入 ¥150 (130357)', AKI.format('130357')),
     ('RC0603FR-07100KL', MARU_Q.format('RC0603FR-07100KL')), N, N, ''),
    ('抵抗', '22kΩ 1/10W', '1608 (0603)', 1, 'R4',
     ('小分けなし', None),
     ('RC0603FR-0722KL', MARU_Q.format('RC0603FR-0722KL')), N, N,
     '秋月は1608の22kΩを小分けで扱っていない'),
    ('抵抗', '10kΩ 1/10W', '1608 (0603)', 7, 'R6, R7, R10-R13, R15 (IG検知)',
     ('100個入 ¥150 (130355)', AKI.format('130355')),
     ('RC0603FR-0710KL', MARU_Q.format('RC0603FR-0710KL')), N, N, ''),
    ('抵抗', '120Ω 1/10W', '1608 (0603)', 1, 'R8 (CAN終端)',
     ('5000個リール ¥980 (130332)', AKI.format('130332')),
     ('RC0603FR-07120RL', MARU_Q.format('RC0603FR-07120RL')), N, N,
     '秋月はリールのみ。1本だけ要るのでマルツ推奨'),
    ('抵抗', '1kΩ 1/10W', '1608 (0603)', 1, 'R14',
     ('5000個リール ¥980 (114122)', AKI.format('114122')),
     ('RC0603FR-071KL', MARU_Q.format('RC0603FR-071KL')), N, N,
     '秋月はリールのみ。1本だけ要るのでマルツ推奨'),

    ('コンデンサ', '100nF / 50V以上 X7R', '1608 (0603)', 5, 'C2-C5, C10',
     ('0.1µF 100V X7R (114559)', AKI.format('114559')),
     ('CC0603KRX7R9BB104', MARU_Q.format('CC0603KRX7R9BB104')), N, N,
     '★F特性(113374)は買わないこと。Y5V相当で-82%変動する'),
    ('コンデンサ', '22pF / 50V C0G(NP0)', '1608 (0603)', 2, 'C7, C8 (水晶負荷)',
     ('40個入 ¥100 (P-11626)', 'https://www.akizukidenshi.com/catalog/g/gP-11626/'),
     ('CC0603JRNPO9BN220', MARU_Q.format('CC0603JRNPO9BN220')), N, N,
     '調査時点で秋月は在庫切れ表示'),
    ('コンデンサ', '10µF / 25V以上', '1608 (0603)', 3, 'C6, C11, C13 (Q2ゲート遅延)',
     ('35V (113161) / 16V (117202)', AKI.format('113161')),
     ('GRM188R61E106KA73D', MARU_Q.format('GRM188R61E106KA73D')),
     ('GRM188R61E106KA73D',
      'https://www.mouser.com/ProductDetail/Murata-Electronics/GRM188R61E106KA73D?qs=5aG0NVq1C4z8ebxPkQZf9A%3D%3D'),
     ('GRM188R61E106KA73D',
      'https://www.digikey.com/en/products/detail/murata-electronics/GRM188R61E106KA73D/9867922'),
     'C13はロードダンプ時に約13Vかかるため25V以上必須。3点とも25V/35V品で統一が楽'),

    ('ダイオード', 'B5819W (40V/1A ショットキー)', 'SOD-123', 2, 'D1, D3',
     ('取扱いなし', None),
     ('1N5819HW-7-F', MARU_Q.format('1N5819HW-7-F')),
     ('1N5819HW-7-F',
      'https://www.mouser.com/ProductDetail/Diodes-Incorporated/1N5819HW-7-F?qs=NQ47qNm99eDyWTEd07miYA%3D%3D'),
     ('1N5819HW-7-F',
      'https://www.digikey.com/product-detail/en/1N5819HW-7-F/1N5819HW-FDICT-ND/815283'),
     '★秋月のSS14はSMAパッケージで載らない'),
    ('ダイオード', 'SMAJ16A (TVS 400W)', 'SMA (DO-214AC)', 1, 'D2',
     ('取扱いなし', None),
     ('Littelfuse SMAJ16A', 'https://www.marutsu.co.jp/pc/i/2079887/'),
     ('SMAJ16A', 'https://www.mouser.com/'),
     ('Littelfuse SMAJ16A',
      'https://www.digikey.com/en/products/detail/littelfuse-inc/SMAJ16A/762278'),
     ''),
# D4 (3.3V zener, ADC clamp) left the BOM in v2 with the IG sense circuit.
    ('LED', 'LP812-010(T) 緑 サイドビュー', '2.8×1.2mm', 1, 'D6',
     ('10個入 ¥100 (116854)', AKI.format('116854')),
     ('取扱いなし', None), N, N,
     '★秋月でしか買えない'),
    ('回路保護', 'PPTC 1.1A hold 16V', '1206', 1, 'F1',
     ('取扱いなし', None),
     ('1206L110/16WR', MARU_Q.format('1206L110')),
     ('1206L110/16WR', 'https://www.mouser.com/'),
     ('1206L110/16WR',
      'https://www.digikey.com/en/products/detail/littelfuse-inc/1206L110-16WR/3997167'),
     '★6V品(SLYR)・8V品(THYR)と間違えないこと'),
    ('トランジスタ', 'MMBT3904 (NPN)', 'SOT-23', 1, 'Q1',
     ('20個入 ¥100 (105969)', AKI.format('105969')),
     ('MMBT3904', MARU_Q.format('MMBT3904')),
     ('onsemi MMBT3904',
      'https://www.mouser.com/ProductDetail/onsemi/MMBT3904?qs=UMEuL5FsraCg3xKjS4uBJQ%3D%3D'),
     ('onsemi MMBT3904',
      'https://www.digikey.com/en/products/detail/onsemi/MMBT3904/458868'),
     ''),
    ('MOSFET', 'ZXMP6A17E6Q (Pch -60V)', 'SOT-23-6', 1, 'Q2 (高側SW)',
     ('取扱いなし', None),
     ('ZXMP6A17E6QTA', MARU_Q.format('ZXMP6A17E6QTA')),
     ('ZXMP6A17E6QTA',
      'https://www.mouser.com/ProductDetail/Diodes-Incorporated/ZXMP6A17E6QTA?qs=JtunSoW0QGE6mLYIV79weA%3D%3D'),
     ('ZXMP6A17E6QTA',
      'https://www.digikey.com/en/products/detail/diodes-incorporated/ZXMP6A17E6QTA/4810942'),
     ''),

    ('電源IC', 'AP63205WU-7 (5V固定 2A バック)', 'TSOT-26', 1, 'U4 (v4)',
     ('取扱いなし', None),
     ('AP63205WU-7', MARU_Q.format('AP63205WU-7')),
     ('AP63205WU-7',
      'https://www.mouser.com/ProductDetail/Diodes-Incorporated/AP63205WU-7?qs=u16ybLDytRZtkj8PzdWCOw%3D%3D'),
     ('AP63205WU-7',
      'https://www.digikey.com/en/products/detail/diodes-incorporated/AP63205WU-7/9858424'),
     'v4でM78AR05を置換。最低入力3.8Vでクランキング問題も解消'),
    ('インダクタ', '6.8µH 4030 飽和≥1.5A', '4×4×3mm', 1, 'L1 (v4)',
     ('取扱いなし', None),
     ('ASPI-4030S-6R8M', MARU_Q.format('ASPI-4030S-6R8M')),
     ('ASPI-4030S-6R8M-T',
      'https://www.mouser.com/ProductDetail/ABRACON/ASPI-4030S-6R8M-T?qs=BLmuIjeT3qE8jeo8oUT4Iw%3D%3D'),
     N, 'シールド品推奨。基板裏面の最背部品(3mm)'),
    ('コンデンサ', '10µF / 50V X7R', '3216 (1206)', 1, 'C14 バック入力 (v4)',
     ('取扱いなし', None),
     ('GRM31CR71H106KA12', MARU_Q.format('GRM31CR71H106KA12')), N, N,
     '★入力はクランプ26Vを直接見るため50V必須。25V品不可'),
    ('コンデンサ', '22µF / 16V', '2012 (0805)', 1, 'C15 バック出力 (v4)',
     ('取扱いなし', None),
     ('GRM21BR61C226ME44', MARU_Q.format('GRM21BR61C226ME44')), N, N, ''),
    ('コンデンサ', '100nF / 50V X7R', '2012 (0805)', 1, 'C16 ブートストラップ (v4)',
     ('取扱いなし', None),
     ('0805 100nF 50V', MARU_Q.format('GRM21BR71H104KA01')), N, N,
     '1608でなく2012なのは「1608全数表面」規則のため（裏面実装）'),
    ('IC', 'MCP25625T-E/SS (CAN)', 'SSOP-28 0.65mm', 1, 'U2',
     ('¥400 (112663)', AKI.format('112663')),
     ('MCP25625', MARU_Q.format('MCP25625')),
     ('MCP25625T-E/SS',
      'https://www.mouser.com/ProductDetail/Microchip-Technology/MCP25625T-E-SS?qs=YlpJWyuQ3PP8xO0AA8P3ZQ%3D%3D'),
     ('MCP25625T-E/SS',
      'https://www.digikey.com/en/products/detail/microchip-technology/MCP25625T-E-SS/4860100'),
     '★秋月が最安（DigiKeyは$3.38）。手はんだ最難関'),
    ('マイコン', 'Raspberry Pi Pico 2', '2×20 2.54mm', 1, 'U3',
     ('Pico 2 H ヘッダ実装済 (130982)', AKI.format('130982')),
     ('Raspberry Pi Pico 2', MARU_Q.format('Raspberry+Pi+Pico+2')), N, N,
     'ヘッダ無しは (129604)。H版なら自分でヘッダを付けずに済む'),

    ('コネクタ', 'TE 1-967658-1 (MQS 8極 RA)', 'THT ライトアングル', 1, 'J1 (車両I/F)',
     ('取扱いなし', None),
     ('1-967658-1', MARU_Q.format('1-967658-1')), N,
     ('DigiKey Marketplace',
      'https://www.digikey.be/en/products/detail/te-connectivity-amp-connectors/1-967658-1/10478177'),
     '★最も入手しにくい。DigiKeyは第三者出品。発注前に在庫確認'),
    ('コネクタ', 'DM3AT-SF-PEJM5 (microSD)', 'SMD ライトアングル', 1, 'J2',
     ('取扱いなし', None),
     ('ヒロセ DM3AT-SF-PEJM5', 'https://www.marutsu.co.jp/pc/i/2571574/'),
     ('DM3AT-SF-PEJM5',
      'https://www.mouser.com/ProductDetail/Hirose-Connector/DM3AT-SF-PEJM5?qs=LZSZKJVF+2WTDKp+R7IYAQ%3D%3D'),
     ('DM3AT-SF-PEJM5',
      'https://www.digikey.com/en/products/detail/hirose-electric-co-ltd/DM3AT-SF-PEJM5/2533565'),
     ''),
    ('モジュール', 'GT-502MGG-N (GPS)', '先バラ5線', 1, 'J3',
     ('¥3,180 (117980)', AKI.format('117980')),
     ('GT-502MGG-N', MARU_Q.format('GT-502MGG-N')), N,
     ('YIC GT-502MGG-N',
      'https://www.digikey.jp/ja/products/detail/yic/GT-502MGG-N/16499054'),
     '秋月が最安'),
    ('水晶', '16MHz HC-49/S CL=16〜20pF', 'THT', 1, 'Y1',
     ('CL=20pF (108671)', AKI.format('108671')),
     ('16MHz HC-49', MARU_Q.format('16MHz+HC-49')), N, N,
     '★CL=12pFや32pFの品は不可（C7/C8が22pFのため）'),

    ('【BOM外】', '1×20 メスピンソケット 2.54mm', 'THT', 2, 'U3 のソケット',
     ('分割ロングピンソケット 1×42 (105779)', AKI.format('105779')),
     N, N, N,
     'Pico をソケット実装する設計のため基板側に必要。1×42を切って使う'),
]

HDR = ['分類', '型番 / 値', 'パッケージ', '数量', '使用箇所',
       '秋月電子', 'マルツ', 'Mouser', 'DigiKey', '備考', '発注済']

wb = Workbook()

# ------------------------------------------------------------------ 調達先一覧
ws = wb.active
ws.title = '購入先一覧'

TITLE = 'MotoRecoPico 部品購入先一覧'
ws['A1'] = TITLE
ws['A1'].font = Font(name=FONT, size=14, bold=True)
ws['A2'] = ('全41点（v4: 5V電源をAP63205化）を値でまとめて25品目＋ソケット。'
            'リンクは実在を確認したページのみ。'
            '空欄＝そのベンダーで確認できなかった（無いとは限らない）。')
ws['A2'].font = Font(name=FONT, size=9, italic=True, color='555555')
ws['A3'] = ('マルツは DigiKey の国内唯一の正規代理店。DigiKey 全品を1個から同価格で買え、'
            '送料は全国一律 ¥240（¥3,000以上で無料）。秋月＋マルツの2社で全部揃う。')
ws['A3'].font = Font(name=FONT, size=9, bold=True, color='1F4E79')

HDR_ROW = 5
thin = Side(style='thin', color='BFBFBF')
border = Border(left=thin, right=thin, top=thin, bottom=thin)
hdr_fill = PatternFill('solid', fgColor='1F4E79')

for c, h in enumerate(HDR, start=1):
    cell = ws.cell(row=HDR_ROW, column=c, value=h)
    cell.font = Font(name=FONT, size=10, bold=True, color='FFFFFF')
    cell.fill = hdr_fill
    cell.alignment = Alignment(horizontal='center', vertical='center')
    cell.border = border

LINK = Font(name=FONT, size=9, color='0563C1', underline='single')
PLAIN = Font(name=FONT, size=9)
GREY = Font(name=FONT, size=9, color='999999')
WARN = Font(name=FONT, size=9, color='C00000')
ALT = PatternFill('solid', fgColor='F2F7FB')

r = HDR_ROW + 1
for row in ROWS:
    cat, mpn, pkg, qty, used = row[0], row[1], row[2], row[3], row[4]
    vendors = row[5:9]
    note = row[9]
    vals = [cat, mpn, pkg, qty, used]
    for c, v in enumerate(vals, start=1):
        cell = ws.cell(row=r, column=c, value=v)
        cell.font = Font(name=FONT, size=9, bold=(c == 1))
        cell.border = border
        cell.alignment = Alignment(vertical='center',
                                   horizontal='center' if c == 4 else 'left')
    for i, (text, url) in enumerate(vendors):
        cell = ws.cell(row=r, column=6 + i)
        cell.border = border
        cell.alignment = Alignment(vertical='center')
        if text is None:
            continue
        cell.value = text
        if url:
            cell.hyperlink = url
            cell.font = LINK
        else:
            cell.font = GREY
    nc = ws.cell(row=r, column=10, value=note)
    nc.font = WARN if note.startswith('★') else PLAIN
    nc.border = border
    nc.alignment = Alignment(vertical='center', wrap_text=True)
    oc = ws.cell(row=r, column=11)
    oc.border = border
    oc.fill = PatternFill('solid', fgColor='FFFF00')
    if (r - HDR_ROW) % 2 == 0:
        for c in range(1, 11):
            if ws.cell(row=r, column=c).fill.fgColor.rgb in (None, '00000000'):
                ws.cell(row=r, column=c).fill = ALT
    r += 1

LAST = r - 1
tot = ws.cell(row=r, column=3, value='合計')
tot.font = Font(name=FONT, size=10, bold=True)
tot.alignment = Alignment(horizontal='right')
tc = ws.cell(row=r, column=4, value=f'=SUM(D{HDR_ROW+1}:D{LAST})')
tc.font = Font(name=FONT, size=10, bold=True)
tc.border = border
tc.alignment = Alignment(horizontal='center')
ws.cell(row=r, column=5, value='点（BOM 38点 + ソケット2本）').font = Font(
    name=FONT, size=9, italic=True, color='555555')

for col, w in zip('ABCDEFGHIJK',
                  [11, 30, 20, 6, 20, 30, 26, 22, 26, 46, 8]):
    ws.column_dimensions[col].width = w
ws.freeze_panes = f'A{HDR_ROW+1}'
ws.auto_filter.ref = f'A{HDR_ROW}:K{LAST}'
ws.row_dimensions[HDR_ROW].height = 22

# ------------------------------------------------------------------- 注意事項
ws2 = wb.create_sheet('注意事項')
ws2['A1'] = '発注・実装時の注意'
ws2['A1'].font = Font(name=FONT, size=14, bold=True)

NOTES = [
    ('買ってはいけない', 'D1/D3 に SS14',
     '秋月のSS14(113240)はSMA(DO-214AC)。基板のランドはSOD-123なので物理的に載らない。'
     '1N5819HW-7-F（40V/1A・SOD-123）を買うこと。'),
    ('買ってはいけない', 'C2-C5/C10/C12 に「F特性」の0.1µF',
     '秋月の0.1µF 50V F特性(113374)はJISのF特性＝Y5V相当で、温度により+22%/-82%変動する。'
     'デカップリングには使えない。X7R か X8L を選ぶこと。'),
    ('買ってはいけない', 'F1 に 6V / 8V 品',
     '1206L110系には6V(1206L110SLYR)と8V(1206L110THYR)もある。12V車載では使えない。'
     '1206L110/16WR を指定すること。秋月のPICOSMDC110S-2も6V。'),
    ('仕様の判断', 'F1 の耐圧16Vは許容済み',
     '1206で1.1A holdのPPTCは16Vが上限。しかもF1はD2(TVS)より前段(J1.8→F1→D1→D2)にあり、'
     '車両側サージをクランプされずに直接受ける唯一の部品。24V/30V品は1812などサイズアップが'
     '必要で基板変更を伴うため、ユーザー判断で16Vのまま進める（2026-07-31）。'),
    ('v3変更', 'C13（10µF/25V）と R15（10k）が増えた',
     'v3（2026-08-09）でIGN OFF遅延を追加。C13はQ2のゲート–ソース間で25V以上必須、'
     'R15はQ1ベース→GP28のIG検知用。10µFは3点、10kΩは7点になった。'),
    ('v2変更', 'R5・D4・C12 は購入不要',
     'v2（2026-08-08）でIG電圧センシングを廃止し、R5（10k 1点）、D4（3.3Vツェナー）、'
     'C12（100nF 1点）を削除した。10kΩは6点、100nFは5点に減っている。'),
    ('仕様の判断', 'Y1 の負荷容量',
     'C7/C8が22pFなので負荷容量CL=16〜20pFの水晶を選ぶこと。秋月の108671はCL=20pFで適合。'
     'CL=12pFや32pFの品は不可。'),
    ('入手性', 'J1（MQSコネクタ）が最難関',
     'DigiKeyではMarketplace（第三者出品）扱いで在庫・納期が通常在庫品と異なる。'
     'マルツ・RSコンポーネンツでも型番検索する価値がある。代替の効かない部品なので'
     '発注前に必ず在庫を確認すること。'),
    ('入手性', '秋月でしか買えないもの',
     'D6（LP812-010(T) サイドビューLED）はKODENSHIの国内向け品で海外流通が無い。'
     'J3（GT-502MGG-N）はDigiKeyにもあるが秋月が最安（¥3,180）。'),
    ('入手性', '秋月の1608抵抗は小分けが限られる',
     '10kΩ(130355)と100kΩ(130357)は100個入 ¥150 があるが、22kΩ・120Ω・1kΩは'
     '5000個リール(¥980)しかない。この3点はマルツで単品購入する方が現実的。'),
    ('手はんだ', 'U2 MCP25625（SSOP-28 0.65mmピッチ）',
     '最難関。フラックスと引きはんだを推奨。'),
    ('手はんだ', 'J2 microSDコネクタ',
     '端子が細かく、シェル脚は熱容量が大きい。'),
    ('手はんだ', 'D6 サイドビューLED',
     '基板端（y=61）に置くため位置決めがシビア。'),
    ('v2変更', 'GNDパッドはサーマル接続になった',
     'v2で全ゾーンをサーマル接続（スポーク0.4mm/ギャップ0.3mm）に変更。'
     'v1のベタ直結（GNDピンだけ異様に熱を食う）は解消済みで、通常のこて先温度で作業できる。'),
    ('BOM外', 'Picoのソケットが要る',
     'Picoをソケット実装する設計なので、基板側に1×20メスピンソケットが2本必要。'
     '分割ロングピンソケット1×42(105779)を切って使う。Pico側のオスヘッダは'
     'Pico 2 H(130982)を買えば実装済みで不要。'),
]

H2 = ['区分', '項目', '内容']
for c, h in enumerate(H2, start=1):
    cell = ws2.cell(row=3, column=c, value=h)
    cell.font = Font(name=FONT, size=10, bold=True, color='FFFFFF')
    cell.fill = hdr_fill
    cell.alignment = Alignment(horizontal='center', vertical='center')
    cell.border = border

CAT_FILL = {'買ってはいけない': 'FFC7CE', '仕様の judgement': None}
r = 4
for cat, title, body in NOTES:
    a = ws2.cell(row=r, column=1, value=cat)
    a.font = Font(name=FONT, size=9, bold=True,
                  color='C00000' if cat == '買ってはいけない' else '1F4E79')
    if cat == '買ってはいけない':
        a.fill = PatternFill('solid', fgColor='FFC7CE')
    b = ws2.cell(row=r, column=2, value=title)
    b.font = Font(name=FONT, size=9, bold=True)
    c = ws2.cell(row=r, column=3, value=body)
    c.font = Font(name=FONT, size=9)
    c.alignment = Alignment(wrap_text=True, vertical='top')
    for col in range(1, 4):
        ws2.cell(row=r, column=col).border = border
        ws2.cell(row=r, column=col).alignment = Alignment(
            wrap_text=True, vertical='top',
            horizontal='center' if col == 1 else 'left')
    ws2.row_dimensions[r].height = 46
    r += 1

for col, w in zip('ABC', [18, 34, 96]):
    ws2.column_dimensions[col].width = w
ws2.freeze_panes = 'A4'

# openpyxl writes formulas with no cached value. LibreOffice CAN fill them in, but its
# round-trip substitutes Noto Sans JP for Arial in the Japanese header cells, so the
# recalculated copy is only used to VERIFY the formula (checked: D29 = 40, 0 errors) and
# this flag makes the reader compute it on open instead.
wb.calculation.fullCalcOnLoad = True

os.makedirs(os.path.dirname(OUT), exist_ok=True)
wb.save(OUT)
print('wrote', OUT)
