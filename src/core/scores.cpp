/*!
 * @file scores.c
 * @brief ハイスコア処理 / Highscores handling
 * @date 2014/07/14
 * @author
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 * 2014 Deskull rearranged comment for Doxygen.
 */

#include "core/scores.h"
#include "cmd-io/cmd-dump.h"
#include "core/asking-player.h"
#include "core/score-util.h"
#include "core/turn-compensator.h"
#include "game-option/birth-options.h"
#include "game-option/game-play-options.h"
#include "io/files-util.h"
#include "io/input-key-acceptor.h"
#include "io/report.h"
#include "io/signal-handlers.h"
#include "io/uid-checker.h"
#include "locale/japanese.h"
#include "player-base/player-race.h"
#include "player-info/class-info.h"
#include "player/player-personality.h"
#include "player/player-status.h"
#include "player/race-info-table.h"
#include "system/angband-version.h"
#include "system/player-type-definition.h"
#include "term/screen-processor.h"
#include "term/term-color-types.h"
#include "util/angband-files.h"
#include "util/enum-converter.h"
#include "util/int-char-converter.h"
#include "util/string-processor.h"
#include "view/display-messages.h"
#include "view/display-scores.h"
#include "world/world.h"

/*!
 * @brief 所定ポインタへスコア情報を書き込む / Write one score to the highscore file
 * @param score スコア情報参照ポインタ
 * @return エラーコード(問題がなければ0を返す)
 */
static int highscore_write(high_score *score)
{
    /* Write the record, note failure */
    return fd_write(highscore_fd, (char *)(score), sizeof(high_score));
}

/*!
 * @brief スコア情報を全て得るまで繰り返し取得する / Just determine where a new score *would* be placed
 * @param score スコア情報参照ポインタ
 * @return 正常ならば(MAX_HISCORES - 1)、問題があれば-1を返す
 */
static int highscore_where(high_score *score)
{
    /* Paranoia -- it may not have opened */
    if (highscore_fd < 0) {
        return -1;
    }

    /* Go to the start of the highscore file */
    if (highscore_seek(0)) {
        return -1;
    }

    /* Read until we get to a higher score */
    high_score the_score;
    int my_score = atoi(score->pts);
    for (int i = 0; i < MAX_HISCORES; i++) {
        int old_score;
        if (highscore_read(&the_score)) {
            return i;
        }
        old_score = atoi(the_score.pts);
        if (my_score > old_score) {
            return i;
        }
    }

    /* The "last" entry is always usable */
    return MAX_HISCORES - 1;
}

/*!
 * @brief スコア情報をバッファの末尾に追加する / Actually place an entry into the high score file
 * @param score スコア情報参照ポインタ
 * @return 正常ならば書き込んだスロット位置、問題があれば-1を返す / Return the location (0 is best) or -1 on "failure"
 */
static int highscore_add(high_score *score)
{
    /* Paranoia -- it may not have opened */
    if (highscore_fd < 0) {
        return -1;
    }

    /* Determine where the score should go */
    int slot = highscore_where(score);

    /* Hack -- Not on the list */
    if (slot < 0) {
        return -1;
    }

    /* Hack -- prepare to dump the new score */
    high_score the_score = (*score);

    /* Slide all the scores down one */
    bool done = false;
    high_score tmpscore;
    for (int i = slot; !done && (i < MAX_HISCORES); i++) {
        /* Read the old guy, note errors */
        if (highscore_seek(i)) {
            return -1;
        }
        if (highscore_read(&tmpscore)) {
            done = true;
        }

        /* Back up and dump the score we were holding */
        if (highscore_seek(i)) {
            return -1;
        }
        if (highscore_write(&the_score)) {
            return -1;
        }

        /* Hack -- Save the old score, for the next pass */
        the_score = tmpscore;
    }

    /* Return location used */
    return slot;
}

/*!
 * @brief スコアサーバへの転送処理
 * @param player_ptr プレイヤーへの参照ポインタ
 * @param do_send 実際に転送ア処置を行うか否か
 * @return 転送が成功したらTRUEを返す
 */
bool send_world_score(PlayerType *player_ptr, bool do_send)
{
#ifdef WORLD_SCORE
    if (!send_score || !do_send) {
        return true;
    }

    auto is_registration = input_check_strict(
        player_ptr, _("スコアをスコア・サーバに登録しますか? ", "Do you send score to the world score server? "), { UserCheck::NO_ESCAPE, UserCheck::NO_HISTORY });
    if (!is_registration) {
        return true;
    }

    prt("", 0, 0);
    prt(_("送信中．．", "Sending..."), 0, 0);
    term_fresh();
    screen_save();
    auto successful_send = report_score(player_ptr);
    screen_load();
    if (!successful_send) {
        return false;
    }

    prt(_("完了。何かキーを押してください。", "Completed.  Hit any key."), 0, 0);
    (void)inkey();
#else
    (void)player_ptr;
    (void)do_send;
#endif
    return true;
}

/*!
 * @brief スコアの過去二十位内ランキングを表示する
 * Enters a players name on a hi-score table, if "legal", and in any
 * case, displays some relevant portion of the high score list.
 * @param player_ptr プレイヤーへの参照ポインタ
 * @return エラーコード
 * @details
 * Assumes "signals_ignore_tstp()" has been called.
 */
errr top_twenty(PlayerType *player_ptr)
{
    high_score the_score = {};
    snprintf(the_score.what, sizeof(the_score.what), "%u.%u.%u", H_VER_MAJOR, H_VER_MINOR, H_VER_PATCH);
    snprintf(the_score.pts, sizeof(the_score.pts), "%9ld", (long)calc_score(player_ptr));
    the_score.pts[9] = '\0';

    snprintf(the_score.gold, sizeof(the_score.gold), "%9lu", (long)player_ptr->au);
    the_score.gold[9] = '\0';

    snprintf(the_score.turns, sizeof(the_score.turns), "%9lu", (long)turn_real(player_ptr, w_ptr->game_turn));
    the_score.turns[9] = '\0';

    auto ct = time((time_t *)0);

    strftime(the_score.day, 10, "@%Y%m%d", localtime(&ct));
    the_score.copy_info(*player_ptr);
    if (player_ptr->died_from.size() >= sizeof(the_score.how)) {
#ifdef JP
        angband_strcpy(the_score.how, player_ptr->died_from, sizeof(the_score.how) - 2);
        angband_strcat(the_score.how, "…", sizeof(the_score.how));
#else
        angband_strcpy(the_score.how, player_ptr->died_from, sizeof(the_score.how) - 3);
        angband_strcat(the_score.how, "...", sizeof(the_score.how));
#endif
    } else {
        angband_strcpy(the_score.how, player_ptr->died_from, sizeof(the_score.how));
    }

    safe_setuid_grab();
    auto err = fd_lock(highscore_fd, F_WRLCK);
    safe_setuid_drop();
    if (err) {
        return 1;
    }

    auto j = highscore_add(&the_score);
    safe_setuid_grab();
    err = fd_lock(highscore_fd, F_UNLCK);
    safe_setuid_drop();
    if (err) {
        return 1;
    }

    if (j < 10) {
        display_scores(0, 15, j, nullptr);
        return 0;
    }

    display_scores(0, 5, j, nullptr);
    display_scores(j - 2, j + 7, j, nullptr);
    return 0;
}

/*!
 * @brief プレイヤーの現在のスコアをランキングに挟む /
 * Predict the players location, and display it.
 * @return エラーコード
 */
errr predict_score(PlayerType *player_ptr)
{
    high_score the_score;
    if (highscore_fd < 0) {
        msg_print(_("スコア・ファイルが使用できません。", "Score file unavailable."));
        msg_print(nullptr);
        return 0;
    }

    snprintf(the_score.what, sizeof(the_score.what), "%u.%u.%u", H_VER_MAJOR, H_VER_MINOR, H_VER_PATCH);
    snprintf(the_score.pts, sizeof(the_score.pts), "%9ld", (long)calc_score(player_ptr));
    snprintf(the_score.gold, sizeof(the_score.gold), "%9lu", (long)player_ptr->au);
    snprintf(the_score.turns, sizeof(the_score.turns), "%9lu", (long)turn_real(player_ptr, w_ptr->game_turn));
    angband_strcpy(the_score.day, _("今日", "TODAY"), sizeof(the_score.day));
    the_score.copy_info(*player_ptr);
    strcpy(the_score.how, _("yet", "nobody (yet!)"));
    auto j = highscore_where(&the_score);
    if (j < 10) {
        display_scores(0, 15, j, &the_score);
        return 0;
    }

    display_scores(0, 5, -1, nullptr);
    display_scores(j - 2, j + 7, j, &the_score);
    return 0;
}

/*!
 * @brief スコアランキングの簡易表示 /
 * show_highclass - selectively list highscores based on class -KMW-
 */
void show_highclass(PlayerType *player_ptr)
{
    screen_save();
    const auto &path = path_build(ANGBAND_DIR_APEX, "scores.raw");
    highscore_fd = fd_open(path, O_RDONLY);
    if (highscore_fd < 0) {
        msg_print(_("スコア・ファイルが使用できません。", "Score file unavailable."));
        msg_print(nullptr);
        return;
    }

    if (highscore_seek(0)) {
        return;
    }

    high_score the_score;
    for (int i = 0; i < MAX_HISCORES; i++) {
        if (highscore_read(&the_score)) {
            break;
        }
    }

    int m = 0;
    int j = 0;
    PLAYER_LEVEL clev = 0;
    int pr;
    char out_val[256];
    while ((m < 9) && (j < MAX_HISCORES)) {
        if (highscore_seek(j)) {
            break;
        }
        if (highscore_read(&the_score)) {
            break;
        }
        pr = atoi(the_score.p_r);
        clev = (PLAYER_LEVEL)atoi(the_score.cur_lev);

#ifdef JP
        snprintf(out_val, sizeof(out_val), "   %3d) %sの%s (レベル %2d)", (m + 1), race_info[pr].title, the_score.who, clev);
#else
        snprintf(out_val, sizeof(out_val), "%3d) %s the %s (Level %2d)", (m + 1), the_score.who, race_info[pr].title, clev);
#endif

        prt(out_val, (m + 7), 0);
        m++;
        j++;
    }

#ifdef JP
    snprintf(out_val, sizeof(out_val), "あなた) %sの%s (レベル %2d)", race_info[enum2i(player_ptr->prace)].title, player_ptr->name, player_ptr->lev);
#else
    snprintf(out_val, sizeof(out_val), "You) %s the %s (Level %2d)", player_ptr->name, race_info[enum2i(player_ptr->prace)].title, player_ptr->lev);
#endif

    prt(out_val, (m + 8), 0);

    (void)fd_close(highscore_fd);
    highscore_fd = -1;
    prt(_("何かキーを押すとゲームに戻ります", "Hit any key to continue"), 0, 0);

    (void)inkey();

    for (j = 5; j < 18; j++) {
        prt("", j, 0);
    }
    screen_load();
}

/*!
 * @brief スコアランキングの簡易表示(種族毎)サブルーチン /
 * Race Legends -KMW-
 * @param race_num 種族ID
 */
void race_score(PlayerType *player_ptr, int race_num)
{
    int i = 0, j, m = 0;
    int pr, clev;
    high_score the_score;
    auto lastlev = 0;

    /* rr9: TODO - pluralize the race */
    prt(std::string(_("最高の", "The Greatest of all the ")).append(race_info[race_num].title), 5, 15);
    const auto &path = path_build(ANGBAND_DIR_APEX, "scores.raw");
    highscore_fd = fd_open(path, O_RDONLY);
    if (highscore_fd < 0) {
        msg_print(_("スコア・ファイルが使用できません。", "Score file unavailable."));
        msg_print(nullptr);
        return;
    }

    if (highscore_seek(0)) {
        return;
    }

    for (i = 0; i < MAX_HISCORES; i++) {
        if (highscore_read(&the_score)) {
            break;
        }
    }

    m = 0;
    j = 0;

    while ((m < 10) || (j < MAX_HISCORES)) {
        if (highscore_seek(j)) {
            break;
        }
        if (highscore_read(&the_score)) {
            break;
        }
        pr = atoi(the_score.p_r);
        clev = atoi(the_score.cur_lev);

        if (pr == race_num) {
            char out_val[256];
#ifdef JP
            snprintf(out_val, sizeof(out_val), "   %3d) %sの%s (レベル %2d)", (m + 1), race_info[pr].title, the_score.who, clev);
#else
            snprintf(out_val, sizeof(out_val), "%3d) %s the %s (Level %3d)", (m + 1), the_score.who, race_info[pr].title, clev);
#endif

            prt(out_val, (m + 7), 0);
            m++;
            lastlev = clev;
        }
        j++;
    }

    /* add player if qualified */
    if ((enum2i(player_ptr->prace) == race_num) && (player_ptr->lev >= lastlev)) {
        char out_val[256];
#ifdef JP
        snprintf(out_val, sizeof(out_val), "あなた) %sの%s (レベル %2d)", race_info[enum2i(player_ptr->prace)].title, player_ptr->name, player_ptr->lev);
#else
        snprintf(out_val, sizeof(out_val), "You) %s the %s (Level %3d)", player_ptr->name, race_info[enum2i(player_ptr->prace)].title, player_ptr->lev);
#endif

        prt(out_val, (m + 8), 0);
    }

    (void)fd_close(highscore_fd);
    highscore_fd = -1;
}

/*!
 * @brief スコアランキングの簡易表示(種族毎)メインルーチン /
 * Race Legends -KMW-
 */
void race_legends(PlayerType *player_ptr)
{
    for (int i = 0; i < MAX_RACES; i++) {
        race_score(player_ptr, i);
        msg_print(_("何かキーを押すとゲームに戻ります", "Hit any key to continue"));
        msg_print(nullptr);
        for (int j = 5; j < 19; j++) {
            prt("", j, 0);
        }
    }
}

/*!
 * @brief スコアファイル出力
 * Display some character info
 */
bool check_score(PlayerType *player_ptr)
{
    term_clear();

    /* No score file */
    if (highscore_fd < 0) {
        msg_print(_("スコア・ファイルが使用できません。", "Score file unavailable."));
        msg_print(nullptr);
        return false;
    }

    /* Wizard-mode pre-empts scoring */
    if (w_ptr->noscore & 0x000F) {
        msg_print(_("ウィザード・モードではスコアが記録されません。", "Score not registered for wizards."));
        msg_print(nullptr);
        return false;
    }

    /* Cheaters are not scored */
    if (w_ptr->noscore & 0xFF00) {
        msg_print(_("詐欺をやった人はスコアが記録されません。", "Score not registered for cheaters."));
        msg_print(nullptr);
        return false;
    }

    /* Interupted */
    if (!w_ptr->total_winner && streq(player_ptr->died_from, _("強制終了", "Interrupting"))) {
        msg_print(_("強制終了のためスコアが記録されません。", "Score not registered due to interruption."));
        msg_print(nullptr);
        return false;
    }

    /* Quitter */
    if (!w_ptr->total_winner && streq(player_ptr->died_from, _("途中終了", "Quitting"))) {
        msg_print(_("途中終了のためスコアが記録されません。", "Score not registered due to quitting."));
        msg_print(nullptr);
        return false;
    }
    return true;
}
