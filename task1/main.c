#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static void reverse_word(char *s) {
    size_t i = 0, j = s ? strlen(s) : 0;
    if (j == 0) return;
    j--;
    while (i < j) {
        char t = s[i];
        s[i++] = s[j];
        s[j--] = t;
    }
}

static void reverse_array(char **a, size_t n) {
    for (size_t i = 0, j = n ? n - 1 : 0; i < j; i++, j--) {
        char *t = a[i];
        a[i] = a[j];
        a[j] = t;
    }
}

static void process_and_print(const char *line, int n) {
    size_t len = strlen(line);
    char *buf = malloc(len + 1);
    if (!buf) return;
    memcpy(buf, line, len + 1);

    char *words[4096];
    size_t count = 0;
    char *p = buf;
    while (*p && count < 4096) {
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        words[count++] = p;
        while (*p && *p != ' ' && *p != '\t') p++;
        if (*p) { *p = '\0'; p++; }
    }

    if (n >= 0 && (size_t)n < count) reverse_word(words[n]);
    reverse_array(words, count);

    for (size_t i = 0; i < count; i++) {
        if (i) putchar(' ');
        fputs(words[i], stdout);
    }
    putchar('\n');

    free(buf);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s n\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);

    char *line = NULL;
    size_t cap = 0;
    ssize_t len;

    char **lines = NULL;
    size_t lcount = 0, lcap = 0;
    int done = 0;

    while (!done && (len = getline(&line, &cap, stdin)) != -1) {
        if (len > 0 && line[len - 1] == '\n') line[--len] = '\0';
        if (strcmp(line, "!!") == 0) { done = 1; break; }
        if (lcount == lcap) {
            lcap = lcap ? lcap * 2 : 16;
            lines = realloc(lines, lcap * sizeof(*lines));
        }
        lines[lcount++] = strdup(line);
    }

    for (size_t i = 0; i < lcount; i++) {
        process_and_print(lines[i], n);
        free(lines[i]);
    }
    free(lines);
    free(line);
    return 0;
}
