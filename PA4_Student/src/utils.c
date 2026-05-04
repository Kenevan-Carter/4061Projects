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
    // Convert the character to an unsigned char
    unsigned char ch = (unsigned char)*c;
    // If the character is a lowercase letter
    if (ch >= 'a' && ch <= 'z') {
        int i = ch - 'a';
        // Shift the letter by the delta
        i = (i + delta) % 26;
        if (i < 0)
            i += 26;
        // Convert the letter back to a char
        *c = (char)('a' + i);
    // If the character is an uppercase letter
    } else if (ch >= 'A' && ch <= 'Z') {
        int i = ch - 'A';
        // Shift the letter by the delta
        i = (i + delta) % 26;
        // If the letter is less than 0 wrap around to the end of the alphabet
        if (i < 0)
            i += 26;
        // Convert the letter back to a char
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
