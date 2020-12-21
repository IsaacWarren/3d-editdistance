import random

NUM_SEQUENCES = 3
SEQUENCE_LENGTH = 300
INSERT_PROB = 0.01
DELETE_PROB = 0.01
SUBSTITUTE_PROB = 0.01
NO_ERR_PROB = 1 - INSERT_PROB - DELETE_PROB - SUBSTITUTE_PROB
assert(NO_ERR_PROB > 0)

def insert(c):
    return c + random.choice(["A","T","C","G"])
def delete(c):
    return ""
def substitute(c):
    return random.choice(["A","T","C","G"])
def no_err(c):
    return c

for i in range(NUM_SEQUENCES):
    out = None
    with open('seq.txt') as f:
        seq = f.read()
        operations = random.choices([insert, delete, substitute, no_err], weights=[INSERT_PROB, DELETE_PROB, SUBSTITUTE_PROB, NO_ERR_PROB], k = len(seq))
        out = [operations[i](seq[i]) for i in range(min(SEQUENCE_LENGTH, len(seq)))]

    with open(f'seq{i}.txt', mode="w") as f:
        f.write("".join(out))
    
