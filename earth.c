#include "gameMachine/game.h"
#include <time.h>

#define GENOME 200
#define CELLS 1200 
typedef struct Cell { char op; int acc; int dest; int col; } Cell;

Object *organism, *camera;
int imgOrganism, imgBlue, imgGreen;
void drawOrganism(Instance *this);
void drawCamera(Instance *this);
void mouseCamera(Instance *this);

Cell *getCell(int loc);
void buildMap();
void birth(char *DNASM);
void live(Instance *i);
int countOrganisms();
void generateCell(Cell *C);

Cell Map[CELLS];
int I = 0;

void startGame(int argc, char **argv) {
	srand(time(NULL));
	GAME.FRAMERATE = 100;
	GAME.STEPTIME = 20;
	createWindow("DNASM", 50,50, 1200, 200);
	organism = createObject();
	camera = createObject();

	imgOrganism= newImage("img/full.png");
	imgBlue = newImage("img/blue.png");
	imgGreen = newImage("img/green.png");
	
	organism->onDraw = drawOrganism;
	organism->onStep = live;
	camera->onDraw = drawCamera;
	camera->onMouseLeft = mouseCamera;
	
	createInstance(camera,0, 0,0,0, 0,0);
	buildMap();

	//birth("(c -1 2) (< 0 2) (x 0 0) (- 2 -3) (< 0 -4)");
	//birth("(c 5 2) (< 0 2) (x 0 0) (+ 6 -3) (< 0 -4)");
	//birth("(c 1 0)");
	//birth("(r 10 0) (+ 10 -1) (< 0 -2)");
}

void generateCell(Cell *C) {
	char ops[] = "<+-c";
	C->op = ops[rand()%strlen(ops)];
	C->acc = (rand()%11)-6;
	C->dest = (rand()%5)-3;
	if(C->op == '/' && C->dest == 0) C->dest = 1;
	if(C->op == 'r') C->acc = rand()%CELLS;
	if(C->op == '<') C->dest = (rand()%21)-17;
	if(C->op == 'c' && C->acc == 1 && !C->dest) C->op = 'x';
}
	
Cell *getCell(int loc) {
	if (loc < 0) loc = CELLS+loc;
	return &Map[(loc%CELLS)];
}

void buildMap() {
	int i;
	for(i=0;i<CELLS;i++)
		//Map[i].op = 'x';
		generateCell(&Map[i]);
}

void birth(char *DNASM) {
	Cell *C;
	char *genes = malloc(sizeof(char)*GENOME);
	strcpy(genes, DNASM);

	Instance *ret = createInstance(organism,0, 0,0,0, 0,0);
	ret->A = rand()%CELLS;

	int loc = ret->A;
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

	free(genes);
	printf("Organism %p born at %d\n", ret, ret->A);
}

void live(Instance *o) {
	Cell *C = getCell(o->A);
	//C->col = 1;
	switch(C->op) {
		case('<'):
			//Jump to closer address
			if((!C->acc)&&(C->dest)) o->A += C->dest;
			else if((!C->dest)&&(C->acc)) o->A += C->acc;
			else o->A += (abs(C->acc) < abs(C->dest))? C->acc:C->dest;
			break;
		case('+'):
			getCell(o->A + C->dest)->acc += C->acc;
			o->A++;
			break;
		case('-'):
			getCell(o->A + C->dest)->acc -= C->acc;
			o->A++;
			break;
		case('*'):
			getCell(o->A + C->dest)->acc *= C->acc;
			o->A++;
			break;
		case('/'):
			getCell(o->A + C->dest)->acc /= C->acc;
			o->A++;
			break;
		case('x'):
			//Kill organism
			//printf("Organism %p at %d dies on iteration %d\n", o, o->A, I);
			destroyInstance(o);
			break;
		case('r'):
			//Reproduce
			/*
			i = malloc(sizeof(Organism));
			i->isp = C->acc;
			i->NEXT = lives;
			lives = i;
			o->isp++;
			*/
			createInstance(organism,0, 0,0,0, 0,0)->A = o->A + C->acc;
			//generateCell(C);
			C->op = 'x';
			break;
		case('c'):
			//Cell duplication
			getCell(o->A + C->acc)->op = getCell(o->A + C->dest)->op;
			getCell(o->A + C->acc)->acc = getCell(o->A + C->dest)->acc;
			getCell(o->A + C->acc)->dest = getCell(o->A + C->dest)->dest;
			getCell(o->A + C->acc)->col = !(getCell(o->A + C->acc)->col);
			//printf("Cell (%c %d %d) duplicated\n", getCell(o->isp + C->acc)->op, getCell(o->isp + C->acc)->acc, getCell(o->isp + C->acc)->dest);
			o->A++;
			break;
		case('d'):
			//MOV dest
			getCell(o->A + C->dest)->dest = C->acc;
			getCell(o->A + C->acc)->col = !(getCell(o->A + C->acc)->col);
			o->A++;
			break;
		case('o'):
			//MOV op
			getCell(o->A + C->dest)->op = C->acc;
			getCell(o->A + C->acc)->col = !(getCell(o->A + C->acc)->col);
			o->A++;
			break;
		case('a'):
			//MOV acc
			getCell(o->A + C->dest)->acc = C->acc;
			getCell(o->A + C->acc)->col = !(getCell(o->A + C->acc)->col);
			o->A++;
			break;
	}
	++I;
}

void drawOrganism(Instance *this) {
	drawImage(imgOrganism, this->A%1200, 120, 2, 8, 0);
	//if(countInstances(organism)>100) destroyInstance(this);
}

void drawCamera(Instance *this) {
	glClearColor(0, 0, 0, 1);
	seeWorld2D(0,0,1200,200);

	int i;
	for(i=0;i<CELLS;i++) {
		Cell *C = getCell(i);
		drawImage((C->col)?imgBlue:imgGreen, i, 100, 1,6,0);
	};

	//if(countInstances(organism) < 2) endGame();
	//if(!(I%2)) createInstance(organism,0, 0,0,0, 0,0)->A = rand()%CELLS;

	if(keyIsHeld('q')) endGame();
	if(keyIsHeld('x')) { wipeInstances(organism); } //GAME.instCount = 3; }
}

void mouseCamera(Instance *this) {
	createInstance(organism,0, 0,0,0, 0,0)->A = GAME.mouseX;
}
