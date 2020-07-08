/*
multi-threaded programming을 기반하여 스케줄러 구현

input file:
4개의 숫자들:
첫번째는 queue에 대한 클래스 number,
두번째는 프로세스의 id (PID),
세번째는 우선순위(priority)
네번째는 수행시간

프로세스 id는 모두 달라야 한다.
priority scheduling사용하지 않을 시 0으로 표기.
multi-level queue 구현하지 않을 시 클래스 number무시.
RR의 quantum, CFS의 weight등 보고서에 명시.

예제: multi-level queue
큐 안의 스케줄링(SJF, RR 등)과 큐 같의 스케줄링이 필요함.
첫번째 큐는 우선순위, 두번째는 RR, 3번째는 SJF.
큐 간 스케줄링은 priority 사용.(1>2>3)
*/

#include<semaphore.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>

#define MAX 20//최대 생성가능 스레드 수 10개로 제한
#define TIME_QUANTUM 4 //Round-Robin time quantum

typedef struct Thread {//스레드 구조체
	int classNum;//스레드 자체 스케줄러를 구분할 클래스 넘버
	int threadID;//스레드 자체를 식별할 스레드 ID
	int priority;//스레드의 우선순위
	int arrivalTime;//스레드의 ready queue에 도착한 시간
	int burstTime;//스레드의 burst-time(run-time, execution time)
	int waitTime;
	int turnAroundTime;
	int currentIndex;//current index in corresponding readyQueue
	int isComplete;//끝났는지 여부(0이면 진행중, 1이면 끝)
}Thread;

typedef struct readyQueue {
	Thread *thread;
	int* que;
	int size;
	int front;//큐 앞부분 (enqueue)
	int rear;//큐 뒷부분 (dequeue)
	int count;//큐에 들어갈 스레드 수
	int avgWT;//average waiting time of the threads in the queue
	int avgTAT;//average turn-around time of the threads in the queue
	int currentTime;//현재 큐의 수행중인 시점(시간)
}readyQueue;

void queueInitilize(readyQueue* q, int size) {
	q->thread = (Thread*)malloc(sizeof(Thread) * size);
	q->size = size;
	q->count = 0;
	q->front = 0;
	q->rear = 0;
	q->avgWT = 0;
	q->avgTAT = 0;
}

//전역변수들
int currentTime;//critical section, 여러 스레드 간 공통변수
readyQueue* rdQueue;//큐 내부의 레디큐
readyQueue mainQueue;//큐들 사이의 레디큐


//each thread will represent a single line from the inputFile
pthread_t newThreads[MAX];//새로 생성할 스레드 배열
int newThreadsIndex, globalIndex;
Thread* th;//구조체포인터 선언
int N;//total number of threads
sem_t mutex;//사용할 세마포어
FILE* inputFile;
FILE* outputFile;

//삽입을 원하는 큐, 해당큐에 삽입할 스레드, 해당스레드의 cpu-bursttime
void enQueue(readyQueue* q, Thread *t) {
	if (q->count == q->size) {//if FULL
		printf("Queue FULL\n");
		return;
	}
	q->thread[q->rear] = *t;
	q->rear = (q->rear + 1) % q->size;
	q->count++;
}

Thread* deQueue(readyQueue *q) {
	if (q->count == 0)//if EMPTY
		printf("Queue Empty\n");
	else {
		Thread* temp = &q->thread[q->front];
		//q->thread[q->front].threadID = 0;
		q->front = (q->front + 1) % q->size;
		q->count--;
		return temp;
	}
}
void swapThread(Thread* a, Thread* b) {
	printf("SWAP %d, %d\n", a->threadID, b->threadID);
	Thread temp = *a;
	*a = *b;
	*b = temp;
	printf("SWAPPED %d, %d\n", a->threadID, b->threadID);
	return;
}

void sortFCFS(int index) {
	int temp = 0;
	for (int i = 0; i < index; i++)
	{
		temp = i;
		for (int j = i + 1; j <= index; j++)
		{
			if (rdQueue[0].thread[j].arrivalTime < rdQueue[0].thread[temp].arrivalTime) {
				swapThread(&rdQueue[0].thread[j], &rdQueue[0].thread[temp]);
			}
		}
	}
}

//First-Come, First-Served Scheduling(FCFS Scheduling)
void* FCFS(void* th) {//rdQueue[0]
	Thread* newThread = (Thread*)th;
	int classNum = newThread->classNum - 1;
	int curIndex = newThread->currentIndex;

	int threadID = rdQueue[classNum].thread[curIndex].threadID;
	int burstTime = rdQueue[classNum].thread[curIndex].burstTime;
	int arrivalTime = rdQueue[classNum].thread[curIndex].arrivalTime;

	int waitTime = rdQueue[classNum].thread[curIndex].waitTime;
	int taTime = rdQueue[classNum].thread[curIndex].turnAroundTime;
	int compTime = rdQueue[classNum].thread[curIndex].completionTime;

	int n = rdQueue[classNum].size;

	sem_wait(&mutex);//lock
	printf("\n---Entered---\n");

	if (n >= MAX) {
		printf("queuesize out of bounds.\n");
		//sem_post(&mutex);
		exit(0);
	}


	printf("\nClass 1(FCFS), Thread ID %d, BurstTime %d, ArrivalTime %d, index %d\n",
		threadID, burstTime, arrivalTime, curIndex);
	//printf("\nCurrently at %d.\n", rdQueue[classNum].currentTime);


	//compute waiting time of current thread
	waitTime = 0;
	//첫번째 이후의 스레드들의 waiting time은 이전 스레드가 끝나는 시점에서 자신의 도착시간(arrival time)을 빼면된다.
	if (rdQueue[classNum].currentTime > arrivalTime)
		waitTime = rdQueue[classNum].currentTime - arrivalTime;

	rdQueue[classNum].currentTime += burstTime;
	printf("*****CURRENTLY AT (%d)SEC in rdQueue[%d]*****\n", rdQueue[classNum].currentTime, classNum);

	//waiting time만큼 기다린다
	printf("waited for %d ms\n", waitTime);

	//for outputFile
	//스레드의 ID를 burstTime만큼 출력
	printf("Thread ID: %d\n", threadID);
	for (int i = 0; i < burstTime; i++) {
		fprintf(outputFile, "%d ", threadID);
	}
	fprintf(outputFile, "\n");

	//compute turnaround time
	taTime = waitTime + burstTime;
	compTime = taTime + arrivalTime;

	fprintf(outputFile, "Thread No.%d's Complete time: %d\n", threadID, compTime);
	printf("\nThread\t\tBurst Time\tWaiting Time\tTurnaround Time");
	printf("\nth%d\t\t%d\t\t%d\t\t%d\n", threadID, burstTime, waitTime, taTime);
	rdQueue[classNum].avgWT += waitTime;
	rdQueue[classNum].avgTAT += taTime;

	if (rdQueue[classNum].count == globalCount) {//if current thread is the last one in the queue
		rdQueue[classNum].avgWT /= (float)globalCount;
		rdQueue[classNum].avgTAT /= (float)globalCount;
		printf("\n\nAverage Waiting Time:%f\n", (float)rdQueue[classNum].avgWT);
		printf("Average Turnaround Time:%f\n", (float)rdQueue[classNum].avgTAT);
		fprintf(outputFile, "Average Waiting Time: %f, Average Turnaround Time: %f\n",
			(float)rdQueue[classNum].avgWT, (float)rdQueue[classNum].avgTAT);
	}

	printf("===================================================================\n");
	printf("Exiting\n");
	sem_post(&mutex);

	pthread_exit(0);
}

void sortSJF(int classNum, int index) {//burst time을 정렬한다
	int temp = 0;
	for (int i = 0; i < index; i++)
	{
		temp = i;
		for (int j = i + 1; j <= index; j++)
		{
			if (rdQueue[classNum].thread[j].burstTime < rdQueue[classNum].thread[temp].burstTime) {
				swapThread(&rdQueue[classNum].thread[j], &rdQueue[classNum].thread[temp]);
				//swap(&rdQueue[classNum].thread[j].arrivalTime, &rdQueue[classNum].thread[j].arrivalTime);
			}
		}
	}
	printf("rdQueue[%d]: %d, %d, %d\n", classNum, rdQueue[classNum].thread[0].burstTime,
		rdQueue[classNum].thread[1].burstTime, rdQueue[classNum].thread[2].burstTime);
}

void* SJF(void* thd) {//rdQueue[1]
	Thread* newThread = (Thread*)thd;
	int classNum = newThread->classNum - 1;
	int curIndex = newThread->currentIndex;

	int threadID = rdQueue[classNum].thread[curIndex].threadID;
	int burstTime = rdQueue[classNum].thread[curIndex].burstTime;
	int arrivalTime = rdQueue[classNum].thread[curIndex].arrivalTime;

	int waitTime = rdQueue[classNum].thread[curIndex].waitTime;
	int taTime = rdQueue[classNum].thread[curIndex].turnAroundTime;
	int n = rdQueue[classNum].size;

	if (n >= MAX) {
		printf("queuesize out of bounds.\n");
		//sem_post(&mutex);
		exit(0);
	}


	/*for (int i = 0; i < burstTime; i++) {
		sem_post(&mutex);
	}

	sem_getvalue(&mutex, &currentTime);
	printf("%d\n", currentTime);*/

	//sem_wait(&mutex);
	//printf("\n---Entered---\n");


	//sorting of burst times
	//update burst time if there's a change.
	printf("\nClass 1(SJF), ThreadID %d, BurstTime %d, ArrivalTime %d, currently at index %d in rdQueue[1]\n",
		threadID, th[globalIndex].burstTime, arrivalTime, curIndex);

	printf("\nCurrently at %d.\n", currentTime);

	//compute waiting time of current thread
	waitTime = 0;
	//첫번째 이후의 스레드들의 waiting time은 이전 스레드가 끝나는 시점에서 자신의 도착시간(arrival time)을 빼면된다.
	if (rdQueue[classNum].currentTime > arrivalTime)
		waitTime = rdQueue[classNum].currentTime - arrivalTime;

	//update currentTime in readyQueue.
	rdQueue[classNum].currentTime += th[globalIndex].burstTime;

	//waiting time만큼 기다린다
	printf("waited for %d ms\n", waitTime);
	sleep(waitTime);

	//for outputFile
	//스레드의 ID를 burstTime만큼 출력
	printf("Thread ID: %d\n", threadID);
	for (int i = 0; i < th[globalIndex].burstTime; i++)
		printf("%d ", threadID);

	//compute turnaround time
	taTime = waitTime + burstTime;

	printf("\nThread\t\tBurst Time\tWaiting Time\tTurnaround Time");
	printf("\nth%d\t\t%d\t\t%d\t\t%d\n", threadID, th[globalIndex].burstTime, waitTime, taTime);
	rdQueue[classNum].avgWT += waitTime;
	rdQueue[classNum].avgTAT += taTime;

	//printf("%d, %d %d\n", rdQueue[0].avgWT, rdQueue[0].avgTAT, rdQueue[0].count);
	if (curIndex == rdQueue[classNum].count - 1) {//if current thread is the last one in the queue
		rdQueue[classNum].avgWT /= (float)rdQueue[classNum].count;
		rdQueue[classNum].avgTAT /= (float)rdQueue[classNum].count;
		printf("\n\nAverage Waiting Time:%f\n", (float)rdQueue[classNum].avgWT);
		printf("Average Turnaround Time:%f\n", (float)rdQueue[classNum].avgTAT);
	}
	printf("===================================================================\n");
	//printf("Exiting\n");
	sem_post(&mutex[globalIndex]);

	pthread_exit(0);
}

void* RR(void* th) {//rdQueue[2]
	//time quantum defined to 4
	printf("RR\n");
}

void* queueSched(void* arg) {//main scheduler
	//큐들 사이를 스케줄링할 함수
	Thread *temp;
	globalIndex = 0;
	printf("Entered\n");
	//critical section
	//enQueue(&mainQueue, &th[index]);
	/*for (int i = 0; i < th[index].burstTime; i++) {
		sem_post(&mutex);
		sem_getvalue(&mutex, currentTime);
		if (currentTime < th[index].arrivalTime) {
			printf("CURRENTLY AT %dSEC.\n", currentTime);
			continue;
		}
	}*/
	

	while (mainQueue.count != 0) {
		sem_init(&mutex[globalIndex], 0, 1);
		temp = deQueue(&mainQueue);//대기큐에서 스레드 꺼냄
		printf("DEQUEUED: %d ", temp->threadID);
		//check for preemption

		sleep(1);
		if (temp->classNum == 1) {
			pthread_create(&newThreads[newThreadsIndex++], NULL, FCFS, (void*)temp);
			
			//만약 현재 스레드 실행 도중 다른 스레드가 들어온 경우, class number 비교
		//만약 현재 스레드보다 높은 class number이고 도착시간이 현재스레드의 실행시간과 겹친다면 preemption실행
			if (globalIndex < N - 1) {
				if (temp->classNum > th[globalIndex + 1].classNum &&
					temp->arrivalTime + temp->burstTime > th[globalIndex + 1].arrivalTime) {//다음스레드와 클래스 비교
					th[globalIndex].burstTime = th[globalIndex + 1].arrivalTime - temp->arrivalTime;
					sem_wait(&mutex[globalIndex]);
					enQueue(&mainQueue, temp);
					printf("ENQUEUED: %d ", temp->threadID);
				}
				//클래스는 높으나 겹치지 않는 경우, 현재 스레드를 완료하고 나서 실행하면 된다
			}
		}
		else if (temp->classNum == 2) {
			pthread_create(&newThreads[newThreadsIndex++], NULL, SJF, (void*)temp);
			//만약 현재 스레드 실행 도중 다른 스레드가 들어온 경우, class number 비교
			//만약 현재 스레드보다 높은 class number이고 도착시간이 현재스레드의 실행시간과 겹친다면 preemption실행
			if (globalIndex < N - 1) {
				if (temp->classNum > th[globalIndex + 1].classNum &&
					temp->arrivalTime + temp->burstTime > th[globalIndex + 1].arrivalTime) {//다음스레드와 클래스 비교
					th[globalIndex].burstTime = th[globalIndex + 1].arrivalTime - temp->arrivalTime;
					sem_wait(&mutex[globalIndex]);//mutex가 0이면 기다리고, 0이 아니면 계속진행한다
					enQueue(&mainQueue, temp);
					printf("ENQUEUED: %d ", temp->threadID);
				}
				//클래스는 높으나 겹치지 않는 경우, 현재 스레드를 완료하고 나서 실행하면 된다
			}
		}
		pthread_join(newThreads[newThreadsIndex], NULL);

		if (++globalIndex >= N)
			break;
		//printf("Main Queue: %d\n", th[index].threadID);
		//currentTime++;
		
		
	} 

	printf("Exiting\n");
	pthread_exit(0);//thread termination
}


//새로 생성된 모든 스레드들에 의해 실행될 함수
//void* threadFunc(void* th) {
//	Thread* newThread = (Thread*)th;
//	pthread_t tid = pthread_self();
//	int threadID = newThread->threadID;
//	int runtime = newThread->burstTime;
//	int priority = newThread->priority;
//	int classNum = newThread->classNum;
//	//printf("\nThread Index %d\n", thIndex);
//	//new_t[thIndex] = pthread_self();
//	//---wait---
//	sem_wait(&mutex);
//	printf("\n---Entered---\n");
//	
//	sleep(runtime);//causes the thread to sleep for a single second then resume exe.
//	//---CRITICAL SECTION---, 각 프로세스에서 공유 데이타를 액세스하는 프로그램 코드 부분
//	//in an attempt to make this thread finish AFTER main thread.
//	printf("Class %d, Thread ID %d, Priority %d, Runtime %d\n", 
//		classNum, threadID, priority, runtime);
//	pthread_create(&tid, NULL, queueSched, (void*)&newThread);
//	sleep(1);
//
//	//---signal---
//	sem_post(&mutex);
//	printf("---Just Exiting---\n");
//	pthread_exit(NULL);
//	//pthread_exit(NULL);
//}




/*첫번째는 queue에 대한 클래스 number,
두번째는 프로세스의 id (PID),
세번째는 우선순위(priority)
네번째는 수행시간*/

/*
클래스number에 대한 정의: 1번-FCFS, 2번-SJF, 3번-RR
프로세스 ID에 대한 정의: 입력받는 순으로 1부터 시작해 1씩 증가.
우선순위에 대한 정의: 숫자가 낮을수록 우선순위가 높음
수행시간에 대한 정의: 수행시간은 최대 5초로 설정.
*/

int main() {
	
	int threadSuc;//스레드 생성 성공여부. 성공 시 0리턴, 실패 시 0이 아닌값 리턴
	//main함수도 하나의 thread에서 실행이 된다.
	//pthread_t main_t = pthread_self();//main스레드의 id
	pthread_t mainScheduler;//스케줄러 스레드의 ID
	pthread_attr_t attr;//스레드를 위한 속성들
	pthread_attr_init(&attr);

	//printf("Type in the number of threads you wish to create(max 10):");
	FILE* inputFile = fopen("input.in", "rt");
	FILE* outputFile = fopen("assignment.out", "wt");
	fscanf(inputFile, "%d", &N);

	//malloc으로 동적할당
	th = malloc(sizeof(Thread) * N);//각 요소에 Thread구조체 크기만큼 malloc
	rdQueue = malloc(sizeof(readyQueue) * 3);//FCFS, SJF, RR

	//class number represents queue number.
	/*
	class 0: FCFS (whatever thread comes first gets to run first)
	class 1: SJF (shortest runtime thread first.)
	class 2: RR (run the threads for a given quantum until it finishes.)
	*/
	for (int i = 0; i < N; i++) {
		//printf("Class Number | Thread ID | Priority | Run-time: ");
		fscanf(inputFile, "%d%d%d%d", 
			&th[i].classNum, &th[i].threadID, &th[i].arrivalTime, &th[i].burstTime);
		if (feof(inputFile) != 0)
			break;
	}

	for (int i = 0; i < N; i++) {
		printf("%d %d %d %d\n", th[i].classNum, th[i].threadID, th[i].arrivalTime, th[i].burstTime);
	}

	//모든 큐들 초기화
	for(int i = 0;i<2;i++)
		queueInitilize(&rdQueue[i], N);

	//큐 간 스케줄러 정렬을 시행.
	//모든 스레드들을 일치하는 큐에 삽입 후
	//큐number(class number)0부터 1, 2 의 우선순위대로 실행

	//mainQueue 초기화
	queueInitilize(&mainQueue, N);

	
	for (int i = 0; i < N; i++) {//insert threads into corresponding 
		if (rdQueue[th[i].classNum - 1].count == 0)
			th[i].currentIndex = 0;
		else
			th[i].currentIndex = rdQueue[th[i].classNum - 1].count;
		enQueue(&rdQueue[th[i].classNum - 1], &th[i]);
		enQueue(&mainQueue, &th[i]);
		//printf("Q%d: %d\n", th[index].classNum - 1, th[index].currentIndex);
	}
	currentTime = 0, newThreadsIndex = 0;
	pthread_create(&mainScheduler, &attr, queueSched, (void*)th);
	pthread_join(mainScheduler, NULL);
	
	/*for (int i = 0; i < rdQueue[0].count; i++) {
		printf("%d\n", rdQueue[0].thread[i].threadID);
	}
	for (int i = 0; i < rdQueue[1].count; i++) {
		printf("%d\n", rdQueue[1].thread[i].threadID);
	}
	for (int i = 0; i < mainQueue.size; i++) {
		printf("%d\n", mainQueue.thread[i].threadID);
	}*/


	
	/*for (int i = 0; i < 3; i++) {
		printf("%d %d\n", rdQueue[i].que[0], rdQueue[i].que[1]);
	}*/
	//멀티스레딩 실시
	//for (int i = 0; i < N; i++) {
	//	pthread_attr_init(&attr);//스레드의 기본값 속성들
	//	//pThread_create()함수는 성공시 0, 실패시 non-zero value를 반환.
	//	switch (th[i].classNum) {
	//	case 1:
	//		pthread_create(&new_t[i], NULL, FCFS, (void*)&th[i]);
	//		break;
	//	case 2:
	//		pthread_create(&new_t[i], NULL, SJF, (void*)&th[i]);
	//		break;
	//	case 3:
	//		//pthread_create(&new_t[i], NULL, RR, (void*)&th[i]);
	//		break;
	//	default:
	//		printf("class number must be among 1, 2, and 3\n");
	//		break;
	//	}
	//	//threadSuc = pthread_create(&new_t[i], NULL, threadFunc, (void*)&th[i]);//스레드 생성
	//	////printf("\nCreating a new Thread with ID: (%d) on iteration %d\n", (int)new_t[thIndex], i + 1);
	//	//sleep(1);
	//	//if (threadSuc) {
	//	//	printf("\n ERROR: return code from pthread_Create is %d\n", threadSuc);
	//	//	exit(1);
	//	//}
	//	pthread_join(new_t[i], NULL);//join with the main thread
	//}

	//It is necessary to use pthread_exit at the end of the main program.
	//Otherwise, when it exits, all running threads will be killed.
	for(int i = 0;i<N;i++)
		sem_destroy(&mutex[i]);//할당했던 semaphore삭제
	pthread_exit(NULL);//thread termination

	//fprintf(outputFile, "%d", result);
	fclose(inputFile);
	fclose(outputFile);
	//메모리해제
	free(th);
	return 0;
}