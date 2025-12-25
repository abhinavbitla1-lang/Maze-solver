#include <Arduino.h>

struct Map {
    int forwardDis;
    char dir;
    struct Map *next;
};

const int motor1Pin1 = 28;
const int motor1Pin2 = 29;
const int motor2Pin1 = 26;
const int motor2Pin2 = 27;

int ena = 23, enb = 25;

unsigned long motorStartTime = 0;
unsigned long motorEndTime = 0;

unsigned long startTime = 0;
unsigned long endTime = 0;
unsigned long elapsedTime = 0;

unsigned long totalDistance = 0;

float distanceTravelled = 0;

void moveForward() {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, HIGH);
    digitalWrite(motor2Pin2, LOW);
}

void moveBackward() {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);
}

void turnRight() {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor2Pin1, HIGH);
    digitalWrite(motor2Pin2, LOW);
    delay(300);
    stopMotors();
}

void turnLeft() {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);
    delay(300);
    stopMotors();
}

void stopMotors() {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, LOW);
}

Map *head = new Map();

void setup() {
    Serial1.println("Started");
    // Set up serial communication with Bluetooth module
    Serial1.begin(9600);

    head->next = NULL;

    pinMode(motor1Pin1, OUTPUT);
    pinMode(motor1Pin2, OUTPUT);
    pinMode(motor2Pin1, OUTPUT);
    pinMode(motor2Pin2, OUTPUT);
    pinMode(ena, OUTPUT);
    pinMode(enb, OUTPUT);

    digitalWrite(ena, HIGH);
    digitalWrite(enb, HIGH);

    // Send initial instructions to mobile device
    Serial1.println("Press 'f' or 'F' for Forward");
    Serial1.println("Press 'b' or 'B' for Backward");
    Serial1.println("Press 'l' or 'L' for Left");
    Serial1.println("Press 'r' or 'R' for Right");
    Serial1.println("Press 's' or 'S' for Stop");
}

void newDir(int forwardDis, char dir) {
    if (head == NULL) {
        head->forwardDis = forwardDis;
        head->dir = dir;
        head->next = NULL;
    } else {
        Map *temp = head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        Map *newVal = new Map();
        newVal->dir = dir;
        newVal->forwardDis = forwardDis;
        newVal->next = NULL;
        temp->next = newVal;
    }
}

void displayMap() {
    Serial1.println("\nThis prints Map\n");

    const int GRID_SIZE = 60; // Define the size of the grid
    char grid[GRID_SIZE][GRID_SIZE];

    // Initialize the grid with spaces
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = ' ';
        }
    }

    int x = GRID_SIZE / 2;
    int y = GRID_SIZE / 2;
    grid[y][x] = 'S'; // Starting point

    Map *temp = head->next; // Skip the dummy head node

    // Initial direction is 'up'
    int dx = 0, dy = -1;

    while (temp != NULL) {
        int cells = temp->forwardDis / 5;

        for (int i = 0; i < cells; i++) {
            x += dx;
            y += dy;
            if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
                grid[y][x] = (dx != 0) ? '-' : '|';
            }
        }

        switch (temp->dir) {
            case 'L':
            case 'l':
                if (dx == 0 && dy == -1) { dx = -1; dy = 0; }
                else if (dx == -1 && dy == 0) { dx = 0; dy = 1; }
                else if (dx == 0 && dy == 1) { dx = 1; dy = 0; }
                else if (dx == 1 && dy == 0) { dx = 0; dy = -1; }
                break;
            case 'R':
            case 'r':
                if (dx == 0 && dy == -1) { dx = 1; dy = 0; }
                else if (dx == 1 && dy == 0) { dx = 0; dy = 1; }
                else if (dx == 0 && dy == 1) { dx = -1; dy = 0; }
                else if (dx == -1 && dy == 0) { dx = 0; dy = -1; }
                break;
            default:
                Serial1.println("Invalid direction");
                break;
        }

        temp = temp->next;
    }

    grid[y][x] = 'E'; // End point

    // Print the grid
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            Serial1.print(grid[i][j]);
        }
        Serial1.println();
    }
}

void displayData(long motorElapsedTime, long totalDistance) {
    Map *temp = head;
    while (temp != NULL) {
        Serial1.print("Distance: ");
        Serial1.print(temp->forwardDis);
        Serial1.print("CM");
        Serial1.print(" Direction: ");
        Serial1.println(temp->dir);
        totalDistance += temp->forwardDis;
        temp = temp->next;
    }
    Serial.println();

    Serial1.print("Total Time travelled: ");
    Serial1.print(motorElapsedTime);
    Serial1.println(" ms");
    Serial1.print("Total Distance travelled: ");
    Serial1.print(totalDistance);
    Serial1.println(" cm");
    Serial1.print("Average Speed travelled: ");
    Serial1.print(totalDistance/motorElapsedTime);
    Serial1.println("cm per second");
}

void loop() {
    if (Serial1.available() > 0) {
        // Read the incoming character
        char buffer_value = Serial1.read();

        // Send appropriate message based on the received character
        // if(buffer_value == "$"){
          motorStartTime = millis();
          switch (buffer_value) {
            case 'f':
            case 'F':
                Serial1.println("Forward");
                startTime = millis();
                moveForward();
                break;
            case 'l':
            case 'L':
                Serial1.println("Left");
                endTime = millis();
                turnLeft();
                elapsedTime = endTime - startTime; //         // Print the elapsed time to the serial monitor//         // Serial1.print("Elapsed Time: ");//         // Serial1.print(elapsedTime);//         // Serial1.println(" ms");
                distanceTravelled = (elapsedTime / 303) * 11.958;
                // totalDistance += distanceTravelled;
                newDir(distanceTravelled/5, 'l');
                break;
            case 'r':
            case 'R':
                Serial1.println("Right");
                endTime = millis();
                turnRight();
                elapsedTime = endTime - startTime;
                distanceTravelled = (elapsedTime / 303) * 11.958;
                // totalDistance += distanceTravelled;
                newDir(distanceTravelled/5, 'r');
                break;
            case 'b':
            case 'B':
                Serial1.println("Break");
                stopMotors();
                break;
            case 'm':
            case 'M':
                // Serial1.println("Break");
                stopMotors();
                displayMap();
                break;
            case 'd':
            case 'D':
                // Serial1.println("Break");
                stopMotors();
                motorEndTime = millis();
                unsigned long motorElapsedTime = motorEndTime - motorStartTime;
                displayData(motorElapsedTime, totalDistance);
                break;
            case '#':
                Serial1.println("Ended");
                motorEndTime = millis();
              // unsigned long motorElapsedTime = motorEndTime - motorStartTime;
              // Serial1.print("Total Time travelled: ");
              // Serial1.print(motorElapsedTime);
              // Serial1.println(" ms");
              // Serial1.print("Total Distance travelled: ");
              // Serial1.print(totalDistance);
              // Serial1.println(" cm");
              // Serial1.print("Average Speed travelled: ");
              // Serial1.print(totalDistance/motorElapsedTime);
              // Serial1.println("cm per second");
                stopMotors();
                break;
        }
        // if(buffer_value = "M"){
        //   // newDir(20,'l');
        //   // newDir(20,'l');
        //   // newDir(20,'l');
        //   // newDir(20,'l');
        //   displayMap();
        // }
        // if(buffer_value = "D"){
        //   motorEndTime = millis();
        //   unsigned long motorElapsedTime = motorEndTime - motorStartTime;
        //   displayData(motorElapsedTime, totalDistance);
        // }
        // }
        // else if(buffer_value == "#"){
        //   Serial1.println("Ended");
        //   motorEndTime = millis();
        //   unsigned long motorElapsedTime = motorEndTime - motorStartTime;
        //   Serial1.print("Total Time travelled: ");
        //   Serial1.print(motorElapsedTime);
        //   Serial1.println(" ms");
        //   Serial1.print("Total Distance travelled: ");
        //   Serial1.print(totalDistance);
        //   Serial1.println(" cm");
        //   Serial1.print("Average Speed travelled: ");
        //   Serial1.print(totalDistance/motorElapsedTime);
        //   Serial1.println("cm per second");
        //   stopMotors();// }
  }
}
