DEBUG = False


def log(*args, **kwargs):

    if DEBUG:
        print(*args, **kwargs)


def try_delist(listed):

    for d in listed:

        if not d:
            return listed

    return ''.join(listed)


class pattern:

    def __init__(self, front_type=None, *pieces):

        self.front_type = front_type
        self.pieces = pieces

        self.arguments = pieces[front_type == OPERATOR::2]


def infix(*pieces):
    return pattern(ARGUMENT, *pieces)


def prefix(*pieces):
    return pattern(OPERATOR, *pieces)


ARGUMENT = True
OPERATOR = False


def match_pattern(p, sequence):

    pieces = p.pieces
    pieces_length = len(pieces)

    if pieces_length == 0:
        return ({}, 0) if len(sequence) == 0 else False

    if pieces_length == 1:

        if p.front_type == ARGUMENT:
            return {p.pieces[0]: try_delist(sequence)}, 0

        if len(sequence) != len(pieces[0]):
            return False

        for pp, d in zip(sequence, pieces[0]):

            if d != pp:
                return False

        return (), 0

    read = []
    view = []

    piece_index = 0
    index = 0

    current = None

    reading = p.front_type

    for d in sequence:

        log('->', d)

        future = pieces[piece_index + 1] if piece_index + 1 < pieces_length else None

        view.append(d)

        log('reading argument' if reading else 'reading operator')
        log('current:', current)
        log('future:', future)
        log('view:', view)
        log('read:', read)
        log('index:', index)

        if reading == ARGUMENT:
            log('looking for', future[0]) if future is not None else ...

            if len(view) > 1 and future is not None and d == future[0]:
                log('trying to match', future)

                log('reading...')
                read.append(view[:-1])
                view = [d]

                piece_index += 1
                current = future

                reading = OPERATOR

            else:
                log('reading as argument')

        if piece_index == pieces_length:
            log('overread trying to salvage')
            view = read.pop()

            found = False
            i = 1

            for i in range(1, len(view)):
                for c, s in zip(current[1:], view[i:]):

                    log('salv: ', c, s)

                    if c != s:
                        found = True
                        break

                if found:
                    break

            log('before view', view, 'read', read, 'index', index, 'i', i)

            read[-1] += view[:i]
            view = view[i:] + [d]
            index = i

            log('after view', view, 'read', read, 'index', index)

            piece_index -= 1
            reading = OPERATOR

        current = pieces[piece_index]

        if reading == OPERATOR:
            log('reading as operator')

            if len(view) == len(current):

                log('partial matched')

                log('full match')

                piece_index += 1

                log('reading...')
                read.append(view)
                view = []

                index = 0
                reading = ARGUMENT

            elif d == current[index]:
                log(d, 'matched at index', index)

                index += 1

            else:
                piece_index -= 1
                index = 0

                reading = ARGUMENT

                if not read:
                    log('hopeless, early exit')

                    return False

                print('rolling back last read...')
                view = read.pop() + view

    if view:
        read.append(view)

    log('read:', read)

    if reading == OPERATOR and len(read[-1]) < len(current):
        return False

    if len(read) != len(pieces):
        return False

    return {k: try_delist(a) for k, a in zip(p.arguments, read[p.front_type == OPERATOR::2])}, len(read[0]) if p.front_type == ARGUMENT else 0


class Syntax:

    def __init__(self, func, name, *patterns):

        if len(patterns) == 1 or not isinstance(patterns[-1], int):
            self.order = None

        else:
            *patterns, self.order = patterns

        self.patterns = patterns

        self.name = name

        self.func = func

    def __call__(self, *args, **kwargs):
        self.func(*args, **kwargs)


def syntax(name, *patterns):

    def creator(func):
        return Syntax(func, name, *patterns)

    return creator


def get_pattern_location_from_match(match):
    return match[1][1]


class parsed:

    def __init__(self, *values):
        self.values = values[0] if len(values) == 1 else values

    def __bool__(self):
        return False

    def __add__(self, other):

        if isinstance(other, parsed):
            return [self, other]

        return [self] + list(other)

    def __radd__(self, other):
        return list(other) + [self]

    def infix(self, left, right):
        return list(left) + [self] + list(right)

    def prextend(self, other):
        return list(other) + [self]

    def extend(self, other):
        if isinstance(other, parsed):
            return [self, other]

        return [self] + list(other)


class parser:

    def __init__(self, syntaxes=()):
        self.syntaxes = list(syntaxes)

        self.patterns = {}
        self.parsers = {}
        self.levels = {}

        self.precedence = []

    def register(self, s: syntax):
        self.syntaxes.append(s)

        if s.order not in self.levels:

            self.levels[s.order] = []
            self.precedence.append(s.order)

            self.precedence.sort(reverse=True)

        self.patterns[s.name] = s.patterns

        self.levels[s.order].append(s.name)
        self.parsers[s.name] = s.func

    def unregister(self, s: Syntax):
        self.syntaxes.remove(s)

        if s.order in self.levels and len(self.levels) == 1:
            del self.levels[s.order]
            self.precedence.remove(s.order)

        self.levels[s.order].remove(s.name)
        del self.parsers[s.name]
        del self.patterns[s.name]

    def parse(self, source, intent: Syntax = None):
        log(f'parsing {source}' + ' with intent ' + str(intent))

        if not source and source != '' and source != [] and source != ():
            return source.value

        if intent:

            for p in intent.patterns:
                match = match_pattern(p, source)

                if match:
                    return intent(*match[0])

        match = None

        for level in self.precedence:

            log(f'checking level {level}')

            matches = []

            result = None

            for op in self.levels[level]:

                log(f'checking operator {op}')

                for p in self.patterns[op]:
                    result = match_pattern(p, source)

                    if result:
                        break

                if result:
                    log('partially matched')
                    matches.append((op, result))
                    match = True
                    result = None

            if matches:
                match = tuple(sorted(matches, key=get_pattern_location_from_match))[0]
                break

        if not match:
            raise ValueError(f'Could not match "{source}" with any existing patterns.')

        log('fully matched')

        return self.parsers[match[0]](**match[1][0])

    __call__ = parse

