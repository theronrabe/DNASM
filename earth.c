#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define GENOME 200
#define CELLS 1600 
typedef struct { char op; int acc; int dest; } Cell;
typedef struct Organism { int isp; struct Organism *NEXT; } Organism;

Cell *getCell(int loc);
void buildMap();
void birth(char *DNASM);
void live();
int countOrganisms();

Cell Map[CELLS];
Organism *lives;
int I = 0;

void main(int argc, char **argv) {
	buildMap();
	lives = malloc(sizeof(Organism));

	//birth("c 0 1");
	birth("(c 5 2) (< 0 2) (x 0 0) (+ 1 -3) (< 0 -4)");

	while(lives->NEXT) { live(); ++I; }
}

Cell *getCell(int loc) {
	return &Map[(loc%CELLS)];
}

void buildMap() {
	int i;
	for(i=0;i<CELLS;i++)
		Map[i].op = 'x';
}

void birth(char *DNASM) {
	Cell *C;
	char *genes = malloc(sizeof(char)*GENOME);
	strcpy(genes, DNASM);

	Organism *ret = malloc(sizeof(Organism));
	ret->isp = rand()%CELLS;
	ret->NEXT= lives;
	lives = ret;

	int loc = ret->isp;
	char *tok = strtok(genes, " \t\n|()");

	while(tok) {
		C = getCell(loc);
		C->op = tok[0];
		C->acc = atoi(strtok(NULL, " \t\n|()"));
		C->dest = atoi(strtok(NULL, " \t\n|()"));
		printf("new cell: %c %d %d\n", C->op, C->acc, C->dest);
		++loc;
		tok = strtok(NULL, " \t\n|()");
	}

	printf("Organism %p born at %d\n", ret, ret->isp);
}

void live() {
	Organism *o = lives;
	Organism *i = lives;

	while(o->NEXT) {
		Cell *C = getCell(o->isp);
		switch(C->op) {
			case('<'):
				//Jump to closer address
				if(!C->acc) o->isp += C->dest;
				else o->isp += (abs(C->acc) < abs(C->dest))? C->acc:C->dest;
				break;
			case('+'):
				getCell(o->isp + C->dest)->acc += C->acc;
				o->isp++;
				break;
			case('-'):
				getCell(o->isp + C->dest)->acc -= C->acc;
				o->isp++;
				break;
			case('*'):
				getCell(o->isp + C->dest)->acc *= C->acc;
				o->isp++;
				break;
			case('/'):
				getCell(o->isp + C->dest)->acc /= C->acc;
				o->isp++;
				break;
			case('x'):
				//Kill organism
				if(i == o) {
					lives = i->NEXT; free(o);
				} else {
					while(i->NEXT) { if(i->NEXT == o) { i->NEXT = o->NEXT; free(o); } ++i; }
				}
				o = i;
				i = lives;
				printf("Organism %p at %d dies on iteration %d\n", o, o->isp, I);
				break;
			case('r'):
				//Reproduce
				i = malloc(sizeof(Organism));
				i->isp = C->acc;
				i->NEXT = lives;
				lives = i;
				o->isp++;
				break;
			case('c'):
				//Cell duplication
				getCell(o->isp + C->acc)->op = getCell(o->isp + C->dest)->op;
				getCell(o->isp + C->acc)->acc = getCell(o->isp + C->dest)->acc;
				getCell(o->isp + C->acc)->dest = getCell(o->isp + C->dest)->dest;
				//printf("Cell (%c %d %d) duplicated\n", getCell(o->isp + C->acc)->op, getCell(o->isp + C->acc)->acc, getCell(o->isp + C->acc)->dest);
				o->isp++;
				break;
			case('d'):
				//MOV dest
				getCell(o->isp + C->dest)->dest = C->acc;
				o->isp++;
				break;
			case('o'):
				//MOV op
				getCell(o->isp + C->dest)->op = C->acc;
				o->isp++;
				break;
			case('a'):
				//MOV acc
				getCell(o->isp + C->dest)->acc = C->acc;
				o->isp++;
				break;
		}
		o = o->NEXT;
	}
}
