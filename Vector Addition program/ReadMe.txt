There are 3 programs in this file.
1. inputFileGenerator.cpp : This c++ program file is for generating the input file. While running this file please provide input file name in command line argument.
                            Then provide the size 'n' of vectors. then It will write n in input file and it will generate two vector of size n. all the numbers will be
                            randomly generated and will be less than 10^5.
2. vectorAdd.cpp : This program is Vector addition program which is going to perform vector addition on GPU. While running this programm you need to pass two thing. 
                    provide input file name (Generated using inputFileGenerator.cpp) and outfile name in the command line argument.
3.compareResult.cpp : This program will take two file. one is input.txt and other one is output.txt. output.txt is written by vectorAdd.cpp. This program will  check 
                      if result generated is correct or not. If output is Match :) then it is correct. Else if it is Doesn't match then it is incorrect.
