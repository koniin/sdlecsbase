#include "maze.h"
#include "engine.h"
//
// Code from:
// https://github.com/munificent/amaranth/blob/b52acd6fc69d1c898e415467d319d0698a08d5ff/Amaranth.Engine/Classes/Dungeon/Maze.cs 
//

Point north(0, -1);
Point south(0, 1);
Point west(-1, 0);
Point east(1, 0);

std::vector<Point> pointDirections = { north, south, east, west };

void maze_generate(Maze &maze, int cols, int rows) {
	Cell* cells = new Cell[cols*rows];
	maze.buffer = cells;
	maze.cols = cols;
	maze.rows = rows;
}

bool maze_connection_is_open(const Maze *maze, const Point start, const Point target) {
	return maze_connection_is_open(maze, start.x, start.y, target.x, target.y);
}

bool maze_connection_is_open(const Maze *maze, const int x, const int y, const int x2, const int y2) {
	Point direction(x2 - x, y2 - y);
	
	if(direction == north) {
		return (maze->buffer[maze->index(x, y)].Openings & Directions::North) == Directions::North;
	}
	if(direction == south) {
		return (maze->buffer[maze->index(x, y)].Openings & Directions::South) == Directions::South;
	}
	if(direction == west) {
		return (maze->buffer[maze->index(x, y)].Openings & Directions::West) == Directions::West;
	}
	if(direction == east) {
		return (maze->buffer[maze->index(x, y)].Openings & Directions::East) == Directions::East;
	}

	ASSERT_WITH_MSG(false, "maze_connection_is_open: No direction defined");
	return false;
}

void maze_close_all(Maze *maze) {
	static const struct Cell EmptyCell;

	for (int y = 0; y < maze->rows; y++) {
        for (int x = 0; x < maze->cols; x++){
			maze->buffer[maze->index(x,y)] = EmptyCell;
		}
	}
}

void maze_close_connection(Maze *maze, Point start, Point target) {
	return maze_close_connection(maze, start.x, start.y, target.x, target.y);
}

void maze_close_connection(Maze *maze, int x, int y, int x2, int y2) {
	Point direction(x2 - x, y2 - y);
	
	if(direction == north) {
		maze->buffer[maze->index(x, y)].Openings = maze->buffer[maze->index(x, y)].Openings & ~Directions::North;
		maze->buffer[maze->index(x2, y2)].Openings = maze->buffer[maze->index(x2, y2)].Openings & ~Directions::South;
	}
	if(direction == south) {
		maze->buffer[maze->index(x, y)].Openings = maze->buffer[maze->index(x, y)].Openings & ~Directions::South;
		maze->buffer[maze->index(x2, y2)].Openings = maze->buffer[maze->index(x2, y2)].Openings & ~Directions::North;
	}
	if(direction == west) {
		maze->buffer[maze->index(x, y)].Openings = maze->buffer[maze->index(x, y)].Openings & ~Directions::West;
		maze->buffer[maze->index(x2, y2)].Openings = maze->buffer[maze->index(x2, y2)].Openings & ~Directions::East;
	}
	if(direction == east) {
		maze->buffer[maze->index(x, y)].Openings &= ~Directions::East;
		maze->buffer[maze->index(x2, y2)].Openings &= ~Directions::West;
	}
}

void maze_open_connection(Maze *maze, Point start, Point target) {
	return maze_open_connection(maze, start.x, start.y, target.x, target.y);
}

void maze_open_connection(Maze *maze, int x, int y, int x2, int y2) {
	Point direction(x2 - x, y2 - y);
	if(direction == north) {
		maze->buffer[maze->index(x, y)].Openings = maze->buffer[maze->index(x, y)].Openings | Directions::North;
		maze->buffer[maze->index(x2, y2)].Openings = maze->buffer[maze->index(x2, y2)].Openings | Directions::South;
	}
	if(direction == south) {
		maze->buffer[maze->index(x, y)].Openings = maze->buffer[maze->index(x, y)].Openings | Directions::South;
		maze->buffer[maze->index(x2, y2)].Openings = maze->buffer[maze->index(x2, y2)].Openings | Directions::North;
	}
	if(direction == west) {
		maze->buffer[maze->index(x, y)].Openings = maze->buffer[maze->index(x, y)].Openings | Directions::West;
		maze->buffer[maze->index(x2, y2)].Openings = maze->buffer[maze->index(x2, y2)].Openings | Directions::East;
	}
	if(direction == east) {
		maze->buffer[maze->index(x, y)].Openings = maze->buffer[maze->index(x, y)].Openings | Directions::East;
		maze->buffer[maze->index(x2, y2)].Openings = maze->buffer[maze->index(x2, y2)].Openings | Directions::West;
	}
}

void maze_grow_tree(Maze* maze) {
	Rectangle bounds(0,0, maze->cols, maze->rows);
	std::vector<Point> cellsToVisit;

	Point p;
	RNG::random_point_i(maze->cols, maze->rows, p.x, p.y);
	cellsToVisit.push_back(p);
	maze->buffer[maze->index(p.x, p.y)].IsOpen = true;
	
	while(!cellsToVisit.empty()) {
		// How we select the next cell determines the properties of the maze
		// Selection is important !!
		int index = RNG::range_i(0, cellsToVisit.size());
		
		std::shuffle(std::begin(pointDirections), std::end(pointDirections), RNG::RNG_generator);
		
		for(auto &direction : pointDirections) {
			Point neighbour = cellsToVisit[index] + direction;
			if (bounds.contains(neighbour) && !maze->cell(neighbour).IsOpen) {
				ASSERT_WITH_MSG(neighbour.x >= 0 && neighbour.y >= 0, "maze generation fucked up");
				
				maze->cell(neighbour).IsOpen = true;
				Point &current = cellsToVisit[index];
				
				if (direction == north) {
					maze->cell(current).IsTopWallOpen = true;
					maze->cell(current).Openings |= Directions::North;
					maze->cell(neighbour).Openings |= Directions::South;
				} else if (direction == south) {
			 		maze->cell(neighbour).IsTopWallOpen = true;
			 		maze->cell(current).Openings |= Directions::South;
			 		maze->cell(neighbour).Openings |= Directions::North;
				} else if (direction == west) {
					if(current.x == 0) {
						Engine::log("hmmm, y: %d", current.y);
					}
					maze->cell(current).IsLeftWallOpen = true;
					maze->cell(current).Openings |= Directions::West;
					maze->cell(neighbour).Openings |= Directions::East;
				} else if (direction == east) {
					if(neighbour.x == 0) {
						Engine::log("hoho, y: %d", neighbour.y);
					}
			 		maze->cell(neighbour).IsLeftWallOpen = true;
			 		maze->cell(current).Openings |= Directions::East;
			 		maze->cell(neighbour).Openings |= Directions::West;
				}

				cellsToVisit.push_back(neighbour);
				index = -1;
				break;
			}
		}
		
		if (index >= 0) 
			cellsToVisit.erase(cellsToVisit.begin() + index); // cellsToVisit.RemoveAt(index);
	}
}

int maze_num_connections(const Maze *maze, const int x, const int y) {
	int count = 0;
	for(auto &dir : pointDirections) {
		// maze_connection_is_open(Maze *maze, int x, int y, int x2, int y2)
		if(maze_connection_is_open(maze, x, y, x + dir.x, y + dir.y))
			count++;
	}
	return count;
}

void maze_close_all_connection(Maze *maze, const int x, const int y) {
	Rectangle bounds(0,0, maze->cols, maze->rows);
	Point pos(x, y);
	for(auto &dir : pointDirections) {
		Point nextRoom = pos + dir;
		if(bounds.contains(nextRoom) && maze_connection_is_open(maze, pos, nextRoom)) {
			maze_close_connection(maze, pos, nextRoom);
		}
	}
}

void maze_rotate_room(Maze *maze, int x, int y, bool rotate_clockwise) {
	Point pos = { x, y };
	Rectangle bounds(0,0, maze->cols, maze->rows);
	std::vector<Point> new_connections;
	
	for(auto &dir : pointDirections) {
		Point nextRoom = pos + dir;
		if(bounds.contains(nextRoom) && maze_connection_is_open(maze, pos, nextRoom)) {
			maze_close_connection(maze, pos, nextRoom);
			Point new_direction;
			
			if(dir == north) {
				Engine::log("\nNorth open");
				new_direction = rotate_clockwise ? east : west;
			} else if(dir == south) {
				Engine::log("\nSouth open");
				new_direction = rotate_clockwise ? west : east;
			} else if(dir == east) {
				Engine::log("\nEast open");
				new_direction = rotate_clockwise ? south : north;
			} else if(dir == west) {
				Engine::log("\nWest open");
				new_direction = rotate_clockwise ? north : south;
			}
			
			Point new_connected_room = pos + new_direction;
			if(bounds.contains(new_connected_room)) {
				new_connections.push_back(new_connected_room);
			}
		}
	}
	
	for(auto &p : new_connections) {
		maze_open_connection(maze, pos, p);
	}
}

void maze_close_and_scramble_room(Maze *maze, const int x, const int y, const int x_from, const int y_from) {
	// calculate number of exits of current room
	int exits = maze_num_connections(maze, x, y);
	// Engine::log("\nExits: %d | x: %d, y: %d | x_from: %d, y_from: %d", exits, x, y, x_from, y_from);
	
	Point pos = { x, y };
	Point from = { x_from, y_from };
	Rectangle bounds(0,0, maze->cols, maze->rows);
	
	// Save the initial entrance connection
	Point forbidden_direction(x_from - x, y_from - y);
	
	// Close all connections 
	maze_close_all_connection(maze, x, y);
	
	// Generate 'exits' number of connections that are not forbidden
	std::vector<int> indexes{ 0, 1, 2, 3 };
	std::shuffle(std::begin(indexes), std::end(indexes), RNG::RNG_seed);
	int generated_exits = 0;
	for(auto &i : indexes) {
		if(pointDirections[i] == forbidden_direction || generated_exits >= exits){
			continue;
		}
		
		Point nextRoom = pos + pointDirections[i];
		if(bounds.contains(nextRoom)) {
			maze_open_connection(maze, pos, nextRoom);
			generated_exits++;
		}
	}
}

void maze_free(Maze* maze) {
	delete[] maze->buffer;
}

void maze_serialize(std::ostream &stream, Maze &maze) {
	stream.write(reinterpret_cast<char *>(&maze.cols), sizeof(int));
	stream.write(reinterpret_cast<char *>(&maze.rows), sizeof(int));
	int cellCount = maze.cols * maze.rows;
	for(int i = 0; i < cellCount; i++) {
		stream.write(reinterpret_cast<char *>(&maze.buffer[i]), sizeof(Cell));
		//cell_serialize(stream, maze.buffer[i]);
	}
}

void maze_deserialize(std::istream &stream, Maze &maze) {
	stream.read(reinterpret_cast<char *>(&maze.cols), sizeof(int));
	stream.read(reinterpret_cast<char *>(&maze.rows), sizeof(int));
	int cellCount = maze.cols * maze.rows;
	if(maze.buffer != nullptr)
		delete [] maze.buffer;

	maze.buffer = new Cell[cellCount];
	for(int i = 0; i < cellCount; i++) {
		stream.read(reinterpret_cast<char *>(&maze.buffer[i]), sizeof(Cell));
		//cell_deserialize(stream, maze.buffer[i]);
	}
}

/*
Maze* generateMaze(int cols, int rows) {
	Cell* maze = new Cell[cols*rows];
	Maze* m = new Maze();
	m->buffer = maze;
	m->cols = cols;
	m->rows = rows;
	return m;
}
*/

// void printMaze(Maze* maze) {
// 	for (int y = 0; y < maze->rows; y++) {
// 		for (int x = 0; x < maze->cols; x++) {
// 			if(maze->buffer[maze->index(x, y)].IsOpen)
// 				printf("\n");
// 			printf("y:%d, x:%d isopen: %s , istopwallopen: %s | ", y, x, maze->buffer[maze->index(x, y)].IsOpen ? "true" : "false", maze->buffer[maze->index(x, y)].IsTopWallOpen ? "true" : "false");
// 		}
// 		printf("\n");
// 	}

// 	int width = maze->cols;
// 	int height = maze->rows;
// 	for (int y = 0, i = 0; y < height * 2; y += 2, i++) {
// 		for (int x = 0, j = 0; x < width * 2; x += 2, j++) {
// 			Point v2(x, y + 1);
// 		}
// 	}
// }


// static void cell_serialize(std::ofstream &stream, Cell &cell) {
// 	stream.write(reinterpret_cast<char *>(&cell.IsOpen), sizeof(bool));
// 	stream.write(reinterpret_cast<char *>(&cell.IsLeftWallOpen), sizeof(bool));
// 	stream.write(reinterpret_cast<char *>(&cell.IsTopWallOpen), sizeof(bool));
// 	stream.write(reinterpret_cast<char *>(&cell.Openings), sizeof(Directions));
// }

// static void cell_deserialize(std::ifstream &stream, Cell &cell) {

// }


/*

struct Cell {
	bool IsOpen = false;
	bool IsLeftWallOpen = false;
	bool IsTopWallOpen = false;
	Directions Openings = Directions::None;
};

struct Maze {
	int cols;
	int rows;
	Cell* buffer;
	size_t index(int x, int y) const { return x + cols * y; }
	Cell &cell(Point &p) { return buffer[index(p.x, p.y)]; };
};

/*
int getNumExits(Maze* maze) {
	assert(false && "not implemented");
	// private int GetNumExits(Vec pos)
    //     {
    //         if (!Bounds.Contains(pos)) throw new ArgumentOutOfRangeException("pos");

    //         int exits = 0;

    //         if (mCells[pos].IsLeftWallOpen) exits++;
    //         if (mCells[pos].IsTopWallOpen) exits++;
    //         if (mCells[pos.OffsetX(1)].IsLeftWallOpen) exits++;
    //         if (mCells[pos.OffsetY(1)].IsTopWallOpen) exits++;

    //         return exits;
    //     }
	return -1;
}
*/