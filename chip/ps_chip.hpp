/*
 * 
 * Using PocketSphinx decoder to control a Samsung TV via spoken words 
 * in both English and Brazilian Portuguese
 * 
 * Author: Feb 2018
 * Cassio Batista - cassio.batista.13@gmail.com
 * Federal University of Pará (UFPA). Belém, Brazil.
 * 
 * References:
 * https://sourceforge.net/p/cmusphinx/discussion/help/thread/5d178cf2/
 * https://cmusphinx.github.io/doc/pocketsphinx/ps__search_8h.html
 * https://cmusphinx.github.io/wiki/tutorialpocketsphinx/
 * https://cmusphinx.github.io/wiki/faq/
 * http://www.robotrebels.org/index.php?topic=239.0
 * https://stackoverflow.com/questions/29054341/stdlogic-error-basic-string-s-construct-null-not-valid
 * https://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c 
 * http://wiki.bash-hackers.org/scripting/terminalcodes
 *
 */

#ifndef _PS_CHIP_HPP_
#define _PS_CHIP_HPP_

#include <iostream>
#include <string>
#include <stdlib.h>
#include <signal.h>

#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/err.h>

#include "resources/simple_gpio.h"
#include "resources/samsung.h"

#define MODELDIR_PT_BR "../pt_br"
#define MODELDIR_EN_US "../en_us"

#define DEGUB false

/* foreground colors */
#define C_FG_K "\x1b[30m" /* black */
#define C_FG_R "\x1b[31m" /* red */
#define C_FG_G "\x1b[32m" /* green */
#define C_FG_Y "\x1b[33m" /* yellow */
#define C_FG_B "\x1b[34m" /* blue */
#define C_FG_M "\x1b[35m" /* magenta */
#define C_FG_C "\x1b[36m" /* cyan */
#define C_FG_W "\x1B[38m" /* white */

/* background colors */
#define C_BG_K "\x1b[40m" /* black */
#define C_BG_R "\x1b[41m" /* red */
#define C_BG_G "\x1b[42m" /* green */
#define C_BG_Y "\x1b[43m" /* yellow */
#define C_BG_B "\x1b[44m" /* blue */
#define C_BG_M "\x1b[45m" /* magenta */
#define C_BG_C "\x1b[46m" /* cyan */
#define C_BG_W "\x1B[47m" /* white */

#define C_RESET      "\x1b[0m"
#define C_BOLD       "\x1b[1m"
#define C_BLINK      "\x1b[5m"
#define C_REVERSE    "\x1b[7m"

void signal_handler(int dummy);
string ps_decode_from_mic(ps_decoder_t* ps, ad_rec_t* ad);

static bool volatile keep_running = true;

#endif /* _PS_CHIP_HPP_ */
