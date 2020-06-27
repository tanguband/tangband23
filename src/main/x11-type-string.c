﻿/*!
 * @brief X11用の文字列処理
 * @date 2020/06/14
 * @author Hourier
 * @details Windowsでは使わない
 */

#include "main/x11-type-string.h"
#include "term/gameterm.h"

/*
 * Add a series of keypresses to the "queue".
 *
 * Return any errors generated by Term_keypress() in doing so, or SUCCESS
 * if there are none.
 *
 * Catch the "out of space" error before anything is printed.
 *
 * NB: The keys added here will be interpreted by any macros or keymaps.
 */
errr type_string(concptr str, uint len)
{
    errr err = 0;
    term *old = Term;
    if (!str)
        return -1;
    if (!len)
        len = strlen(str);

    Term_activate(term_screen);
    for (concptr s = str; s < str + len; s++) {
        if (*s == '\0')
            break;

        err = Term_keypress(*s);
        if (err)
            break;
    }

    Term_activate(old);
    return err;
}