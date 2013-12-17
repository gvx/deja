import re

__all__ = ['quote', 'unquote']

quoting = [
    ('\\\\', '\\'),
    ('\\t', '\t'),
    ('\\n', '\n'),
    ('\\r', '\r'),
    ('\\q', '"'),
]

def quote(s):
    for rep, c in quoting:
        s = s.replace(c, rep)
    return s

escapes = re.compile(r'(?P<single>\\[\\tnrq])|\\\{(?P<ord>\d+)\}')
single_escapes = dict(quoting)
def _quotesub(match):
    if match.group('single'):
        return single_escapes[match.group('single')]
    return unichr(int(match.group('ord'))).encode('utf-8')

def unquote(s):
    return escapes.sub(_quotesub, s)
