class Program:
    def __init__(self, expr):
        self.expr = expr
    def compile(self, in_tail_pos):
        bc = self.expr.compile(in_tail_pos)
        bc.append(('halt',))
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
                bc.append(('pop',))
            else:
                # if this is in tail position, then it's only in tail pos
                # if the parent is also
                bc += expr.compile(in_tal_pos)
        return bc
class Symbol:
    def __init__(self, value):
        self.value = value 
    def compile(self, _in_tail_pos):
        return [('load', self.value)]
class Literal:
    def __init__(self, value):
        self.value = value 
    def compile(self, _in_tail_pos):
        return [('literal', self.value)]
class Define:
    def __init__(self, symbol, expr):
        self.symbol = symbol
        self.expr = expr
    def compile(self, _in_tail_pos):
        bc = self.expr.compile(False) # never in tail pos
        bc.append(('define', self.symbol))
        return bc
class Set:
    def __init__(self, symbol, expr):
        self.symbol = symbol
        self.expr = expr
    def compile(self, _in_tail_pos):
        bc = self.expr.compile(False) # never in tail pos
        bc.append(('set', self.symbol))
        return bc
class Lambda:
    def __init__(self, params, expr):
        self.params = params
        self.expr = expr
    def compile(self, _in_tail_pos):
        inner = self.expr.compile(True) # lambda has one expression, always in
        inner.append(('return',))
        return [
            ('lambda', self.params, inner)
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
        ontrue.append(('jump', len(onfalse) + 1))
        test.append(('jumpiffalse', len(ontrue) + 1))
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
            bc.append(('invoketail', len(self.exprs)))
        else:
            bc.append(('invoke', len(self.exprs)))
        return bc

"""
(define (factorial n)
    (if (= n 0)
        1
        (* n (factorial (- n 1)))))

(factorial 5)
"""

compiled = Program(
    Sequence([
        Define('factorial', 
            Lambda(
                ['n'], 
                Sequence([
                    Define('forever', 
                        Lambda([], Invoke([ Symbol('forever') ])
                        )
                    ),
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
        )
    ])
)

import pprint

pprint.pprint(compiled.compile(True))