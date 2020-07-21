import random

dictionary = ['A','G','T','C']
res = ""
for i in range(50):
    res += dictionary[random.randint(0,3)]
    
print(res)