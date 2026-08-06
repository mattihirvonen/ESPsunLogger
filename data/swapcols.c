//
// Pile: swapcols.c
//
// Dummy one time usage AI generated C application
// to swap numerical text data column order.
// Compatible with Gnuplot & Octave & Matlab data.
// Original code slightly modified by MH.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LEN 1024
#define MAX_COLS 100


// Function to detect comment line (by MH)
int is_comment_line( const char *str )
{
    if ( *str == '%' ) {
        return 1;
    }
    if ( *str == '#' ) {
        return 1;
    }
    return 0;
}

// Function to check if a string is numeric (integer or float)
int is_numeric(const char *str) {
    if (!str || *str == '\0') return 0;
    char *endptr;
    strtod(str, &endptr);
    return (*endptr == '\0'); // True if entire string is a number
}

// Swap two columns in an array of strings
void swap_columns(char *cols[], int col1, int col2, int total_cols) {
    if (col1 >= total_cols || col2 >= total_cols) {
        fprintf(stderr, "Error: Column index out of range.\n");
        exit(EXIT_FAILURE);
    }
    char *tmp = cols[col1];
    cols[col1] = cols[col2];
    cols[col2] = tmp;
}

int main( int argc, char *argv[] ) {
    char input_filename[256], output_filename[256];
    int col1, col2;
/*
    printf("Enter input file name: ");
    if (scanf("%255s", input_filename) != 1) {
        fprintf(stderr, "Invalid input.\n");
        return EXIT_FAILURE;
    }

    printf("Enter output file name: ");
    if (scanf("%255s", output_filename) != 1) {
        fprintf(stderr, "Invalid input.\n");
        return EXIT_FAILURE;
    }

    printf("Enter first column index to swap (0-based): ");
    if (scanf("%d", &col1) != 1 || col1 < 0) {
        fprintf(stderr, "Invalid column index.\n");
        return EXIT_FAILURE;
    }

    printf("Enter second column index to swap (0-based): ");
    if (scanf("%d", &col2) != 1 || col2 < 0) {
        fprintf(stderr, "Invalid column index.\n");
        return EXIT_FAILURE;
    }
*/
    // Replace AI generated code with taking arguments fron command line
    // User see column numbers: 1, 2, ... N
    //
    col1 = atoi( argv[1] ) - 1;
    col2 = atoi( argv[2] ) - 1;
    strcpy(  input_filename, argv[3] );
    strcpy( output_filename, argv[4] );


    FILE *fin = fopen(input_filename, "r");
    if (!fin) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }

    FILE *fout = fopen(output_filename, "w");
    if (!fout) {
        perror("Error opening output file");
        fclose(fin);
        return EXIT_FAILURE;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), fin)) {
        char *columns[MAX_COLS];
        int   col_count = 0;

        // Skip comment lines
        if ( is_comment_line(line) ) {
            fprintf(fout, "%s", line);
            continue;
        }

        // Tokenize line by whitespace
        char *token = strtok(line, ", \t\n");

        while (token && col_count < MAX_COLS) {
            columns[col_count++] = token;
            token = strtok(NULL, " \t\n");
        }

        if (col_count > 0) {
            // Only swap if both columns are numeric
            if (col1 < col_count && col2 < col_count &&
                is_numeric(columns[col1]) && is_numeric(columns[col2])) {
                swap_columns(columns, col1, col2, col_count);
            }

            // Output the row with tab separation
            fprintf(fout, " %s\t", columns[0]);
            for (int i = 1; i < col_count; i++) {
                fprintf(fout, "%s", columns[i]);
                if (i < col_count - 1) fprintf(fout, "\t");
            }
            fprintf(fout, "\n");
        }
    }

    fclose(fin);
    fclose(fout);

    printf("Column swap completed (numeric columns only). Output saved to '%s'.\n", output_filename);
    return EXIT_SUCCESS;
}
