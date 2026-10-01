# -*- coding: utf-8 -*-
"""Minimal s-expression reader/writer for KiCad files."""


def parse(text):
    """-> nested lists; atoms are str (quoted strings keep a leading '\"' marker)."""
    out, stack, i, n = [], [], 0, len(text)
    cur = out
    while i < n:
        c = text[i]
        if c == '(':
            new = []
            cur.append(new)
            stack.append(cur)
            cur = new
            i += 1
        elif c == ')':
            cur = stack.pop()
            i += 1
        elif c == '"':
            j = i + 1
            buf = []
            while j < n:
                if text[j] == '\\':
                    buf.append(text[j + 1]); j += 2; continue
                if text[j] == '"':
                    break
                buf.append(text[j]); j += 1
            cur.append('"' + ''.join(buf))
            i = j + 1
        elif c in ' \t\r\n':
            i += 1
        else:
            j = i
            while j < n and text[j] not in ' \t\r\n()"':
                j += 1
            cur.append(text[i:j])
            i = j
    return out


def q(s):
    """Quote a value for output."""
    return '"' + str(s).replace('\\', '\\\\').replace('"', '\\"') + '"'


def val(a):
    """Strip the quoted marker."""
    return a[1:] if isinstance(a, str) and a.startswith('"') else a


def find(node, key):
    return [c for c in node if isinstance(c, list) and c and c[0] == key]


def first(node, key):
    f = find(node, key)
    return f[0] if f else None


def dump(node, indent=0):
    if not isinstance(node, list):
        return str(node) if not str(node).startswith('"') else q(node[1:])
    head = node[0] if node else ''
    parts = [dump(c, indent + 1) for c in node[1:]]
    simple = all(not isinstance(c, list) for c in node[1:])
    if simple:
        return '(' + ' '.join([str(head)] + parts) + ')'
    pad = '\t' * (indent + 1)
    body = ''.join('\n' + pad + p for p in parts)
    return '(' + str(head) + body + '\n' + '\t' * indent + ')'
