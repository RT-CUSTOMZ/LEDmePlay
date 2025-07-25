
/******************************************************************************

Welcome to GDB Online.
GDB online is an online compiler and debugger tool for C, C++, Python, Java, PHP, Ruby, Perl,
C#, OCaml, VB, Swift, Pascal, Fortran, Haskell, Objective-C, Assembly, HTML, CSS, JS, SQLite, Prolog.
Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <stdio.h>

#define MAX_SNAKE_LEN 100
#define EMPTY -1
#define END -2

struct Point
{
	int x;
	int y;
};

struct ListElement {
	int up;
	int down;
	Point p;
};

ListElement snake[MAX_SNAKE_LEN];

int snakeHead;
int snakeTail;

Point getHead() {
	return snake[snakeHead].p;
};

Point getTail() {
	return snake[snakeTail].p;
};

void addPoint(Point p) {

	//Find empty Element
	int pos=0;

	//Walk through all elements
	do {
		// Do we find an empty element?
		if(snake[pos].up==EMPTY)
			break;
		pos++;
	}
	while(pos!=MAX_SNAKE_LEN);

	if(pos==MAX_SNAKE_LEN) {
		printf("Error: No space left in array. ");
		do {} while(true);
	}

	//Found a gap. Writing the new element.
	snake[pos].p=p;
	snake[pos].down=snakeHead;
	snake[pos].up=END;

	//Give the old head the pointer of the new head
	snake[snakeHead].up=pos;

	// Remember the new head
	snakeHead=pos;
}

void removePoint() {
	//Who is the next tail?
	int newTail=snake[snakeTail].up;

	//Is the Tail equal to the head?
	if(newTail==END) {
		printf("Error: Can`t remove the head of the snake. That would kill it. ;-)");
		do {} while(true);
	}

	//Cut the Tail
	snake[newTail].down=END;

	//free old element
	snake[snakeTail].up=EMPTY;
	
	//Remember new tail
	snakeTail=newTail;
}

void init(Point p) {

	for(int i=0; i<MAX_SNAKE_LEN; i++)
		snake[i].up=EMPTY;

	snakeHead=0;
	snakeTail=0;

	snake[snakeHead].p=p;
	snake[snakeHead].up=END;
	snake[snakeHead].down=END;
}
void printPoint(Point p) {
	printf("P(%d,%d)\n", p.x,p.y);

}
void printArray(){
    for(int i=0;i<MAX_SNAKE_LEN;i++){
        ListElement e=snake[i];
        printf("%d: u=%d d=%d ",i,e.up,e.down);
        printPoint(e.p);
    }
    
}

void testSnake() {

	ListElement aktElem=snake[snakeHead];
	
	printPoint(aktElem.p);
	
	do {
		aktElem=snake[aktElem.down];
		printPoint(aktElem.p);
	} while(aktElem.down!=END);

}

int test()
{
	init({10,10});

	addPoint({12,12});
	addPoint({14,14});
	addPoint({15,15});
	addPoint({16,16});
	addPoint({17,17});

	testSnake();
    printArray();
    
    printf("Head:\n");
	printPoint(getHead());

    printf("Tail:\n");
	printPoint(getTail());

    printf("Remove Tail\n");
    removePoint();

    printf("Tail:\n");
	printPoint(getTail());

    printf("\nNew Snake\n");
    testSnake();
    
    
	return 0;
}
