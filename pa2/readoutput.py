proc1 = 0
proc2 = 0
proc3 = 0
proc4 = 0
total = 0

with open('out.txt') as f:
  for line in f:
    if '1' in line:
      proc1 += 1
    elif '2' in line:
      proc2 += 1
    elif '3' in line:
      proc3 += 1
    elif '4' in line:
      proc4 += 1
    total += 1

print ("proc1: " + str(proc1))
print ("proc2: " + str(proc2))
print ("proc3: " + str(proc3))
print ("proc4: " + str(proc4))
print ("total: " + str(total))
