import pprint
import uuid

class Symbol:
    def __init__(self, value):
        self.value = value
    def compile(self, in_tail_pos):
        return [
            (Symbol('load'), Symbol(self.value))
        ]
    def __repr__(self):
        return self.value
    def __str__(self):
        return self.value
class Program:
    def __init__(self, expr):
        self.expr = expr
    def compile(self, in_tail_pos):
        bc = self.expr.compile(in_tail_pos)
        bc.append((Symbol('pop'),))
        return bc
class Sequence:
    def __init__(self, exprs):
        self.exprs = exprs
    def compile(self, in_tal_pos):
        bc = []
        for idx, expr in enumerate(self.exprs):
            if idx < len(self.exprs) - 1:
                # intermediate ones are not ever in tail position
                bc += expr.compile(False)
                bc.append((Symbol('pop'),))
            else:
                # if this is in tail position, then it's only in tail pos
                # if the parent is also
                bc += expr.compile(in_tal_pos)
        return bc
class Literal:
    def __init__(self, value):
        self.value = value 
    def compile(self, _in_tail_pos):
        return [(Symbol('literal'), self.value)]
class Define:
    def __init__(self, symbol, expr):
        self.symbol = symbol
        self.expr = expr
    def compile(self, _in_tail_pos):
        bc = self.expr.compile(False) # never in tail pos
        bc.append((Symbol('define'), self.symbol))
        return bc
class Set:
    def __init__(self, symbol, expr):
        self.symbol = symbol
        self.expr = expr
    def compile(self, _in_tail_pos):
        bc = self.expr.compile(False) # never in tail pos
        bc.append((Symbol('set'), self.symbol))
        return bc
class Lambda:
    def __init__(self, params, expr):
        self.params = params
        self.expr = expr
    def compile(self, _in_tail_pos):
        inner = self.expr.compile(True) # lambda has one expression, always in
        inner.append((Symbol('return'),))
        return [
            (Symbol('lambda'), self.params, inner)
        ]
class If:
    def __init__(self, test, ontrue, onfalse):
        self.test = test
        self.ontrue = ontrue
        self.onfalse = onfalse
    def compile(self, in_tail_pos):
        bc = []
        test = self.test.compile(False)
        ontrue = self.ontrue.compile(in_tail_pos)
        onfalse = self.onfalse.compile(in_tail_pos)
        ontrue.append((Symbol('jump'), len(onfalse) + 1))
        test.append((Symbol('jumpiffalse'), len(ontrue) + 1))
        bc += test
        bc += ontrue
        bc += onfalse
        return bc
class Invoke:
    def __init__(self, exprs):
        self.exprs = exprs
    def compile(self, in_tail_pos):
        bc = []
        for expr in self.exprs:
            bc += expr.compile(False)
        if in_tail_pos:
            bc.append((Symbol('invoketail'), len(self.exprs)))
        else:
            bc.append((Symbol('invoke'), len(self.exprs)))
        return bc

class SourceTransform:
    def __init__(self, source, lines, variable):
        self._source = source
        self._lines = lines
        self._variable = variable
    @property
    def lines(self):
        return self._lines
    @property
    def variable(self):
        return self._variable
    @property
    def source(self):
        return self._source
    def __repr__(self):
        return str(self)
    def __str__(self):
        return str(self.__dict__)

def randomize(var):
    u = uuid.uuid4()
    s = str(u)
    return f"{var}{s.replace('-', '')}"

def transpile_to_vector(data):
    variable = randomize('vector')
    lines = [
        f"{variable} = Vector({len(data)})",
    ]
    for idx, i in enumerate(data):
        itransform = transpile(i)
        lines += itransform.lines
        lines += [
            f"{variable}[{idx}] = {itransform.variable}"
        ]
    return SourceTransform(data, lines, variable)

def transpile_to_list(data):
    variable = randomize('list')
    current = randomize('current')
    lines = [
        f"{variable} = Pair()",
        f"{current} = {variable}"
    ]
    for idx, i in enumerate(data):
        itransform = transpile(i)
        lines += itransform.lines
        lines += [
            f"{current}.first = {itransform.variable}"
        ]
        if idx < len(data) - 1:
            lines += [
                f"{current}.second = Pair()",
                f"{current} = {current}.second"
            ]
    return SourceTransform(data, lines, variable)

def transpile_to_string(data):
    variable = randomize('str')
    lines = [
        f"{variable} = '{str(data)}'"
    ]
    return SourceTransform(data, lines, variable)

def transpile_to_int(data):
    variable = randomize('int')
    lines = [
        f"{variable} = {str(data)}"
    ]
    return SourceTransform(data, lines, variable)

def transpile_to_symbol(data):
    variable = randomize('symbol')
    lines = [
        f"{variable} = rt.intern('{str(data)}')"
    ]
    return SourceTransform(data, lines, variable)

def transpile_to_bool(data):
    variable = randomize('bool')
    lines = [
        f"{variable} = {str(data)}"
    ]
    return SourceTransform(data, lines, variable)

def transpile(data):
    transforms = {
        list: transpile_to_vector,
        tuple: transpile_to_list,
        str: transpile_to_string,
        int: transpile_to_int,
        Symbol: transpile_to_symbol,
        bool: transpile_to_bool,
    }

    datatype = type(data)
    return transforms[datatype](data)

def transpile_to_module(name, ast):
    compiled = ast.compile(True)
    pprint.pprint(compiled)
    transpiled = transpile(compiled)
    lines = transpiled.lines
    result = [
        'def create(rt):'
    ]
    result += [ f'  {line}' for line in lines ]
    result += [ f'  return {transpiled.variable}' ]
    result += [f'source = "{transpiled.source}"']
    concated = '\n'.join(result)
    #print(concated)
    with open(f'./transpiled/{name}.py', 'w') as out:
        out.write(concated)


def main():
    """
    (define (factorial n)
        (if (= n 0)
            1
            (* n (factorial (- n 1)))))

    (factorial 5)
    """

    ast = Program(
        Sequence([
            Define(
                Symbol('factorial'), 
                Lambda(
                    [Symbol('n')], 
                    Sequence([
                        If(
                            Invoke([
                                Symbol("="), Symbol("n"), Literal(0)
                            ]),
                            Literal(1),
                            Invoke([
                                Symbol("*"), Symbol("n"), Invoke([
                                    Symbol("factorial"), Invoke([
                                        Symbol("-"), Symbol("n"), Literal(1)
                                    ])
                                ])
                            ])
                        )
                    ])
                )
            ),
            Invoke([
                Symbol('factorial'), Literal(5)
            ])
        ])
    )

    transpile_to_module('factorial', ast)

if __name__ == '__main__':
    main()