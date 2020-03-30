#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "execute.h"

#define PROGRAM_NAME "snrub"
#define PROGRAM_VERSION "v0.1.0"

static int run_script(char *path);
static char *read_file(char *path, size_t limit);
static void print_version();
static void print_usage();
static void print_error(execute_error_t error);
static void print_object(execute_object_t *object);
static int get_flag(int argc, char **argv, char *name);
static char *get_option(int argc, char **argv, char *name);

int main(int argc, char **argv)
{
    if (get_flag(argc, argv, "--version") || get_flag(argc, argv, "-v"))
    {
        print_version();
        return 0;
    }

    if (get_option(argc, argv, "--file"))
    {
        return run_script(get_option(argc, argv, "--file"));
    }

    if (get_option(argc, argv, "-f"))
    {
        return run_script(get_option(argc, argv, "-f"));
    }

    print_usage();
    return 0;
}

static int run_script(char *path)
{
    char *document;
    int limit, size;
    execute_store_t *store;

    printf("Checking %s for a script...\n", path);

    limit = 1024 * 4;
    document = read_file(path, limit);

    if (!document)
    {
        fprintf(stderr, "No script found.\n");
        return 1;
    }

    size = strlen(document);

    if (size >= limit - 1)
    {
        free(document);
        fprintf(stderr, "Script exceeds the size limit of %d bytes.\n", limit);
        return 1;
    }

    store = execute_do_document(document);

    if (store)
    {
        if (store->error != EXECUTE_ERROR_UNKNOWN)
        {
            print_error(store->error);
            execute_destroy_store(store);
            return 1;
        }
        else
        {
            execute_object_t *object;

            printf("[OBJECTS]\n");

            for (object = store->objects; object != NULL; object = object->next)
            {
                print_object(object);
            }

            execute_destroy_store(store);
        }
    }
    else
    {
        fprintf(stderr, "Ran out of memory.\n");
        return 1;
    }

    return 0;
}

static char *read_file(char *path, size_t limit)
{
    FILE *file;

    file = fopen(path, "r");

    if (file)
    {
        char *buffer;

        buffer = malloc(sizeof(char) * limit);

        if (buffer)
        {
            size_t index;

            index = 0;

            while (1)
            {
                int current;

                current = fgetc(file);

                if (current == EOF || index >= limit - 1)
                {
                    buffer[index] = '\0';
                    break;
                }
                else
                {
                    buffer[index] = (char) current;
                }

                index += 1;
            }
        }

        fclose(file);

        return buffer;
    }

    return NULL;
}

static void print_version()
{
    printf("%s\n", PROGRAM_VERSION);
}

static void print_usage()
{
    printf("Usage:\n");
    printf("  %s [options]\n", PROGRAM_NAME);
    printf("\n");
    printf("Options:\n");
    printf("  -v --version  Show version.\n");
    printf("  -f --file     Execute a script file.\n");
}

static void print_error(execute_error_t error)
{
    switch (error)
    {
        case EXECUTE_ERROR_SYNTAX:
            printf("A language syntax error was found!\n");
            break;
        case EXECUTE_ERROR_DEPTH:
            printf("An expression violated the depth limit!\n");
            break;
        case EXECUTE_ERROR_TYPE:
            printf("An invalid literal value was found!\n");
            break;
        case EXECUTE_ERROR_ARGUMENT:
            printf("An expression was found with the wrong arguments!\n");
            break;
        case EXECUTE_ERROR_ARITHMETIC:
            printf("An expression found an arithmetic error!\n");
            break;
        case EXECUTE_ERROR_UNSUPPORTED:
            printf("An expression found unsupported functionality!\n");
            break;
        default:
            break;
    }
}

static void print_object(execute_object_t *object)
{
    switch (object->type)
    {
        case EXECUTE_TYPE_NULL:
            printf("(NULL) %s\n", object->key);
            break;
        case EXECUTE_TYPE_NUMBER:
            printf("(NUMBER) %s %d\n", object->key, ((int *) object->unsafe)[0]);
            break;
        case EXECUTE_TYPE_STRING:
            printf("(STRING) %s %s\n", object->key, (char *) object->unsafe);
            break;
        default:
            break;
    }
}

static int get_flag(int argc, char **argv, char *name)
{
    int index;

    for (index = 0; index < argc; index++)
    {
        char *arg = argv[index];

        if (strcmp(arg, name) == 0)
        {
            return 1;
        }
    }

    return 0;
}

static char *get_option(int argc, char **argv, char *name)
{
    int index;

    for (index = 0; index < argc; index++)
    {
        char *arg = argv[index];

        if (strcmp(arg, name) == 0)
        {
            if (index + 1 < argc)
            {
                return argv[index + 1];
            }
        }
    }

    return NULL;
}
