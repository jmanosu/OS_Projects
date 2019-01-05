#Jared Tence
#11/19/18
import random

def main():
    #opens file 1 to write to and clears contents using trunk
    f1 = open("file1", "w")
    f1.truncate()
    #generates a random string and writes it to file and prints it
    temp = generateRanString()
    f1.write(temp)
    print(temp, end ="")

    #same operations as file 1
    f2 = open("file2", "w")
    f2.truncate()
    temp = generateRanString()
    f2.write(temp)
    print(temp, end ="")

    #same operations as file 1
    f3 = open("file3", "w")
    f3.truncate()
    temp = generateRanString()
    f3.write(temp)
    print(temp, end ="")

    #generates two random INT's and prints them and their product
    rInt1 = random.randint(1,42)
    rInt2 = random.randint(1,42)
    print(rInt1)
    print(rInt2)
    print(rInt1 * rInt2)

#random string generator
def generateRanString():
    #output string
    output = ""
    for i in range(10):
        #generates a random number between 94 and 122
        rInt = random.randint(97,122)
        #converts random number to ascii value (94 = a) and (122 = z)
        output += chr(rInt)
    #returns random string and adds new line character
    return output + '\n'

#runs main function
main()
