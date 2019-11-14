import numpy as np


def pivot_matrix(matrix, N):
    identity_matrix = [[0 for i in range(0, N)] for j in range(0, N)]
    for i in range(0, N):
        for j in range(0, N):
            if i == j:
                identity_matrix[i][j] = 1

    for j in range(0, N):
        row = max(range(j, N), key=lambda i: matrix[i][j])
        if j != row:
            identity_matrix[j], identity_matrix[row] \
                = identity_matrix[row], identity_matrix[j]
    return identity_matrix

def LU(matrix, N):
    #벡터 x, y, b선언 (모두 0으로 초기화된 N차원 벡터들)

    # L행렬의 대각선 원소들은 1이다.
    lower_triangle = [[0 for i in range(0, N)] for i in range(0, N)]
    for i in range(0, N):
        lower_triangle[i][i] = 1    #인덱스가 같으면 diagonal component

    # U행렬은 우선 행렬A와 똑같이 한다
    upper_triangle = np.zeros((N, N))
    for i in range(0, N):
        for j in range(0, N):
            upper_triangle[i][j] = matrix[i][j]

    for i in range(0, N):   #인덱스 i는 행(row)
        max_component = abs(upper_triangle[i][i])   #일단은 맨 첫 원소로 지정후 비교한다
        max_row = i #최대 행 역시 맨 처음 행으로 지정해둔다
        for k in range(i + 1, N):   #그 다음 행(i+1)과 비교
            if max_component < abs(upper_triangle[k][i]):
                max_component = abs(upper_triangle[k][i]) # Next line on the diagonal
                max_row = k #최대값이 있는 행 갱신(k번째 행과 i번째(현재 기준 행)비교

        # N개의 행 중 가장 큰 원소값이 맨 첫 행으로 오게 swap연산을 한다
        #i는 현재 행을 나타낸다(전체반복문 인덱스)
        for k in range(i, N):
            # i번째 행의 성분들과 k번째 행의 성분들을 하나씩 바꿔치기해준다
            temp = upper_triangle[max_row][k]
            upper_triangle[max_row][k] = upper_triangle[i][k]
            upper_triangle[i][k] = temp

        #multiple을 구해 pivot행의 component를 곱한 값을 그 밑 행의 component들과 더한다(+=)
        for k in range(i + 1, N):
            multiplier = -upper_triangle[k][i] / float(upper_triangle[i][i])
            lower_triangle[k][i] = -multiplier   #행렬 L의 k번째 행, i번째 열에 multiplier 값 대입
            for j in range(i, N):   #덧셈반복은 column개수만큼.
                upper_triangle[k][j] += multiplier * upper_triangle[i][j]  #피봇 행에 있는 성분들에 배수(multiplier)를 곱해서 더해준다

        # (3 * 3) 행렬기준,
        # upper[1][0], upper[2][0], upper[2][1]을 0으로 만들어준다
        # (upper triangle이 되도록 하는 작업)
        for k in range(i + 1, N):
            upper_triangle[k][i] = 0

    vector_b = [0 for i in range(0, N)]
    # 벡터b는 행렬L과 벡터 y를 곱했을때 나오는 벡터
    # 행렬A의 마지막 column을 벡터b로 지정한다.
    for i in range(0, N):
        vector_b[i] = matrix[i][N-1]

    vector_y = [0 for i in range(0, N)]
    #lower triangle의 행렬 L과 벡터y를 곱하면 b벡터가 나오기때문에
    #그 성질을 이용하여 성분들의 연산을 해준다
    for i in range(0, N):
        vector_y[i] = vector_b[i] / float(lower_triangle[i][i])
        for k in range(0, i):
            vector_y[i] -= vector_y[k] * lower_triangle[i][k]

    vector_x = [0  for i in range(0, N)]
    #upper triangle의 행렬 U과 벡터x를 곱하면 벡터y가 나오므로
    #그 성질을 이용해 성분연산
    for i in range(N - 1, -1, -1):  #for(int i = N-1; i>=0;i--) 거꾸로
        vector_x[i] = vector_y[i] / float(upper_triangle[i][i]) #벡터 y와 행렬U의 대각선 성분을 나눈 값을 벡터x로 지정
        for k in range(i - 1, -1, -1):
            upper_triangle[i] -= vector_x[i] * upper_triangle[i][k]

    return lower_triangle, upper_triangle

def main():
    print("정방행렬 N * N matrix 입니다.")
    N = int(input("N을 입력하세요: "))

    matrix = [[0 for i in range(0, N)] for j in range(0, N)]

    print("\n행-열 순으로 입력해주세요.\n"
          "(각 입력마다 엔터 키로 구분이 됩니다.)\n"
          "1행 1열부터 입력을 시작합니다.\n")

    for i in range(0, N):  # row
        for j in range(0, N):  # column
            matrix[i][j] = int(input("row %d, column %d: " % (i + 1, j + 1)))

    #diagonals of L set to 1
    #lower_triangle = [[int(i ==j) for i in range(0, N)] for j in range(0, N)]
    #upper_triangle = [[0] * N for i in range(N)]

    print("Initial Matrix A:")
    for row in matrix:
        print(row)

    print("\nLower and Upper Triangle:")
    for part in LU(matrix, N):
        print('\t')
        for sub_part in part:
            print(sub_part)

    print("\nPermutation:")
    for row in pivot_matrix(matrix, N):
        print(row)

main()