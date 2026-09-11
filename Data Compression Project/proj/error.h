/**
 * @file error.h
 * @author Avery Murray (aemurra3)
 * 
 * Describes the behaviors of the error.c file
 * to set and get error messages from invalid user input. 
 */

#ifndef ERROR_H
#define ERROR_H

#include <stdbool.h>

#endif

/**
 * Sets the global error message variable to the
 * provided constant char pointer. 
 * 
 * @param message A constant char pointer to the desired message
 * @return True if the message was successfully changed
 *         False if the message was not successfilly changed
 */
bool setErrorMessage(char const *message);

/**
 * Returns a reference to the current error message variable. 
 * 
 * @return A constant char pointer to the current error message
 *         NULL if there is no error message
 */
char const *getErrorMessage();