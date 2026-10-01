# -*- coding: utf-8 -*-
"""Read KiCad symbol libraries, resolve `extends`, and expose pin geometry."""
import os
from sexp import parse, find, first, val

SYM = r"D:\Program Files\KiCad\10.0\share\kicad\symbols"
LOCAL = r"E:\Claude\MotoRecoPico\cad\kicad\MotoRecoPico.kicad_sym"
_LIBS = {}


def _load(libname):
    if libname not in _LIBS:
        path = LOCAL if libname == 'MotoRecoPico' else os.path.join(
            SYM, libname + ".kicad_sym")
        txt = open(path, encoding='utf-8', errors='replace').read()
        _LIBS[libname] = {val(s[1]): s for s in find(parse(txt)[0], 'symbol')}
    return _LIBS[libname]


def definition(lib_id):
    """Full symbol node, with any `extends` folded in, renamed to 'Lib:Name'."""
    libname, name = lib_id.split(':', 1)
    table = _load(libname)
    sym = table[name]
    ext = first(sym, 'extends')
    if ext:
        parent = definition(f"{libname}:{val(ext[1])}")
        # keep this symbol's own properties, take the parent's drawing units
        out = ['symbol', '"' + lib_id]
        seen = set()
        for c in sym[2:]:
            if isinstance(c, list) and c[0] == 'extends':
                continue
            if isinstance(c, list) and c[0] == 'property':
                seen.add(val(c[1]))
            out.append(c)
        for c in parent[2:]:
            if isinstance(c, list) and c[0] == 'property' and val(c[1]) in seen:
                continue
            if isinstance(c, list) and c[0] == 'symbol':
                # rename the child unit so it belongs to this symbol
                child = list(c)
                tail = val(child[1]).split('_')[-2:]
                child[1] = '"' + name + '_' + '_'.join(tail)
                out.append(child)
            elif isinstance(c, list) and c[0] not in ('property',):
                out.append(c)
            elif isinstance(c, list):
                out.append(c)
        return out
    out = list(sym)
    out[1] = '"' + lib_id
    return out


def pins(lib_id):
    """-> [(number, name, x, y, rotation, electrical_type)] in symbol coordinates."""
    d = definition(lib_id)
    out = []
    for sub in find(d, 'symbol'):
        for p in find(sub, 'pin'):
            at = first(p, 'at')
            out.append((val(first(p, 'number')[1]), val(first(p, 'name')[1]),
                        float(at[1]), float(at[2]), float(at[3]), p[1]))
    return out
