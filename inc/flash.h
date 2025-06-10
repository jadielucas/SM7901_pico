#ifndef FLASH_H
#define FLASH_H

void init_filesystem();

void initialize_file_counter();

void save_payload_to_flash(const char *payload);

void resend_saved_data();

#endif