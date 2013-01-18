#include "gameMachine/game.h"
#include <time.h>

#define GENOME 200
#define CELLS 1200
#define MINLIVES 10
typedef struct Cell { char op; int acc; int dest; int col; } Cell;

Object *organism, *camera;
int imgOrganism, imgBlue, imgGreen, imgOrange;
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
	GAME.STEPTIME = 50;
	createWindow("DNASM", 50,50, 1200, 200);
	organism = createObject();
	camera = createObject();

	imgOrganism= newImage("img/full.png");
	imgBlue = newImage("img/blue.png");
	imgGreen = newImage("img/green.png");
	imgOrange = newImage("img/orange.png");
	
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
	char ops[] = "<<+++---xdaco";
	C->op = ops[rand()%strlen(ops)];
	C->acc = (rand()%16)-8;
	C->dest = (rand()%16)-8;
	if(C->op == '<') {
		int r = -(rand()%15)-1;
		C->dest = r;
		C->acc = 0;
	}
}
	
Cell *getCell(int loc) {
	if (loc < 0) loc = CELLS+loc;
	return &Map[(loc%CELLS)];
}

void buildMap() {
	int i;
	for(i=0;i<CELLS;i++) {
		//Map[i].op = 'x';
		generateCell(&Map[i]);
		Map[i].col = 0;
	}
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
	Cell *cc, *C = getCell(o->A);
	Cell *ca = getCell(o->A + C->acc), *cd = getCell(o->A + C->dest);

	C->col = 2;
	//printf("o");
	switch(C->op) {
		case('<'):
			//printf("<");
			//Jump to closer address
			if((!C->acc)&&(C->dest)) o->A += C->dest;
			else if((!C->dest)&&(C->acc)) o->A += C->acc;
			else o->A += (abs(C->acc) < abs(C->dest))? C->acc:C->dest;
			break;
		case('+'):
			//printf("+");
			//cc = getCell(o->A + C->dest);
			//cc->acc = (cc->acc+C->acc)%CELLS;
			cd->dest = (cd->dest+C->acc)%CELLS;
			cd->col = 1;
			o->A++;
			break;
		case('-'):
			//printf("-");
			//cc = getCell(o->A + C->dest);
			//cc->acc = (cc->acc-C->acc)%CELLS;
			cd->dest = (cd->dest-C->acc)%CELLS;
			cd->col = 1;
			o->A++;
			break;
		case('*'):
			//printf("*");
			cc = getCell(o->A + C->dest);
			cc->acc = (cc->acc*C->acc)%CELLS;
			o->A++;
			break;
		case('/'):
			//printf("/");
			if(C->acc) {
				cc = getCell(o->A + C->dest);
				cc->acc = (cc->acc/C->acc)%CELLS;
			}
			o->A++;
			break;
		case('x'):
			//Kill organism
			//printf("X");
			destroyInstance(o);
			break;
		case('r'):
			//Reproduce
			createInstance(organism,0, 0,0,0, 0,0)->A = o->A + C->acc;
			//generateCell(C);
			C->op = 'x';
			break;
		case('c'):
			//printf("c");
			//Cell duplication
			if(C->acc == 1 && !C->dest) { destroyInstance(o); break; }

			ca->op = cd->op;
			ca->acc = cd->acc;
			ca->dest = cd->dest;
			ca->col = 2;
			cd->col = 1;
			o->A++;
			break;
		case('d'):
			//printf("d");
			//MOV dest
			cd->dest = C->acc;
			cd->col = 1;
			o->A++;
			break;
		case('o'):
			//printf("o");
			//MOV occp
			cd->op = ca->op;
			ca->col = 2;
			cd->col = 1;
			o->A++;
			break;
		case('a'):
			//printf("a");
			//MOV acc
			cd->acc = C->acc;
			cd->col = 1;
			o->A++;
			break;
	}
	//printf("|");
	++I;
}

void drawOrganism(Instance *this) {
	drawImage(imgOrganism, this->A%CELLS, 120, 2, 8, 0);
	//if(countInstances(organism)>100) destroyInstance(this);
}

void drawCamera(Instance *this) {
	glClearColor(0.1, 0.1, 0.1, 1);
	seeWorld2D(0,0,1200,200);

	int i;
	for(i=0;i<CELLS;i+=3) {
		Cell *C = getCell(i);
		switch(C->col) {
			case(0):
				drawImage(imgGreen, i, 100, 1,6,0);
				break;
			case(1):
				drawImage(imgBlue, i, 100, 1,6,0);
				break;
			case(2):
				drawImage(imgOrange, i, 100, 1,6,0);
				break;
		}
		drawImage((C->col)?imgBlue:imgGreen, i, 100, 1,6,0);
		}

	//if(countInstances(organism) < 10) endGame();
	if(countInstances(organism) < MINLIVES) {
		createInstance(organism,0, 0,0,0, 0,0)->A = rand()%CELLS;
		printf("Birth on %d\n", I);
	}

	if(keyIsHeld('q')) endGame();
	if(keyIsHeld('x')) { wipeInstances(organism); buildMap(); } //GAME.instCount = 3; }
	if(keyIsHeld('c')) {
		int i;
		for(i=0;i<CELLS;i++) Map[i].col = 0;
	}
	//printf("\n");
}

void mouseCamera(Instance *this) {
	createInstance(organism,0, 0,0,0, 0,0)->A = GAME.mouseX;
}
