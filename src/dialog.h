/*
 * dialog.h
 *
 * Common dialog for error messages and cheats input.
 *
 * Copyright (C) 2021 Andy Nguyen
 * Copyright (C) 2021 fgsfds
 * Copyright (C) 2022 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef VML_DIALOG_H
#define VML_DIALOG_H

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((unused)) int init_ime_dialog(const char *title,
                                            const char *initial_text);

__attribute__((unused)) char *get_ime_dialog_result(void);

int init_msg_dialog(const char *msg);

int get_msg_dialog_result(void);

void fatal_error(const char *fmt, ...) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif // SOLOADER_DIALOG_H