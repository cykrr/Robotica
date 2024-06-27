#include <SoftwareSerial.h>


// Motors
#define LA 5
#define LB 6
#define RA 10
#define RB 11

#define set_left_speed(x) \
  if (x > 0) { \
    analogWrite(LA, 0); \
    analogWrite(LB, x); \
  } else { \
    analogWrite(LA, abs(x)); \
    analogWrite(LB, 0); \
  }
#define set_right_speed(x) \
  if (x > 0) { \
    analogWrite(RA, 0); \
    analogWrite(RB, x); \
  } else { \
    analogWrite(RA, abs(x)); \
    analogWrite(RB, 0); \
  }

// IRs
#define IR1 2
#define IR2 3
#define IR3 4
#define IR4 7
#define IR5 12

// Bluetooth
#define BRX 8
#define BTX 9

#define baudRate 38400  // BT BaudRate

#define ROWS 10
#define COLS 10

typedef struct {
  int x;
  int y;
} Coord;

typedef struct Node {
  Coord coord;
  struct Node* parent;
  int g;  // Coste desde el nodo de inicio
  int h;  // Heurística (distancia de Manhattan al nodo objetivo)
  int f;  // f = g + h
} Node;


typedef struct Timer {
  unsigned long start = 0;
  bool recorded = 0;
} Timer;

void start_timer(Timer* t) {
  if (!t->recorded) {
    t->recorded = 1;
    t->start = millis();
  }
}


void stop_timer(Timer* t) {
  t->recorded = 0;
  t->start = 0;
}

unsigned long get_time(Timer* t) {
  if (t->recorded)
    return millis() - t->start;
  else
    return 0;
}


#define RUNNING(x) x.recorded


//static int matriz[ROWS][COLS] = {
//  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
//  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
//  { 0, 0, 0, 1, 1, 1, 1, 1, 0, 0 },
//  { 0, 0, 0, 1, 0, 0, 1, 0, 0, 0 },
//  { 0, 0, 0, 1, 0, 0, 1, 0, 0, 0 },
//  { 0, 0, 1, 1, 0, 0, 1, 0, 0, 0 },
//  { 0, 0, 0, 1, 0, 1, 1, 0, 0, 0 },
//  { 0, 0, 0, 1, 0, 0, 1, 0, 0, 0 },
//  { 0, 0, 0, 1, 0, 0, 1, 1, 0, 0 },
//  { 0, 0, 0, 1, 0, 0, 1, 0, 0, 0 }
//};
//
static int matriz[ROWS][COLS] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 1, 1, 1, 1, 0, 0, 0 }
};


static Coord directions[] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };

// Función para crear un nuevo nodo
Node* create_node(Coord coord, Node* parent, int g, int h) {
  Node* new_node = (Node*)malloc(sizeof(Node));
  new_node->coord = coord;
  new_node->parent = parent;
  new_node->g = g;
  new_node->h = h;
  new_node->f = g + h;
  return new_node;
}

// Función para calcular la distancia de Manhattan
int manhattan_distance(Coord a, Coord b) {
  return abs(a.x - b.x) + abs(a.y - b.y);
}

// Función para verificar si una coordenada es válida
int is_valid_coord(Coord coord) {
  return coord.x >= 0 && coord.x < ROWS && coord.y >= 0 && coord.y < COLS && matriz[coord.x][coord.y] == 1;
}

// Función para verificar si dos coordenadas son iguales
int coords_equal(Coord a, Coord b) {
  return a.x == b.x && a.y == b.y;
}

// Función para buscar un nodo en una lista
Node* find_node(Node* nodes[], int count, Coord coord) {
  for (int i = 0; i < count; i++) {
    if (coords_equal(nodes[i]->coord, coord)) {
      return nodes[i];
    }
  }
  return NULL;
}

// Función para eliminar un nodo de una lista
void remove_node(Node* nodes[], int* count, Node* node) {
  int index = -1;
  for (int i = 0; i < *count; i++) {
    if (nodes[i] == node) {
      index = i;
      break;
    }
  }
  if (index != -1) {
    for (int i = index; i < *count - 1; i++) {
      nodes[i] = nodes[i + 1];
    }
    (*count)--;
  }
}

// Función para reconstruir el camino desde el nodo final hasta el nodo inicial
Node* reconstruct_path(Node* node, Node* path[], int* path_length) {
  if (node->parent != NULL) {
    reconstruct_path(node->parent, path, path_length);
  }
  path[(*path_length)++] = node;
}

// Implementación del algoritmo A*
void a_star(Coord start, Coord end, Node* path[], int* path_length) {
  Node* open_list[ROWS * COLS];
  int open_count = 0;
  Node* closed_list[ROWS * COLS];
  int closed_count = 0;

  Node* start_node = create_node(start, NULL, 0, manhattan_distance(start, end));
  open_list[open_count++] = start_node;

  while (open_count > 0) {
    // Buscar el nodo con el valor f más bajo en la lista abierta
    Node* current = open_list[0];
    for (int i = 1; i < open_count; i++) {
      if (open_list[i]->f < current->f) {
        current = open_list[i];
      }
    }

    // Si hemos alcanzado el nodo objetivo, reconstruir el camino
    if (coords_equal(current->coord, end)) {
      reconstruct_path(current, path, path_length);
      return;
    }

    // Mover el nodo actual de la lista abierta a la lista cerrada
    remove_node(open_list, &open_count, current);
    closed_list[closed_count++] = current;

    // Explorar los vecinos del nodo actual
    for (int i = 0; i < 4; i++) {
      Coord neighbor_coord = { current->coord.x + directions[i].x, current->coord.y + directions[i].y };

      if (!is_valid_coord(neighbor_coord) || find_node(closed_list, closed_count, neighbor_coord)) {
        continue;
      }

      int tentative_g = current->g + 1;
      Node* neighbor = find_node(open_list, open_count, neighbor_coord);

      if (neighbor == NULL) {
        // Si el vecino no está en la lista abierta, añadirlo
        int h = manhattan_distance(neighbor_coord, end);
        neighbor = create_node(neighbor_coord, current, tentative_g, h);
        open_list[open_count++] = neighbor;
      } else if (tentative_g < neighbor->g) {
        // Si encontramos un camino mejor hacia el vecino, actualizar el nodo
        neighbor->parent = current;
        neighbor->g = tentative_g;
        neighbor->f = tentative_g + neighbor->h;
      }
    }
  }
}

int has_intersection(Coord coord, Coord currentDir) {
  for (int i = 0; i < 4; i++) {
    if (abs(currentDir.x) == abs(directions[i].x) && abs(currentDir.y) == abs(directions[i].y)) continue;
    Coord neighbor_coord = { coord.x + directions[i].x, coord.y + directions[i].y };
    if (is_valid_coord(neighbor_coord)) return 1;
  }
  return 0;
}

void get_movements(Node* path[], int path_length, int movements[50], int* j) {
  Coord currentDir = { path[1]->coord.x - path[0]->coord.x, path[1]->coord.y - path[0]->coord.y };

  for (int i = 0; i < path_length - 1; i++) {
    int prev_j = *j;

    if (currentDir.x == -1 && currentDir.y == 0) {
      if (path[i]->coord.y + 1 == path[i + 1]->coord.y) {
        currentDir = { 0, 1 };
        movements[*j] = 0;
        (*j)++;
      } else if (path[i]->coord.y - 1 == path[i + 1]->coord.y) {
        currentDir = { 0, -1 };
        movements[*j] = 1;
        (*j)++;
      }
    } else if (currentDir.x == 0 && currentDir.y == 1) {
      if (path[i]->coord.x + 1 == path[i + 1]->coord.x) {
        currentDir = { 1, 0 };
        movements[*j] = 0;
        (*j)++;
      } else if (path[i]->coord.x - 1 == path[i + 1]->coord.x) {
        currentDir = { -1, 0 };
        movements[*j] = 1;
        (*j)++;
      }
    } else if (currentDir.x == 1 && currentDir.y == 0) {
      if (path[i]->coord.y - 1 == path[i + 1]->coord.y) {
        currentDir = { 0, -1 };
        movements[*j] = 0;
        (*j)++;
      } else if (path[i]->coord.y + 1 == path[i + 1]->coord.y) {
        currentDir = { 0, 1 };
        movements[*j] = 1;
        (*j)++;
      }
    } else if (currentDir.x == 0 && currentDir.y == -1) {
      if (path[i]->coord.x - 1 == path[i + 1]->coord.x) {
        currentDir = { -1, 0 };
        movements[*j] = 0;
        (*j)++;
      } else if (path[i]->coord.x + 1 == path[i + 1]->coord.x) {
        currentDir = { 1, 0 };
        movements[*j] = 1;
        (*j)++;
      }
    }

    if (prev_j == *j && has_intersection(path[i]->coord, currentDir)) {
      movements[*j] = 2;
      (*j)++;
    }
  }
}


SoftwareSerial BTserial(8, 9);  // RX | TX



int i1, i2, i3, i4, i5;  // IRs
#define NO_LINE ((i1 + i2 + i3 + i4 + i5) == 0)
#define LINE_CENTER i3
#define LINE_RIGHT (i3 + i4 +  i5) > 1)
#define LINE_LEFT (i1 +  i2 + i3) > 1)
#define OFF_LEFT ((i1 || i2) && !i3)
#define OFF_RIGHT ((i4 || i5) && !i3)
#define INTERSECTION ((i1 + i2 + i3 + i4 + i5) > 1)

int speed = 0;         // Motor speed
char buffer[50] = "";  // BTSerial in buffer


void readIRs() {
  i5 = !digitalRead(IR1);
  i4 = !digitalRead(IR2);
  i3 = !digitalRead(IR3);
  i2 = !digitalRead(IR4);
  i1 = !digitalRead(IR5);
}

#define FACTOR 0.65
int start_curve(unsigned int t) {
  if (t == 0) return 0;
  float tt = (float)t / 1000;
  float val;

  if (tt < 0.3)
    val = 200 - tt * 2;
  else
    val = (float)10 / tt + 120;

  return floor(val * FACTOR);
}


int movements[50];
int movements_length = 0;
void get_route() {
  Coord start = { 9, 6 };
  Coord end = { 9, 3 };
  Node* path[50];
  int path_length = 0;


  a_star(start, end, path, &path_length);
  get_movements(path, path_length, movements, &movements_length);

  for (int i = 0; i < movements_length; i++) {
    if (movements[i] == 0) {
      //      Serial.print("Derecha\n");
    } else if (movements[i] == 1) {
      //      Serial.print("Izquierda\n");
    } else {
      //      Serial.print("Seguir\n");
    }
  }
}



Timer t, left, right, all, turning, fix_left, fix_right;
void setup() {
  pinMode(8, INPUT);
  pinMode(9, OUTPUT);
  Serial.begin(9600);
  //  Serial.println("xd");
  BTserial.begin(baudRate);
  const int IRs[5] = { IR1, IR2, IR3, IR4, IR5 };
  for (int i = 0; i < 5; i++)
    pinMode(IRs[i], INPUT);
  pinMode(13, OUTPUT);

}


bool llog=0;

void start_routine() {
  if (!movements_length) get_route();
  start_timer(&t);
  llog=1;
}

bool stop = 0;
void stop_sequence() {
  if (stop) return;
  stop = 1;
  delay(300);
  stop_timer(&t);
  stop_timer(&turning);
  set_left_speed(-180);
  set_right_speed(180);

  digitalWrite(13, HIGH);

  //  BTserial.println("IN");
  delay(2100);
  //  BTserial.println("OFF");
  set_left_speed(0);
  set_right_speed(0);

}




//char out[50];
bool start = 0;
bool started = 0;
int left_offset = 0, right_offset = 0;
unsigned long right_duration, left_duration;
int left_speed, right_speed;
int xcount = 0;
void loop() {
  if (stop) return;
  //  Serial.println("loop");

  static char out[30] = {0};
  readIRs();

  // Receives a line break from [BT]Serial
  while (Serial.available()) {
    const char c = Serial.read();
    switch (c) {
      case 's':
        start_routine();

        break;
      case 'S':
        stop_timer(&t);
        stop_timer(&turning);
        stop_timer(&all);
        stop_timer(&left);
        stop_timer(&right);
        left_speed = 0;
        right_speed = 0;
        break;
      case 'p':
        //        Serial.println(movements_length);
        break;
    }
  }

  while (BTserial.available()) {
    const char c = BTserial.read();
    Serial.print(c);

    if (c == '\n') {
      start_routine();
    }
  }

  if (OFF_RIGHT || OFF_LEFT || LINE_CENTER) {
    stop_timer(&all);
    start_timer(&all);
  }

  left_offset = 30 * fix_left.recorded;
  right_offset = 30 * fix_right.recorded;


  if (t.recorded) {
    if (OFF_LEFT) start_timer(&fix_left);
    if (OFF_RIGHT) start_timer(&fix_right);
    int speed = start_curve(get_time(&t));
    left_speed = speed - left_offset;
    right_speed = speed - right_offset;
  }

  if (get_time(&fix_left) > 300)
    stop_timer(&fix_left);

  if (get_time(&fix_right) > 300)
    stop_timer(&fix_right);

  if (get_time(&all) > 300)
    stop_timer(&all);


  if (INTERSECTION && t.recorded && xcount < movements_length) {
    start_timer(&turning);
    int movement = movements[xcount];

    delay(350);
    stop_timer(&fix_right);
    stop_timer(&fix_left);
    sprintf(out, "%d,%d,%d,%d,%d,%d,%d",
            i1, i2, i3 , i4, i5, left_speed, right_speed);
    BTserial.println(out);
    set_left_speed(0);
    set_right_speed(0);
    stop_timer(&t);
    xcount++;

    // BTserial.println("Intersección");
    delay(1000);
    switch (movement) {
      case 0:
        // BTserial.println("Derecha");
        start_timer(&right);
        start_timer(&turning);
        break;
      case 1:
        // BTserial.println("Izquierda");
        start_timer(&left);
        start_timer(&turning);

        break;
      case 2:
        start_timer(&t);
        // BTserial.println("Seguir");
        break;
    }
  }




  if (get_time(&turning) > 1)
    stop_timer(&turning);

  if (left.recorded) {
    speed = start_curve(get_time(&left));
    left_speed = -speed;
    right_speed = speed;

  } else if (right.recorded) {
    speed = start_curve(get_time(&right));
    left_speed = speed;
    right_speed = -speed;
  }



  bool c1 = left.recorded && i1;
  bool c2 = right.recorded && i5;

  if (c1 || c2) {
    stop_timer(&right);
    stop_timer(&left);
    set_left_speed(0);
    set_right_speed(0);
    delay(500);
    start_timer(&t);
  }
  sprintf(out, "%d,%d,%d,%d,%d,%d,%d",
          i1, i2, i3 , i4, i5, left_speed, right_speed);
  if (llog) BTserial.println(out);
  set_left_speed(left_speed * 0.9);
  set_right_speed(right_speed);
  //  Serial.println(String(left_speed) + " " + String(right_speed) + " " + String(get_time(&t)));
  if (RUNNING(t) && !RUNNING(all) && NO_LINE && xcount >= movements_length)
    stop_sequence();



}
