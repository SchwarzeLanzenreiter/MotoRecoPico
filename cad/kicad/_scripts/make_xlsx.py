# -*- coding: utf-8 -*-
"""Build the parts-sourcing workbook from docs/parts-sourcing.md's verified links."""
import os
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter

OUT = r"E:\Claude\MotoRecoPico\docs\MotoRecoPico-parts-sourcing.xlsx"
FONT = 'Arial'

AKI = 'https://akizukidenshi.com/catalog/g/g{}/'
MARU_Q = 'https://www.marutsu.co.jp/GoodsListNavi.jsp?q={}'

# category, part/value, package, qty, used in, Akizuki(text,url), Marutsu, Mouser, DigiKey, note
N = (None, None)
ROWS = [
    ('Resistor', '100kOhm 1/10W', '1608 (0603)', 3, 'R1, R2, R3',
     ('pack of 100, Y150 (130357)', AKI.format('130357')),
     ('RC0603FR-07100KL', MARU_Q.format('RC0603FR-07100KL')), N, N, ''),
    ('Resistor', '22kOhm 1/10W', '1608 (0603)', 1, 'R4',
     ('no small pack', None),
     ('RC0603FR-0722KL', MARU_Q.format('RC0603FR-0722KL')), N, N,
     'Akizuki does not carry 1608 22kOhm in a small pack'),
    ('Resistor', '10kOhm 1/10W', '1608 (0603)', 7, 'R6, R7, R10-R13, R15 (IG sense)',
     ('pack of 100, Y150 (130355)', AKI.format('130355')),
     ('RC0603FR-0710KL', MARU_Q.format('RC0603FR-0710KL')), N, N, ''),
    ('Resistor', '120Ohm 1/10W', '1608 (0603)', 1, 'R8 (CAN termination)',
     ('reel of 5000, Y980 (130332)', AKI.format('130332')),
     ('RC0603FR-07120RL', MARU_Q.format('RC0603FR-07120RL')), N, N,
     'Akizuki has reels only. Only one is needed, so Marutsu is recommended'),
    ('Resistor', '1kOhm 1/10W', '1608 (0603)', 1, 'R14',
     ('reel of 5000, Y980 (114122)', AKI.format('114122')),
     ('RC0603FR-071KL', MARU_Q.format('RC0603FR-071KL')), N, N,
     'Akizuki has reels only. Only one is needed, so Marutsu is recommended'),

    ('Capacitor', '100nF / 50V or more, X7R', '1608 (0603)', 5, 'C2-C5, C10',
     ('0.1uF 100V X7R (114559)', AKI.format('114559')),
     ('CC0603KRX7R9BB104', MARU_Q.format('CC0603KRX7R9BB104')), N, N,
     '* Do not buy the F characteristic (113374). It is equivalent to Y5V and moves -82%'),
    ('Capacitor', '22pF / 50V C0G(NP0)', '1608 (0603)', 2, 'C7, C8 (crystal load)',
     ('pack of 40, Y100 (P-11626)', 'https://www.akizukidenshi.com/catalog/g/gP-11626/'),
     ('CC0603JRNPO9BN220', MARU_Q.format('CC0603JRNPO9BN220')), N, N,
     'Shown as out of stock at Akizuki when this was researched'),
    ('Capacitor', '10uF / 25V or more', '1608 (0603)', 3, 'C6, C11, C13 (Q2 gate delay)',
     ('35V (113161) / 16V (117202)', AKI.format('113161')),
     ('GRM188R61E106KA73D', MARU_Q.format('GRM188R61E106KA73D')),
     ('GRM188R61E106KA73D',
      'https://www.mouser.com/ProductDetail/Murata-Electronics/GRM188R61E106KA73D?qs=5aG0NVq1C4z8ebxPkQZf9A%3D%3D'),
     ('GRM188R61E106KA73D',
      'https://www.digikey.com/en/products/detail/murata-electronics/GRM188R61E106KA73D/9867922'),
     'C13 sees about 13V during a load dump, so 25V or more is mandatory. '
     'Easiest to use the same 25V/35V part for all three'),

    ('Diode', 'B5819W (40V/1A Schottky)', 'SOD-123', 2, 'D1, D3',
     ('not carried', None),
     ('1N5819HW-7-F', MARU_Q.format('1N5819HW-7-F')),
     ('1N5819HW-7-F',
      'https://www.mouser.com/ProductDetail/Diodes-Incorporated/1N5819HW-7-F?qs=NQ47qNm99eDyWTEd07miYA%3D%3D'),
     ('1N5819HW-7-F',
      'https://www.digikey.com/product-detail/en/1N5819HW-7-F/1N5819HW-FDICT-ND/815283'),
     '* Akizuki\'s SS14 is an SMA package and will not fit'),
    ('Diode', 'SMAJ16A (TVS 400W)', 'SMA (DO-214AC)', 1, 'D2',
     ('not carried', None),
     ('Littelfuse SMAJ16A', 'https://www.marutsu.co.jp/pc/i/2079887/'),
     ('SMAJ16A', 'https://www.mouser.com/'),
     ('Littelfuse SMAJ16A',
      'https://www.digikey.com/en/products/detail/littelfuse-inc/SMAJ16A/762278'),
     ''),
# D4 (3.3V zener, ADC clamp) left the BOM in v2 with the IG sense circuit.
    ('LED', 'LP812-010(T) green, side view', '2.8x1.2mm', 1, 'D6',
     ('pack of 10, Y100 (116854)', AKI.format('116854')),
     ('not carried', None), N, N,
     '* Only available from Akizuki'),
    ('Circuit protection', 'PPTC 1.1A hold 16V', '1206', 1, 'F1',
     ('not carried', None),
     ('1206L110/16WR', MARU_Q.format('1206L110')),
     ('1206L110/16WR', 'https://www.mouser.com/'),
     ('1206L110/16WR',
      'https://www.digikey.com/en/products/detail/littelfuse-inc/1206L110-16WR/3997167'),
     '* Do not confuse it with the 6V (SLYR) or 8V (THYR) parts'),
    ('Transistor', 'MMBT3904 (NPN)', 'SOT-23', 1, 'Q1',
     ('pack of 20, Y100 (105969)', AKI.format('105969')),
     ('MMBT3904', MARU_Q.format('MMBT3904')),
     ('onsemi MMBT3904',
      'https://www.mouser.com/ProductDetail/onsemi/MMBT3904?qs=UMEuL5FsraCg3xKjS4uBJQ%3D%3D'),
     ('onsemi MMBT3904',
      'https://www.digikey.com/en/products/detail/onsemi/MMBT3904/458868'),
     ''),
    ('MOSFET', 'ZXMP6A17E6Q (P-ch -60V)', 'SOT-23-6', 1, 'Q2 (high-side switch)',
     ('not carried', None),
     ('ZXMP6A17E6QTA', MARU_Q.format('ZXMP6A17E6QTA')),
     ('ZXMP6A17E6QTA',
      'https://www.mouser.com/ProductDetail/Diodes-Incorporated/ZXMP6A17E6QTA?qs=JtunSoW0QGE6mLYIV79weA%3D%3D'),
     ('ZXMP6A17E6QTA',
      'https://www.digikey.com/en/products/detail/diodes-incorporated/ZXMP6A17E6QTA/4810942'),
     ''),

    ('Power IC', 'AP63205WU-7 (fixed 5V, 2A buck)', 'TSOT-26', 1, 'U4 (v4)',
     ('not carried', None),
     ('AP63205WU-7', MARU_Q.format('AP63205WU-7')),
     ('AP63205WU-7',
      'https://www.mouser.com/ProductDetail/Diodes-Incorporated/AP63205WU-7?qs=u16ybLDytRZtkj8PzdWCOw%3D%3D'),
     ('AP63205WU-7',
      'https://www.digikey.com/en/products/detail/diodes-incorporated/AP63205WU-7/9858424'),
     'Replaced the M78AR05 in v4. Its 3.8V minimum input also resolved the cranking issue'),
    ('Inductor', '6.8uH 4030, saturation >=1.5A', '4x4x3mm', 1, 'L1 (v4)',
     ('not carried', None),
     ('ASPI-4030S-6R8M', MARU_Q.format('ASPI-4030S-6R8M')),
     ('ASPI-4030S-6R8M-T',
      'https://www.mouser.com/ProductDetail/ABRACON/ASPI-4030S-6R8M-T?qs=BLmuIjeT3qE8jeo8oUT4Iw%3D%3D'),
     N, 'A shielded part is recommended. The tallest part on the back of the board (3mm)'),
    ('Capacitor', '10uF / 50V X7R', '3216 (1206)', 1, 'C14 buck input (v4)',
     ('not carried', None),
     ('GRM31CR71H106KA12', MARU_Q.format('GRM31CR71H106KA12')),
     ('810-CNA5L171H106K160',
      'https://www.mouser.jp/en/ProductDetail/TDK/CNA5L1X7R1H106K160AE?qs=Y0Uzf4wQF3kvAkifwkQiJA%3D%3D'),
     N,
     '* The input sees the 26V clamp directly, so 50V is mandatory. A 25V part will not do'),
    ('Capacitor', '22uF / 16V', '2012 (0805)', 1, 'C15 buck output (v4)',
     ('not carried', None),
     ('GRM21BR61C226ME44', MARU_Q.format('GRM21BR61C226ME44')),
     ('81-GXT21BR61C226ME3L',
      'https://www.mouser.jp/en/ProductDetail/Murata/GXT21BR61C226ME13L?qs=d0WKAl%252BL4Kb74FO4906dbA%3D%3D'),
     N, ''),
    ('Capacitor', '100nF / 50V X7R', '2012 (0805)', 1, 'C16 bootstrap (v4)',
     ('not carried', None),
     ('0805 100nF 50V', MARU_Q.format('GRM21BR71H104KA01')),
     ('581-KAF21BR71H104KM',
      'https://www.mouser.jp/en/ProductDetail/KYOCERA-AVX/KAF21BR71H104KM?qs=Jm2GQyTW%2FbhmrEcuwq3xrw%3D%3D'),
     N,
     '2012 rather than 1608 to keep the "all 1608 on the front" rule (this one is on the back)'),
    ('IC', 'MCP25625T-E/SS (CAN)', 'SSOP-28 0.65mm', 1, 'U2',
     ('Y400 (112663)', AKI.format('112663')),
     ('MCP25625', MARU_Q.format('MCP25625')),
     ('MCP25625T-E/SS',
      'https://www.mouser.com/ProductDetail/Microchip-Technology/MCP25625T-E-SS?qs=YlpJWyuQ3PP8xO0AA8P3ZQ%3D%3D'),
     ('MCP25625T-E/SS',
      'https://www.digikey.com/en/products/detail/microchip-technology/MCP25625T-E-SS/4860100'),
     '* Cheapest at Akizuki (DigiKey is $3.38). The hardest part to hand solder'),
    ('MCU', 'Raspberry Pi Pico 2', '2x20 2.54mm', 1, 'U3',
     ('Pico 2 H, headers fitted (130982)', AKI.format('130982')),
     ('Raspberry Pi Pico 2', MARU_Q.format('Raspberry+Pi+Pico+2')), N, N,
     'Without headers it is (129604). The H version saves fitting the headers yourself'),

    ('Connector', 'TE 1-967658-1 (MQS 8 way RA)', 'THT right angle', 1, 'J1 (vehicle I/F)',
     ('not carried', None),
     ('1-967658-1', MARU_Q.format('1-967658-1')),
     ('1-967658-1',
      'https://www.mouser.jp/en/ProductDetail/TE-Connectivity-AMP/1-967658-1?qs=6Z%252BHCgJKjncTVf7nPH%2FL8g%3D%3D'),
     ('DigiKey Marketplace',
      'https://www.digikey.be/en/products/detail/te-connectivity-amp-connectors/1-967658-1/10478177'),
     '* The hardest part to obtain. DigiKey sells it through a third party. Confirm stock before ordering'),
    ('Connector', 'DM3AT-SF-PEJM5 (microSD)', 'SMD right angle', 1, 'J2',
     ('not carried', None),
     ('Hirose DM3AT-SF-PEJM5', 'https://www.marutsu.co.jp/pc/i/2571574/'),
     ('DM3AT-SF-PEJM5',
      'https://www.mouser.com/ProductDetail/Hirose-Connector/DM3AT-SF-PEJM5?qs=LZSZKJVF+2WTDKp+R7IYAQ%3D%3D'),
     ('DM3AT-SF-PEJM5',
      'https://www.digikey.com/en/products/detail/hirose-electric-co-ltd/DM3AT-SF-PEJM5/2533565'),
     ''),
    ('Module', 'GT-502MGG-N (GPS)', '5 bare wires', 1, 'J3',
     ('Y3,180 (117980)', AKI.format('117980')),
     ('GT-502MGG-N', MARU_Q.format('GT-502MGG-N')), N,
     ('YIC GT-502MGG-N',
      'https://www.digikey.jp/ja/products/detail/yic/GT-502MGG-N/16499054'),
     'Cheapest at Akizuki'),
    ('Crystal', '16MHz HC-49/S CL=16-20pF', 'THT', 1, 'Y1',
     ('CL=20pF (108671)', AKI.format('108671')),
     ('16MHz HC-49', MARU_Q.format('16MHz+HC-49')), N, N,
     '* CL=12pF and CL=32pF parts will not do (because C7/C8 are 22pF)'),

    ('[not in BOM]', '1x20 female pin socket, 2.54mm', 'THT', 2, 'the socket for U3',
     ('breakable long pin socket 1x42 (105779)', AKI.format('105779')),
     N, N, N,
     'The design socket-mounts the Pico, so the board needs these. Cut down the 1x42 strip'),
]

HDR = ['Category', 'Part / value', 'Package', 'Qty', 'Used in',
       'Akizuki Denshi', 'Marutsu', 'Mouser', 'DigiKey', 'Note', 'Ordered']

wb = Workbook()

# ------------------------------------------------------------------- sourcing
ws = wb.active
ws.title = 'Sourcing'

TITLE = 'MotoRecoPico parts sourcing list'
ws['A1'] = TITLE
ws['A1'].font = Font(name=FONT, size=14, bold=True)
ws['A2'] = ('All 41 parts (v4: the 5V supply moved to the AP63205), grouped by value into '
            '25 line items plus the sockets. Links are only given for pages whose existence '
            'was confirmed. A blank cell means it could not be confirmed at that vendor '
            '(not that it does not exist).')
ws['A2'].font = Font(name=FONT, size=9, italic=True, color='555555')
ws['A3'] = ('Marutsu is the only authorized DigiKey distributor in Japan: every DigiKey line item, '
            'one at a time, at the same price, with flat Y240 shipping nationwide (free above '
            'Y3,000). Akizuki plus Marutsu covers the whole list.')
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
    nc.font = WARN if note.startswith('*') else PLAIN
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
tot = ws.cell(row=r, column=3, value='Total')
tot.font = Font(name=FONT, size=10, bold=True)
tot.alignment = Alignment(horizontal='right')
tc = ws.cell(row=r, column=4, value=f'=SUM(D{HDR_ROW+1}:D{LAST})')
tc.font = Font(name=FONT, size=10, bold=True)
tc.border = border
tc.alignment = Alignment(horizontal='center')
ws.cell(row=r, column=5, value='pieces (38 in the BOM + 2 socket strips)').font = Font(
    name=FONT, size=9, italic=True, color='555555')

for col, w in zip('ABCDEFGHIJK',
                  [18, 32, 20, 6, 24, 32, 26, 22, 26, 60, 9]):
    ws.column_dimensions[col].width = w
ws.freeze_panes = f'A{HDR_ROW+1}'
ws.auto_filter.ref = f'A{HDR_ROW}:K{LAST}'
ws.row_dimensions[HDR_ROW].height = 22

# ------------------------------------------------------------------- cautions
ws2 = wb.create_sheet('Cautions')
ws2['A1'] = 'Cautions when ordering and assembling'
ws2['A1'].font = Font(name=FONT, size=14, bold=True)

DO_NOT_BUY = 'Do not buy'

NOTES = [
    (DO_NOT_BUY, 'SS14 for D1/D3',
     "Akizuki's SS14 (113240) is SMA (DO-214AC). The board land is SOD-123, so it physically "
     'will not fit. Buy the 1N5819HW-7-F (40V/1A, SOD-123).'),
    (DO_NOT_BUY, 'An "F characteristic" 0.1uF for C2-C5/C10/C12',
     "Akizuki's 0.1uF 50V F characteristic (113374) is the JIS F characteristic, equivalent to "
     'Y5V, and moves +22%/-82% over temperature. It cannot be used for decoupling. '
     'Choose X7R or X8L.'),
    (DO_NOT_BUY, 'A 6V or 8V part for F1',
     'The 1206L110 family also comes in 6V (1206L110SLYR) and 8V (1206L110THYR). Those cannot '
     "be used in a 12V vehicle. Specify the 1206L110/16WR. Akizuki's PICOSMDC110S-2 is 6V too."),
    ('Specification call', "F1's 16V rating is accepted",
     'For a 1206 PPTC with 1.1A hold, 16V is the ceiling. On top of that F1 is upstream of D2 '
     '(the TVS) (J1.8 -> F1 -> D1 -> D2), making it the only part that takes vehicle-side '
     'surges without them being clamped. A 24V/30V part would need a larger size such as 1812 '
     'and therefore a board change, so by the user\'s decision it stays at 16V (2026-07-31).'),
    ('v3 change', 'C13 (10uF/25V) and R15 (10k) were added',
     'v3 (2026-08-09) added the IGN OFF delay. C13 goes across Q2\'s gate and source and must be '
     'rated 25V or more; R15 is for IG sensing from Q1\'s base to GP28. That makes three 10uF '
     'parts and seven 10kOhm parts.'),
    ('v2 change', 'R5, D4 and C12 need not be bought',
     'v2 (2026-08-08) retired IG voltage sensing and removed R5 (one 10k), D4 (a 3.3V zener) '
     'and C12 (one 100nF). That brings 10kOhm down to six and 100nF down to five.'),
    ('Specification call', "Y1's load capacitance",
     'C7/C8 are 22pF, so choose a crystal with a load capacitance CL of 16-20pF. Akizuki\'s '
     '108671 is CL=20pF and fits. CL=12pF and CL=32pF parts will not do.'),
    ('Availability', 'J1 (the MQS connector) is the hardest',
     'DigiKey handles it through the Marketplace (a third-party seller), so stock and lead time '
     'differ from normal stock items. It is worth searching the part number at Marutsu and RS '
     'Components too. There is no substitute for this part, so always confirm stock before '
     'ordering.'),
    ('Availability', 'Parts only available from Akizuki',
     'D6 (the LP812-010(T) side-view LED) is a KODENSHI part for the Japanese market with no '
     'overseas distribution. J3 (the GT-502MGG-N) is on DigiKey too, but Akizuki is cheapest '
     '(Y3,180).'),
    ('Availability', "Akizuki's 1608 resistors come in few small packs",
     '10kOhm (130355) and 100kOhm (130357) come as packs of 100 for Y150, but 22kOhm, 120Ohm '
     'and 1kOhm only come as reels of 5000 (Y980). For those three it is more realistic to buy '
     'singles from Marutsu.'),
    ('Hand soldering', 'U2 MCP25625 (SSOP-28, 0.65mm pitch)',
     'The hardest. Flux and drag soldering are recommended.'),
    ('Hand soldering', 'J2 microSD connector',
     'Fine terminals, and the shell legs have a large thermal mass.'),
    ('Hand soldering', 'D6 side-view LED',
     'Positioning is critical because it sits on the board edge (y=61).'),
    ('v2 change', 'GND pads now use thermal relief',
     'v2 changed every zone to thermal relief (spoke 0.4mm / gap 0.3mm). The direct pour '
     'connection of v1 (where the GND pins alone sank an absurd amount of heat) is resolved, '
     'and a normal iron tip temperature works.'),
    ('Not in the BOM', 'The Pico needs sockets',
     'The design socket-mounts the Pico, so the board needs two 1x20 female pin sockets. Cut '
     'down a breakable long pin socket 1x42 (105779). Male headers on the Pico side are not '
     'needed if you buy the Pico 2 H (130982), which has them fitted.'),
]

H2 = ['Category', 'Item', 'Details']
for c, h in enumerate(H2, start=1):
    cell = ws2.cell(row=3, column=c, value=h)
    cell.font = Font(name=FONT, size=10, bold=True, color='FFFFFF')
    cell.fill = hdr_fill
    cell.alignment = Alignment(horizontal='center', vertical='center')
    cell.border = border

r = 4
for cat, title, body in NOTES:
    a = ws2.cell(row=r, column=1, value=cat)
    a.font = Font(name=FONT, size=9, bold=True,
                  color='C00000' if cat == DO_NOT_BUY else '1F4E79')
    if cat == DO_NOT_BUY:
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

for col, w in zip('ABC', [20, 40, 96]):
    ws2.column_dimensions[col].width = w
ws2.freeze_panes = 'A4'

# openpyxl writes formulas with no cached value, so a reader that does not calculate would
# show the total as blank. check_xlsx.py verifies the formula against a LibreOffice-
# recalculated copy (checked: D29 = 40, 0 errors), and this flag makes the reader compute
# it on open instead of relying on a cached value.
wb.calculation.fullCalcOnLoad = True

os.makedirs(os.path.dirname(OUT), exist_ok=True)
wb.save(OUT)
print('wrote', OUT)
