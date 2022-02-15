
class Nil:
    def __init__(self):
        pass
    def __str__(self):
        return 'nil'
    def __repr__(self):
        return str(self)

NIL = Nil()

class Symbol:
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return self.value
    def __repr__(self):
        return str(self)

class Vector:
    def __init__(self, length):
        self.data = length * [NIL]
    def __str__(self):
        return str(self.data)
    def __repr__(self):
        return str(self)
    def __getitem__(self, key):
        return self.data[key]
    def __setitem__(self, key, value):
        self.data[key] = value
    def __len__(self):
        return len(self.data)

class Pair:
    def __init__(self):
        self._first = NIL 
        self._second = NIL 
    @property
    def first(self):
        return self._first
    @first.setter
    def first(self, val):
        self._first = val
    @property
    def second(self):
        return self._second
    @second.setter
    def second(self, val):
        self._second = val
    def __str__(self):
        return f'({self.first} . {self.second})'
    def __repr__(self):
        return str(self)

class Environment:
    def __init__(self, outer):
        self._lookup = {}
        self.outer = outer
    def define(self, key, value):
        if type(key) is not Symbol:
            raise Exception('Can only define symbols')
        self._lookup[key] = value
    @property
    def locals(self):
        return self._lookup.keys()
    def lookup(self, sym):
        if type(sym) is not Symbol:
            raise Exception('Can only lookup symbols')
        e = self
        while e is not None:
            if sym in e._lookup:
                return e._lookup[sym]
            e = e.outer
        return NIL
    def __str__(self):
        return f'{{lookup: {self._lookup}, outer: {self.outer}}}'
    def __repr__(self):
        return str(self)

class Frame:
    def __init__(self, bc, env, outer):
        self.bc = bc
        self.pc = 0
        self.env = env
        self.outer = outer
        self.temps = []
    @property
    def locals(self):
        return self.env.locals
    def lookup(self, sym):
        return self.env.lookup(sym)
    def push(self, val):
        self.temps.append(val)
    def pop(self):
        return self.temps.pop()
    def __str__(self):
        return f'{{pc: {self.pc}, temps: {self.temps}, env: {self.env}, outer: {self.outer}}}'
    def __repr__(self):
        return str(self)
    def define(self, key, val):
        self.env.define(key, val)

class Lambda:
    def __init__(self, args, bc, env):
        self._args = args
        self._bc = bc
        self._env = env
    @property
    def args(self):
        return self._args
    @property
    def bc(self):
        return self._bc
    @property
    def env(self):
        return self._env
    def __str__(self):
        return f'(lambda)'
    def __repr__(self):
        return str(self)

class NativeFunction:
    def __init__(self, args, fn):
        self._args = args
        self._fn = fn
    @property
    def args(self):
        return self._args
    @property
    def fn(self):
        return self._fn
    def __str__(self):
        return '(nativefn)'
    def __repr__(self):
        return str(self)

class Continuation:
    def __init__(self, frame):
        self._frame = frame
    @property
    def frame(self):
        return self._frame
    def __str__(self):
        return '(continuation)'
    def __repr__(self):
        return str(self)

class Runtime:
    def __init__(self):
        self.symbols = {}
        self.globalenv = Environment(None)

        # intial interned symbols
        self._load = self.intern('load')
        self._define = self.intern('define')
        self._set = self.intern('set')
        self._invoke = self.intern('invoke')
        self._lambda_ = self.intern('lambda')
        self._literal = self.intern('literal')
        self._pop = self.intern('pop')
        self._invoketail = self.intern('invoketail')
        self._jumpiffalse = self.intern('jumpiffalse')
        self._jump = self.intern('jump')
        self._return_ = self.intern('return')

        self.dispatch = {
            self._load: self.load,
            self._define: self.define,
            self._set: self.set,
            self._invoke: self.invoke,
            self._lambda_: self.lambda_,
            self._literal: self.literal,
            self._pop: self.pop,
            self._invoketail: self.invoketail,
            self._jumpiffalse: self.jumpiffalse,
            self._jump: self.jump,
            self._return_: self.return_,
        }

        self.define_natives()

    def intern(self, string):
        if string not in self.symbols:
            self.symbols[string] = Symbol(string)
        return self.symbols[string]
        
    def load_transpiled(self, fn):
        file_name = f'./transpiled/{fn}.py'
        #print('Loading', file_name)
        with open(file_name) as f:
            #print('Executing', file_name)
            exec(f.read())
            #print('Executed', file_name)
            #print('Invoking', file_name)
            f = locals()['create']
            #print(f)
            result = f(self)
            #print('Invoked!', file_name)
            return result

    def import_transpiled(self, module):
        bc = self.load_transpiled(module)
        env = Environment(self.globalenv)
        frame = Frame(bc, env, None)
        self.evaluate(frame)
        for l in frame.locals:
            self.globalenv.define(l, frame.lookup(l))

    def frame_height(self, frame):
        ret = 0
        while frame is not None:
            ret += 1
            frame = frame.outer
        return ret

    def evaluate(self, frame):
        while frame.pc < len(frame.bc):
            current = frame.bc[frame.pc]
            op = current.first
            #print('Frame height is:', self.frame_height(frame))
            #print('Current bc is', op)
            #print('Frame before')
            #print(frame)
            frame = self.dispatch[op](frame, op, current)
            #print('Frame after')
            #print(frame)
            #print('\n')
            #input('Enter anything to continue...')

    def is_truthy(self, val):
        return type(val) is bool and val

    def load(self, frame, op, bc):
        frame.pc += 1
        sym = bc.second.first
        val = frame.lookup(sym)
        #print(f'Lookup of \'{sym}\' -> {val}')
        frame.push(val)
        return frame

    def define(self, frame, op, bc):
        frame.pc += 1
        val = frame.pop()
        name = bc.second.first
        #print(f'{name} <- {val}')
        frame.define(name, val)
        frame.push(NIL)
        return frame

    def set(self, frame, op, bc):
        frame.pc += 1
        raise Exception('todo')

    def invoke(self, frame, op, bc, returnframe=None):
        if returnframe is None:
            # implements tail recursion, if return frame
            # is different from the frame that is invoking
            # the method (eg: don't return to me, return to outer/don't push more stack frames)
            returnframe = frame
        frame.pc += 1
        #print('Temps:', frame.temps)
        num_items = bc.second.first
        #print('Invoke with', num_items)
        args = [NIL] * (num_items - 1)
        for i in reversed(range(len(args))):
            #print('Getting arg', i)
            arg = frame.pop()
            #print('Arg ', i, ' = ', arg)
            args[i] = arg
        reciever = frame.pop()
        #print('Reciever', reciever)
        if type(reciever) is Lambda:
            # create a closure
            innerframe = Frame(reciever.bc, Environment(reciever.env), returnframe)
            for argname, argval in zip(reciever.args, args):
                #print(f'Defining {argname} <- {argval}')
                innerframe.define(argname, argval)
            return innerframe
        elif type(reciever) is NativeFunction:
            innerenv = Environment(self.globalenv)
            for argname, argval in zip(reciever.args, args):
                #print(f'Defining {argname} <- {argval}')
                innerenv.define(argname, argval)
            frame = reciever.fn(frame, innerenv)
            #print('Native function returned ', result)
            return frame
        elif type(reciever) is Continuation:
            if len(args) != 1:
                raise Exception('Continuation takes 1 argument')
            frame = reciever.frame
            frame.push(args[0])
            return frame
        else:
            raise Exception(f'Cannot invoke {type(reciever)}')
        
    def invoketail(self, frame, op, bc):
        return self.invoke(frame, op, bc, returnframe=frame.outer)

    def jumpiffalse(self, frame, op, bc):
        loc = bc.second.first
        val = frame.pop()
        #print('Popped', val)
        if not self.is_truthy(val):
            #print('Value was false, jumping +', loc)
            frame.pc += loc
        else:
            #print('Value was true, advancing')
            frame.pc += 1
        return frame

    def jump(self, frame, op, bc):
        loc = bc.second.first
        #print('Unconditionally jumping +', loc)
        frame.pc += loc
        return frame

    def return_(self, frame, op, bc):
        result = frame.pop()
        #print('Returning', result)
        frame = frame.outer
        frame.push(result)
        return frame

    def lambda_(self, frame, op, bc):
        frame.pc += 1
        args = bc.second.first
        innerbc = bc.second.second.first
        l = Lambda(args, innerbc, frame.env)
        #print(l)
        frame.push(l)
        return frame

    def literal(self, frame, op, bc):
        frame.pc += 1
        value = bc.second.first
        #print('Pushing literal', value)
        frame.push(value)
        return frame

    def pop(self, frame, op, bc):
        frame.pc += 1
        discard = frame.pop()
        #print('Popped', discard)
        return frame

    def start(self):
        self.import_transpiled('tailrec')
        #print(self.globalenv)

    def __repr__(self):
        return str(self)

    def __str__(self):
        return f'({str(self.symbols)} {self.globalenv})'

    def define_natives(self):
        arg0 = self.intern('arg0')
        arg1 = self.intern('arg1')

        def equals(frame, env):
            arg1val = env.lookup(arg0)
            arg2val = env.lookup(arg1)
            if type(arg1val) != type(arg2val):
                frame.push(False)
            else:
                frame.push(arg1val == arg2val)
            return frame

        self.globalenv.define(self.intern('='), NativeFunction([arg0, arg1], equals))

        def define_binary_numeric(name, fn):
            def to_register(frame, env):
                arg0val = env.lookup(arg0)
                arg1val = env.lookup(arg1)
                if type(arg0val) is not int:
                    raise Exception('arg0 must be an integer')
                if type(arg1val) is not int:
                    raise Exception('arg1 must be an integer')
                frame.push(fn(arg0val, arg1val))
                return frame
            self.globalenv.define(self.intern(name), NativeFunction([arg0, arg1], to_register))

        define_binary_numeric('*', lambda a1, a2: a1 * a2)
        define_binary_numeric('/', lambda a1, a2: a1 / a2)
        define_binary_numeric('-', lambda a1, a2: a1 - a2)
        define_binary_numeric('+', lambda a1, a2: a1 + a2)

        def not_(frame, env):
            arg0val = env.lookup(arg0)
            frame.push(not self.is_truthy(arg0val))
            return frame

        self.globalenv.define(self.intern('not'), NativeFunction([arg0], not_))

        def display(frame, env):
            arg0val = env.lookup(arg0)
            print(arg0val, end='')
            frame.push(NIL)
            return frame

        self.globalenv.define(self.intern('display'), NativeFunction([arg0], display))

        def newline(frame, env):
            print('\n', end='')
            frame.push(NIL)
            return frame

        self.globalenv.define(self.intern('newline'), NativeFunction([], newline))

        def callcc(frame, env):
            arg0val = env.lookup(arg0)
            if type(arg0val) is not Lambda:
                raise Exception(f'Argument to contuation must be a lambda: {type(arg0val)}')
            if len(arg0val.args) != 1:
                raise Exception(f'Argument to contuation must be a lambda with 1 argument, got {len(arg0val.args)}')
            continuation = Continuation(frame)
            innerenv = Environment(arg0val.env)
            innerenv.define(arg0val.args[0], continuation)
            nextframe = Frame(arg0val.bc, innerenv, frame)
            return nextframe

        self.globalenv.define(self.intern('call-with-current-continuation'), NativeFunction([arg0], callcc))
