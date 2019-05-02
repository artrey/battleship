#include <stdlib.h>
#include <string.h>
#include "battle_map.h"
#include "utils.h"
#include "smart_console.h"

#define MAX_ATTEMPTS_PUT_SHIP 100
#define MAX_ATTEMPTS_GENERATE_MAP 100
#define MAX_ATTEMPTS_RANDOM_COORD 100

typedef enum direction
{
	DIRECTION_RIGHT,
	DIRECTION_UP,
	DIRECTION_LEFT,
	DIRECTION_DOWN,
} direction_t;

int validate_position(const canvas_t* canvas, int x1, int y1, int x2, int y2)
{
	int x, y;
	for (y = y1 - 1; y < y2 + 2; ++y)
	{
		if (y < -1 || y > BATTLE_MAP_SIZE_Y) return 0; // not OK
		if (y == -1 || y == BATTLE_MAP_SIZE_Y) continue;
		
		for (x = x1 - 1; x < x2 + 2; ++x)
		{
			if (x < -1 || x > BATTLE_MAP_SIZE_X) return 0; // not OK
			if (x == -1 || x == BATTLE_MAP_SIZE_X) continue;

			if (canvas->canvas[y][x] != BATTLE_MAP_EMPTY) return 0; // not OK
		}
	}
	return 1; // OK
}

void put_ship(battle_map_t* map, int x1, int y1, int x2, int y2)
{
	int x, y;
	for (y = y1; y < y2 + 1; ++y)
	{
		for (x = x1; x < x2 + 1; ++x)
		{
			map->map.canvas[y][x] = BATTLE_MAP_SHIP;
		}
	}
	map->ships[map->ships_count].x1 = x1;
	map->ships[map->ships_count].y1 = y1;
	map->ships[map->ships_count].x2 = x2;
	map->ships[map->ships_count].y2 = y2;
	++map->ships_count;
}

int inner_init_map(battle_map_t* map)
{
	int i, j, k, angle, length, x, y, attempts, put;

	memset(map->map.canvas, BATTLE_MAP_EMPTY, sizeof(char) * BATTLE_MAP_SIZE_X * BATTLE_MAP_SIZE_Y);
	map->ships_count = 0;

	init_randomizer();

	for (i = 0; i < 4; ++i)
	{
		length = 4 - i;
		for (j = 0; j < i + 1; ++j)
		{
			attempts = MAX_ATTEMPTS_PUT_SHIP;
			put = 0;
			
			while (!put && attempts--)
			{
				x = random_int(0, BATTLE_MAP_SIZE_X - 1);
				y = random_int(0, BATTLE_MAP_SIZE_Y - 1);
				angle = random_int(0, 3);
				
				// find location and put ship into map
				for (k = 0; k < 4; ++k)
				{
					switch ((angle + k) % 4)
					{
					case DIRECTION_RIGHT:
						if (x + length - 1 < BATTLE_MAP_SIZE_X)
						{
							// check nearest cells
							if (!validate_position(&map->map, x, y, x + length - 1, y)) continue;
							put_ship(map, x, y, x + length - 1, y);
							put = 1;
						}
						break;
					case DIRECTION_UP:
						if (y - length + 1 >= 0)
						{
							// check nearest cells
							if (!validate_position(&map->map, x, y - length + 1, x, y)) continue;
							put_ship(map, x, y - length + 1, x, y);
							put = 1;
						}
						break;
					case DIRECTION_LEFT:
						if (x - length + 1 >= 0)
						{
							// check nearest cells
							if (!validate_position(&map->map, x - length + 1, y, x, y)) continue;
							put_ship(map, x - length + 1, y, x, y);
							put = 1;
						}
						break;
					case DIRECTION_DOWN:
						if (y + length - 1 < BATTLE_MAP_SIZE_Y)
						{
							// check nearest cells
							if (!validate_position(&map->map, x, y, x, y + length - 1)) continue;
							put_ship(map, x, y, x, y + length - 1);
							put = 1;
						}
						break;
					default:
						break;
					}
					break;
				}
			}

			if (attempts < 0) return 0; // not OK
		}
	}

	return 1; // OK
}

battle_map_t* generate_random_map()
{
	int attempts;
	battle_map_t* ret = (battle_map_t*)malloc(sizeof(battle_map_t));

	attempts = MAX_ATTEMPTS_GENERATE_MAP;
	while (attempts--)
	{
		if (inner_init_map(ret)) break;
	}

	if (attempts < 0)
	{
		free_map(ret);
	}
	
	return attempts < 0 ? NULL : ret;
}

void free_map(battle_map_t* map)
{
	free(map);
}

battle_map_view_t* create_and_link_view(const battle_map_t* map, int empty_view)
{
	battle_map_view_t* ret = (battle_map_view_t*)malloc(sizeof(battle_map_view_t));
	ret->linked_map = map;
	ret->ships_alive = map->ships_count;
	if (empty_view) memset(ret->view.canvas, BATTLE_MAP_EMPTY, sizeof(char) * BATTLE_MAP_SIZE_X * BATTLE_MAP_SIZE_Y);
	else memcpy(ret->view.canvas, map->map.canvas, sizeof(char) * BATTLE_MAP_SIZE_X * BATTLE_MAP_SIZE_Y);
	return ret;
}

void free_view(battle_map_view_t* view)
{
	free(view);
}

void merge_view_with_map(battle_map_view_t* view)
{
	int x, y;
	for (y = 0; y < BATTLE_MAP_SIZE_Y; ++y)
	{
		for (x = 0; x < BATTLE_MAP_SIZE_X; ++x)
		{
			if (view->linked_map->map.canvas[y][x] == BATTLE_MAP_SHIP
				&& view->view.canvas[y][x] == BATTLE_MAP_EMPTY)
			{
				view->view.canvas[y][x] = BATTLE_MAP_SHIP;
			}
		}
	}
}

const ship_t* find_ship(const battle_map_t* map, int x, int y)
{
	const ship_t* ret;
	int i;
	for (i = 0; i < map->ships_count; ++i)
	{
		ret = &map->ships[i];
		if (ret->x1 <= x && x <= ret->x2 && ret->y1 <= y && y <= ret->y2)
		{
			return ret;
		}
	}
	return NULL;
}

void process_damage(battle_map_view_t* view, int x, int y)
{
	int i, j;
	const ship_t* ship = find_ship(view->linked_map, x, y);
	if (ship == NULL) return;
	
	for (j = ship->y1; j < ship->y2 + 1; ++j)
	{
		for (i = ship->x1; i < ship->x2 + 1; ++i)
		{
			if (view->view.canvas[j][i] != BATTLE_MAP_DAMAGE) return;
		}
	}

	for (j = ship->y1 - 1; j < ship->y2 + 2; ++j)
	{
		if (j < 0 || j >= BATTLE_MAP_SIZE_Y) continue;
		for (i = ship->x1 - 1; i < ship->x2 + 2; ++i)
		{
			if (i < 0 || i >= BATTLE_MAP_SIZE_X) continue;
			if (view->view.canvas[j][i] == BATTLE_MAP_DAMAGE) continue;
			view->view.canvas[j][i] = BATTLE_MAP_MISS;
		}
	}
	--view->ships_alive;
}

int available_cell(const canvas_t* canvas, int x, int y)
{
	return canvas->canvas[y][x] == BATTLE_MAP_EMPTY || canvas->canvas[y][x] == BATTLE_MAP_SHIP;
}

fire_result_t fire(battle_map_view_t* view, int x, int y)
{
	if (!available_cell(&view->view, x, y)) return FIRE_DENY;

	switch (view->linked_map->map.canvas[y][x])
	{
	case BATTLE_MAP_EMPTY:
		view->view.canvas[y][x] = BATTLE_MAP_MISS;
		return FIRE_MISS;
	case BATTLE_MAP_SHIP:
		view->view.canvas[y][x] = BATTLE_MAP_DAMAGE;
		process_damage(view, x, y);
		return FIRE_SUCCESS;
	default:
		return FIRE_DENY;
	}
}

int is_alive(const battle_map_view_t* view)
{
	if (view->ships_alive > 0) return 1; // is alive
	return 0; // not alive
}

void random_next_coord(const battle_map_view_t* view, int* x, int* y)
{
	int attempts = MAX_ATTEMPTS_RANDOM_COORD;

	do
	{
		*x = random_int(0, BATTLE_MAP_SIZE_X - 1);
		*y = random_int(0, BATTLE_MAP_SIZE_Y - 1);
	} while (!available_cell(&view->view, *x, *y) && attempts--);

	if (attempts < 0)
	{
		for (*y = 0; *y < BATTLE_MAP_SIZE_Y; ++*y)
		{
			for (*x = 0; *x < BATTLE_MAP_SIZE_X; ++*x)
			{
				if (available_cell(&view->view, *x, *y)) return;
			}
		}
	}
}

void normal_next_coord(const battle_map_view_t* view, int* x, int* y)
{
	const ship_t* ship;
	int i, j, damaged, p1, p2;

	for (*y = 0; *y < BATTLE_MAP_SIZE_Y; ++*y)
	{
		for (*x = 0; *x < BATTLE_MAP_SIZE_X; ++*x)
		{
			if (view->view.canvas[*y][*x] == BATTLE_MAP_DAMAGE)
			{
				damaged = 0;
				ship = find_ship(view->linked_map, *x, *y);
				for (j = ship->y1; j < ship->y2 + 1; ++j)
				{
					for (i = ship->x1; i < ship->x2 + 1; ++i)
					{
						if (view->view.canvas[j][i] == BATTLE_MAP_DAMAGE) ++damaged;
					}
				}

				if (damaged > 0 && damaged < ship->x2 - ship->x1 + ship->y2 - ship->y1 + 1)
				{
					// one cell
					if (damaged == 1)
					{
						p1 = 0b1111;
						p2 = 4;
						
						if (*x == 0 || !available_cell(&view->view, *x - 1, ship->y1))
						{
							p1 &= 0b0111;
							--p2;
						}
						if (*x == BATTLE_MAP_SIZE_X - 1 || !available_cell(&view->view, *x + 1, ship->y1))
						{
							p1 &= 0b1011;
							--p2;
						}
						if (*y == 0 || !available_cell(&view->view, ship->x1, *y - 1))
						{
							p1 &= 0b1101;
							--p2;
						}
						if (*y == BATTLE_MAP_SIZE_Y - 1 || !available_cell(&view->view, ship->x1, *y + 1))
						{
							p1 &= 0b1110;
							--p2;
						}

						p2 = random_int(0, p2 - 1);
						for (i = 0; i < 4; ++i)
						{
							j = p1 >> (3 - i) & 1;
							if (!j) continue;
							if (p2-- == 0)
							{
								switch (i)
								{
								case 0: --*x; break;
								case 1: ++*x; break;
								case 2: --*y; break;
								case 3: ++*y; break;
								default: break;
								}
								break;
							}
						}
					}
					// horizontal
					else if (ship->y1 == ship->y2)
					{
						*y = ship->y1;
						p1 = p2 = *x;
						for (i = ship->x1; i < ship->x2 + 1; ++i)
						{
							if (view->view.canvas[ship->y1][i] == BATTLE_MAP_DAMAGE)
							{
								p1 = i - 1;
								break;
							}
						}
						for (i = ship->x2; i > ship->x1 - 1; --i)
						{
							if (view->view.canvas[ship->y1][i] == BATTLE_MAP_DAMAGE)
							{
								p2 = i + 1;
								break;
							}
						}
						if (p1 < 0 || !available_cell(&view->view, p1, ship->y1)) *x = p2;
						else if (p2 >= BATTLE_MAP_SIZE_X || !available_cell(&view->view, p2, ship->y1)) *x = p1;
						else *x = random_bool() ? p1 : p2;
					}
					// vertical
					else if (ship->x1 == ship->x2)
					{
						*x = ship->x1;
						p1 = p2 = *y;
						for (i = ship->y1; i < ship->y2 + 1; ++i)
						{
							if (view->view.canvas[i][ship->x1] == BATTLE_MAP_DAMAGE)
							{
								p1 = i - 1;
								break;
							}
						}
						for (i = ship->y2; i > ship->y1 - 1; --i)
						{
							if (view->view.canvas[i][ship->x1] == BATTLE_MAP_DAMAGE)
							{
								p2 = i + 1;
								break;
							}
						}
						if (p1 < 0 || !available_cell(&view->view, ship->x1, p1)) *y = p2;
						else if (p2 >= BATTLE_MAP_SIZE_Y || !available_cell(&view->view, ship->x1, p2)) *y = p1;
						else *y = random_bool() ? p1 : p2;
					}

					return;
				}
			}
		}
	}

	random_next_coord(view, x, y);
}

void dishonest_next_coord(const battle_map_view_t* view, int* x, int* y)
{
	for (*y = 0; *y < BATTLE_MAP_SIZE_Y; ++*y)
	{
		for (*x = 0; *x < BATTLE_MAP_SIZE_X; ++*x)
		{
			if (view->linked_map->map.canvas[*y][*x] == BATTLE_MAP_SHIP
				&& available_cell(&view->view, *x, *y))
			{
				return;
			}
		}
	}
}

void get_next_coord(const battle_map_view_t* view, complexity_t complexity, int* x, int* y)
{
	switch (complexity)
	{
	case COMPLEXITY_RANDOM:
		random_next_coord(view, x, y);
		break;
	case COMPLEXITY_DISHONEST:
		dishonest_next_coord(view, x, y);
		break;
	case COMPLEXITY_NORMAL:
	default:
		normal_next_coord(view, x, y);
		break;		
	}
}

void print_canvas(const canvas_t* canvas, int offset_x, int offset_y)
{
	int x, y;

	set_cursor_pos(offset_x, offset_y++);
	printf("  А Б В Г Д Е Ж З И К");
	set_cursor_pos(offset_x, offset_y++);
	printf(" |-------------------|");
	
	for (y = 0; y < BATTLE_MAP_SIZE_Y; ++y)
	{
		set_cursor_pos(offset_x, offset_y + 2 * y);
		printf("%d|", y);
		for (x = 0; x < BATTLE_MAP_SIZE_X; ++x)
		{
			printf("%c|", canvas->canvas[y][x]);
		}

		set_cursor_pos(offset_x, offset_y + 2 * y + 1);
		printf(" |-------------------|");
	}
}
