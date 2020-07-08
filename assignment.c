/*
multi-threaded programming�� ����Ͽ� �����ٷ� ����

input file:
4���� ���ڵ�:
ù��°�� queue�� ���� Ŭ���� number,
�ι�°�� ���μ����� id (PID),
����°�� �켱����(priority)
�׹�°�� ����ð�

���μ��� id�� ��� �޶�� �Ѵ�.
priority scheduling������� ���� �� 0���� ǥ��.
multi-level queue �������� ���� �� Ŭ���� number����.
RR�� quantum, CFS�� weight�� ������ ���.

����: multi-level queue
ť ���� �����ٸ�(SJF, RR ��)�� ť ���� �����ٸ��� �ʿ���.
ù��° ť�� �켱����, �ι�°�� RR, 3��°�� SJF.
ť �� �����ٸ��� priority ���.(1>2>3)
*/

#include<semaphore.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>

#define MAX 20//�ִ� �������� ������ �� 10���� ����
#define TIME_QUANTUM 4 //Round-Robin time quantum

typedef struct Thread {//������ ����ü
	int classNum;//������ ��ü �����ٷ��� ������ Ŭ���� �ѹ�
	int threadID;//������ ��ü�� �ĺ��� ������ ID
	int priority;//�������� �켱����
	int arrivalTime;//�������� ready queue�� ������ �ð�
	int burstTime;//�������� burst-time(run-time, execution time)
	int waitTime;
	int turnAroundTime;
	int currentIndex;//current index in corresponding readyQueue
	int isComplete;//�������� ����(0�̸� ������, 1�̸� ��)
}Thread;

typedef struct readyQueue {
	Thread *thread;
	int* que;
	int size;
	int front;//ť �պκ� (enqueue)
	int rear;//ť �޺κ� (dequeue)
	int count;//ť�� �� ������ ��
	int avgWT;//average waiting time of the threads in the queue
	int avgTAT;//average turn-around time of the threads in the queue
	int currentTime;//���� ť�� �������� ����(�ð�)
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

//����������
int currentTime;//critical section, ���� ������ �� ���뺯��
readyQueue* rdQueue;//ť ������ ����ť
readyQueue mainQueue;//ť�� ������ ����ť


//each thread will represent a single line from the inputFile
pthread_t newThreads[MAX];//���� ������ ������ �迭
int newThreadsIndex, globalIndex;
Thread* th;//����ü������ ����
int N;//total number of threads
sem_t mutex;//����� ��������
FILE* inputFile;
FILE* outputFile;

//������ ���ϴ� ť, �ش�ť�� ������ ������, �ش罺������ cpu-bursttime
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
	//ù��° ������ ��������� waiting time�� ���� �����尡 ������ �������� �ڽ��� �����ð�(arrival time)�� ����ȴ�.
	if (rdQueue[classNum].currentTime > arrivalTime)
		waitTime = rdQueue[classNum].currentTime - arrivalTime;

	rdQueue[classNum].currentTime += burstTime;
	printf("*****CURRENTLY AT (%d)SEC in rdQueue[%d]*****\n", rdQueue[classNum].currentTime, classNum);

	//waiting time��ŭ ��ٸ���
	printf("waited for %d ms\n", waitTime);

	//for outputFile
	//�������� ID�� burstTime��ŭ ���
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

void sortSJF(int classNum, int index) {//burst time�� �����Ѵ�
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
	//ù��° ������ ��������� waiting time�� ���� �����尡 ������ �������� �ڽ��� �����ð�(arrival time)�� ����ȴ�.
	if (rdQueue[classNum].currentTime > arrivalTime)
		waitTime = rdQueue[classNum].currentTime - arrivalTime;

	//update currentTime in readyQueue.
	rdQueue[classNum].currentTime += th[globalIndex].burstTime;

	//waiting time��ŭ ��ٸ���
	printf("waited for %d ms\n", waitTime);
	sleep(waitTime);

	//for outputFile
	//�������� ID�� burstTime��ŭ ���
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
	//ť�� ���̸� �����ٸ��� �Լ�
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
		temp = deQueue(&mainQueue);//���ť���� ������ ����
		printf("DEQUEUED: %d ", temp->threadID);
		//check for preemption

		sleep(1);
		if (temp->classNum == 1) {
			pthread_create(&newThreads[newThreadsIndex++], NULL, FCFS, (void*)temp);
			
			//���� ���� ������ ���� ���� �ٸ� �����尡 ���� ���, class number ��
		//���� ���� �����庸�� ���� class number�̰� �����ð��� ���罺������ ����ð��� ��ģ�ٸ� preemption����
			if (globalIndex < N - 1) {
				if (temp->classNum > th[globalIndex + 1].classNum &&
					temp->arrivalTime + temp->burstTime > th[globalIndex + 1].arrivalTime) {//����������� Ŭ���� ��
					th[globalIndex].burstTime = th[globalIndex + 1].arrivalTime - temp->arrivalTime;
					sem_wait(&mutex[globalIndex]);
					enQueue(&mainQueue, temp);
					printf("ENQUEUED: %d ", temp->threadID);
				}
				//Ŭ������ ������ ��ġ�� �ʴ� ���, ���� �����带 �Ϸ��ϰ� ���� �����ϸ� �ȴ�
			}
		}
		else if (temp->classNum == 2) {
			pthread_create(&newThreads[newThreadsIndex++], NULL, SJF, (void*)temp);
			//���� ���� ������ ���� ���� �ٸ� �����尡 ���� ���, class number ��
			//���� ���� �����庸�� ���� class number�̰� �����ð��� ���罺������ ����ð��� ��ģ�ٸ� preemption����
			if (globalIndex < N - 1) {
				if (temp->classNum > th[globalIndex + 1].classNum &&
					temp->arrivalTime + temp->burstTime > th[globalIndex + 1].arrivalTime) {//����������� Ŭ���� ��
					th[globalIndex].burstTime = th[globalIndex + 1].arrivalTime - temp->arrivalTime;
					sem_wait(&mutex[globalIndex]);//mutex�� 0�̸� ��ٸ���, 0�� �ƴϸ� ��������Ѵ�
					enQueue(&mainQueue, temp);
					printf("ENQUEUED: %d ", temp->threadID);
				}
				//Ŭ������ ������ ��ġ�� �ʴ� ���, ���� �����带 �Ϸ��ϰ� ���� �����ϸ� �ȴ�
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


//���� ������ ��� ������鿡 ���� ����� �Լ�
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
//	//---CRITICAL SECTION---, �� ���μ������� ���� ����Ÿ�� �׼����ϴ� ���α׷� �ڵ� �κ�
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




/*ù��°�� queue�� ���� Ŭ���� number,
�ι�°�� ���μ����� id (PID),
����°�� �켱����(priority)
�׹�°�� ����ð�*/

/*
Ŭ����number�� ���� ����: 1��-FCFS, 2��-SJF, 3��-RR
���μ��� ID�� ���� ����: �Է¹޴� ������ 1���� ������ 1�� ����.
�켱������ ���� ����: ���ڰ� �������� �켱������ ����
����ð��� ���� ����: ����ð��� �ִ� 5�ʷ� ����.
*/

int main() {
	
	int threadSuc;//������ ���� ��������. ���� �� 0����, ���� �� 0�� �ƴѰ� ����
	//main�Լ��� �ϳ��� thread���� ������ �ȴ�.
	//pthread_t main_t = pthread_self();//main�������� id
	pthread_t mainScheduler;//�����ٷ� �������� ID
	pthread_attr_t attr;//�����带 ���� �Ӽ���
	pthread_attr_init(&attr);

	//printf("Type in the number of threads you wish to create(max 10):");
	FILE* inputFile = fopen("input.in", "rt");
	FILE* outputFile = fopen("assignment.out", "wt");
	fscanf(inputFile, "%d", &N);

	//malloc���� �����Ҵ�
	th = malloc(sizeof(Thread) * N);//�� ��ҿ� Thread����ü ũ�⸸ŭ malloc
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

	//��� ť�� �ʱ�ȭ
	for(int i = 0;i<2;i++)
		queueInitilize(&rdQueue[i], N);

	//ť �� �����ٷ� ������ ����.
	//��� ��������� ��ġ�ϴ� ť�� ���� ��
	//ťnumber(class number)0���� 1, 2 �� �켱������� ����

	//mainQueue �ʱ�ȭ
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
	//��Ƽ������ �ǽ�
	//for (int i = 0; i < N; i++) {
	//	pthread_attr_init(&attr);//�������� �⺻�� �Ӽ���
	//	//pThread_create()�Լ��� ������ 0, ���н� non-zero value�� ��ȯ.
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
	//	//threadSuc = pthread_create(&new_t[i], NULL, threadFunc, (void*)&th[i]);//������ ����
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
		sem_destroy(&mutex[i]);//�Ҵ��ߴ� semaphore����
	pthread_exit(NULL);//thread termination

	//fprintf(outputFile, "%d", result);
	fclose(inputFile);
	fclose(outputFile);
	//�޸�����
	free(th);
	return 0;
}