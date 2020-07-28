# Multileve-Queue Scheduler


##개요
Multi-level Queue기법에서 Queue간 스케줄링을 Round Robin이지만 Queue Class사이의 약간의 우선순위 개념으로 시간할당량을 우선순위가 높은 Queue Class일수록 더 크게 주는 스케줄링 기법을 구현

##구성 파일
tools.h : 구조체 Queue, Node, Process에 대한 정의와 사용 함수 구현
algorithms.h : 스케줄링 알고리즘 함수 구현 (SJF, Priority, RR)
main.c : main 실행파일 

##자료형
Queue : Process가 저장된 Node들로 구성
Node : Process와 앞 뒤로 연결된 Node들의 주소를 담고 있는 구조체 
Process 구조체
- int class : 각 프로세스의 class (class에 따라 저장되는 큐가 다름. 범위: 1~3)
- int pid : 프로세스의 id
- int priority : 우선순위 (작을수록 높음)
- int ptime : 처리시간

##전체 구조 (실제 Queue를 구성하는 것은 process를 담은 Node지만 직관성을 위해 process로 표기) 
- Multi-level Queue를 기반으로 하는 스케줄러.
- pthread_t thread[4] : 4개의 스케줄러 thread가 존재 (Queue 내부 스케줄러 3개 + Queue간 스케줄러 1개)
- 개별Queue는 3개가 존재하며 각각 class 1,2,3을 가진다. 각 Queue에는 스케줄러가 존재한다.
- Thread간 공유 Queue인 ready queue가 존재하며 각 Queue의 내부 스케줄러가 Process를 스케줄링하여 ready큐에 적재하면 Queue간 스케줄러가 이를 스케줄링 하는 방식이다. – flag 활용

##Queue간 스케줄러 – thread[0] -> Round robin
- 3개의 Queue에서 담겨지는 ready큐에 대해서 round robin 스케줄링을 진행하며 Process의 Class별로 시간할당량이 달라진다.
-> Process class별 시간할당량 : class1 = 7 , class2 = 5 , class3 = 3
- 만약 Queue1, Queue2, Queue3중 완료되는 Queue가 존재할 시 해당 자리에 다른 Queue중 높은 class우선순위를 가진 Queue의 Process가 적재된다. -> Class 값이 낮을수록 우선순위가 높음
Ex) Queue2의 Process가 모두 스케줄링 되어 thread[2]가 종료되면 빈 ready queue의 자리에 Queue1의 스케줄러가 Process를 적재한다.


##Queue 내부 스케줄러 – thread[1], thread[2], thread[3] 
내부 동작 : 스케줄러가 자신의 Queue를 스케줄링 후 ready queue에 적재. 
- thread[1] -> Queue1 (class 1) : 우선순위 스케줄링
- thread[2] -> Queue2 (class 2) : round robin 스케줄링 (시간할당량 = 5)
- thread[3] -> Queue3 (class 3) : SJF 스케줄링
-> 더 이상 Queue의 Process가 존재하지 않으면 thread를 종료한다. 

##동기화 
###flag 
3개의 Queue 내부 스케줄러를 제어할 정수형 배열 flag[3] 존재 -> 초기값 = 1
-> flag[0] : thread[1], flag[1] : thread[2], flag[2] : thread[3]  
-> Queue 내부 스케줄러는 자기 자신의 flag값만 변경가능
-> Queue간 스케줄러는 모든 flag값 변경가능 
전역 변수인 3개의 flag사이의 동기화가 필요 -> Semaphore 활용
flag 값
1 : 해당 Queue 내부 스케줄러 block 해제 
0 : 해당 Queue 내부 스케줄러 block 
-1 : 해당 Queue가 비어있고 Queue 내부 스케줄러의 종료를 알림
동작 – thread[1]로 가정 
flag[0] = 1이면 while(!flag[0])를 통과
- Queue가 비어 있다면 flag[0] = -1로 변경 후 thread[1] 종료
- Queue가 비어 있지 않다면 내부 동작 수행 후 flag[0] = 0로 변경
 -> 다음 반복 때 while(!flag[0])을 통과하지 못한 채 block

###Semaphore 
full, empty 두 개의 Semaphore가 존재
empty 
- 공유자원인 ready queue, flag에 대한 접근 제어를 위한 Queue 내부 스케줄러 사이의 동기화 -> 초기값=1
동작 -> flag의 동작도 함께 작성
1. Queue 내부 스케줄러 thread[1], thread[2], thread[3] 중 먼저 접근한 thread가 while(!flag[flag_num]), sem_wait(&empty)를 통과하고 공유자원인 ready queue를 점유한다. 이때 다른 두 개의 thread들은 sem_wait(&empty)에서 block된다.
2. 먼저 접근한 thread가 스케줄링 진행하고 자신의 flag값을 변경시킨 후 다른 Queue 내부 스케줄러의 flag값을 확인한다. -> empty로 인해 확인도중 다른 Queue 내부 스케줄러가 자신의 flag값을 변경하지 못한다.
3. 만약 다른 Queue 내부 스케줄러의 flag값중 하나라도 1이면 sem_post(&empty)를 통해 다른 Queue 내부 스케줄러의 block을 해제한다.


###full 
- Queue간 스케줄러, Queue 내부 스케줄러의 동기화, flag값에 대한 동기화
동작 -> flag의 동작도 함께 작성
1. thread[0]이 생성되고 sem_wait(&full)에 도달하여 block된다. -> full 초기값 = 0 
2. Queue 내부 스케줄러는 내부동작 수행 후 다른 Queue 내부 스케줄러의 flag값을 확인하고 만약 모두 완료되었다면(flag[others] = 0 or -1) sem_post(&full)를 통해 full의 값을 증가시킨다.
3. Queue간 스케줄러가 sem_wait(&full)를 통과해 block이 해제되고 스케줄링을 수행한다.
4. Queue간 스케줄러가 1개의 프로세스를 처리하면 새로운 프로세스를 받아오기 위해 처리된 프로세스가 속한 class의 flag값을 1로 변경, sem_post(&empty)후 sem_wait(&full)로 block상태로 전환된다.
-> 단, flag값이 -1이라면 flag값이 -1이 아닌 다른 Class중 순위가 높은 Class의 flag값을 1로 변경.
5. while(!flag[flag_num])에서 block되어있던 해당 class의 Queue 내부 스케줄러가 while(!flag[flag_num]), sem_wait(&empty)를 통과해 내부 동작을 수행 후 선정된 Process를 ready queue에 적재하고 다른 Queue 내부 스케줄러의 flag값을 확인하고 만약 모두 완료되었다면(flag[others] = 0 or -1) sem_post(&full)를 통해 full의 값을 증가시킨다. 
6. Queue간 스케줄러가 block이 해제되어 다음 작업을 수행한다.
7. 모든 Queue 내부 스케줄러가 종료될 때까지 4,5,6번을 반복한다.

##전체 동작 – 정리
1. Input.txt로부터 프로세스를 정보를 읽어와 프로세스 Class 별로 Queue1, Queue2, Queue3에 저장. 
2. thread[0], thread[1], thread[2], thread[3] 생성
3. thread[0]은 대기상태, thread[1], thread[2], thread[3]는 ready queue에 각자의 스케줄링으로 선정된 프로세스 적재 -> 상단의 설명에 따른 thread간 동기화 지원 
4. thread[1], thread[2], thread[3]의 수행이 전부 완료되면 thread[0]의 block이 해제되고 ready queue에 담긴 3개의 process에 대한 Round Robin 스케줄링 진행 -> output.txt에 프로세스 id별 실행 내역 작성
5. ready queue에 담긴 Process의 처리시간이 완료되면 해당 Process가 속한 Class의 Queue를 가진 Queue 내부 스케줄러를 실행 
-> 만약 해당 Queue 내부 스케줄러가 종료되었다면 class우선순위가 높은 다른 Queue 내부 스케줄러를 실행 
6. thread[0]은 Queue 내부 스케줄러가 ready queue에 적재할 때 까지 block
7. thread[0]의 block이 해제되면 ready queue에 담긴 3개의 process에 대한 Round Robin 스케줄링 진행
8. 모든 Queue 내부 스케줄러가 종료될 때까지 5,6,7번을 반복한다.
9. thread[0] 종료 

##콘솔 출력
테이블 : Queue간 스케줄러인 thread[0]가 현재 ready queue에 존재하는 process정보 출력
Scheduler : ~  -> Queue간 스케줄러가 처리한 프로세스 id 출력 (처리시간 = 횟수) 
