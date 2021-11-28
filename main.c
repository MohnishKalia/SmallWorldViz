#include "sqlite3.h"
#include <stdio.h>
#include <stdlib.h>

#define EH_SQL                                       \
    if (rc != SQLITE_OK)                             \
    {                                                \
        fprintf(stderr, "SQL error: %s\n", zErrMsg); \
        sqlite3_free(zErrMsg);                       \
    }

enum Type
{
    Monster = 0x1,
    Fusion = 0x40,
    Synchro = 0x2000,
    Xyz = 0x800000,
    Link = 0x4000000,
    ExtraDeck = Fusion | Synchro | Xyz | Link,
};

typedef struct
{
    char *id;
    char *name;
    char *race;
    char *attribute;
    char *level;
    char *atk;
    char *def;
} CardInfo;

void printCardInfo(FILE *sink, CardInfo *ci)
{
    fprintf(sink, "\n");
    fprintf(sink, "id = %s\n", ci->id);
    fprintf(sink, "name = %s\n", ci->name);
    fprintf(sink, "race = %s\n", ci->race);
    fprintf(sink, "attribute = %s\n", ci->attribute);
    fprintf(sink, "level = %s\n", ci->level);
    fprintf(sink, "atk = %s\n", ci->atk);
    fprintf(sink, "def = %s\n", ci->def);
    fprintf(sink, "\n");
}

int gi = 0;

static int callback(void *cis, int argc, char **argv, char **azColName)
{
    int type = atoi(argv[2]);
    if ((type & Monster) != 0 && (type & ExtraDeck) == 0)
    {
        CardInfo ci = {
            .id = argv[0],
            .name = argv[1],
            .race = argv[3],
            .attribute = argv[4],
            .level = argv[5],
            .atk = argv[6],
            .def = argv[7],
        };
        printCardInfo(stdout, &ci);
        CardInfo** arr = cis;
        arr[gi++] = &ci;
    }
    return 0;
}

/*
* Reveal 1 monster in your hand, choose 1 monster from your Deck that has exactly 1 of the same 
* Type, Attribute, Level, ATK or DEF, and banish the revealed monster from your hand face-down.
* Then add, from the Deck to your hand, 1 monster that has exactly 1 of the same 
* Type, Attribute, Level, ATK or DEF as the monster chosen from your Deck, and banish the card chosen from the Deck face-down.
* You can only activate 1 "Small World" per turn.
*/

int main(void)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open("/Applications/ProjectIgnis/repositories/delta-utopia/cards.cdb", &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return (1);
    }

    // char *len_stmt = "select count(*) from datas d inner join texts t on d.id = t.id";
    // sqlite3_stmt *ppStmt;
    // rc = sqlite3_prepare_v2(db, len_stmt, -1, &ppStmt, NULL);
    // EH_SQL
    // rc = sqlite3_step(ppStmt);
    // EH_SQL
    // int rowcount = sqlite3_column_int(ppStmt, 0);
    // EH_SQL
    // rc = sqlite3_finalize(ppStmt);
    // EH_SQL

    CardInfo cis[10631] = {0};
    char *data_stmt = "select d.id, name, type, race, attribute, level, atk, def from datas d inner join texts t on d.id = t.id";
    FILE *out = fopen("cards.txt", "w");
    FILE *old_out = stdout;
    stdout = out;
    rc = sqlite3_exec(db, data_stmt, callback, &cis, &zErrMsg);
    fclose(out);
    stdout = old_out;
    EH_SQL
    printf("Number of cards: %d\n", gi + 1);
    printf("Card 100:\n");
    printCardInfo(stdout, &(cis[100]));
    sqlite3_close(db);
    return 0;
}