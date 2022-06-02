from parser import *


o2 = parser()
parse = o2.parse
register = o2.register


literal_number_characters = tuple('-.1234567890')
non_syntactic_characters = tuple(' \n')


def remove_non_syntactic_characters(sequence):
    return [d for d in sequence if d not in non_syntactic_characters]


def strip_non_syntactic_characters(sequence, reverse=False):

    if reverse:
        sequence.reverse()

    out = []

    stripping = sequence[0] in non_syntactic_characters

    for d in sequence:

        if stripping and d not in non_syntactic_characters:
            stripping = False

        if not stripping:
            out.append(d)

    if reverse:
        out.reverse()

        return out

    return strip_non_syntactic_characters(out, reverse=True)


def is_literal_number(value):

    for c in value:

        if c not in literal_number_characters:
            return False

    return True


@syntax('literal', infix('source'), -2)
def literal(source):

    source = strip_non_syntactic_characters(source)

    if is_literal_number(source):
        return '', '', f'create_internal_object({source})'

    return '', '', f'stack.get("{source}")'


@syntax('arguments', infix('left', ',', 'right'),  infix('left', ','))
def arguments(left, right=None):

    d1, c1, r1 = parse(left)

    if right:
        d2, c2, r2 = parse(right, arguments)

        a1 = isinstance(r1, list)
        a2 = isinstance(r2, list)

        if a1 and a2:
            return d1 + d2, c1 + c2, (r1, r2)

        if a1:
            r1.append(r2)

            return d1 + d2, c1 + c2, r1

        if a2:
            r2.append(r1)

            return d1 + d2, c1 + c2, r2

        return d1 + d2, c1 + c2, [r1, r2]

    return d1, c1, [r1]


def make_internal_array(items):

    name = 'internal_array' + str(id(items))

    return 'int[] ' + name + ' = {' + ','.join(items) + '};', name


@syntax('assign', infix('left', '=', 'right'), 5)
def assign(left, right):
    defs, ctx, rv = parse(right)

    return defs, ctx + f'stack.set("{left.strip()}", {rv});', ''


@syntax('newline', infix('left', '\n', 'right'), 7)
def newline(left, right):

    print('l', left, 'r', right)

    d, c, r = parse(left)

    definitions = [d]
    context = [c]
    returns = [r]

    line = []

    for d in right:

        if d == '\n':
            d, c, r = parse(line)

            definitions.append(d)
            context.append(c)
            returns.append(r)

            line.clear()

        else:
            line.append(d)

    if line:
        d, c, r = parse(line)

        definitions.append(d)
        context.append(c)
        returns.append(r)

    return '\n'.join(definitions), '\n'.join(context), ';'.join(returns)


@syntax('list', infix('left', '[]', 'right'), infix('left', '[]'),
                prefix('[]', 'right'), infix('left', '[', 'middle', ']', 'right'),
                infix('left', '[', 'middle', ']'), prefix('[', 'middle', ']', 'right'), 8)
def list_literal(left='', middle='', right=''):

    d = ''
    c = ''
    name = ''

    if remove_non_syntactic_characters(middle):

        d, c, r = parse(middle, arguments)
        c2, name = make_internal_array(r)
        c += c2

    return parse(parsed((d, c, f'create_internal_object(InternalList({name}))')).infix(left, right))


@syntax('nothing', pattern(), 9)
def nothing():
    return '', '', ''


@syntax('c++', prefix('/', 'stmt', ';'), 10)
def direct_cpp(stmt):
    return '', stmt, ''


register(assign)
register(nothing)
register(literal)
register(newline)
register(list_literal)



print('\n'.join(parse(s)))
