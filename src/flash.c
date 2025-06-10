#include "hardware/flash.h"
#include "lfs.h"
#include "pico_lfs.h"
#include "inc/flash.h"
#include "pico/stdlib.h"
#include "inc/mqtt.h"

#define LFS_STORAGE_SIZE (64 * 1024) // 64KB for LittleFS storage
#define LFS_STORAGE_OFFSET (PICO_FLASH_SIZE_BYTES - LFS_STORAGE_SIZE) // Offset for LittleFS storage (Last 64KB of flash)

static lfs_t lfs; // LittleFS instance
static struct lfs_config *lfs_cfg_ptr = NULL; // Pointer to the LittleFS configuration
static int file_counter = 0; // Counter for the number of files saved

/**
 * @brief Initialize the LittleFS filesystem.
 * This function sets up the filesystem, mounts it, and initializes the file counter.
 * If the filesystem is not mounted successfully, it formats the storage and retries mounting.
 */

void init_filesystem()
{
    
    //littlefs config file
    lfs_cfg_ptr = pico_lfs_init(LFS_STORAGE_OFFSET, LFS_STORAGE_SIZE);

    if (!lfs_cfg_ptr)
    {
        printf("Erro ao inicializar a configuração do LFS.\n");
        while (1);
    }

    // Get the context from the config
    struct pico_lfs_context *ctx = (struct pico_lfs_context*)lfs_cfg_ptr;

    // Disable multicore lockout for this context
    // This is necessary for the filesystem to work correctly in a single-core environment
    ctx->multicore_lockout_enabled = false;

    // Try to mount the filesystem
    int err = lfs_mount(&lfs, lfs_cfg_ptr);

    // If mounting fails, format the filesystem and try to mount again
    if (err)
    {
        printf("Falha ao montar LFS, formatando... (erro: %d)\n", err);
        int format_err = lfs_format(&lfs, lfs_cfg_ptr);
        if (format_err)
        {
            printf("Erro ao formatar LFS: %d\n", format_err);
            while (1)
                ;
        }
        err = lfs_mount(&lfs, lfs_cfg_ptr);
    }

    printf("Sistema de arquivos LittleFS montado com sucesso!\n");

    // Initialize the file counter
    // This will scan the filesystem for existing files and set the counter accordingly
    initialize_file_counter();
}

/**
 * @brief Initialize the file counter by scanning the filesystem for existing files.
 * This function looks for files named "data_X.json" and sets the file_counter to the next available number.
 */

void initialize_file_counter()
{
    lfs_dir_t dir; // Directory handle for scanning files
    struct lfs_info info; // File information structure
    int max_num = -1; // Variable to keep track of the maximum file number found

    lfs_dir_open(&lfs, &dir, "/"); // Open the root directory of the filesystem

    // Read through the directory entries
    while (lfs_dir_read(&lfs, &dir, &info) > 0)
    {
        int num = -1;

        if (sscanf(info.name, "data_%d.json", &num) == 1)
        {
            if (num > max_num)
            {
                max_num = num;
            }
        }
    }

    lfs_dir_close(&lfs, &dir); // Close the directory handle

    // Set the file counter to the next available number
    file_counter = max_num + 1;

    if (file_counter < 0)
    {
        file_counter = 0; // Ensure the counter starts at 0 if no files are found
    }

    printf("Contador de arquivos inicializado em: %d\n", file_counter);
}

/**
 * @brief Save the payload to a file in the LittleFS filesystem.
 * This function creates a new file with a unique name based on the file_counter,
 * writes the payload to it, and increments the file_counter.
 * If the filesystem is not mounted, it will print an error message.
 *
 * @param payload The data to be saved in JSON format.
 */

void save_payload_to_flash(const char *payload)
{
    char filename[32];
    snprintf(filename, sizeof(filename), "data_%d.json", file_counter++);

    printf("Conexão offline. Salvando dados em: %s\n", filename);

    lfs_file_t file;

    int err = lfs_file_open(&lfs, &file, filename, LFS_O_WRONLY | LFS_O_CREAT);
    if (err)
    {
        printf("Erro ao abrir arquivo %s para escrita.\n", filename);
        return;
    }

    if (lfs_file_write(&lfs, &file, payload, strlen(payload)) < 0)
    {
        printf("Erro ao escrever no arquivo %s.\n", filename);
    }

    lfs_file_close(&lfs, &file);
}

/**
 * @brief Resend saved data from the LittleFS filesystem.
 * This function checks for files in the filesystem, reads their contents,
 * and attempts to publish them via MQTT. If successful, it deletes the file.
 * If it encounters an error while publishing, it stops processing further files.
 */

void resend_saved_data() {
    printf("Verificando dados salvos para reenviar...\n");

    lfs_dir_t dir;
    struct lfs_info info;

    lfs_dir_open(&lfs, &dir, "/");

    char read_buffer[256];

    while (lfs_dir_read(&lfs, &dir, &info) > 0) {

        if (info.name[0] == '.') {
            continue;
        }

        printf("Reenviando arquivo: %s\n", info.name);

        lfs_file_t file;
        lfs_file_open(&lfs, &file, info.name, LFS_O_RDONLY);

        lfs_ssize_t size = lfs_file_read(&lfs, &file, read_buffer, sizeof(read_buffer) - 1);
        lfs_file_close(&lfs, &file);

        if (size > 0) {
            read_buffer[size] = '\0';

            err_t err = mqtt_publish(global_mqtt_client, MQTT_TOPIC, read_buffer, size, 1, 0, NULL, NULL);

            if (err == ERR_OK) {
                printf("Arquivo %s reenviado com sucesso. Deletando...\n", info.name);

                lfs_remove(&lfs, info.name);
            } else {
                printf("Falha ao reenviar %s. Interrompendo para tentar mais tarde.\n", info.name);

                break;
            }
        }
    }

    lfs_dir_close(&lfs, &dir);
    printf("Verificação de dados salvos concluída.\n");
}