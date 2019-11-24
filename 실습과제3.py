from sympy import Symbol, solve

#Diagonalization이 가능한지 아닌지 출력.
def diagonalization(matrix, N):
    #대각화가 가능한지의 여부를 확인하는 함수입니다.
    #N*N 행렬인 경우, N개의 eigenvalue를 찾아야 합니다.
    x = Symbol('x')
    eigenvalues = [x] * N   #N개의 원소를 갖는 리스트를 생성합니다
    #EigenValue를 구하는 과정
    # 행렬A의 대각선 성분들을 -Lambda함
    for i in range(0, N):
        matrix[i][i] = matrix[i][i] - eigenvalues[i]

    # determinant를 통해 식 구하기
    for i in range(0, N): #첫번째 열
        for j in range(0, N):
            #방정식 변수 equation
            equation = matrix[i][j]

def main():
    print("정방행렬 N * N matrix 입니다.")
    N = int(input("N을 입력하세요: "))

    #N*N 행렬 생성.
    matrix = [[0 for i in range(0, N)] for j in range(0, N)]

    print("\n행-열 순으로 입력해주세요.\n"
          "(각 입력마다 엔터 키로 구분이 됩니다.)\n"
          "1행 1열부터 입력을 시작합니다.\n")

    # 정방향 행렬을 입력받는 부분입니다.
    for i in range(0, N):  # row
        for j in range(0, N):  # column
            matrix[i][j] = int(input("row %d, column %d: " % (i + 1, j + 1)))

    #입력받은 행렬을 diagonalization 함수로 보냅니다.
    diagonalization(matrix, N)

main()