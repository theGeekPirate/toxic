/*  windows.c
 *
 *
 *  Copyright (C) 2014 Toxic All Rights Reserved.
 *
 *  This file is part of Toxic.
 *
 *  Toxic is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Toxic is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Toxic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#include "friendlist.h"
#include "prompt.h"
#include "toxic.h"
#include "windows.h"
#include "groupchat.h"
#include "chat.h"
#include "line_info.h"

extern char *DATA_FILE;
extern struct _Winthread Winthread;
static ToxWindow windows[MAX_WINDOWS_NUM];
static ToxWindow *active_window;

extern ToxWindow *prompt;

static int num_active_windows;

/* CALLBACKS START */
void on_request(Tox *m, const uint8_t *public_key, const uint8_t *data, uint16_t length, void *userdata)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onFriendRequest != NULL)
            windows[i].onFriendRequest(&windows[i], m, public_key, data, length);
    }
}

void on_connectionchange(Tox *m, int32_t friendnumber, uint8_t status, void *userdata)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onConnectionChange != NULL)
            windows[i].onConnectionChange(&windows[i], m, friendnumber, status);
    }
}

void on_typing_change(Tox *m, int32_t friendnumber, uint8_t is_typing, void *userdata)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onTypingChange != NULL)
            windows[i].onTypingChange(&windows[i], m, friendnumber, is_typing);
    }
}

void on_message(Tox *m, int32_t friendnumber, uint8_t *string, uint16_t length, void *userdata)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onMessage != NULL)
            windows[i].onMessage(&windows[i], m, friendnumber, string, length);
    }
}

void on_action(Tox *m, int32_t friendnumber, uint8_t *string, uint16_t length, void *userdata)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onAction != NULL)
            windows[i].onAction(&windows[i], m, friendnumber, string, length);
    }
}

void on_nickchange(Tox *m, int32_t friendnumber, uint8_t *string, uint16_t length, void *userdata)
{
    if (friendnumber < 0 || friendnumber > MAX_FRIENDS_NUM)
        return;

    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onNickChange != NULL)
            windows[i].onNickChange(&windows[i], m, friendnumber, string, length);
    }

    if (store_data(m, DATA_FILE))
        wprintw(prompt->window, "\nCould not store Tox data\n");
}

void on_statusmessagechange(Tox *m, int32_t friendnumber, uint8_t *string, uint16_t length, void *userdata)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onStatusMessageChange != NULL)
            windows[i].onStatusMessageChange(&windows[i], friendnumber, string, length);
    }
}

void on_statuschange(Tox *m, int32_t friendnumber, uint8_t status, void *userdata)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onStatusChange != NULL)
            windows[i].onStatusChange(&windows[i], m, friendnumber, status);
    }
}

void on_friendadded(Tox *m, int32_t friendnumber, bool sort)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onFriendAdded != NULL)
            windows[i].onFriendAdded(&windows[i], m, friendnumber, sort);
    }

    if (store_data(m, DATA_FILE))
        wprintw(prompt->window, "\nCould not store Tox data\n");
}

void on_groupmessage(Tox *m, int groupnumber, int peernumber, uint8_t *message, uint16_t length,
                     void *userdata)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onGroupMessage != NULL)
            windows[i].onGroupMessage(&windows[i], m, groupnumber, peernumber, message, length);
    }
}

void on_groupaction(Tox *m, int groupnumber, int peernumber, uint8_t *action, uint16_t length,
                    void *userdata)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onGroupAction != NULL)
            windows[i].onGroupAction(&windows[i], m, groupnumber, peernumber, action, length);
    }
}

void on_groupinvite(Tox *m, int32_t friendnumber, uint8_t *group_pub_key, void *userdata)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onGroupInvite != NULL)
            windows[i].onGroupInvite(&windows[i], m, friendnumber, group_pub_key);
    }
}

void on_group_namelistchange(Tox *m, int groupnumber, int peernumber, uint8_t change, void *userdata)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onGroupNamelistChange != NULL)
            windows[i].onGroupNamelistChange(&windows[i], m, groupnumber, peernumber, change);
    }
}

void on_file_sendrequest(Tox *m, int32_t friendnumber, uint8_t filenumber, uint64_t filesize,
                         uint8_t *filename, uint16_t filename_length, void *userdata)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onFileSendRequest != NULL)
            windows[i].onFileSendRequest(&windows[i], m, friendnumber, filenumber, filesize,
                                         filename, filename_length);
    }
}

void on_file_control (Tox *m, int32_t friendnumber, uint8_t receive_send, uint8_t filenumber,
                      uint8_t control_type, uint8_t *data, uint16_t length, void *userdata)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onFileControl != NULL)
            windows[i].onFileControl(&windows[i], m, friendnumber, receive_send, filenumber,
                                     control_type, data, length);
    }
}

void on_file_data(Tox *m, int32_t friendnumber, uint8_t filenumber, uint8_t *data, uint16_t length,
                  void *userdata)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].onFileData != NULL)
            windows[i].onFileData(&windows[i], m, friendnumber, filenumber, data, length);
    }
}

/* CALLBACKS END */

int add_window(Tox *m, ToxWindow w)
{
    if (LINES < 2)
        return -1;

    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; i++) {
        if (windows[i].active)
            continue;

        w.window = newwin(LINES - 2, COLS, 0, 0);

        if (w.window == NULL)
            return -1;

#ifdef URXVT_FIX
        /* Fixes text color problem on some terminals. */
        wbkgd(w.window, COLOR_PAIR(6));
#endif
        windows[i] = w;

        if (w.onInit)
            w.onInit(&w, m);

        ++num_active_windows;

        return i;
    }

    return -1;
}

/* Deletes window w and cleans up */
void del_window(ToxWindow *w)
{
    active_window = windows; /* Go to prompt screen */

    delwin(w->window);
    memset(w, 0, sizeof(ToxWindow));

    clear();
    refresh();
    --num_active_windows;
}

/* Shows next window when tab or back-tab is pressed */
void set_next_window(int ch)
{
    ToxWindow *end = windows + MAX_WINDOWS_NUM - 1;
    ToxWindow *inf = active_window;

    while (true) {
        if (ch == T_KEY_NEXT) {
            if (++active_window > end)
                active_window = windows;
        } else if (--active_window < windows)
            active_window = end;

        if (active_window->window)
            return;

        if (active_window == inf)    /* infinite loop check */
            exit_toxic_err("failed in set_next_window", FATALERR_INFLOOP);
    }
}

void set_active_window(int index)
{
    if (index < 0 || index >= MAX_WINDOWS_NUM)
        return;

    active_window = windows + index;
}

ToxWindow *init_windows(Tox *m)
{
    int n_prompt = add_window(m, new_prompt());

    if (n_prompt == -1 || add_window(m, new_friendlist()) == -1)
        exit_toxic_err("failed in init_windows", FATALERR_WININIT);

    prompt = &windows[n_prompt];
    active_window = prompt;

    return prompt;
}

void on_window_resize(int sig)
{
    refresh();
    clear();
}

static void draw_window_tab(ToxWindow toxwin)
{
    /* alert0 takes priority */
    if (toxwin.alert0)
        attron(COLOR_PAIR(GREEN));
    else if (toxwin.alert1)
        attron(COLOR_PAIR(RED));
    else if (toxwin.alert2)
        attron(COLOR_PAIR(MAGENTA));

    clrtoeol();
    printw(" [%s]", toxwin.name);

    if (toxwin.alert0)
        attroff(COLOR_PAIR(GREEN));
    else if (toxwin.alert1)
        attroff(COLOR_PAIR(RED));
    else if (toxwin.alert2)
        attroff(COLOR_PAIR(MAGENTA));
}

static void draw_bar(void)
{
    attron(COLOR_PAIR(BLUE));
    mvhline(LINES - 2, 0, '_', COLS);
    attroff(COLOR_PAIR(BLUE));

    move(LINES - 1, 0);

    attron(COLOR_PAIR(BLUE) | A_BOLD);
    printw(" TOXIC " TOXICVER " |");
    attroff(COLOR_PAIR(BLUE) | A_BOLD);

    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (!windows[i].active)
            continue;

        if (windows + i == active_window)

#ifdef URXVT_FIX
            attron(A_BOLD | COLOR_PAIR(GREEN));
        else
#endif

            attron(A_BOLD);

        draw_window_tab(windows[i]);

        if (windows + i == active_window)

#ifdef URXVT_FIX
            attroff(A_BOLD | COLOR_PAIR(GREEN));
        else
#endif

            attroff(A_BOLD);
    }

    refresh();
}

void draw_active_window(Tox *m)
{
    ToxWindow *a = active_window;
    a->alert0 = false;
    a->alert1 = false;
    a->alert2 = false;

    wint_t ch = 0;

    draw_bar();

    touchwin(a->window);
#ifndef WIN32
    wresize(a->window, LINES - 2, COLS);
#endif

    a->onDraw(a, m);
    wrefresh(a->window);

    /* Handle input */
    bool ltr;
#ifdef HAVE_WIDECHAR
    int status = wget_wch(stdscr, &ch);

    if (status == ERR)
        return;

    if (status == OK)
        ltr = iswprint(ch);
    else /* if (status == KEY_CODE_YES) */
        ltr = false;

#else
    ch = getch();

    if (ch == ERR)
        return;

    /* TODO verify if this works */
    ltr = isprint(ch);
#endif

    if (!ltr && (ch == T_KEY_NEXT || ch == T_KEY_PREV)) {
        set_next_window((int) ch);
    } else {
        pthread_mutex_lock(&Winthread.lock);
        a->onKey(a, m, ch, ltr);
        pthread_mutex_unlock(&Winthread.lock);
    }
}

/* refresh inactive windows to prevent scrolling bugs. 
   call at least once per second */
void refresh_inactive_windows(void)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        ToxWindow *a = &windows[i];

        if (a->active && a != active_window && (a->is_chat || a->is_groupchat))
            line_info_print(a);
    }
}

int get_num_active_windows(void)
{
    return num_active_windows;
}

/* destroys all chat and groupchat windows (should only be called on shutdown) */
void kill_all_windows(void)
{
    int i;

    for (i = 0; i < MAX_WINDOWS_NUM; ++i) {
        if (windows[i].is_chat)
            kill_chat_window(&windows[i]);
        else if (windows[i].is_groupchat)
            kill_groupchat_window(&windows[i]);
    }
}
