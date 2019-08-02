#ifndef _MAZE_H
#define _MAZE_H

enum Directions : int {
	None = 0,
	North = 1,
	South = 2,
	West = 4,
	East = 8
};
inline Directions operator~(Directions a) {
	return static_cast<Directions>(~static_cast<int>(a));
}
inline Directions operator|(Directions a, Directions b) {
	return static_cast<Directions>(static_cast<int>(a) | static_cast<int>(b));
}
inline Directions operator&(Directions a, Directions b) {
	return static_cast<Directions>(static_cast<int>(a) & static_cast<int>(b));
}
inline Directions operator|=(Directions& a, Directions b) {
    return a = a |b;
}
inline Directions operator&=(Directions& a, Directions b) {
    return a = a & b;
}

struct Cell {
	bool IsOpen = false;
	bool IsLeftWallOpen = false;
	bool IsTopWallOpen = false;
	Directions Openings = Directions::None;
};

struct Maze {
	int cols;
	int rows;
	Cell* buffer = nullptr;
	size_t index(int x, int y) const { return x + cols * y; }
	Cell &cell(Point &p) { return buffer[index(p.x, p.y)]; };
	Cell &cell(int x, int y) { return buffer[index(x, y)]; };
};

void maze_generate(Maze &maze, int cols, int rows);
void maze_grow_tree(Maze* maze);
int maze_num_connections(const Maze *maze, const int x, const int y);
bool maze_connection_is_open(const Maze *maze, const Point start, const Point target);
bool maze_connection_is_open(const Maze *maze, const int x, const int y, const int x2, const int y2);
void maze_open_connection(Maze *maze, Point start, Point target);
void maze_open_connection(Maze *maze, int x, int y, int x2, int y2);
void maze_close_connection(Maze *maze, Point start, Point target);
void maze_close_all(Maze *maze);
void maze_open_all(Maze *maze);
void maze_close_connection(Maze *maze, int x, int y, int x2, int y2);
void maze_close_all_connection(Maze *maze, const int x, const int y);
void maze_rotate_room(Maze *maze, int x, int y, bool rotate_clockwise = false);
void maze_close_and_scramble_room(Maze *maze, const int x, const int y, const int x_from, const int y_from);
void maze_free(Maze* maze);
void maze_serialize(std::ostream &stream, Maze &maze);
void maze_deserialize(std::istream &stream, Maze &maze);
void maze_log(Maze* maze, std::ostringstream &ss);

//void connectRooms(Maze *maze, int x, int y, int x2, int y2);
//int getNumExits(Maze* maze);
//void printMaze(Maze* maze);

#endif