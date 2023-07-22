#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Page {
    char * lines[10]; // 각 요소가 char *인 크기가 10인 배열
    int count; //현재 페이지에 저장된 line 수
    struct Page * prevPage; // 이전 페이지의 포인터
    struct Page * nextPage; // 이후 페이지의 포인터
} Page;

Page* new_page() { // 새 페이지를 만드는 메소드
    Page* page = (Page*)malloc(sizeof(Page));
    page->count = 0;
    page->prevPage = NULL;
    page->nextPage = NULL;
    return page;
}

Page* current_Page; // 현재 보고있는 페이지

int countSum(Page* page) { // page부터 마지막 페이지까지 count를 더해서 반환하는 메소드
    int sum = page->count;
    while (page->nextPage != NULL) {
        sum += page->nextPage->count;
        page = page->nextPage;
    }
    return sum;
}

Page* firstPage(Page* page) { // 제일 첫 페이지를 반환하는 메소드
    while (page->prevPage != NULL) {
        page = page->prevPage;
    }
    return page;
}

Page* lastPage(Page* page) { // 제일 마지막 페이지를 반환하는 메소드
    while (page->nextPage != NULL) {
        page = page->nextPage;
    }
    return page;
}

void reText(FILE * fp, Page * page) { // page처럼 .txt 파일을 다시 쓰는 메소드
    while (page != NULL) {
        for (int i = 0; i < page->count; i++) {
            fprintf(fp, "%s", page->lines[i]);
        }
        page = page->nextPage;
    }
}

void append_line(Page* page, char* linetext) {
    if (page->count >= 5) { // 5줄 이상 채워져있으면 새로운 페이지 생성
        Page* changed_Page = new_page();
        page->nextPage = changed_Page;
        changed_Page->prevPage = page;
        page = changed_Page;
        current_Page = page;
    }
    page->lines[page->count++] = strdup(linetext);
}

void insert_line(Page* page, int i, char* linetext) {
    page = firstPage(page); // 첫 페이지로 초기화
    int counts = countSum(page); // 모든 count의 합
    if (i == 0 || counts < i) { // i가 valid 한 지 검사
        printf("index is invalid\n");
        return;
    }

    int insert_Index = i; // 삽입되어야 하는 인덱스
    while (1) {
        if (insert_Index - page->count > 0) { // 삽입해야되는 페이지로 이동
            insert_Index -= page->count;
            page = page->nextPage;
        } else {
            insert_Index--; // 인덱스 맞게 바꿈
            break;
        }
    }

    char * temp1;
    char * temp2;
    int a = insert_Index;
    // 삽입할 자리에 있던 원소를 temp1에 저장하고 그 자리에 삽입하는 과정
    temp1 = page->lines[a];
    page->lines[a++] = strdup(strcat(strdup(linetext), "\n"));

    while (a < page->count) { // 나머지 원소를 뒤로 한 칸씩 미루어주는 과정
        temp2 = page->lines[a];
        page->lines[a] = temp1;
        temp1 = temp2;
        a++;
    }

    if (a == 10) { // 마지막 원소가 들어갈 곳이 없으면 새로운 페이지 생성 후 삽입
        Page * newPage = new_page();
        page->nextPage->prevPage = newPage;
        newPage->nextPage = page->nextPage;
        page->nextPage = newPage;
        newPage->prevPage = page;
        newPage->lines[0] = temp1;
        page = newPage;
    } else { // 마지막 원소가 들어갈 곳이 있으면 그냥 삽입
        page->lines[a] = temp1;
    }
    page->count++; // 삽입 후 count 증가
}

void remove_line(Page* page, int i) {
    page = firstPage(page); // 첫 페이지로 초기화
    int counts = countSum(page); // 모든 count의 합
    if (counts == 0) { // count == 0이면? 오류 메시지 출력 후 return
        printf("count is 0\n");
        return;
    }
    if (i == 0 || counts < i) { // i가 valid 한 지 검사
        printf("index is invalid\n");
        return;
    }

    int remove_Index = i;
    while (1) {
        if (remove_Index - page->count > 0) {
            remove_Index -= page->count;
            page = page->nextPage;
        } else {
            remove_Index--; // 인덱스 맞게 바꿈
            break;
        }
    }
    // line 순서를 바꿔서 삭제해야될 line을 마지막 인덱스로 옭기는 과정
    int a = remove_Index;
    while (a+1 < page->count) { // a가 마지막 line에 도착하면 멈춤
        page->lines[a] = page->lines[a+1]; // 한 line씩 앞으로 당김
        a++;
    }
    page->lines[--page->count] = NULL; // 삭제 후 count 감소
}

int main(int argc, char *argv[]) {
    char * filename = argv[2];
    FILE * fp = fopen(filename, "r");
    char line[80];
    current_Page = new_page();
    while (fgets(line, 80, fp) != NULL) { // 이미 쓰여져 있는 페이지를 읽어오기
        append_line(current_Page, line);
    }
    fclose(fp);

    while (1) { // 단발성 실행이 아닌 연속적인 명령어 입력
        char command[100];
        char * words[3];
        int wordCnt = 0;
        scanf("%[^\n]s", command);
        if (strcmp(command, "exit") == 0) { // exit이 입력되면 종료
            break;
        }
        char * ptr = strtok(command, " ");
        while (ptr != NULL) {
            words[wordCnt] = malloc(strlen(ptr) + 1);
            strcpy(words[wordCnt++], ptr);
            ptr = strtok(NULL, " ");
        }

        if (strcmp(words[0], "-a") == 0) {
            append_line(lastPage(current_Page), strcat(words[1], "\n"));
            FILE * fp = fopen(filename, "a");
            fprintf(fp, "%s", words[1]); // .txt의 마지막 줄에 쓰기 
            fclose(fp);
        } else if (strcmp(words[0], "-i") == 0) {
            insert_line(current_Page, atoi(words[1]), words[2]);
            FILE * fp = fopen(filename, "w");
            reText(fp, firstPage(current_Page));
            fclose(fp);
        } else if (strcmp(words[0], "-r") == 0) {
            remove_line(current_Page, atoi(words[1]));
            FILE * fp = fopen(filename, "w");
            reText(fp, firstPage(current_Page));
            fclose(fp);
        }

        Page* checkP = firstPage(current_Page);
        int pageCnt = 1;
        while (checkP != NULL) {
            printf("========== Page %d (count = %d) ==========\n", pageCnt++, checkP->count);
            for (int j = 0; j < checkP->count; j++) {
                printf("line[%d] : %s", j, checkP->lines[j]);
            }
            checkP = checkP->nextPage;
        }
        scanf(" "); // \n 입력 받음
    }
    return 0;
}