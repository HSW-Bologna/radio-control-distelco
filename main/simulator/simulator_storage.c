#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "model/model.h"
#include "simulator_storage.h"


#define STORAGE_FILE ".app.storage.bin"


void storage_save_model(model_t *model) {
    FILE *file = fopen(STORAGE_FILE, "w");

    if (!file) {
        printf("Errore nell'aprire il file %s: %s\n", STORAGE_FILE, strerror(errno));
        return;
    }

    if (fwrite(model, sizeof(model_t), 1, file) == 0) {
        printf("Errore nello scrivere il file %s: %s\n", STORAGE_FILE, strerror(errno));
    }

    fclose(file);
}


void storage_load_model(model_t *model) {
    FILE *file = fopen(STORAGE_FILE, "r");

    if (!file) {
        printf("Errore nell'aprire il file %s: %s\n", STORAGE_FILE, strerror(errno));
        return;
    }

    if (fread(model, sizeof(model_t), 1, file) == 0) {
        printf("Errore nel leggere il file %s: %s\n", STORAGE_FILE, strerror(errno));
    }

    fclose(file);
}