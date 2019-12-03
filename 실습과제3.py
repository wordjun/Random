from sympy import *
from numpy import *
import numpy as np
import scipy.linalg as la

np.set_printoptions(formatter={'float': lambda x: "{0:0.5f}".format(x)})

def calculate_vector(matrix, N):

    #***매개변수로 받은 matrix는 rref된 행렬***
    # span으로 나타낼 벡터 찾기
    # 벡터의 마지막 원소를 t로 두고 계산
    #ex) 3x3 행렬에서 x3 = t로 두고 x1과 x2를 t로 표현
    #Ax = Lv 에서의벡터x의 성분들 x1,x2,x3
    #x벡터의 마지막 성분인 x3는 항상 미지수 t
    new_matrix = np.zeros((N, N))
    temp = 0
    #convert back to numpy array
    for i in range(0, N):
        for j in range(0, N):
            new_matrix[i][j] = matrix[0][temp]
            temp += 1

    x = Symbol('x')
    y = Symbol('y')
    z = Symbol('z')

    #matrix[0][i]
    vector_x = [1] * N #행렬 A에 곱하는 벡터 x
    vector_x[0], vector_x[1], vector_x[2] = x, y, z
    equation = [0] * N #
    for i in range(0, N):
        for j in range(0, N):
            equation[i] += (new_matrix[i][j] * vector_x[j])
            equation[i].subs({'z': 1}) #z에 1대입

    x = solve(equation[0].subs({'z': 1}))
    y = solve(equation[1].subs({'z': 1}))
    z = 1
    vector_x[0], vector_x[1], vector_x[2] = float(x[0]), float(y[0]), z

    return vector_x

def swap(matrix, idx1, idx2, idx3):
    temp = matrix[idx1][idx3]
    matrix[idx1][idx3] = matrix[idx2][idx3]
    matrix[idx2][idx3] = temp


def getDeterminant(matrix, N):
    rowArray = [0 for m in range(0, N)]
    total = 1
    det = 1

    # 대각선 원소들 순회하며 반복
    for i in range(0, N):
        '''
        NxN 행렬에서 총 N번 반복하며
        0, 1, ... , N-1번째 행을 순회
        
        3x3 행렬의 경우
        |A| = a11|a22 a23| - a12|a21 a23| + a13|a21 a22|
        |a32 a33| |a31 a33| |a31 a32|
        a11*(-1)^(행+열) *(a22*a33 - a32*a23)
        a12*(-1)^(행+열) *(a21*a33 - a31*a23)
        a13*(-1)^(행+열) *(a21*a32 - a31*a22)
        
        '''
        for j in range(0, N):
            if matrix[0][j] == 0:
                continue
            elif matrix[0][j] != 0:
                break

        row = i
        # 각 행(row)와 열(i) 반복
        # 성분이 0인 행은 아닌 것과 스왑해줘야함
        while (matrix[row][i] == 0 and row < N):
            row += 1

        if (row != i):
            for j in range(0, N):
                swap(matrix, row, i, j)

        for j in range(0, N):
            rowArray[j] = matrix[i][j]

        # (-1)^(행+열)
        det = det * (-1) ** (row - i)

        for j in range(i + 1, N):
            num1 = rowArray[i]
            num2 = matrix[j][i]
            for k in range(0, N):
                matrix[j][k] = (num1 * matrix[j][k]) - (num2 * rowArray[k])
            total *= num1

    for k in range(0, N): # 대각선 성분들 곱한값
        det *= matrix[k][k]

    return det / total


# Diagonalization이 가능한지 아닌지 출력.
def diagonalization(matrix, N, original_matrix):
    # 대각화가 가능한지의 여부를 확인하는 함수
    # N*N 행렬인 경우, N개의 eigenvalue를 찾아야함
    P_matrix = np.zeros((N, N))
    for i in range(0, N):
        for j in range(0, N):
            P_matrix[i][j] = matrix[i][j]
    Eigen_Vectors = np.zeros((N, N))
    D_matrix = [[0 for i in range(0, N)] for j in range(0, N)]

    x = Symbol('x')
    eigenvalues = [x] * N # N개의 원소를 갖는 리스트를 생성
    '''
    A = [1-x,2,1]
    [6,-1-x,0]
    [-1,-2,-1-x]
    '''
    # EigenValue를 구하는 과정
    # 행렬A의 대각선 성분들에 -x
    for i in range(0, N):
        matrix[i][i] = matrix[i][i] - eigenvalues[i]

    # getDeterminant를 통해 식 구하기
    determinant = getDeterminant(matrix, N)
    eigenvalues = solve(determinant)

    # calculate P matrix
    new_P_matrix = np.zeros((N, N))

    # 첫번째 eigenvector 구하는 과정
    temp_matrix1 = np.zeros((N, N))
    for i in range(0, N):
        for j in range(0, N):
            temp_matrix1[i][j] = P_matrix[i][j]
    for j in range(0, N):
        temp_matrix1[j][j] -= eigenvalues[0]
    matrix_sympy = Matrix(temp_matrix1)
    matrix_rref = matrix_sympy.rref()
    new_P_matrix[0] = calculate_vector(matrix_rref, N)

    # 두번째 eigenvector 구하는 과정
    temp_matrix2 = np.zeros((N, N))
    for i in range(0, N):
        for j in range(0, N):
            temp_matrix2[i][j] = P_matrix[i][j]
    for j in range(0, N):
        temp_matrix2[j][j] -= eigenvalues[1]
    matrix_sympy = Matrix(temp_matrix2)
    matrix_rref = matrix_sympy.rref()
    new_P_matrix[1] = calculate_vector(matrix_rref, N)

    # 세번째 eigenvector 구하는 과정
    temp_matrix3 = np.zeros((N, N))
    for i in range(0, N):
        for j in range(0, N):
            temp_matrix3[i][j] = P_matrix[i][j]
    for j in range(0, N):
        temp_matrix3[j][j] -= eigenvalues[2]
    matrix_sympy = Matrix(temp_matrix3)
    matrix_rref = matrix_sympy.rref()
    new_P_matrix[2] = calculate_vector(matrix_rref, N)

    P_transpose = new_P_matrix.transpose()
    print("\nP행렬")
    print(P_transpose)

    P_inverse = np.linalg.inv(P_transpose)
    print("\nP 역행렬")
    print(P_inverse)

    multiplied_result1 = np.matmul(P_inverse, original_matrix)
    multiplied_result2 = np.matmul(multiplied_result1, P_transpose)
    print("\nP_inverse * A * P")
    print(multiplied_result2)

    for i in range(0, N):
        D_matrix[i][i] = eigenvalues[i] # D의 대각선 성분들은 eigenvalue값들

    return D_matrix


def main():
    '''
    print("정방행렬 N * N matrix 입니다.")
    N = int(input("N을 입력하세요: "))

    # N*N 행렬 생성.
    matrix = [[0 for i in range(0, N)] for j in range(0, N)]

    print("\n행-열 순으로 입력해주세요.\n"
    "(각 입력마다 엔터 키로 구분이 됩니다.)\n"
    "1행 1열부터 입력을 시작합니다.\n")

    # 정방향 행렬을 입력받는 부분입니다.
    for i in range(0, N): # row
    for j in range(0, N): # column
    matrix[i][j] = int(input("row %d, column %d: " % (i + 1, j + 1)))
    '''
    #test case
    matrix = [[1,2,1,],
                [6,-1,0],
                [-1,-2,-1]]
    N = 3
    original_matrix = [[1,2,1,],
                        [6,-1,0],
                        [-1,-2,-1]]
    D_matrix = diagonalization(matrix, N, original_matrix)
    # 입력받은 행렬을 diagonalization 함수로 보냅니다.
    print("\nD 행렬")
    for part in D_matrix:
        print(part)


main()