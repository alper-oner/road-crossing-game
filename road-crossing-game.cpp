// road-crossing-game.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <cmath>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <math.h>
#include <string> 

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

#pragma comment(lib, "glew32.lib") 

#define PI 3.14159265

using namespace std;
using std::cout;

GLsizei height = 600, width = 500; /* initial window size */
GLint agentSpeed = 25;
GLint gamePause = 0; // 0 continue  - 1 pause
GLint timer = 0;
GLint score = 0;
GLint maxScore = 0;
GLint gameOver = 0; // 0 no  - 1 yes
GLboolean waitInputFromKeyboard = false;
GLboolean showLines = false;

typedef struct {
	GLint x;
	GLint y;
} Point2D;

typedef struct {
	GLubyte r;
	GLubyte g;
	GLubyte b;
} Color;

typedef struct {
	GLint speed;
	GLint direction;
	GLint v_width;
	GLint v_length;
} VehicleInfo;

typedef struct {
	Point2D center;
	Color color;
	VehicleInfo info;
} Vehicle;

typedef struct {
	Point2D center;
	Color color;
	GLint direction; // 0 up  1 down
	GLint edge;
} Agent;

typedef struct {
	Point2D center;
	Color color;
	GLdouble r;
	GLdouble life;
} Coin;

vector<Vehicle> carVector;
vector<Vehicle> truckVector;
vector<Coin> coinVector;
Agent agent;

// create a agent
void makeAgent(int x, int y) {
	y = height - y;
	agent.center.x = x;
	agent.center.y = y;
	agent.color.r = 255;
	agent.color.g = 0;
	agent.color.b = 0;
	agent.direction = 0; // 0 is up   1 is down
	agent.edge = 20;
	glutPostRedisplay();
}

// Returns the y-axis value such that it is in lines
int createYaxisWithoutSidewalks(int direction) {
	int y = 0;
	int choose = 0;

	if (direction == 0) { // left
		choose = rand() % 8;
		switch (choose) {
		case 0:
			y = 50 + 12.5; // lane 2
			break;
		case 1:
			y = 100 + 12.5; // lane 4
			break;
		case 2:
			y = 175 + 12.5;  // lane 6
			break;
		case 3:
			y = 250 + 12.5;  // lane 8
			break;
		case 4:
			y = 275 + 12.5;  // lane 9
			break;
		case 5:
			y = 400 + 12.5;  // lane 13
			break;
		case 6:
			y = 450 + 12.5; // lane 15
			break;
		case 7:
			y = 525 + 12.5; // lane 17
			break;
		}
	}
	else { // right
		choose = rand() % 10;
		switch (choose) {
		case 0:
			y = 25 + 12.5; // lane 1
			break;
		case 1:
			y = 75 + 12.5;  // lane 3
			break;
		case 2:
			y = 150 + 12.5;  // lane 5
			break;
		case 3:
			y = 200 + 12.5;  // lane 7
			break;
		case 4:
			y = 300 + 12.5;  // lane 10
			break;
		case 5:
			y = 325 + 12.5;  // lane 11
			break;
		case 6:
			y = 375 + 12.5;  // lane 12
			break;
		case 7:
			y = 425 + 12.5; // lane 14
			break;
		case 8:
			y = 500 + 12.5; // lane 16
			break;
		case 9:
			y = 550 + 12.5; // lane 18
			break;
		}
	}
	return y;
}

// create a new coin
void makeCoin() {
	Coin newCoin;
	int x = rand() % ((width - 10) - 25 + 1) + 25.0;
	x = x - x % 25;
	int y = createYaxisWithoutSidewalks((GLubyte)rand() % 2);
	newCoin.center.x = x;
	newCoin.center.y = y;
	newCoin.color.r = 235;
	newCoin.color.g = 186;
	newCoin.color.b = 50;
	newCoin.r = 10;
	newCoin.life = rand() % (15 - 6 + 1) + 6.0; // second
	coinVector.push_back(newCoin);
	glutPostRedisplay();
}

// checks if there is a collision between vehicle and vehicle
bool checkCollisionVehicle(Vehicle ca1, Vehicle ca2) {
	bool collisionX = false;
	bool collisionY = false;
	// Collision x-axis
	if (ca1.center.x <= ca1.center.x)
		collisionX = ca1.center.x + ca1.info.v_width / 2 >= ca2.center.x - ca2.info.v_width / 2;
	else
		collisionX = ca1.center.x - ca1.info.v_width / 2 <= ca2.center.x + ca2.info.v_width / 2;
	// Collision y-axis
	if (ca1.center.y <= ca2.center.y)
		collisionY = ca1.center.y + ca1.info.v_length / 2 >= ca2.center.y - ca2.info.v_length / 2;
	else
		collisionY = ca1.center.y - ca1.info.v_length / 2 <= ca2.center.y + ca2.info.v_length / 2;
	// Collision only if on both axes
	return collisionX && collisionY;
}

// create a new car
void makeCar() {
	Vehicle newCar;
	newCar.color.r = 155;
	newCar.color.g = 110;
	newCar.color.b = 120;
	newCar.info.speed = 1;
	newCar.info.v_width = 20;
	newCar.info.v_length = 20;
	newCar.info.direction = (GLubyte)rand() % 2; // 0 is left   1 is right
	int x = 0;
	if (newCar.info.direction == 0)
		x = width + newCar.info.v_width / 2;
	else
		x = -newCar.info.v_width / 2;
	int y = createYaxisWithoutSidewalks(newCar.info.direction);
	newCar.center.x = x;
	newCar.center.y = y;

	GLboolean collisonBetweenVehicle = false; // checks that there are no collision with another vehicle while the new car is being created
	for (int i = 0; i < carVector.size(); i++) {
		if (checkCollisionVehicle(newCar, carVector[i])) {
			collisonBetweenVehicle = true;
		}
	}
	for (int i = 0; i < truckVector.size(); i++) {
		if (checkCollisionVehicle(newCar, truckVector[i])) {
			collisonBetweenVehicle = true;
		}
	}
	if (!collisonBetweenVehicle)
		carVector.push_back(newCar);
	glutPostRedisplay();
}

// create a new truck
void makeTruck() {
	Vehicle newTruck;
	newTruck.color.r = 65;
	newTruck.color.g = 60;
	newTruck.color.b = 88;
	newTruck.info.speed = 1;
	newTruck.info.v_width = 40;
	newTruck.info.v_length = 20;
	newTruck.info.direction = (GLubyte)rand() % 2; // 0 is left   1 is right
	int x = 0;
	if (newTruck.info.direction == 0)
		x = width + newTruck.info.v_width / 2;
	else
		x = 0 - newTruck.info.v_width / 2;
	int y = createYaxisWithoutSidewalks(newTruck.info.direction);
	newTruck.center.x = x;
	newTruck.center.y = y;

	GLboolean collisonBetweenVehicle = false; // checks that there are no collision with another vehicle while the new truck is being created
	for (int i = 0; i < carVector.size(); i++) {
		if (checkCollisionVehicle(carVector[i], newTruck)) {
			collisonBetweenVehicle = true;
		}
	}
	for (int i = 0; i < truckVector.size(); i++) {
		if (checkCollisionVehicle(newTruck, truckVector[i])) {
			collisonBetweenVehicle = true;
		}
	}
	if (!collisonBetweenVehicle)
		truckVector.push_back(newTruck);
	glutPostRedisplay();
}

void myinit(void)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLdouble)width, 0.0, (GLdouble)height);

	/* set clear color to white and clear window */
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

// checks if the given point is inside the rectangle
bool pointInRectangle(int pointX, int pointY, Point2D rectCenter, int rectWidth, int rectlength) {
	return ((pointX <= rectCenter.x + rectWidth / 2) && (pointX >= rectCenter.x - rectWidth / 2) &&
		(pointY <= rectCenter.y + rectlength / 2) && (pointY >= rectCenter.y - rectlength / 2));
}

// checks if the given point is inside the triangle
bool pointInTriangle(int pointX, int pointY, int aX, int aY, int bX, int bY, int cX, int cY) {
	bool s_ab = (bX - aX) * (pointY - aY) - (bY - aY) * (pointX - aX) > 0;
	if ((cX - aX) * (pointY - aY) - (cY - aY) * (pointX - aX) > 0 == s_ab)
		return false;
	if ((cX - bX) * (pointY - bY) - (cY - bY) * (pointX - bX) > 0 != s_ab)
		return false;
	return true;
}

// checks if there is a collision between agent and vehicle
bool checkCollisionAgentAndVehicle(Vehicle ca) {
	int agentX = agent.center.x;
	int agentY = agent.center.y;
	int carX = ca.center.x;
	int carY = ca.center.y;

	return (pointInTriangle(carX + ca.info.v_width / 2, carY + ca.info.v_length / 2, agentX, agentY + agent.edge / 2, agentX - (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2, agentX + (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2) ||
		pointInTriangle(carX - ca.info.v_width / 2, carY + ca.info.v_length / 2, agentX, agentY + agent.edge / 2, agentX - (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2, agentX + (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2) ||
		pointInTriangle(carX - ca.info.v_width / 2, carY - ca.info.v_length / 2, agentX, agentY + agent.edge / 2, agentX - (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2, agentX + (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2) ||
		pointInTriangle(carX + ca.info.v_width / 2, carY - ca.info.v_length / 2, agentX, agentY + agent.edge / 2, agentX - (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2, agentX + (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2) ||
		pointInRectangle(agentX, agentY + agent.edge / 2, ca.center, ca.info.v_width, ca.info.v_length) ||
		pointInRectangle(agentX - (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2, ca.center, ca.info.v_width, ca.info.v_length) ||
		pointInRectangle(agentX + (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2, ca.center, ca.info.v_width, ca.info.v_length)
		);
}

// checks if there is a collision between agent and coin
bool checkCollisionAgentAndCoin(Coin co) {
	int agentX = agent.center.x;
	int agentY = agent.center.y;
	int coX = co.center.x;
	int coY = co.center.y;

	return (pointInTriangle(coX + co.r / 2, coY + co.r / 2, agentX, agentY + agent.edge / 2, agentX - (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2, agentX + (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2) ||
		pointInTriangle(coX - co.r / 2, coY + co.r / 2, agentX, agentY + agent.edge / 2, agentX - (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2, agentX + (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2) ||
		pointInTriangle(coX - co.r / 2, coY - co.r / 2, agentX, agentY + agent.edge / 2, agentX - (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2, agentX + (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2) ||
		pointInTriangle(coX + co.r / 2, coY - co.r / 2, agentX, agentY + agent.edge / 2, agentX - (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2, agentX + (agent.edge / 2) * sqrt(2), agentY - agent.edge / 2) ||
		pointInRectangle(agentX, agentY + agent.edge / 2, co.center, co.r, co.r) ||
		pointInRectangle(agentX - (agent.edge / 2) * sqrt(2), agentY - agent.edge, co.center, co.r, co.r) ||
		pointInRectangle(agentX + (agent.edge / 2) * sqrt(2), agentY - agent.edge, co.center, co.r, co.r));
}

void myKeyboard(unsigned char key, int x, int y) {
	if ((key == 'Q') || (key == 'q'))
		exit(0);

	else if ((key == 'R') || (key == 'r')) { // the game restarts when the r button is pressed
		cout << "GAME RESTARTED" << endl;

		if (score > maxScore) {
			maxScore = score;
		}

		gameOver = 0;
		gamePause = 0;
		score = 0;
		timer = 0;
		carVector.clear();
		truckVector.clear();
		coinVector.clear();
		makeAgent(width / 2, height - 12.5);
	}
	else if ((key == 'L') || (key == 'l')) { // Line numbers and y-axis coor. are displayed/undisplayed when l button is pressed
		showLines = !showLines;
	}
}

void SpecialInput(int key, int x, int y) {
	if ((!gamePause && !gameOver) || ((!gameOver) && gamePause && waitInputFromKeyboard)) {
		waitInputFromKeyboard = false;
		switch (key)
		{
		case GLUT_KEY_UP:
			if (agent.direction == 0) { // 0 is up
				if (agent.center.y < height - 37.5) {
					agent.center.y = agent.center.y + agentSpeed;
				}
				else {
					agent.center.y = agent.center.y + agentSpeed;
					agent.direction = 1;
					agent.edge = agent.edge *= -1;
				}
				score += 1;
			}
			else {
				gameOver = 1;
				if (score > maxScore) {
					maxScore = score;
				}
				cout << "GAME OVER" << endl;
			}
			break;
		case GLUT_KEY_DOWN:
			if (agent.direction == 1) { // 1 is down
				if (agent.center.y > 50) {
					agent.center.y = agent.center.y - agentSpeed;
				}
				else {
					agent.center.y = agent.center.y - agentSpeed;
					agent.direction = 0;
					agent.edge = agent.edge *= -1;
				}
				score += 1;
			}
			else {
				gameOver = 1;
				if (score > maxScore) {
					maxScore = score;
				}
				cout << "GAME OVER" << endl;
			}
			break;
		case GLUT_KEY_LEFT:
			if (agent.center.x > agentSpeed) {
				agent.center.x = agent.center.x - agentSpeed;
			}
			break;
		case GLUT_KEY_RIGHT:
			if (agent.center.x < width - agentSpeed) {
				agent.center.x = agent.center.x + agentSpeed;
			}
			break;
		}

		// coin collision detection
		for (int i = 0; i < coinVector.size(); i++) {
			if (checkCollisionAgentAndCoin(coinVector[i])) {
				coinVector.erase(coinVector.begin() + i);
				score += 5;
			}
		}

	}

	glutPostRedisplay();
}

void RenderString(GLdouble x, GLdouble y, const std::string& string) {
	glColor3ub(255, 102, 0);

	glRasterPos2d(x, y);
	for (int n = 0; n < string.size(); ++n) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, string[n]);
	}
}

/* display callback required by GLUT 3.0 */
void myDisplay(void) {
	int i, x, y;
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.756, 0.811, 0.858, 1);


	// Cars view
	for (i = 0; i < carVector.size(); i++) {
		x = carVector[i].center.x;
		y = carVector[i].center.y;
		glColor3ub(carVector[i].color.r, carVector[i].color.g, carVector[i].color.b);
		glBegin(GL_POLYGON);
		glVertex2f(x + carVector[i].info.v_width / 2, y + carVector[i].info.v_length / 2);
		glVertex2f(x - carVector[i].info.v_width / 2, y + carVector[i].info.v_length / 2);
		glVertex2f(x - carVector[i].info.v_width / 2, y - carVector[i].info.v_length / 2);
		glVertex2f(x + carVector[i].info.v_width / 2, y - carVector[i].info.v_length / 2);
		glEnd();
	}

	// Trucks view
	for (i = 0; i < truckVector.size(); i++) {
		x = truckVector[i].center.x;
		y = truckVector[i].center.y;
		glColor3ub(truckVector[i].color.r, truckVector[i].color.g, truckVector[i].color.b);
		glBegin(GL_POLYGON);
		glVertex2f(x + truckVector[i].info.v_width / 2, y + truckVector[i].info.v_length / 2);
		glVertex2f(x - truckVector[i].info.v_width / 2, y + truckVector[i].info.v_length / 2);
		glVertex2f(x - truckVector[i].info.v_width / 2, y - truckVector[i].info.v_length / 2);
		glVertex2f(x + truckVector[i].info.v_width / 2, y - truckVector[i].info.v_length / 2);
		glEnd();
	}

	glColor3f(0.0, 0.0, 0.0);  // black

	// lines
	for (int i = 0; i <= height; i += 25) {
		for (int j = 0; j <= width; j += 10) {
			glBegin(GL_LINES);
			glVertex2f(j, i);
			glVertex2f(j + 5, i);
			glEnd();
		}
	}

	// side walk
	glBegin(GL_POLYGON);
	glVertex2f(0, 0);
	glVertex2f(0, 25);
	glVertex2f(width, 25);
	glVertex2f(width, 0);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0, 125);
	glVertex2f(0, 150);
	glVertex2f(width, 150);
	glVertex2f(width, 125);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0, 225);
	glVertex2f(0, 250);
	glVertex2f(width, 250);
	glVertex2f(width, 225);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0, 350);
	glVertex2f(0, 375);
	glVertex2f(width, 375);
	glVertex2f(width, 350);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0, 475);
	glVertex2f(0, 500);
	glVertex2f(width, 500);
	glVertex2f(width, 475);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0, 575);
	glVertex2f(0, 600);
	glVertex2f(width, 600);
	glVertex2f(width, 575);
	glEnd();

	// coin view
	for (i = 0; i < coinVector.size(); i++) {
		x = coinVector[i].center.x;
		y = coinVector[i].center.y;
		glColor3ub(coinVector[i].color.r, coinVector[i].color.g, coinVector[i].color.b);
		glBegin(GL_TRIANGLE_FAN); //BEGIN CIRCLE
		glVertex2f(x, y); // center of circle
		for (int j = 0; j <= 30; j++) {
			glVertex2f(
				(x + (coinVector[i].r * cos(j * 2.0 * PI / 30))), (y + (coinVector[i].r * sin(j * 2.0 * PI / 30)))
			);
		}
		glEnd(); //END
	}

	// text area
	RenderString(20, 580, "Score:");
	RenderString(100, 580, to_string(score));
	RenderString(250, 580, to_string(timer));
	RenderString(325, 580, "MaxScore:");
	RenderString(440, 580, to_string(maxScore));

	if (gameOver) {
		RenderString(100, 130, "GAME OVER, PRESS R TO TRY AGAIN");
		RenderString(100, 230, "GAME OVER, PRESS R TO TRY AGAIN");
		RenderString(100, 355, "GAME OVER, PRESS R TO TRY AGAIN");
		RenderString(100, 480, "GAME OVER, PRESS R TO TRY AGAIN");
	}

	if (gamePause) {
		RenderString(10, 5, "PRESS L TO SEE LINES INFO");
	}

	if (showLines) {
		RenderString(10, 25 + 5, "lane 1");
		RenderString(10, 50 + 5, "lane 2");
		RenderString(10, 75 + 5, "lane 3");
		RenderString(10, 100 + 5, "lane 4");
		RenderString(10, 150 + 5, "lane 5");
		RenderString(10, 175 + 5, "lane 6");
		RenderString(10, 200 + 5, "lane 7");
		RenderString(10, 250 + 5, "lane 8");
		RenderString(10, 275 + 5, "lane 9");
		RenderString(10, 300 + 5, "lane 10");
		RenderString(10, 325 + 5, "lane 11");
		RenderString(10, 375 + 5, "lane 12");
		RenderString(10, 400 + 5, "lane 13");
		RenderString(10, 425 + 5, "lane 14");
		RenderString(10, 450 + 5, "lane 15");
		RenderString(10, 500 + 5, "lane 16");
		RenderString(10, 525 + 5, "lane 17");
		RenderString(10, 550 + 5, "lane 18");
		for (int i = 0; i < height - 25; i += 25) { // show y-axis coordinates
			RenderString(450, i - 5, to_string(i));

		}
	}

	//agent view
	x = agent.center.x;
	y = agent.center.y;
	glColor3ub(agent.color.r, agent.color.g, agent.color.b);
	glBegin(GL_POLYGON);
	glVertex2f(x, y + agent.edge / 2);
	glVertex2f(x - (agent.edge / 2) * sqrt(2), y - agent.edge / 2);
	glVertex2f(x + (agent.edge / 2) * sqrt(2), y - agent.edge / 2);
	glEnd();

	glFlush();
}

void makeCarTimer(int value) {
	if (!gamePause && !gameOver) {
		makeCar();
		glutPostRedisplay();
	}
	glutTimerFunc(rand() % (700 - 100 + 1) + 100, makeCarTimer, 0);
}

void makeTruckTimer(int value) {
	if (!gamePause && !gameOver) {
		makeTruck();
		glutPostRedisplay();
	}
	glutTimerFunc(rand() % (700 - 100 + 1) + 100, makeTruckTimer, 0);
}

void makeCoinTimer(int value) {
	if (!gamePause && !gameOver) {
		makeCoin();
		glutPostRedisplay();
	}
	glutTimerFunc(rand() % (3000 - 1000 + 1) + 1000, makeCoinTimer, 0);
}

void deleteCoinTimer(int value) {
	if (!gamePause && !gameOver) {
		for (int i = 0; i < coinVector.size(); i++) {
			coinVector[i].life -= 1;
			if (coinVector[i].life == 0) {
				coinVector.erase(coinVector.begin() + i);
			}
		}
		glutPostRedisplay();
	}
	glutTimerFunc(1000, deleteCoinTimer, 0);
}

void myStep() {
	// move vehicle
	for (int i = 0; i < carVector.size(); i++) {
		if (carVector[i].info.direction == 0) { // move to left
			carVector[i].center.x = carVector[i].center.x - carVector[i].info.speed;
		}
		else { // move to right
			carVector[i].center.x = carVector[i].center.x + carVector[i].info.speed;
		}
	}

	for (int i = 0; i < truckVector.size(); i++) {
		if (truckVector[i].info.direction == 0) { // move to left
			truckVector[i].center.x = truckVector[i].center.x - truckVector[i].info.speed;
		}
		else { // move to right
			truckVector[i].center.x = truckVector[i].center.x + truckVector[i].info.speed;
		}
	}

	// delete vehicle that no longer are visible
	for (int i = 0; i < carVector.size(); i++) {
		if (carVector[i].center.x - carVector[i].info.v_width / 2 > width || carVector[i].center.x + carVector[i].info.v_width / 2 < 0) {
			carVector.erase(carVector.begin() + i);
		}
	}

	for (int i = 0; i < truckVector.size(); i++) {
		if (truckVector[i].center.x - truckVector[i].info.v_width / 2 > width || truckVector[i].center.x + truckVector[i].info.v_width / 2 < 0) {
			truckVector.erase(truckVector.begin() + i);
		}
	}

	// collision detection
	for (int i = 0; i < carVector.size(); i++) {
		if (checkCollisionAgentAndVehicle(carVector[i])) {
			gameOver = 1;
			if (score > maxScore) {
				maxScore = score;
			}
			cout << "GAME OVER Collision" << endl;
		}
	}
	for (int i = 0; i < truckVector.size(); i++) {
		if (checkCollisionAgentAndVehicle(truckVector[i])) {
			gameOver = 1;
			if (score > maxScore) {
				maxScore = score;
			}
			cout << "GAME OVER Collision" << endl;
		}
	}
	/* update animation */
	glutPostRedisplay();
}

void moveAndDeleteVehicleTimer(int value) {
	if (!gamePause && !gameOver) {
		myStep();
	}
	/* update animation */
	glutPostRedisplay();
	glutTimerFunc(1000.0 / 60.0, moveAndDeleteVehicleTimer, 0);
}

void timeTimer(int value) {
	if (!gamePause && !gameOver) {
		timer += 1;
	}
	glutTimerFunc(1000, timeTimer, 0);
}

// print standart output for debugging purpose
void consoleOutput() {
	cout << "agent location - x: " << agent.center.x << " y: " << agent.center.y << endl;

	for (int i = 0; i < carVector.size(); i++) {
		cout << "car location - x: " << carVector[i].center.x << " y: " << carVector[i].center.y << endl;
	}

	for (int i = 0; i < truckVector.size(); i++) {
		cout << "truck location - x: " << truckVector[i].center.x << " y: " << truckVector[i].center.y << endl;
	}
	for (int i = 0; i < coinVector.size(); i++) {
		cout << "coin location - x: " << coinVector[i].center.x << " y: " << coinVector[i].center.y << endl;
	}
	cout << " ---------------- " << endl;
}

void myMouse(int btn, int state, int x, int y) {
	if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		if (!gameOver) {
			if (gamePause == 0) {
				gamePause = 1;
				cout << "Game Paused" << endl;
				consoleOutput();
				waitInputFromKeyboard = true;
			}
			else {
				waitInputFromKeyboard = true;
				if (gamePause && !gameOver) {
					myStep();
					consoleOutput();
				}
			}
		}
	}
	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (!gameOver) {
			if (gamePause == 0) {
				gamePause = 1;
				cout << "Game Paused" << endl;
			}
			else {
				gamePause = 0;
				cout << "Game Continue" << endl;
				waitInputFromKeyboard = false;
			}
		}
	}
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("road-crossing-game");
	myinit();
	makeAgent(width / 2, height - 12.5);
	glutMouseFunc(myMouse);
	glutDisplayFunc(myDisplay);
	glutKeyboardFunc(myKeyboard);

	glutTimerFunc(0, makeCarTimer, 0);
	glutTimerFunc(0, makeTruckTimer, 0);
	glutTimerFunc(1000.0 / 60.0, moveAndDeleteVehicleTimer, 0);
	glutTimerFunc(1000, makeCoinTimer, 0);
	glutTimerFunc(2000, deleteCoinTimer, 0);
	glutTimerFunc(1000, timeTimer, 0);

	glutSpecialFunc(SpecialInput);
	srand(time(NULL));

	glutMainLoop();
}
