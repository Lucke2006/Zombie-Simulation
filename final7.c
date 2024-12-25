#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define PI 3.14
#define N 300
#define L 500.0
#define RADIUS 5.0
#define GUN_RADIUS 2.0
#define MAX_VELOCITY 10.0
#define RATE 16 // ms between time updates
#define TIME_STEP 0.1
#define RECOVERY_TIME 20.0
#define WALL_BREAK 3.0
#define BULLET_TIME 1.0
int wallDown1 = 0;
int wallDown2 = 0;
int wallDown3 = 0;

typedef struct {
	double x, y;
	int claimed;
} Gun;

typedef struct {
	double x1,x2,y;
}hWall;

typedef struct {
	double x,y1,y2;
}vWall;

typedef struct {
	double x, y, vx, vy;
	int available;
	double air_time;
	double air_duration;
} bullet;

typedef struct {  // define the traits of a person
	double x, y, vx, vy;
	int infected;
	double Svx, Svy; //SAVE VALUES
	double wallTime; //TIME SPENT ON WALL
	double wallDuration;
	int hitWall;
	int gun;
	int dead;
	int checked;
} Person;

void saveVal(Person *a){
	a->Svx = a->vx;
	a->Svy = a->vy;	
}
void zero(Person *a){
	a->vx = 0;
	a->vy = 0;
}

void giveVel(Person *a){
	a->vx = a->Svx;
	a->vy = a->Svy;
}


//SETS VERTICAL WALL VALUES
void vWallValues(double x,double y1, double y2,vWall* wall){
	wall->x = x;
	wall->y1 = y1;
	wall->y2 = y2;
}

//SETS HORIZONTAL WALL VALUES
void hWallValues(double x1, double x2, double y, hWall* wall){
	wall->x1 = x1;
	wall->x2 = x2;
	wall->y = y;
}


vWall vWalls[20];
hWall hWalls[20];
Person person[N]; // create N persons
Gun gun[3];
int guns[3] = {0};
bullet bullets[5]; //ARRAY TO HOLD BULLET INFO
time_t start_time;
double elapsed_time = 0.0;
int infected = 0;
int dead = 0;


void initialize() {
	vWallValues(-L,-L/4,L/4,&vWalls[0]); //WALL 1
	hWallValues(-L,-L/2,L/4,&hWalls[0]); //WALL 2
	hWallValues(-L/2,-L/3,L/4,&hWalls[11]); //DOOR3
	vWallValues(-L/3,L/4,L,&vWalls[1]); //WALL 3
	hWallValues(-L/3,L/3,L,&hWalls[1]); //WALL 4
	vWallValues(L/3,L/4,L,&vWalls[2]); //WALL 5
	hWallValues(L/3,L,L/4,&hWalls[2]); //WALL 6
	vWallValues(L,-L/4,L/4,&vWalls[3]); //WALL 7
	hWallValues(L/2,L,-L/4,&hWalls[3]); //WALL 8
	vWallValues(L/3,-L,-L/4,&vWalls[4]); //WALL 9
	hWallValues(-L/3,L/3,-L,&hWalls[4]); //WALL 10
	vWallValues(-L/3,-L,-L/3,&vWalls[5]); //WALL 11
	hWallValues(-L,-L/3,-L/4,&hWalls[5]); //WALL 12
	vWallValues(L,-L,-L/4,&vWalls[6]); //GLASS ROOM RIGHT VERTICAL WALL
	hWallValues(L/3,L,-L,&hWalls[6]); //GLASS ROOM RIGHT HORIZONTAL WALL	
	hWallValues(L/3,L/2,-L/4,&hWalls[7]); //DOOR 1	
	vWallValues(-L,-L,-L/4,&vWalls[7]); //OFFICE VERTICAL WALL
	hWallValues(-L,-L/3,-L,&hWalls[8]); //OFFICE HORIZONTAL WALL	
	vWallValues(-L/3,-L/3,-L/4,&vWalls[8]); //DOOR 2
	hWallValues(-L,-L/3,L,&hWalls[9]); //HORIZONTAL WALL IN MECHANICAL ROOM	
	vWallValues(-L,L/4,L,&vWalls[9]); //VERTICAL WALL IN MECHANICAL ROOM	
	hWallValues(L/3,L,L,&hWalls[10]); //HORIZONATAL WALL IN TOP RIGH	
	vWallValues(L,L/4,L,&vWalls[10]); //HORIZONATAL WALL IN TOP RIGH	

	
	glClearColor(0.0, 0.0, 0.0, 1.0);
	gluOrtho2D(-1.2*L,1.2*L,-1.2*L,1.2*L);
	float sign;
	for (int i = 0; i < N; i++){
		sign = (rand()%2)? -1.0 : 1.0;
		person[i].x = 2*L*((double)rand()/RAND_MAX)-L;
		person[i].y = 2*L*((double)rand()/RAND_MAX)-L;
		person[i].vx = sign*((double)rand()/RAND_MAX)*MAX_VELOCITY;
		person[i].vy = sign*((double)rand()/RAND_MAX)*MAX_VELOCITY;
		person[i].infected = 0;
		person[i].checked = 0;
	}

	//MAKE 3 GUNS APPEAR AND FIXES OUT OF BOUNDS
	for (int i = 0; i < 3; i++){
		gun[i].x = 2*L*((double)rand()/RAND_MAX)-L; 
		gun[i].y = 2*L*((double)rand()/RAND_MAX)-L;
	}
	for (int i = 0; i < 3; i++){
		if (gun[i].x < -L/3 - 10 && gun[i].y > L/4 + 5){
			gun[i].x = 2*L*((double)rand()/RAND_MAX)-L;
			gun[i].y = 2*L*((double)rand()/RAND_MAX)-L;
		}
		if (gun[i].x > L/3 + 10 && gun[i].y > L/4 + 5){
			gun[i].x = 2*L*((double)rand()/RAND_MAX)-L;
			gun[i].y = 2*L*((double)rand()/RAND_MAX)-L;
		}
		if (gun[i].y < -L - 10){
			gun[i].x = 2*L*((double)rand()/RAND_MAX)-L;
			gun[i].y = 2*L*((double)rand()/RAND_MAX)-L;
		}
		if (gun[i].y > L + 10){
			gun[i].x = 2*L*((double)rand()/RAND_MAX)-L;
			gun[i].y = 2*L*((double)rand()/RAND_MAX)-L;
		}
		if (gun[i].x < -L - 10){
			gun[i].x = 2*L*((double)rand()/RAND_MAX)-L;
			gun[i].y = 2*L*((double)rand()/RAND_MAX)-L;
		}
		if (gun[i].x > L + 10){
			gun[i].x = 2*L*((double)rand()/RAND_MAX)-L;
			gun[i].y = 2*L*((double)rand()/RAND_MAX)-L;
		}
	}
	
	//MAKE EVERY BULLET AVAILABLE
	for(int i=0; i<5; i++){
		bullets[i].available = 1;
		bullets[i].x = -2*L; //MOVE OUT OF BOUNDS
	}
	
	// set one sphere to red
	person[0].infected = 1;
	infected++;
}

//MOVE BULLETS
void moveBullet(){
	for(int i=0; i<5; i++){
		//CHECK FOR HOW LONG BULLET IS MID AIR
		if(bullets[i].available == 0){
			bullets[i].air_duration = elapsed_time - bullets[i].air_time;
			if(bullets[i].air_duration > BULLET_TIME){
				bullets[i].available = 1;
				bullets[i].vx = 0;
				bullets[i].vy = 0;
				bullets[i].y = -2*L;
			}
		}
		bullets[i].x += bullets[i].vx*TIME_STEP; //CHANGES X AND Y COORDINATES
		bullets[i].y += bullets[i].vy*TIME_STEP;
	}
}

//COLLISIONS FOR VERTICAL WALLS
void verticalCollision(vWall* w, Person* p){
	if(p->vx > 0){
		if(p->x + RADIUS > w->x && p->x + RADIUS < w->x + 1 && p->y < w->y2 && p->y > w->y1){
			p->vx *= -1;
		}	
	}
	if(p->vx < 0){
		if(p->x - RADIUS < w->x && p->x - RADIUS > w->x - 1 && p->y < w->y2 && p->y > w->y1){
			p->vx *= -1;
		}	
	}
}

//COLLISIONS FOR HORIZONTAL WALLS
void horizontalCollision(hWall* w, Person* p){
	if(p->vy > 0){
		if(p->x > w->x1 && p->x < w->x2 && p->y + RADIUS > w->y && p->y + RADIUS < w->y + 1){
			p->vy *= -1;
		}	
	}
	if(p->vy < 0){
		if(p->x > w->x1 && p->x < w->x2 && p->y - RADIUS < w->y && p->y - RADIUS > w->y - 1){
			p->vy *= -1;
		}
	}
}

//COLLISIONS FOR HORIZONTAL DOORS
void doorCollisionH(hWall* w, Person* p){
	if(p->vy > 0){
		if(p->x > w->x1 && p->x < w->x2 && p->y + RADIUS > w->y && p->y + RADIUS < w->y + 1){
			p->vy *= -1;
			if(p->infected > 0 && p->hitWall == 0){
				saveVal(p);
				zero(p);
				p->wallTime = elapsed_time;
				p->hitWall = 1;
			}
		}	
	}
	if(p->vy < 0){
		if(p->x > w->x1 && p->x < w->x2 && p->y - RADIUS < w->y && p->y - RADIUS > w->y - 1){
			p->vy *= -1;
			if(p->infected > 0 && p->hitWall == 0){
				saveVal(p);
				zero(p);
				p->wallTime = elapsed_time;
				p->hitWall = 1;
			}
		}	
	}
}

//COLLISIONS FOR VERTICAL DOORS
void doorCollisionV(vWall* w, Person* p){
	if(p->vx > 0){
		if(p->x + RADIUS > w->x && p->x + RADIUS < w->x + 1 && p->y > w->y1 && p->y < w->y2){
			p->vx *= -1;
			if(p->infected > 0 && p->hitWall == 0){
				saveVal(p);
				zero(p);
				p->wallTime = elapsed_time;
				p->hitWall = 1;
			}
		}	
	}
	if(p->vx < 0){
		if(p->x - RADIUS < w->x && p->x - RADIUS > w->x - 1 && p->y > w->y1 && p->y < w->y2){
			p->vx *= -1;
			if(p->infected > 0 && p->hitWall == 0){
				saveVal(p);
				zero(p);
				p->wallTime = elapsed_time;
				p->hitWall = 1;
			}
		}	
	}
}


// move people
void movePeople() {
	for (int i = 0; i < N; i++) { //CHECK IF PERSON IS INFECTED
		//CHECK TO OPEN WALL AFTER TIME
		if(person[i].hitWall == 1){
			person[i].wallDuration = elapsed_time - person[i].wallTime;
			if(person[i].wallDuration >= WALL_BREAK){
				giveVel(&person[i]);
				person[i].hitWall = 0;
				if(person[i].x > 0 && person[i].y < 0) wallDown1 = 1;
				if(person[i].x < 0 && person[i].y < 0) wallDown2 = 1;
				if(person[i].x < 0 && person[i].y > 0) wallDown3 = 1;
			}
		}
	 
		person[i].x += person[i].vx*TIME_STEP; //CHANGES X AND Y COORDINATES
		person[i].y += person[i].vy*TIME_STEP;
		// Wall collisions (perfectly elastic)
		//REFLECTIONS

		//1
		verticalCollision(&vWalls[0],&person[i]);

		//2
		horizontalCollision(&hWalls[0],&person[i]);
		
		//3
		verticalCollision(&vWalls[1],&person[i]);
		
		//4
		horizontalCollision(&hWalls[1],&person[i]);
	
		//5	
		verticalCollision(&vWalls[2],&person[i]);
	
		//6
		horizontalCollision(&hWalls[2],&person[i]);
	
		//7
		verticalCollision(&vWalls[3],&person[i]);

		//8
		horizontalCollision(&hWalls[3],&person[i]);
		
		//9	
		verticalCollision(&vWalls[4],&person[i]);
		
		//10
		horizontalCollision(&hWalls[4],&person[i]);

		//11	
		verticalCollision(&vWalls[5],&person[i]);
		
		//12
		horizontalCollision(&hWalls[5],&person[i]);
	
		//GLASS BOTTOM RIGHT V
		verticalCollision(&vWalls[6],&person[i]);
		
		//GLASS BOTTOM RIGHT H
		horizontalCollision(&hWalls[6],&person[i]);	

		//DOOR 1
		if(wallDown1 != 1){
			doorCollisionH(&hWalls[7],&person[i]);	
		}
	
		//OFFICE VERTICAL WALL
		verticalCollision(&vWalls[7],&person[i]);
		
		//OFFICE HORIZONTAL WALL
		horizontalCollision(&hWalls[8],&person[i]);
	
		//DOOR 2
		if(wallDown2 != 1){
			doorCollisionV(&vWalls[8],&person[i]);	
		}
		
		//HORIZONTAL WALL IN MECHANICAL ROOM
		horizontalCollision(&hWalls[9],&person[i]);
		
		//VERTICAL WALL IN MECHANICAL ROOM
		verticalCollision(&vWalls[9],&person[i]);
		
		//HORIZONATAL WALL IN TOP RIGH
		horizontalCollision(&hWalls[10],&person[i]);
	
		//VERTICAL WALL IN TOP RIGHT
		verticalCollision(&vWalls[10],&person[i]);
		
		//DOOR 3
		if(wallDown3 != 1){
			doorCollisionH(&hWalls[11],&person[i]);
		}	
		
		//MOVE PEOPLE THAT ARE OUT OF BOUNDS
		/*if (person[i].x < -L/3 - 10 && person[i].y > L/4 + 5){
			person[i].x = 2*L*((double)rand()/RAND_MAX)-L;
			person[i].y = 2*L*((double)rand()/RAND_MAX)-L;
		}
		if (person[i].x > L/3 + 10 && person[i].y > L/4 + 5){
			person[i].x = 2*L*((double)rand()/RAND_MAX)-L;
			person[i].y = 2*L*((double)rand()/RAND_MAX)-L;
		}
		if (person[i].y < -L - 10){
			person[i].x = 2*L*((double)rand()/RAND_MAX)-L;
			person[i].y = 2*L*((double)rand()/RAND_MAX)-L;
		}
		if (person[i].y > L + 10){
			person[i].x = 2*L*((double)rand()/RAND_MAX)-L;
			person[i].y = 2*L*((double)rand()/RAND_MAX)-L;
		}
		if (person[i].x < -L - 10){
			person[i].x = 2*L*((double)rand()/RAND_MAX)-L;
			person[i].y = 2*L*((double)rand()/RAND_MAX)-L;
		}
		if (person[i].x > L + 10){
			person[i].x = 2*L*((double)rand()/RAND_MAX)-L;
			person[i].y = 2*L*((double)rand()/RAND_MAX)-L;
		}*/
	}
}

void checkCollisions() {
	float dx = 0;
	float dy = 0;
	float dr = 0;
	float Gdx = 0;
	float Gdy = 0;
	float Gdr = 0;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if(j != i){
				dx = person[i].x-person[j].x;
				dy = person[i].y-person[j].y;
				dr = dx*dx + dy*dy;
				if( dr < RADIUS*RADIUS ){
					if ((person[i].infected + person[j].infected) == 1){
						if(person[i].infected == 0 && person[i].dead == 0) {
							person[i].infected = 1;
							infected++;
						}
						if(person[j].infected == 0 && person[j].dead == 0) {
							person[j].infected = 1;
							infected++;
						}
					}
				}
			}
		}
	}
	//GRAB GUN
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < 3; j++) {
			if(j != i){
				Gdx = person[i].x-gun[j].x;
				Gdy = person[i].y-gun[j].y;
				Gdr = Gdx*Gdx + Gdy*Gdy;
				if( Gdr < RADIUS*RADIUS ){
					if (person[i].infected == 0 && person[i].gun < 1){
						person[i].gun++;
						gun[j].claimed = 1;
						gun[j].x = -2*L;
						guns[j]++;						
					}
				}
			}
		}
	}
}
	

//SHOOT
void shoot(){
	float dx = 0;
	float dy = 0;
	float dr = 0;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if(j != i){
				dx = person[i].x-person[j].x;
				dy = person[i].y-person[j].y;
				dr = dx*dx + dy*dy;
				if( dr < 50*RADIUS*RADIUS ){ 
					if ((person[i].gun + person[j].gun) == 1 && (person[i].infected + person[j].infected) == 1){
						for(int b=0; b<5; b++){
							if(person[i].gun == 1 && bullets[b].available == 1) {
								bullets[b].x = person[i].x;
								bullets[b].y = person[i].y;
								bullets[b].vx = person[j].x - person[i].x;
								bullets[b].vy = person[j].y - person[i].y;
								bullets[b].available = 0;
								bullets[b].air_time = elapsed_time;
								break;						
							}
							if(person[j].gun == 1 && bullets[b].available == 1) {
								bullets[b].x = person[j].x;
								bullets[b].y = person[j].y;
								bullets[b].vx = person[i].x - person[j].x;
								bullets[b].vy = person[i].y - person[j].y;
								bullets[b].available = 0;
								bullets[b].air_time = elapsed_time;
								break;
							}
						}
					}
				}
			}
		}
	}
}

void bulletHit(){
	float dx = 0;
	float dy = 0;
	float dr = 0;	
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < 5; j++) {
			if(j != i){ // don't infect yourself
				dx = person[i].x-bullets[j].x;
				dy = person[i].y-bullets[j].y;
				dr = dx*dx + dy*dy;
				if( dr < RADIUS*RADIUS + 10 && person[i].gun != 1){
					person[i].dead = 1;
					zero(&person[i]);
					bullets[j].available = 1;
					bullets[j].x = -2*L;
					bullets[j].vx = 0;
					bullets[j].vy = 0;
					if(person[i].infected == 1){
						person[i].infected = 0;
					}
				}
			}
		}
	}
}

//DROPS GUN IF PERSON IS INFECTED
void dropGun(){
	for(int i=0; i<N; i++){
		if(person[i].infected == 1 && person[i].gun >= 1){
			for(int g=0; g<3; g++){
				if(guns[g]%2 != 0){
					gun[g].x = person[i].x;
					gun[g].y = person[i].y;
					gun[g].claimed = 0;
					person[i].gun--;
					break;
				}
			}
		}		
	}
}

void checkDead(){
	for(int i=0; i<N; i++){
		if(person[i].dead == 1 && person[i].checked != 1){
			dead++;
			person[i].checked = 1;
		}
		if(person[i].dead == 1 && person[i].infected == 1){
			infected--;	
			person[i].infected = 0;
		}
	}
}

void drawWalls(){
	glColor3f(1.0, 1.0, 1.0);
	glLineWidth(1.0);
	glBegin(GL_LINES);
		for(int i=0; i<11; i++){
			if(wallDown2 != 1 && i == 8){
				glVertex3f (vWalls[i].x, vWalls[i].y1, 0.0);
				glVertex3f (vWalls[i].x, vWalls[i].y2, 0.0);
			}
			if(i != 8){
				glVertex3f (vWalls[i].x, vWalls[i].y1, 0.0);
				glVertex3f (vWalls[i].x, vWalls[i].y2, 0.0);
			}
		}
		for(int i=0; i<12; i++){
			if(wallDown1 != 1 && i == 7 || wallDown3 != 1 && i == 11){
				glVertex3f (hWalls[i].x1, hWalls[i].y, 0.0);
				glVertex3f (hWalls[i].x2, hWalls[i].y, 0.0);
			}
			if(i != 7 || i != 11){
				glVertex3f (hWalls[i].x1, hWalls[i].y, 0.0);
				glVertex3f (hWalls[i].x2, hWalls[i].y, 0.0);
			}
		}
	glEnd();
}

void drawPeople(double r){
	int sides = 20;
	glEnable(GL_LINE_SMOOTH);
	glColor3f(1.0, 0.0, 0.0);
	glLineWidth(2.0);
	for (int i = 0; i < N; i++) { // different colored people
		if( person[i].infected == 0 ) glColor3f(0.0,0.0,1.0); // Susceptible Blue
		if( person[i].infected == 1 ) glColor3f(1.0,0.0,0.0); // Infected Red
		if( person[i].gun != 0 ) glColor3f(1.0,1.0,0.0); //GRABBED A GUN
		if( person[i].dead == 1 ) glColor3f(0.0,1.0,1.0); //DEAD;
		float x = person[i].x;
		float y = person[i].y;
		glBegin(GL_POLYGON); // people are circles
			for(int k=0; k<sides; k++){
				glVertex2f(x+r*cos(2*PI*k/sides),y+r*sin(2*PI*k/sides));
				glVertex2f(x+r*cos(2*PI*(k+1)/sides),y+r*sin(2*PI*(k+1)/sides));
			}
		glEnd();
	}
}

void drawGun(double r){
	int sides = 20;
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0);
	for (int i = 0; i < 3; i++){
		if(gun[i].claimed == 0){
			float x = gun[i].x;
			float y = gun[i].y;
			glColor3f(1.0,1.0,0.0);
			glBegin(GL_POLYGON);
				for(int k=0; k<sides; k++){
					glVertex2f(x+r*cos(2*PI*k/sides),y+r*sin(2*PI*k/sides));
					glVertex2f(x+r*cos(2*PI*(k+1)/sides),y+r*sin(2*PI*(k+1)/sides));
				}
			glEnd();
		}
	}
}

void drawBullets(double r){
	int sides = 20;
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0);
	for (int i = 0; i < 5; i++){
		float x = bullets[i].x;
		float y = bullets[i].y;
		glColor3f(1.0,1.0,0.0);
		glBegin(GL_POLYGON);
			for(int k=0; k<sides; k++){
				glVertex2f(x+r*cos(2*PI*k/sides),y+r*sin(2*PI*k/sides));
				glVertex2f(x+r*cos(2*PI*(k+1)/sides),y+r*sin(2*PI*(k+1)/sides));
			}
		glEnd();
	}
}


// Display callback function
//REFRESHES SCREEN
void display() { // every timestep 
	glClear(GL_COLOR_BUFFER_BIT);
	drawWalls(); 
	drawPeople(RADIUS);
	drawGun(RADIUS);
	shoot();
	drawBullets(RADIUS/2);
	moveBullet();
	bulletHit();
	dropGun();
	checkDead();
	glutSwapBuffers();
}

// Timer callback for each timestep
//MOVES PEOPLE, CHECKS FOR COLLISIONS, AND UPDATES THE TIME
void timer(int value) {
	movePeople();
	checkCollisions();
	elapsed_time = difftime(time(NULL),start_time);;
	// print out the data each time step
	// time, Susceptible, Infected, Recovered
	printf("%lf %d %d %d\n", elapsed_time, N-infected-dead, infected, dead);
	if(infected + dead == N){
		fprintf(stderr, "FIRST EXIT\n");
		exit(0);
	} // end when no infected left
	else if(infected == 0){
		fprintf(stderr, "SECOND EXIT\n");
		exit(0);
	}
	glutPostRedisplay();
	glutTimerFunc(RATE, timer, 0); // RATE is the dt between timesteps
}

// Keyboard callback function for view rotation
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'q': exit(0);
	}
}

// Main function
int main(int argc, char** argv) {
	srand(time(NULL));
	start_time = time(NULL);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(700, 700);
	glutCreateWindow("Active Zombie Simulation");
	initialize();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(RATE, timer, 0);
	glutMainLoop();
	return 0;
}
