#행사다리꼴행렬(Row Echelon Form)을 구현.
#입력받은 행렬을
#1. 행교환
#2. 행곱하기
#3. 행끼리 더하고 빼기 세가지의 과정을 선택 수행

#get input (m, n), m by n matrix

def row_echelon_form(matrix, row, column):
    #calculate
    for k in range(0, row-1):
        for i in range(k, row-1):
            temp = 0 #column index

            # first and second number for calculating multiplier
            first_num = matrix[k][temp]
            if matrix[i+1][temp] != 0:
                second_num = matrix[i+1][temp]
            else:
                if k == 0:
                    k += 1
                first_num = matrix[k][temp+1]
                second_num = matrix[i+1][temp+1]

            #set multiplier
            if first_num == 0 or second_num == 0:
                multiplier = 0
            else:
                multiplier = (-second_num) / first_num

            #temporary number for multiplication then addition
            temp_num = 0
            for j in range(0, column):
                temp_num = matrix[k][j] * multiplier  # multiply temp_row
                matrix[i+1][j] += temp_num  # then add it to the next row
    return matrix

# main function
def main():
    print("*****Rows and Columns are limited from 0 to 20*****")
    row = int(input("Enter M(Row): "))
    column = int(input("Enter N(Column): "))

    # initialize matrix with 0s
    matrix = [[0 for i in range(0, column)] for j in range(0, row)]

    # get input for each
    print("\nPlease type in the order of Row by Column\n"
          "(Each input must be distinguished by pressing Enter)\n"
          "Starting from (1, 1)\n")

    for i in range(0, row):  # row
        for j in range(0, column):  # column
            matrix[i][j] = int(input("row %d, column %d: " % (i+1, j+1)))


    # send matrix to function row_echelon_form
    row_echelon_form(matrix, row, column)

    # Print Each row
    print("Printing Matrix:")
    for i in range(0, row):
        print(matrix[i])

main()