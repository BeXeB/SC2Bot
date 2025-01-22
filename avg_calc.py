file = open("exploration.csv", "r")

sum = 0
averages = []
for line in file:
    if line == "\n":
        averages.append(sum/5)
        sum = 0
        continue
    sections = line.split(",")
    sum += float(sections[-1])

averages.append(sum/5)

file.close()
print(averages)