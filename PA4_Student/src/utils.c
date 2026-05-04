#include "utils.h"

void _removeOutputDir()
{
    pid_t pid = fork();
    if(pid == 0)
    {
        char *argv[] = {"rm", "-rf", "output", NULL};
        if (execvp(*argv, argv) < 0)
        {
            printf("ERROR: exec failed\n");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else{
        wait(NULL);
    }
}

void _createOutputDir()
{
    mkdir("output", ACCESSPERMS);
}

void bookeepingCode()
{
    _removeOutputDir();
    sleep(1);
    _createOutputDir();
}

// --- BONUS: Caesar shift-right-by-2 cipher ---
// Used to lightly obfuscate the query sent by ENC_SEARCH_ITEM.
// Shifts letters only; non-letters are passthrough.
// Wraps at the end of the alphabet: 'y'->'a', 'z'->'b', 'Y'->'A', 'Z'->'B'.
static void shift_letter(char *c, int delta)
{
    unsigned char ch = (unsigned char)*c;
    if (ch >= 'a' && ch <= 'z') {
        int i = ch - 'a';
        i = (i + delta) % 26;
        if (i < 0)
            i += 26;
        *c = (char)('a' + i);
    } else if (ch >= 'A' && ch <= 'Z') {
        int i = ch - 'A';
        i = (i + delta) % 26;
        if (i < 0)
            i += 26;
        *c = (char)('A' + i);
    }
}

// Encrypts a string by shifting each letter right by 2.
void encrypt_str(char *s)
{
    for (; *s; s++)
        shift_letter(s, 2);
}

// Decrypts a string by shifting each letter left by 2.
void decrypt_str(char *s)
{
    for (; *s; s++)
        shift_letter(s, -2);
}
