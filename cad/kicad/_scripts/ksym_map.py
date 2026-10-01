# -*- coding: utf-8 -*-
"""Which KiCad symbol stands in for each part, and in what left-to-right order."""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mrp_design as D
import ksym

SYMMAP = {
    'J1': 'Connector_Generic:Conn_01x08',
    'J2': 'Connector:Micro_SD_Card_Det_Hirose_DM3AT',
    'J3': 'Connector_Generic:Conn_01x05',
    'U4': 'Regulator_Switching:AP63205WU',
    'L1': 'Device:L',
    'U2': 'Interface_CAN_LIN:MCP25625x-x-SS',
    'U3': 'MCU_Module:RaspberryPi_Pico',
    'Q1': 'Transistor_BJT:MMBT3904',
    'Q2': 'MotoRecoPico:ZXMP6A17E6Q',
    'Y1': 'Device:Crystal',
    'F1': 'Device:Polyfuse',
    'JP1': 'Jumper:SolderJumper_2_Bridged',
    'D1': 'Device:D_Schottky',
    'D2': 'Diode:SMAJ16A',
    'D3': 'Device:D_Schottky',
    'D4': 'Device:D_Zener',
    'D5': 'Device:D_TVS_Dual_ACA',
    'D6': 'Device:LED',
}
for _r in D.PARTS:
    SYMMAP.setdefault(_r, 'Device:R' if _r.startswith('R') else 'Device:C')

ORDER = ['J1', 'F1', 'D2', 'R1', 'R2', 'Q1', 'Q2', 'D1',
         'R3', 'R4', 'R5', 'C12', 'D4', 'C2',
         'U4', 'C14', 'L1', 'C16', 'C15', 'D3', 'C6', 'C5', 'C4', 'C3',
         'U2', 'R6', 'R7', 'R8', 'R9', 'C9', 'JP1', 'D5', 'Y1', 'C7', 'C8',
         'U3', 'J2', 'R10', 'R11', 'R12', 'R13', 'C10', 'C11',
         'J3', 'R14', 'D6']

PINS = {r: ksym.pins(SYMMAP[r]) for r in D.PARTS}
PINNUMS = {r: {n for n, *_ in PINS[r]} for r in D.PARTS}


def netpins(net):
    """Intended (ref, symbol pin) set for a net, dropping pads the symbol has no
    pin for - the DM3AT symbol carries one SHIELD pin for four SH pads."""
    out = set()
    for ref, epin in D.NETS[net]:
        for pad in D.pads_of(ref, epin):
            if pad in PINNUMS[ref]:
                out.add((ref, pad))
    return out
