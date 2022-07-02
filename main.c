#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

// Making a solver for the rummykub game consisting of tiles composed with number going from 1 to 13 and colors red, blue, green, and yellow.
// A tile set is a collection of tiles.
// A tile set is valid if it contains at least 3 tiles of the same number but different colors, or if it contains at least 3 tiles of the same color but adjacent numbers.
// A group is a tile set containing tiles with different colors but with the same number.
// A run is a tile set containing tiles with the same color but with consecutive number.
// A valid hand is a collection of tiles that contains at least one group or one run.
// The score of a tile set is the sum of the number of tiles in the set.
// There is a player tile set and a table tile set.
// The player tile set is the collection of tiles that the player has in his hand.
// The table tile set is the collection of tiles on the table.
// All tiles sets on the table are valid.
// The player can use the tiles in his hand to form a valid tile set on the table.
// The player can also use the tiles on the table to form a valid tile set in his hand.
// The goal of the game is to place all tiles on the table in a valid tile set.
// The first player to place all tiles on the table in a valid tile set wins.
// The solver need to find the best move for the player.

// Game tiles constitued of fields number (0-13) and color (R,B,G,Y)
struct Tile_s{
    int number;
    char color;
    struct Tile_s* previous_tile;
    struct Tile_s* next_tile;
    struct TileSet_s* tile_set;
};

// A Set of tiles constitued of an array of tiles and its size
struct TileSet_s{
    struct Tile_s* tiles;
    struct Tile_s* last_tile;
    struct TileSet_s* first_set;
    struct TileSet_s* next_set;
    struct TileSet_s* previous_set;
    int number;
};

struct PriorityQueue_s{
    struct TileSet_s* tile_set;
    int g;
    float h;
    struct PriorityQueue_s* next_set;
};

// Creates a new tile with the given number and color
struct Tile_s* CreateTile(int number, char color);
// Create the initial set of tiles for the game
struct TileSet_s* CreateInitialTilesSet();
void PrintTile(struct Tile_s* tile);
void PrintTileSets(struct TileSet_s* tileset);
void PrintPlayerTileSet(struct TileSet_s* tileset);
// Pick a random tile from the given tile set and update the tile set
struct Tile_s* PickTileFromSet(struct TileSet_s* tileset);
// Return player starting tile set of 13 tiles
struct TileSet_s* CreatePlayerTileSet(struct TileSet_s* tileset);
// Put a tile at the end of the given tile set and update the tile set size
void PlaceTileInSet(struct TileSet_s* tileset, struct Tile_s* tile);
// Split tile set in tiles sets of same color
struct TileSet_s** SplitTileSetByColor(struct TileSet_s* tileset);
void AddTileToSortedTileSet(struct TileSet_s** tileset, struct Tile_s* tile);
struct TileSet_s* CreateTilesSet();
// Solver
bool isPartialSet(struct TileSet_s* tileset);

struct TileSet_s* GetAdjacentTileSets(struct TileSet_s* tileset){
    if(!tileset)
        return NULL;
    struct TileSet_s* tilesets = NULL;

    // Split the initial tile set in tiles sets of same color
    struct TileSet_s** tilesets_by_color = SplitTileSetByColor(tileset);
    // For each color
    for(int i = 0; i < 4; i++){
        // Get sequence of consecutive numbers
        struct TileSet_s* tileset_by_color = tilesets_by_color[i];  
        // For each tile in the set
        struct Tile_s* cursor = tileset_by_color->tiles;
        while(cursor)
        {
            int number = cursor->number;
            struct Tile_s* tile = cursor->next_tile;
            int sequence = 1;
            while(tile)
            {
                if(tile->number == number)
                {
                    tile = tile->next_tile;
                    continue;
                }
                if(tile->number == number + 1)
                {
                    sequence++;
                    number = tile->number;
                }
                else
                {
                    if (sequence >= 3)
                    {  
                        struct Tile_s* start = cursor;
                        AddTileSetToTileSet(&tilesets, CreateTilesSet());
                        while(start != tile)
                        {
                            AddTileToTileSet(&tilesets, CreateTile(start->number, start->color));
                            start = start->next_tile;
                            number = tile->number;
                        }
                    }
                    number = tile->number;
                    cursor = tile;
                    sequence = 1;
                }
                tile = tile->next_tile;
            }

            if (sequence >= 3)
            {  
                struct Tile_s* start = cursor;
                AddTileSetToTileSet(&tilesets, CreateTilesSet());
                int i = 0;
                while(start && (start != tile))
                {
                    AddTileToSortedTileSet(&tilesets, CreateTile(start->number, start->color));
                    start = start->next_tile;                            
                }
            }

            cursor = tile;
        }

    }
    return tilesets;
}

// Remove tiles doublons from the given tile set
struct TileSet_s* RemoveDoublons(struct TileSet_s* tileset){
    if((!tileset) || (tileset->tiles == NULL))
        return NULL;
    struct TileSet_s* cursor = tileset;
    while(cursor)
    {
        struct Tile_s* tile = cursor->tiles;
        struct Tile_s* tile2 = tile;;
        while(tile && tile->next_tile)
        {
            tile2 = tile;
            while(tile2->next_tile)
            {
                if((tile->number == tile2->next_tile->number) && (tile->color == tile2->next_tile->color))
                {
                    struct tile* dup = tile2->next_tile;
                    tile2->next_tile = tile2->next_tile->next_tile;
                    free(dup);
                    cursor->number--;
                }
                else
                {
                    tile2 = tile2->next_tile;
                }
            }
            tile = tile->next_tile;
        }
        cursor = cursor->next_set;
    }
    return tileset;
}

// Get all combinations of 3 tiles from adjacent tile set
struct TileSet_s* GetAdjacentTileSetsCombinations(struct TileSet_s* tileset_colors){
    if(!tileset_colors)
        return NULL;
    //struct TileSet_s* tileset = RemoveDoublons(tileset_colors);
    struct TileSet_s* tilesets = CreateTilesSet();
    struct TileSet_s* cursor = tileset_colors;
    while(cursor)
    {
        struct Tile_s* tile = cursor->tiles;
        struct Tile_s* start = tile;
        
        int i = 0;
        int start_idx = 0;
        int offset = 3;
        while(offset <= cursor->number)
        {
            tile = cursor->tiles;
            start_idx = 0;
            while(tile && (start_idx+offset <= cursor->number))
                    {
                        AddTileToSortedTileSet(&tilesets, CreateTile(tile->number, tile->color));
                        i++;
                        tile = tile->next_tile;

                        if(i == offset)
                        {
                            start = start->next_tile;
                            tile = start;
                            start_idx++;
                            i = 0;
                            AddTileSetToTileSet(&tilesets, CreateTilesSet());
                        }
                    }
            offset++;
        }

        cursor = cursor->next_set;
    }
    if(tilesets)
    {
        struct TileSet_s* next = tilesets->next_set;
        FreeTileSet(tilesets);
        tilesets = next;
    }
    return tilesets;
}

void FreeTiles(struct Tile_s* tiles)
{
    struct Tile_s* next;
    while(tiles)
    {
        next = tiles->next_tile;
        free(tiles);
        tiles = next;
    }
    
}

void FreeTileSet(struct TileSet_s* tileset)
{
    if(!tileset)
        return;
    FreeTiles(tileset->tiles);
    free(tileset);
}

void FreeTileSets(struct TileSet_s* tileset)
{
    struct TileSet_s* cursor_set = tileset;
    struct TileSet_s* next_set;
    while (cursor_set)
    {
        next_set = cursor_set->next_set;
        FreeTileSet(cursor_set);
        cursor_set = next_set;
    }
    
}


// Split tiles set by number
struct TileSet_s** SplitTileSetByNumber(struct TileSet_s* tileset){
    struct TileSet_s** tilesets = malloc(sizeof(struct TileSet_s*)*14);
    for(int i = 0; i < 13; i++)
    {
        tilesets[i] = CreateTilesSet();
    }
    struct Tile_s* cursor = tileset->tiles;
    while(cursor)
    {
        AddTileToSortedTileSet(&tilesets[cursor->number-1], CreateTile(cursor->number, cursor->color));
        cursor = cursor->next_tile;
    }
    return tilesets;
}

// Get all combinations of 3 tiles from number tile set
struct TileSet_s* GetNumberTileSetsCombinations(struct TileSet_s** tilesets){
    struct TileSet_s* tileset = NULL;
    for(int i = 0; i < 13; i++)
    {
        struct TileSet_s* cursor = tilesets[i];
        //cursor = RemoveDoublons(tilesets[i]);
        if(!cursor)
            continue;

            if(cursor->number == 3)
            {
                //cursor = RemoveDoublons(cursor);
                struct Tile_s* tile = cursor->tiles;
                AddTileSetToTileSet(&tileset, CreateTilesSet());
                while (tile)
                {
                    AddTileToSortedTileSet(&tileset, CreateTile(tile->number, tile->color));
                    tile = tile->next_tile;
                }
                
            }
            else if(cursor->number == 4)
            {
                struct Tile_s* tile = cursor->tiles;
                AddTileSetToTileSet(&tileset, CreateTilesSet());
                while(tile)
                {
                    AddTileToSortedTileSet(&tileset, CreateTile(tile->number, tile->color));
                    tile = tile->next_tile;
                }
                int i = 0;
                //cursor = RemoveDoublons(cursor);
                while(i <= 3)
                {
                    struct Tile_s* tile = cursor->tiles;
                    AddTileSetToTileSet(&tileset, CreateTilesSet());
                    for (int j = 0; j <= 3; j++, tile = tile->next_tile)
                    {
                        if(j == i)
                            continue;
                        AddTileToSortedTileSet(&tileset, CreateTile(tile->number, tile->color));
                    }
                    
                    i++;
                }

            }
    
    }
    return tileset;
}

// Return a copy of a given tile set
struct TileSet_s* CopyTileSet(struct TileSet_s* tileset){
    if(!tileset)
        return NULL;
    struct TileSet_s* new_tileset = CreateTilesSet();
    struct Tile_s* cursor = tileset->tiles;
    while(cursor)
    {
        AddTileToTileSet(&new_tileset, CreateTile(cursor->number, cursor->color));
        cursor = cursor->next_tile;
    }
    return new_tileset;
}

// Copy tiles sets
struct TileSet_s* CopyTileSets(struct TileSet_s* tileset)
{
    struct TileSet_s* new_set = NULL;
    struct TileSet_s* cursor = tileset;
    struct Tile_s* cursor_tile;
    while(cursor)
    {
        AddTileSetToTileSet(&new_set, CreateTilesSet());
        cursor_tile = cursor->tiles;
        while(cursor_tile)
        {
            AddTileToSortedTileSet(&new_set, CreateTile(cursor_tile->number, cursor_tile->color));
            cursor_tile = cursor_tile->next_tile;
        }
        cursor = cursor->next_set;
    }
  
    return new_set;
}

struct TileSet_s* CopyTileSetsWithoutSet(struct TileSet_s* tileset, struct TileSet_s* tileset_to_exclude)
{
    struct TileSet_s* new_set = NULL;
    struct TileSet_s* cursor = tileset;
    struct Tile_s* cursor_tile;
    while(cursor)
    {
        if(cursor == tileset_to_exclude)
        {
            cursor = cursor->next_set;
            continue;
        }

        AddTileSetToTileSet(&new_set, CreateTilesSet());
        cursor_tile = cursor->tiles;
        while(cursor_tile)
        {
            AddTileToSortedTileSet(&new_set, CreateTile(cursor_tile->number, cursor_tile->color));
            cursor_tile = cursor_tile->next_tile;
        }
        cursor = cursor->next_set;
    }
  
    return new_set;
}

struct TileSet_s* CopyTileSetsWithoutTile(struct TileSet_s* tileset, struct Tile_s* tile)
{
    struct TileSet_s* new_set = NULL;
    struct TileSet_s* cursor = tileset;
    struct Tile_s* cursor_tile;
    while(cursor)
    {
        AddTileSetToTileSet(&new_set, CreateTilesSet());
        cursor_tile = cursor->tiles;
        while(cursor_tile)
        {
            if(cursor_tile == tile)
            {
                cursor_tile = cursor_tile->next_tile;
                continue;
            }
            AddTileToSortedTileSet(&new_set, CreateTile(cursor_tile->number, cursor_tile->color));
            cursor_tile = cursor_tile->next_tile;
        }
        cursor = cursor->next_set;
    }
    if(!new_set->tiles)
        RemoveTileSet(&new_set);
    return new_set;
}

// Remove tiles in tile set from a given tile set
struct TileSet_s* RemoveTilesFromTileSet(struct TileSet_s* tileset, struct TileSet_s* tileset_to_remove){
    if(!tileset || !tileset_to_remove)
        return NULL;
    struct Tile_s* cursor = tileset_to_remove->tiles;
    while(cursor)
    {
        RemoveTileFromTileSet(tileset, cursor->number, cursor->color);
        cursor = cursor->next_tile;
    }
    return tileset;
}


// Check if a tile is in a given tile set
bool IsTileInTileSet(struct TileSet_s* tileset, int number, int color){
    if(!tileset)
        return false;
    struct Tile_s* cursor = tileset->tiles;
    while(cursor)
    {
        if((cursor->number == number) && (cursor->color == color))
            return true;
        cursor = cursor->next_tile;
    }
    return false;
}

// Check if tile set is a subset of a given tile set
bool IsTileSetSubsetOf(struct TileSet_s* tileset, struct TileSet_s* tileset_to_check){
    if(!tileset || !tileset_to_check)
        return false;
    struct Tile_s* cursor = tileset->tiles;
    while(cursor)
    {
        if(!IsTileInTileSet(tileset_to_check, cursor->number, cursor->color))
            return false;
        cursor = cursor->next_tile;
    }
    return true;
}

// Remove a tile from a tile set
void RemoveTileFromTileSet(struct TileSet_s* tileset, int number, int color){
    if(!tileset)
        return;
    struct Tile_s* cursor = tileset->tiles;
    struct Tile_s* prev = NULL;
    while(cursor)
    {
        if(cursor->number == number && cursor->color == color)
        {
            if(prev){
                prev->next_tile = cursor->next_tile;
                if(cursor->next_tile)
                    cursor->next_tile->previous_tile = prev;
            }
            else{
                tileset->tiles = cursor->next_tile;
                if(cursor->next_tile)
                    cursor->next_tile->previous_tile = NULL;
            }
            free(cursor);
            tileset->number--;
            return;
        }
        prev = cursor;
        cursor = cursor->next_tile;
    }
}

// Add tiles from a set to a given tile set 
void AddTilesFromTileSetToTileSet(struct TileSet_s* tileset, struct TileSet_s* tileset_to_add)
{
    struct Tile_s* cursor = tileset_to_add->tiles;
    while(cursor)
    {
        AddTileToTileSet(&tileset, CreateTile(cursor->number, cursor->color));
        cursor = cursor->next_tile;
    }
    
}

// Is a tile set a partially valid tile set ?

// Pick last tile from a tile set (last_tile = last tile in the tile set)
struct Tile_s* PickLastTile(struct TileSet_s* tileset){
    struct Tile_s* last_tile = tileset->last_tile;
    if(last_tile->previous_tile)
        last_tile->previous_tile->next_tile = NULL;
    tileset->last_tile = last_tile->previous_tile;

    last_tile->next_tile = NULL;
    last_tile->previous_tile = NULL;
    return last_tile;
}

// Pick first tile from a tile set (first_tile = first tile in the tile set)
struct Tile_s* PickFirstTile(struct TileSet_s* tileset){
    struct Tile_s* first_tile = tileset->tiles;
    tileset->tiles = first_tile->next_tile;
    if(first_tile->next_tile)
        first_tile->next_tile->previous_tile = NULL;

    first_tile->next_tile = NULL;
    first_tile->previous_tile = NULL;
    return first_tile;
}

// Put a tile at the end of a tile set
void PutTileAtEndOfTileSet(struct TileSet_s* tileset, struct Tile_s* tile){
    if(!tileset->tiles)
    {
        tileset->tiles = tile;
        return;
    }
    tile->previous_tile = tileset->last_tile;
    tile->next_tile = NULL;
    tileset->last_tile->next_tile = tile;
    tileset->last_tile = tile;
    tile->tile_set = tileset;
    tileset->number++;
}


int GetScoreFromTileSet(struct TileSet_s* tileset)
{
    int score = 0;
    struct Tile_s* cursor = tileset->tiles;
    while(cursor)
    {
        score += cursor->number;
        cursor = cursor->next_tile;
    }
    return score;
}

struct TileSet_s* GetAllCombinations(struct TileSet_s* tileset)
{
    tileset = RemoveDoublons(tileset);
    struct TileSet_s* runs_tileset = GetAdjacentTileSets(tileset);
    struct TileSet_s* runs_tileset_combinations = GetAdjacentTileSetsCombinations(runs_tileset);
    printf("Possible runs :\n");
    PrintTileSets(runs_tileset_combinations);
    FreeTileSet(runs_tileset);

    struct TileSet_s** groups_tilesets = SplitTileSetByNumber(tileset);
    struct TileSet_s* groups_tileset_combinations = GetNumberTileSetsCombinations(groups_tilesets);
    printf("Possible groups :\n");
    PrintTileSets(groups_tileset_combinations);
    FreeTileSet(groups_tilesets);

    struct TileSet_s* copy_tileset = CopyTileSet(tileset);
    AddTileSetToTileSetQueu(&runs_tileset_combinations, groups_tileset_combinations);
    struct TileSet_s* possible_combinations_tileset = runs_tileset_combinations;

    struct TileSet_s* cursor = possible_combinations_tileset;
    if(!cursor)
        return NULL;
    struct TileSet_s* combinations_tileset = NULL;
    struct TileSet_s* cursor2;

    struct TileSet_s* best_score_set = NULL;
    int best_score = INT_MIN;
    while(cursor)
    {
        cursor2 = cursor->next_set;
        copy_tileset = CopyTileSet(tileset);
        // Remove cursor from possible_combinations_tileset
        RemoveTilesFromTileSet(copy_tileset, cursor);
        // Add cursor to combinations_tileset
        AddTileSetToTileSet(&combinations_tileset, CopyTileSet(cursor));

        while(cursor2)
        {
            // Is cursor2 a subset of possible_combinations_tileset ?
            if(IsTileSetSubsetOf(cursor2, copy_tileset))
            {
                // Remove cursor2 from possible_combinations_tileset
                RemoveTilesFromTileSet(copy_tileset, cursor2);
                // Add cursor2 to combinations_tileset
                AddTilesFromTileSetToTileSet(combinations_tileset, cursor2);
            }
            cursor2 = cursor2->next_set;
        }
        cursor = cursor->next_set;
        int score = GetScoreFromTileSet(combinations_tileset);
        if (score > best_score)
        {
            best_score = score;
            best_score_set = combinations_tileset;
        }

        FreeTileSet(copy_tileset);
    }
    printf("Best Score combinations :\n");
    PrintTileSet(best_score_set);
    return combinations_tileset;
}

// Check if the given tile set is a run
bool IsRun(struct TileSet_s* tileset)
{
    if(!tileset || !tileset->tiles)
        return false;
    if(tileset->number < 3)
        return false;
    struct Tile_s* cursor = tileset->tiles;
    int previous_number = cursor->number;
    char previous_color = cursor->color;
    cursor = cursor->next_tile;
    while(cursor)
    {
        if((cursor->number != previous_number + 1) || (cursor->color != previous_color))
            return false;
        previous_number = cursor->number;
        previous_color = cursor->color;
        cursor = cursor->next_tile;
    }
    return true;
}

bool isPartialRun(struct TileSet_s* tileset)
{
    if(!tileset || !tileset->tiles)
        return false;
    if(tileset->number > 2)
        return false;
    struct Tile_s* cursor = tileset->tiles;
    int previous_number = cursor->number;
    char previous_color = cursor->color;
    cursor = cursor->next_tile;
    while(cursor)
    {
        if((cursor->number != previous_number + 1) || (cursor->color != previous_color))
            return false;
        previous_number = cursor->number;
        previous_color = cursor->color;
        cursor = cursor->next_tile;
    }
    return true;
}

// Check if the given tile set is a group
bool IsGroup(struct TileSet_s* tileset)
{
    if(!tileset || !tileset->tiles)
        return false;
    if(tileset->number < 3)
        return false;
    struct Tile_s* cursor = tileset->tiles;
    int previous_number = cursor->number;
    char previous_color = cursor->color;
    cursor = cursor->next_tile;
    while(cursor)
    {
        if((cursor->number != previous_number) || (cursor->color == previous_color))
            return false;
        previous_number = cursor->number;
        previous_color = cursor->color;
        cursor = cursor->next_tile;
    }
    return true;
}

bool isPartialGroup(struct TileSet_s* tileset)
{
    if(!tileset || !tileset->tiles)
        return false;
    if(tileset->number > 2)
        return false;
    struct Tile_s* cursor = tileset->tiles;
    int previous_number = cursor->number;
    char previous_color = cursor->color;
    cursor = cursor->next_tile;
    while(cursor)
    {
        if((cursor->number != previous_number) == (cursor->color == previous_color))
            return false;
        previous_number = cursor->number;
        previous_color = cursor->color;
        cursor = cursor->next_tile;
    }
    return true;
}

// Check if a given tile set is a valid set (IsRun or IsGroup)
bool isValidSet(struct TileSet_s* tileset)
{
    if(!tileset->tiles)
        return true;
    if(!tileset)
        return false;
    if(tileset->number > 0 && tileset->number < 3)
        return false;
    if(IsRun(tileset) || IsGroup(tileset))
        return true;
    return false;
}

bool isPartialSet(struct TileSet_s* tileset)
{
    if(!tileset || !tileset->tiles)
        return false;
    if(isPartialRun(tileset) || isPartialGroup(tileset))
        return true;
    return false;
}

float heuristic(struct TileSet_s* tileset)
{
    int nb_partials_sets = 0;
    struct TileSet_s* cursor = tileset;;
    while(cursor)
    {
        if(isPartialSet(cursor))
            nb_partials_sets++;

        cursor = cursor->next_set;
    }
    return (float)nb_partials_sets/2;
}

// Move one tile from a tile set to another tile set
void MoveTileFromTileSetToTileSet(struct TileSet_s* to_tileset, struct Tile_s* tile)
{
    if(!to_tileset || !tile)
        return;
    if(tile->next_tile)
        tile->next_tile->previous_tile = tile->previous_tile;
    AddTileToTileSet(to_tileset, tile);
}

// Remove white space from a given string
void RemoveSpaceFromString(char* s) {
    char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}

// Return tile set from a given string : "1R 2R 3R, 1G 2G 3G, 1B 2B 3B" this string will create 3 tile sets : 1R 2R 3R, 1G 2G 3G, 1B 2B 

struct TileSet_s* GetTileSetFromString(char* string)
{
    struct TileSet_s* tileset = NULL;
    char buffer[100];
    strcpy(buffer, string);
    char* token = strtok(buffer, ",");
    char* cursor;
    while(token)
    {
        cursor = token;
        AddTileSetToTileSet(&tileset, CreateTilesSet());
        while(*cursor != '\n')
        {
            int number = 0;
            char color = 0;
            while(isdigit(*cursor))
            {
                number = number * 10 + (*cursor - '0');
                cursor++;
            }
            if(*cursor == 'R')
                color = 'R';
            else if(*cursor == 'B')
                color = 'B';
            else if(*cursor == 'G')
                color = 'G';
            else if(*cursor == 'Y')
                color = 'Y';
            else
                break;

            AddTileToTileSet(&tileset, CreateTile(number, color));
            cursor++;
        }
        token = strtok(NULL, ",");
    }
    return tileset;
}

// Return a tile from a given string : "1R"
struct Tile_s* GetTileFromString(char* string)
{
    int number = 0;
    char color = 0;
    char* cursor = string;
    while(isdigit(*cursor))
    {
        number = number * 10 + (*cursor - '0');
        cursor++;
    }
    if(*cursor == 'R')
        color = 'R';
    else if(*cursor == 'B')
        color = 'B';
    else if(*cursor == 'G')
        color = 'G';
    else if(*cursor == 'Y')
        color = 'Y';
    else
        return NULL;

    return CreateTile(number, color);
}


// Get string input from stdio
char* GetStringInput()
{
    char* string = malloc(sizeof(char) * 100);
    fgets(string, 100, stdin);
    RemoveSpaceFromString(string);
    return string;
}

// Make a menu selection where the user can input a string to create tile sets
struct TileSet_s* GetTileSetFromMenu()
{
    char* string = NULL;
    struct TileSet_s* tileset = NULL;
    while(!tileset)
    {
        printf("Enter a string to create a tile set :\n");
        string = GetStringInput();
        printf("%s\n", string);
        tileset = GetTileSetFromString(string);
        if(!tileset)
            printf("Invalid string\n");
        free(string);
    }
    return tileset;
}

// Make a menu where user can select :
// 1 - Create a player tile set 
// 2 - Create a table tile set
// 3 - Get all possible combinations of a tile set
// 4 - Exit
int GetMenuSelection()
{

// Test
    struct TileSet_s* player_tileset = CreateTilesSet();
    AddTileToTileSet(&player_tileset, CreateTile(5, 'R'));

    struct TileSet_s* table_tileset = CreateTilesSet();
    AddTileToSortedTileSet(&table_tileset, CreateTile(2, 'R'));
    AddTileToSortedTileSet(&table_tileset, CreateTile(3, 'R'));
    AddTileToSortedTileSet(&table_tileset, CreateTile(4, 'R'));
    AddTileSetToTileSet(&table_tileset, CreateTilesSet());
    AddTileToSortedTileSet(&table_tileset, CreateTile(2, 'B'));
    AddTileToSortedTileSet(&table_tileset, CreateTile(3, 'B'));
    AddTileToSortedTileSet(&table_tileset, CreateTile(4, 'B'));

    AStar(player_tileset, table_tileset);

    int selection = 0;
    printf("1 - Create a player tile set\n");
    printf("2 - Add a tile to player tile set\n");
    printf("3 - Remove tile set from player tile set\n");
    printf("4 - Create a table tile set\n");
    printf("5 - Add tiles to table tile set\n");
    printf("6 - Get all possible combinations of the player tile set\n");
    printf("7 - Get all possible combinations of the player/table tile set\n");
    printf("8 - Exit\n");
    printf("Enter your selection :\n");
    scanf("%d", &selection);
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    return selection;
}

// Main loop based on return value from GetMenuSelection()
void MainLoop()
{
    int selection = 0;
    struct TileSet_s* player_tileset = NULL;
    struct TileSet_s* table_tileset = NULL;
    struct TileSet_s* player_table_tileset = NULL;
    while(selection != 8)
    {
        selection = GetMenuSelection();
        if(selection == 1)
        {
            printf("Enter a string to create a player tile set :\n");
            char* string = GetStringInput();
            player_tileset = GetTileSetFromString(string);

            free(string);
        }
        else if(selection == 2)
        {
            printf("Enter a string to add a tile to the player tile set :\n");
            char* string = GetStringInput();
            struct Tile_s* tile = GetTileFromString(string);
            AddTileToTileSet(&player_tileset, tile);
            free(string);
        }
        else if(selection == 3)
        {
            printf("Enter a string to remove a tile from the player tile set :\n");
            char* string = GetStringInput();
            struct TileSet_s* tileset_to_remove = GetTileSetFromString(string);
            RemoveTilesFromTileSet(player_tileset, tileset_to_remove);
        }
        /*else if(selection == 3)
        {
            printf("Enter a string to create a table tile set :\n");
            char* string = GetStringInput();
            table_tileset = GetTileSetFromString(string);
            free(string);
        }
        else if(selection == 4)
        {
            printf("Enter a string to add tiles to the table tile set :\n");
            char* string = GetStringInput();
            struct TileSet_s* tiles = GetTileSetFromString(string);
            AddTilesFromTileSetToTileSet(table_tileset, tiles);
            free(string);
        }

        else if(selection == 5)
        {
            if(!player_tileset)
            {
                printf("You must create a tile set first\n");
                continue;
            }
            struct TileSet_s* combinations_tileset = GetAllCombinations(player_tileset);
            if(!combinations_tileset)
                printf("No possible combinations\n");
            FreeTileSet(combinations_tileset);
        }

        else if (selection == 6)
        {
            if(!player_tileset)
            {
                printf("You must create a tile set first\n");
                continue;
            }
            else if(!table_tileset)
            {
                printf("You must create a table tile set first\n");
                continue;
            }
            AddTileSetToTileSetQueu(&player_table_tileset, player_tileset);
            AddTileSetToTileSetQueu(&player_table_tileset, table_tileset);

            struct TileSet_s* combinations_tileset = GetAllCombinations(player_table_tileset);
            if(!combinations_tileset)
                printf("No possible combinations\n");
            FreeTileSet(combinations_tileset);

        }*/

        if(player_tileset)
        {
            printf("Player tile set :\n");
            PrintPlayerTileSet(player_tileset);
            printf("\n");
        }
        if(table_tileset)
        {
            printf("Table tile set :\n");
            PrintTileSet(table_tileset);
        }
    }

    printf("Exiting\n");
}

struct PriorityQueue_s* CreatePriorityQueue(struct TileSet_s* tileset, int depth)
{
    struct PriorityQueue_s* queue = malloc(sizeof(struct PriorityQueue_s));
    queue->tile_set = CopyTileSets(tileset);
    queue->h = heuristic(tileset);
    queue->g = depth;
    queue->next_set = NULL;

    return queue;
}

void AddToPriorityQueue(struct PriorityQueue_s** queue, struct PriorityQueue_s* new_queue)
{
    if(!*queue)
    {
        *queue = new_queue;
        return;
    }
    if((float)(new_queue->g + new_queue->h) < (float)((*queue)->g + (*queue)->h))
    {
        new_queue->next_set = *queue;
        *queue = new_queue;
        return;
    }
    struct PriorityQueue_s* current = *queue;
    while(current->next_set)
    {
        if((float)(new_queue->g + new_queue->h) < (float)(current->next_set->g + current->next_set->h))
        {
            new_queue->next_set = current->next_set;
            current->next_set = new_queue;
            return;
        }
        current = current->next_set;
    }
    current->next_set = new_queue;
}

struct PriorityQueue_s* PopFromPriorityQueue(struct PriorityQueue_s** queue)
{
    if(!queue)
        return NULL;
    struct PriorityQueue_s* current = *queue;
    *queue = (*queue)->next_set;
    current->next_set = NULL;
    return current;
}

bool areValidSets(struct TileSet_s* tileset)
{
    struct TileSet_s* cursor = tileset;
    while(cursor)
    {
        if(!isValidSet(cursor))
            return false;
        cursor = cursor->next_set;
    }
    return true;
}

// Remove tile set from tile set list
void RemoveTileSet(struct TileSet_s** tileset_to_remove)
{
    if(!*tileset_to_remove)
        return;
    if((*tileset_to_remove)->next_set)
        (*tileset_to_remove)->next_set->previous_set = (*tileset_to_remove)->previous_set;
    if((*tileset_to_remove)->previous_set)
        (*tileset_to_remove)->previous_set->next_set = (*tileset_to_remove)->next_set;
    (*tileset_to_remove) = (*tileset_to_remove)->next_set;
}

// Get shortest non valid set 
struct TileSet_s* GetShortestNonValidSet(struct TileSet_s* tileset)
{
    int min_number = INT_MAX;
    struct TileSet_s* min_set = NULL;
    struct TileSet_s* cursor = tileset;
    while(cursor)
    {
        if(!isValidSet(cursor) && cursor->number < min_number)
        {
            min_number = cursor->number;
            min_set = cursor;
        }
    
        cursor = cursor->next_set;
    }
    return min_set;
}

struct Tile_s** wichTilesCanAddOnStart(struct TileSet_s* illegal_tileset, struct TileSet_s* tileset)
{

    // Store the tiles pointer in a array
    struct Tile_s** set_to_add = calloc(10, sizeof(struct Tile_s*));
    int idx = 0;
    struct Tile_s* tile_cursor = tileset->tiles;
    struct TileSet_s* tileset_cursor = tileset;
    if(!tile_cursor)
        return set_to_add;
    // if the first tile of the illegal set is superior to 1 we can complete it with 2, 3, 4...
    if((illegal_tileset->tiles->number > 1) )
    {
        // Search a tile for completion
        while(tileset_cursor)
        {
            tile_cursor = tileset_cursor->tiles;
            while(tile_cursor)
            {
                if((tile_cursor->number == illegal_tileset->tiles->number - 1) && (tile_cursor->color == illegal_tileset->tiles->color))
                    set_to_add[idx++] = tile_cursor;
                tile_cursor = tile_cursor->next_tile;
            }
            tileset_cursor = tileset_cursor->next_set;
        }
    }

    if(illegal_tileset->number < 4)
    {
        bool red, green, blue, yellow;
        struct Tile_s* cursor = illegal_tileset->tiles;
        while(cursor)
        {
            switch(cursor->color)
            {
                case 'R':
                    red = true;
                    break;
                case 'G':
                    green = true;
                    break;
                case 'B':
                    blue = true;
                    break;
                case 'Y':
                    yellow = true;
                    break;
            }
            cursor = cursor->next_tile;
        }

        tile_cursor = tileset->tiles;
        tileset_cursor = tileset;
    
        // Search a tile for completion
        while(tileset_cursor)
        {
            tile_cursor = tileset_cursor->tiles;
            while(tile_cursor)
            {
                if(!red && (tile_cursor->number == illegal_tileset->tiles->number) && (tile_cursor->color == 'R'))
                    set_to_add[idx++] = tile_cursor;
                if(!green && (tile_cursor->number == illegal_tileset->tiles->number) && (tile_cursor->color == 'G'))
                    set_to_add[idx++] = tile_cursor;
                if(!blue && (tile_cursor->number == illegal_tileset->tiles->number) && (tile_cursor->color == 'B'))
                    set_to_add[idx++] = tile_cursor;
                if(!yellow && (tile_cursor->number == illegal_tileset->tiles->number) && (tile_cursor->color == 'Y'))
                    set_to_add[idx++] = tile_cursor;
                tile_cursor = tile_cursor->next_tile;
            }
            tileset_cursor = tileset_cursor->next_set;
        }
    }
 
    return set_to_add;
}

struct Tile_s** wichTilesCanAddOnEnd(struct TileSet_s* illegal_tileset, struct TileSet_s* tileset)
{
    // Store the tiles pointer in a array
    struct Tile_s** set_to_add = calloc(10, sizeof(struct Tile_s*));
    int idx = 0;
    struct Tile_s* tile_cursor = tileset->tiles;
    struct TileSet_s* tileset_cursor = tileset;
    if(!tile_cursor)
        return set_to_add;
    // if the first tile of the illegal set is superior to 1 we can complete it with 2, 3, 4...
    if(illegal_tileset->last_tile->number < 13)
    {
        // Search a tile for completion
        while(tileset_cursor)
        {
            tile_cursor = tileset_cursor->tiles;
            while(tile_cursor)
            {
                if((tile_cursor->number == illegal_tileset->tiles->number + 1) && (tile_cursor->color == illegal_tileset->tiles->color))
                    set_to_add[idx++] = tile_cursor;
                tile_cursor = tile_cursor->next_tile;
            }
            tileset_cursor = tileset_cursor->next_set;
        }
    }
     if(illegal_tileset->number < 4)
    {
        bool red, green, blue, yellow;
        struct Tile_s* cursor = illegal_tileset->tiles;
        while(cursor)
        {
            switch(cursor->color)
            {
                case 'R':
                    red = true;
                    break;
                case 'G':
                    green = true;
                    break;
                case 'B':
                    blue = true;
                    break;
                case 'Y':
                    yellow = true;
                    break;
            }
            cursor = cursor->next_tile;
        }

        tile_cursor = tileset->tiles;
        tileset_cursor = tileset;
    
        // Search a tile for completion
        while(tileset_cursor)
        {
            tile_cursor = tileset_cursor->tiles;
            while(tile_cursor)
            {
                if(!red && (tile_cursor->number == illegal_tileset->tiles->number) && (tile_cursor->color == 'R'))
                    set_to_add[idx++] = tile_cursor;
                if(!green && (tile_cursor->number == illegal_tileset->tiles->number) && (tile_cursor->color == 'G'))
                    set_to_add[idx++] = tile_cursor;
                if(!blue && (tile_cursor->number == illegal_tileset->tiles->number) && (tile_cursor->color == 'B'))
                    set_to_add[idx++] = tile_cursor;
                if(!yellow && (tile_cursor->number == illegal_tileset->tiles->number) && (tile_cursor->color == 'Y'))
                    set_to_add[idx++] = tile_cursor;
                tile_cursor = tile_cursor->next_tile;
            }
            tileset_cursor = tileset_cursor->next_set;
        }
    }
    
    return set_to_add;
}

// Remove first tile from tileset
struct Tile_s* RemoveFirstTileFromTileSet(struct TileSet_s* tileset)
{
    struct Tile_s* first_tile = tileset->tiles;
    if(tileset->tiles->next_tile)
        tileset->tiles->next_tile->previous_tile = NULL;
    tileset->tiles = tileset->tiles->next_tile;

    tileset->number--;
    return first_tile;
}

// Remove last tile from tileset
struct Tile_s* RemoveLastTileFromTileSet(struct TileSet_s* tileset)
{
    struct Tile_s* last_tile = tileset->last_tile;
    if(tileset->last_tile->previous_tile)
        tileset->last_tile->previous_tile->next_tile = NULL;
    tileset->last_tile = tileset->last_tile->previous_tile;

    tileset->number--;
    return last_tile;
}

struct TileSet_s* ResolvedTileset(struct TileSet_s* tileset, int depth)
{
    // The open priority queue of the A* algorithm
    struct PriorityQueue_s* queue = CreatePriorityQueue(tileset, 0);
    // While the priority queu is not empty
    while(queue)
    {
        // Pop the first element of the queue, wich is one table tile set after certain moves
        struct PriorityQueue_s* best = PopFromPriorityQueue(&queue);
        //printf("Poped from queue :\n");
        //PrintTileSets(best->tile_set);
        // If the best is a valid set
        if(areValidSets(best->tile_set))
            return best->tile_set;
        // If the search limit was exceed
        if(depth > 10)
            return NULL;
        // Get the shortest non valid set from the table
        struct TileSet_s* table_sets = CopyTileSets(best->tile_set);
        struct TileSet_s* illegal_set = GetShortestNonValidSet(table_sets);
        // Try to concatenate each tile from the illegal tile set to every other set
        struct Tile_s* tile = illegal_set->tiles;
        struct TileSet_s* concatenated_set = NULL;
        // Take each tile from the illegal set and try to concatenate it with every other set
        while(tile)
        {
            // Get the table sets without the tile that we are trying to add
            //table_sets = CopyTileSetsWithoutTile(best->tile_set, tile);
            struct TileSet_s* cursor = table_sets;
            while(cursor)
            {
                if(cursor == illegal_set)
                {
                    cursor = cursor->next_set;
                    continue;
                }
                // Remove the tile from the illegal set
                PopTileFromTileSet(&tile->tile_set->tiles, tile);
                // Try to put the tile at the beggining of one of the sets
                AddTileToTileSet(&cursor, tile);
                // If the created set is semi legal add it to the priority queue
                if(isPartialSet(cursor) || isValidSet(cursor))
                {
                    // Put the tile at the start of the set
                    AddToPriorityQueue(&queue, CreatePriorityQueue(table_sets, depth + 1));             
                    PrintTileSets(table_sets);
                }           
                // Remove the tile from the set
                PopTileFromTileSet(&tile->tile_set->tiles, tile);
                // Add the tile back to the illegal set
                AddTileToTileSet(&illegal_set, tile);

                // Remove the tile from the illegal set
                PopTileFromTileSet(&tile->tile_set->tiles, tile);
                // Try to put the tile at the end of one of the sets
                PutTileAtEndOfTileSet(cursor, tile);
                // If the created set is semi legal add it to the priority queue
                if(isPartialSet(cursor) || isValidSet(cursor))
                 {
                    // Put the tile at the start of the set
                    AddToPriorityQueue(&queue, CreatePriorityQueue(table_sets, depth + 1));             
                    PrintTileSets(table_sets);
                }  
                // Remove the tile from the set
                PopTileFromTileSet(&tile->tile_set->tiles, tile);
                // Add the tile back to the illegal set
                AddTileToTileSet(&illegal_set, tile);

                cursor = cursor->next_set;
            }
            tile = tile->next_tile;
        }

        // Check wich tiles can be added to the illegal set
        table_sets = CopyTileSets(best->tile_set);
        illegal_set = GetShortestNonValidSet(table_sets);
        struct Tile_s* cursor = NULL;

        // Get tiles in table sets that can be added to the illegal set
        struct Tile_s** set_to_add_left = wichTilesCanAddOnStart(illegal_set, table_sets);
        

        int idx = 0;
        while(set_to_add_left[idx])
        {
            // Remove the tile from where it was and add it at the start of the illegal set
            PopTileFromTileSet(&set_to_add_left[idx]->tile_set->tiles, set_to_add_left[idx]);
            struct TileSet_s* previous_tileset = set_to_add_left[idx]->tile_set;
            AddTileToTileSet(&illegal_set, set_to_add_left[idx]);

            // Add the new table states to the priority queue
            AddToPriorityQueue(&queue, CreatePriorityQueue(table_sets, depth + 1));
            PrintTileSets(table_sets);
            // Put back the tile in the table sets
            PopTileFromTileSet(&set_to_add_left[idx]->tile_set->tiles, set_to_add_left[idx]);
            AddTileToSortedTileSet(&previous_tileset, set_to_add_left[idx]);
            
            idx++;
        }
        FreeTileSets(table_sets);
        free(set_to_add_left);
        
        table_sets = CopyTileSets(best->tile_set);
        illegal_set = GetShortestNonValidSet(table_sets);
        struct Tile_s** set_to_add_right = wichTilesCanAddOnEnd(illegal_set, table_sets);
        idx = 0;
        while(set_to_add_right[idx])
        {
            // Remove the tile from where it was and add it at the start of the illegal set
            PopTileFromTileSet(&set_to_add_right[idx]->tile_set->tiles, set_to_add_right[idx]);
            struct TileSet_s* previous_tileset = set_to_add_right[idx]->tile_set;
            PutTileAtEndOfTileSet(illegal_set, set_to_add_right[idx]);
            // Add the new table states to the priority queue
            AddToPriorityQueue(&queue, CreatePriorityQueue(table_sets, depth + 1));
            PrintTileSets(table_sets);
              // Put back the tile in the table sets
            PopTileFromTileSet(&set_to_add_right[idx]->tile_set->tiles, set_to_add_right[idx]);
            AddTileToSortedTileSet(&previous_tileset, set_to_add_right[idx]);
            idx++;;
        }
        FreeTileSets(table_sets);
        free(set_to_add_right);
    } 
    printf("END OF RESOLVED TILESET\n");
}

void AStar(struct TileSet_s* player_tileset, struct TileSet_s* table_tileset)
{
    struct Tile_s* player_tile = player_tileset->tiles;
    struct TileSet_s* copy_table_tileset = CopyTileSets(table_tileset);
    AddTileSetToTileSet(&copy_table_tileset, CreateTilesSet());
    struct TileSet_s* resolved_set;
    // Try to add every tile on the rack to the table
    while(player_tile)
    {
        AddTileToTileSet(&copy_table_tileset, player_tile);
        resolved_set = ResolvedTileset(copy_table_tileset, 0);
        if(resolved_set)
        {
            printf("Solution Found :\n");
            PrintTileSets(resolved_set);
        }
        player_tile = player_tile->next_tile;
    }
}

int main(int argc, char** argv) {
    srand(time(NULL));
    MainLoop();
}



// Return a tile allocated on the heap with number and color field
struct Tile_s* CreateTile(int number, char color)
{
    struct Tile_s* tile = malloc(sizeof(struct Tile_s));
    *tile = (struct Tile_s){number, color, NULL, NULL, NULL};
    return tile;
}

// Create tile set with 0 tiles, size 0 and next set NULL
struct TileSet_s* CreateTilesSet()
{
    struct TileSet_s* tileset = malloc(sizeof(struct TileSet_s));
    *tileset = (struct TileSet_s){NULL, NULL, NULL, NULL, NULL, 0};
    return tileset;
}

void AddTileSetToTileSetQueu(struct TileSet_s** tileset, struct TileSet_s* next_tileset)
{
      if(!next_tileset)
        return;
    if(!*tileset)
    {
        *tileset = next_tileset;
        next_tileset->first_set = *tileset;
        return;
    }

    struct TileSet_s* cursor = *tileset;
    while(cursor->next_set)
    {
        cursor = cursor->next_set;
    }
    cursor->next_set = next_tileset;
}

// Add tile set to tile set next field
void AddTileSetToTileSet(struct TileSet_s** tileset, struct TileSet_s* next_tileset)
{
    if(!next_tileset)
        return;
    if(!*tileset)
    {
        *tileset = next_tileset;
        next_tileset->first_set = *tileset;
        return;
    }
    (*tileset)->previous_set = next_tileset;
    next_tileset->next_set = *tileset;
    *tileset = next_tileset;
    next_tileset->first_set = *tileset;
}

void AddTileToTileSet(struct TileSet_s** tileset, struct Tile_s* tile)
{
    if(!*tileset)
        return;
    if(!(*tileset)->tiles)
    {
        tile->previous_tile = NULL;
        (*tileset)->tiles = tile;
        (*tileset)->number++;
        (*tileset)->last_tile = tile;
        tile->tile_set = *tileset;
        return;
    }

    (*tileset)->tiles->previous_tile = tile;
    tile->next_tile = (*tileset)->tiles;
    (*tileset)->tiles = tile;
    (*tileset)->number++;
    tile->tile_set = *tileset;

}

struct TileSet_s* CreateInitialTilesSet()
{
    struct TileSet_s* tileset = CreateTilesSet();
    
    // Create Red tiles and copy
    for (int i = 0, j = 1; i < 13 * 2; i++, j++)
    {
        AddTileToTileSet(&tileset, CreateTile(j, 'R'));
       j %= 13;
    }
    // Create Blue tiles and copy
    for (int i = 13 * 2, j = 1; i < (13 * 2 * 2); i++, j++)
    {
        AddTileToTileSet(&tileset, CreateTile(j, 'B'));
        j %= 13;
    }

    // Create Green tiles and copy
    for (int i = 13 * 2 * 2, j = 1; i < (13 * 2 * 3); i++, j++)
    {
        AddTileToTileSet(&tileset, CreateTile(j, 'G'));
        j %= 13;
    }

    
    // Create Yellow tiles and copy
    for (int i = 13 * 2 * 3, j = 1; i < (13 * 2 * 4); i++, j++)
    {
        AddTileToTileSet(&tileset, CreateTile(j, 'Y'));
        j %= 13;
    }
    
    return tileset;
}

// Delete a tile set from sets list
void DeleteTileSetFromSets(struct TileSet_s** tilesets, struct TileSet_s* tileset)
{
     /* base case */
    if (*tilesets == NULL || tileset == NULL) 
        return; 
  
    /* If node to be deleted is head node */
    struct TileSet_s* next_set = tileset->next_set;
    if (*tilesets == tileset) 
        *tilesets = next_set;
    /* Change next only if node to be 
    deleted is NOT the last node */
    if (tileset->next_set != NULL) 
        tileset->next_set->previous_set = tileset->previous_set; 
  
    /* Change prev only if node to be 
    deleted is NOT the first node */
    if (tileset->previous_set != NULL) 
        tileset->previous_set->next_set = tileset->next_set; 

}


void PopTileFromTileSet(struct Tile_s** tile_head, struct Tile_s* tile) 
{ 
    /* base case */
    if (*tile_head == NULL || tile == NULL) 
        return; 
  
    /* If node to be deleted is head node */
    if (*tile_head == tile) 
        *tile_head = tile->next_tile; 
  
    /* Change next only if node to be 
    deleted is NOT the last node */
    if (tile->next_tile != NULL) 
        tile->next_tile->previous_tile = tile->previous_tile; 
  
    /* Change prev only if node to be 
    deleted is NOT the first node */
    if (tile->previous_tile != NULL) 
        tile->previous_tile->next_tile = tile->next_tile; 
  
    tile->previous_tile = NULL;
    tile->next_tile = NULL;
    tile->tile_set->number--;

    if(!tile->tile_set->number)
        DeleteTileSetFromSets(&(tile->tile_set->first_set), tile->tile_set);
 } 

struct Tile_s* PickTileFromSet(struct TileSet_s* tileset)
{
    int index = rand()%(tileset->number);

    // Pick the tile
    struct Tile_s* cursor = tileset->tiles;
    struct Tile_s* picked_tile;
    // If first element
    if(index == 0)
    {
        picked_tile = cursor;
        tileset->tiles = cursor->next_tile;
        tileset->number--;
        picked_tile->next_tile = NULL;
        return picked_tile;
    }
    // If last element
    else if(index == tileset->number - 1)
    {
        for(int i = 0; i < index - 1; i++)
        {
            cursor = cursor->next_tile;
        }
        picked_tile = cursor->next_tile;
        cursor->next_tile = NULL;
        tileset->number--;
        picked_tile->next_tile = NULL;
        return picked_tile;
    }
    // If in the middle
    else
    {
        for(int i = 0; i < index - 1; i++)
        {
            cursor = cursor->next_tile;
        }
        picked_tile = cursor->next_tile;
        cursor->next_tile = cursor->next_tile->next_tile;
        tileset->number--;
        picked_tile->next_tile = NULL;
        return picked_tile;
    }

}

struct TileSet_s* CreatePlayerTileSet(struct TileSet_s* tileset)
{
    struct TileSet_s* player_tileset = CreateTilesSet();

    for (int i = 0; i < 13; i++)
    {
        AddTileToTileSet(&player_tileset, PickTileFromSet(tileset));
    }
    
    return player_tileset;
}

// Split tile set in tiles sets of same color
struct TileSet_s** SplitTileSetByColor(struct TileSet_s* tileset)
{
    struct TileSet_s** tilesets = malloc(sizeof(struct TileSet_s*) * 4);
    for (int i = 0; i < 4; i++)
    {
        tilesets[i] =  CreateTilesSet();

    }
    struct Tile_s* cursor = tileset->tiles;
    while(cursor)
    {
        switch(cursor->color)
        {
            case 'R':
                AddTileToSortedTileSet(&tilesets[0], CreateTile(cursor->number, cursor->color));
                break;
            case 'B':
                AddTileToSortedTileSet(&tilesets[1], CreateTile(cursor->number, cursor->color));
                break;
            case 'G':
                AddTileToSortedTileSet(&tilesets[2], CreateTile(cursor->number, cursor->color));
                break;
            case 'Y':
                AddTileToSortedTileSet(&tilesets[3], CreateTile(cursor->number, cursor->color));
                break;
        }
        cursor = cursor->next_tile;
    }
    return tilesets;
}

// Add tile to tile set in sorted order
void AddTileToSortedTileSet(struct TileSet_s** tileset, struct Tile_s* tile)
{
    /* Special case for the head end */
    if ((*tileset)->tiles == NULL || (*tileset)->tiles->number >= tile->number) {
        if(!(*tileset)->tiles)
            tile->previous_tile = NULL;
        else
            tile->previous_tile = (*tileset)->tiles;

        tile->next_tile = (*tileset)->tiles;
        (*tileset)->tiles = tile;
        (*tileset)->last_tile = tile;
        (*tileset)->number++;
    }
    else {
        struct Tile_s* cursor = (*tileset)->tiles;
        /* Locate the node before the point of insertion
         */
        while (cursor->next_tile != NULL
               && cursor->next_tile->number < tile->number) {
            cursor = cursor->next_tile;
        }
        tile->next_tile = cursor->next_tile;
        tile->previous_tile = cursor;
        cursor->next_tile = tile;
        (*tileset)->number++;
    }

    if(!tile->next_tile)
        (*tileset)->last_tile = tile;
    tile->tile_set = *tileset;
}

// Return if a tile set only contains adjacent number and same color tiles
bool IsAdjacentTileSet(struct TileSet_s* tileset)
{
    struct Tile_s* cursor = tileset->tiles;
    int previous_number = cursor->number;
    cursor = cursor->next_tile;
    while(cursor)
    {
        if((cursor->number != previous_number + 1) || (cursor->color != cursor->next_tile->color))
        {
            return false;
        }
        previous_number = cursor->number;
        cursor = cursor->next_tile;
    }
    return true;
}


void PrintTile(struct Tile_s* tile)
{
    if(!tile)
        return;
    switch (tile->color)
    {
    case 'R':
        printf("\033[0;31m");
        break;
    case 'B':
        printf("\033[0;34m");
        break;
    case 'G':
        printf("\033[0;32m");
        break;
    case 'Y':
        printf("\033[0;33m");
        break;
    default:
        break;
    }
    printf("%d ", tile->number);
    printf(" \033[0;37m");
}

void PrintTileSets(struct TileSet_s* tileset)
{
    while(tileset)
    {
        struct Tile_s* cursor = tileset->tiles;
        printf("| ");
        while(cursor)
        {
            PrintTile(cursor);
            cursor = cursor->next_tile;
        }
        printf(" |");

        tileset = tileset->next_set;
    }
    printf("\n");
    
}

void PrintTileSet(struct TileSet_s* tileset)
{
        struct Tile_s* cursor = tileset->tiles;
        printf("| ");
        for (int i = 0; i < tileset->number; i++, cursor = cursor->next_tile)
        {
            PrintTile(cursor);
        }
        printf(" |");
        tileset = tileset->next_set;
    
}

void PrintPlayerTileSet(struct TileSet_s* tileset)
{
    printf("Player's Tiles : ");
    PrintTileSets(tileset);
}


