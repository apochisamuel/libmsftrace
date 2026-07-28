#include "msftrace.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s FILE.pdb\n", argv[0]);
        return 2;
    }
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) return 1;
    if (fseek(file, 0, SEEK_END) != 0) { fclose(file); return 1; }
    long length = ftell(file);
    if (length < 0 || fseek(file, 0, SEEK_SET) != 0) { fclose(file); return 1; }
    uint8_t *data = (uint8_t *)malloc((size_t)length);
    if (data == NULL) { fclose(file); return 1; }
    size_t read_count = fread(data, 1, (size_t)length, file);
    fclose(file);
    if (read_count != (size_t)length) { free(data); return 1; }
    mt_context *ctx = mt_context_create();
    mt_database *db = NULL;
    mt_status status = mt_database_open_memory(ctx, data, read_count, &db);
    if (status != MT_OK) {
        fprintf(stderr, "parse error: %s\n", mt_context_message(ctx));
        mt_context_destroy(ctx);
        free(data);
        return 1;
    }
    mt_summary summary;
    (void)mt_database_summary(db, &summary);
    printf("blocks: %u\nstreams: %u\nmodules: %u\ntypes: %u\nsymbols: %u\nfiles: %u\n",
           summary.block_count, summary.stream_count, summary.module_count,
           summary.type_count, summary.symbol_count, summary.source_file_count);
    mt_database_close(db);
    mt_context_destroy(ctx);
    free(data);
    return 0;
}
