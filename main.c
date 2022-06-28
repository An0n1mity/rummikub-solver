#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>


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
    struct Tile_s* next_tile;
};

// A Set of tiles constitued of an array of tiles and its size
struct TileSet_s{
    struct Tile_s* tiles;
    struct TileSet_s* next_set;
    int number;
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
        AddTileToSortedTileSet(&new_tileset, CreateTile(cursor->number, cursor->color));
        cursor = cursor->next_tile;
    }
    return new_tileset;
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
            if(prev)
                prev->next_tile = cursor->next_tile;
            else
                tileset->tiles = cursor->next_tile;
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
    if(!tileset)
        return false;
    if(tileset->number < 3)
        return false;
    struct Tile_s* cursor = tileset->tiles;
    int previous_number = cursor->number;
    cursor = cursor->next_tile;
    while(cursor)
    {
        if((cursor->number != previous_number + 1) || (cursor->color != cursor->next_tile->color))
            return false;
        previous_number = cursor->number;
        cursor = cursor->next_tile;
    }
    return true;
}

// Check if the given tile set is a group
bool IsGroup(struct TileSet_s* tileset)
{
    if(!tileset)
        return false;
    if(tileset->number < 3)
        return false;
    struct Tile_s* cursor = tileset->tiles;
    int previous_number = cursor->number;
    cursor = cursor->next_tile;
    while(cursor)
    {
        if(cursor->number != previous_number)
            return false;
        previous_number = cursor->number;
        cursor = cursor->next_tile;
    }
    return true;
}

// Check if a given tile set is a valid set (IsRun or IsGroup)
bool IsValidSet(struct TileSet_s* tileset)
{
    if(!tileset)
        return false;
    if(tileset->number < 3)
        return false;
    if(IsRun(tileset))
        return true;
    if(IsGroup(tileset))
        return true;
    return false;
}


// Return a tile set from a given string : "1R 2R 3R"
struct TileSet_s* GetTileSetFromString(char* string)
{
    struct TileSet_s* tileset = CreateTilesSet();
    char* cursor = string;
    while(*cursor)
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
            return NULL;

        AddTileToTileSet(&tileset, CreateTile(number, color));
        cursor++;
        if(!*cursor)
            return tileset;
        cursor++;
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
            PrintTileSet(player_tileset);
        }
        if(table_tileset)
        {
            printf("Table tile set :\n");
            PrintTileSet(table_tileset);
        }
    }
    printf("Exiting\n");
}

int main(int argc, char** argv) {
    srand(time(NULL));
    MainLoop();
}



// Return a tile allocated on the heap with number and color field
struct Tile_s* CreateTile(int number, char color)
{
    struct Tile_s* tile = malloc(sizeof(struct Tile_s));
    *tile = (struct Tile_s){number, color, NULL};
    return tile;
}

// Create tile set with 0 tiles, size 0 and next set NULL
struct TileSet_s* CreateTilesSet()
{
    struct TileSet_s* tileset = malloc(sizeof(struct TileSet_s));
    *tileset = (struct TileSet_s){NULL, 0, NULL};
    return tileset;
}

void AddTileSetToTileSetQueu(struct TileSet_s** tileset, struct TileSet_s* next_tileset)
{
      if(!next_tileset)
        return;
    if(!*tileset)
    {
        *tileset = next_tileset;
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
        return;
    }
    next_tileset->next_set = *tileset;
    *tileset = next_tileset;
}

void AddTileToTileSet(struct TileSet_s** tileset, struct Tile_s* tile)
{
    if(!(*tileset)->tiles)
    {
        (*tileset)->tiles = tile;
        (*tileset)->number++;
        return;
    }

    tile->next_tile = (*tileset)->tiles;
    (*tileset)->tiles = tile;
    (*tileset)->number++;

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
        tile->next_tile = (*tileset)->tiles;
        (*tileset)->tiles = tile;
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
        cursor->next_tile = tile;
        (*tileset)->number++;
    }
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
        for (int i = 0; i < tileset->number; i++, cursor = cursor->next_tile)
        {
            PrintTile(cursor);
        }
        printf("\n");
        tileset = tileset->next_set;
    }
    
}

void PrintTileSet(struct TileSet_s* tileset)
{
        struct Tile_s* cursor = tileset->tiles;
        for (int i = 0; i < tileset->number; i++, cursor = cursor->next_tile)
        {
            PrintTile(cursor);
        }
        printf("\n");
        tileset = tileset->next_set;
    
}

void PrintPlayerTileSet(struct TileSet_s* tileset)
{
    printf("Player's Tiles : ");
    PrintTileSets(tileset);
}


