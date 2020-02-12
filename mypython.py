##############################################################
##                    Random Files                      ##
##  The purpose of this file is to generate 3 files (cat,   ##
##      dog, and mouse) with 10 random characters and a new ##
##      line. Then print the contents of the 3 files. Then  ##
##      generate 2 random integers, print them and their    ##
##      product.                                            ##
##############################################################
import random
import string
#predefine the file names for ease of grading and execution
file1="cat"
file2="dog"
file3="mouse"

##############################################################
##                    Write to File                     ##
##  The purpose of this function is to have an abstract     ##
##      write function to call by filename. The function    ##
##      first opens the file name provided, then using      ##
##      string.ascii_lowercase we can specify to generate   ##
##      only lowercase ascii letters, and by calling for id ##
##      in range(0, 10) we can specify to only generate 10. ##
##      Finally, we write the line to the file with a new   ##
##      line character to finish it off, then close the file##
##############################################################
def writeToFile(file) :
    f = open(file, "w")
    line = ''.join(random.choice(string.ascii_lowercase) for i in range(0, 10))
    f.write(line)
    f.write("\n")
    f.close()
    
##############################################################
##                      Print File                      ##
##  The purpose of this function is to open a file by a     ##
##      provided file name, and print the contents of the   ##
##      file to output, and close the file.                 ##
##############################################################
def printFile(file) :
    f = open(file, "r")
    print(f.read(), end='')
    f.close()

##############################################################
##                 Print Random Numbers                 ##
##  The purpose of this function is to generate two random  ##
##      integers from a range of 1 to 42 (inclusive), print ##
##      the generated integers, and their product.          ##
##############################################################
def printRanNums() :
    num1 = random.randint(1, 42)
    num2 = random.randint(1, 42)
    print(num1)
    print(num2)
    print(num1 * num2)


##calling the functions
writeToFile(file1) #file 1
writeToFile(file2) #file 2
writeToFile(file3) #file 3

printFile(file1)
printFile(file2)
printFile(file3)

printRanNums()
