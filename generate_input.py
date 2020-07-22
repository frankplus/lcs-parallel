import random

dictionary = ['A','G','T','C']
sizes = [8,50,100,1000,10000,100000,1000000]

def generate(size):
    res = ""
    for i in range(size):
        res += dictionary[random.randint(0,len(dictionary)-1)]
    return res

for size in sizes:
    filename = f"input{size}.txt"
    with open(filename, 'w') as file:
        file.write(f"{size}\n")
        file.write(f"{size}\n")
        file.write(generate(size))
        file.write('\n')
        file.write(generate(size))
        file.write('\n')