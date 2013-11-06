import struct
from bytecode import *
from collections import defaultdict
from strquot import quote

DECODE_OPCODES = {}
for k in OPCODES:
    DECODE_OPCODES[OPCODES[k] / 0x1000000] = k

ops_with_arg = valued_opcodes | set(['LINE_NUMBER', 'PUSH_INTEGER'])

def d_signed_int(x):
    if x[0] >= '\x80':
        x = '\xff' + x
    else:
        x = '\x00' + x
    return signed_int_s.unpack(x)[0]

def d_double(x):
    return str(double_s.unpack(x)[0])

def op_arg(chunk):
    return DECODE_OPCODES[ord(chunk[0])], d_signed_int(chunk[1:])

def find_labels(code):
    lbls = {}
    count = 0
    for i, (op, arg) in enumerate(code):
        if op not in positional_instructions:
            continue
        loc = i + arg
        if loc not in lbls:
            count += 1
            lbls[loc] = '$' + op[0] + str(count)
    return lbls

def make_line_00(i, op, arg, labels):
    #prefix = labels[i] + '\n' if i in labels else ''
    prefix = '{0:6}'.format(labels.get(i, ''))
    if op in positional_instructions:
        arg = labels[i + arg]
    elif op not in ops_with_arg:
        arg = ''
    return prefix + op.lower() + ' ' + str(arg)

def unsigned_int(x):
    return unsigned_int_s.unpack(x)[0]

def get_literals(code):
    i = 0
    while i < len(code):
        s = code[i]
        if s == '\x00':
            length = unsigned_int(code[i+1:i+5])
            yield "i" + code[i+5:i+5 + length]
            i += 5 + length
        elif s == '\x01':
            length = unsigned_int(code[i+1:i+5])
            yield "s" + quote(code[i+5:i+5 + length])
            i += 5 + length
        elif s == '\x80':
            length = ord(code[i+1])
            yield "i" + code[i+2:i+2 + length]
            i += 1 + length
        elif s == '\x81':
            length = ord(code[i+1])
            yield "s" + quote(code[i+2:i+2 + length])
            i += 1 + length
        elif s == '\x02':
            yield 'n' + d_double(code[i+1:i+9])
            i += 9
        elif s == '\x82':
            yield 'n' + d_signed_int(code[i+1:i+4])
            i += 4
        elif s == '\x07':
            n = signed_long_int(code[i+1:i+9])
            d = signed_long_int(code[i+9:i+17])
            yield 'f' + str(n) + '/' + str(d)
            i += 17
        elif s == '\x87':
            n = signed_char(code[i+1])
            d = ord(code[i+2])
            yield 'f' + str(n) + '/' + str(d)
            i += 3
        i += 1

def dis_00(bc):
    if len(bc) < 4:
        raise Exception("Code file too short")
    size = unsigned_int(bc[:4])
    bc = bc[4:]
    code = [op_arg(bc[j * 4:j * 4 + 4]) for j in range(size)]
    labels = find_labels(code)
    literals = bc[size * 4:]
    return '\n'.join(make_line_00(i, op, arg, labels) for i, (op, arg) in enumerate(code)) + '\n...\n' + '\n'.join(get_literals(literals)) + '\n'

def dis(bc):
    if not bc.startswith('\x07DV'):
        raise Exception("Not a Deja Vu byte code file.")
    elif bc[3] in '\x00\x01\x02\x03':
        return dis_00(bc[4:])
    else:
        raise Exception("Byte code version not recognised.")

if __name__ == '__main__':
    import sys
    sys.stdout.write(dis(sys.stdin.read()))
