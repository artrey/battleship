#ifndef BATTLE_MAP_H
#define BATTLE_MAP_H

#define BATTLE_MAP_SIZE_X 10
#define BATTLE_MAP_SIZE_Y 10
#define BATTLE_MAP_SHIPS_COUNT 10

#define BATTLE_MAP_SHIP '#'
#define BATTLE_MAP_EMPTY ' '
#define BATTLE_MAP_DAMAGE 'X'
#define BATTLE_MAP_MISS '*'

typedef struct ship
{
	int x1;
	int y1;
	int x2;
	int y2;
} ship_t;

typedef struct canvas
{
	char canvas[BATTLE_MAP_SIZE_Y][BATTLE_MAP_SIZE_X];
} canvas_t;

typedef struct battle_map
{
	canvas_t map;
	ship_t ships[BATTLE_MAP_SHIPS_COUNT];
	int ships_count;
} battle_map_t;

typedef struct battle_map_view
{
	const battle_map_t* linked_map;
	canvas_t view;
	int ships_alive;
} battle_map_view_t;

typedef enum fire_result
{
	FIRE_DENY,
	FIRE_MISS,
	FIRE_SUCCESS,
} fire_result_t;

typedef enum complexity
{
	COMPLEXITY_RANDOM,
	COMPLEXITY_NORMAL,
	COMPLEXITY_DISHONEST,
} complexity_t;

battle_map_t* generate_random_map();
void free_map(battle_map_t* map);

battle_map_view_t* create_and_link_view(const battle_map_t* map, int empty_view);
void free_view(battle_map_view_t* view);
void merge_view_with_map(battle_map_view_t* view);

fire_result_t fire(battle_map_view_t* view, int x, int y);
int is_alive(const battle_map_view_t* view);
void get_next_coord(const battle_map_view_t* view, complexity_t complexity, int* x, int* y);

void print_canvas(const canvas_t* canvas, int offset_x, int offset_y);

#endif