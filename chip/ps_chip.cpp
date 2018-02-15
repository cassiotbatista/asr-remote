/*
 * 
 * Using PocketSphinx decoder to control a TV via spoken words 
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
 *
 */

#include <iostream>
#include <string>
#include <stdlib.h>
#include <signal.h>

#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/err.h>

#include "bluetooth.h"
#include "bluetooth.c"

#include "simple_gpio.h"
#include "simple_gpio.c"

#define MODELDIR_PT_BR "../pt_br"
#define MODELDIR_EN_US "../en_us"

#define DEGUB false

#define C_FG_K "\x1b[30m"
#define C_FG_R "\x1b[31m"
#define C_FG_G "\x1b[32m"
#define C_FG_Y "\x1b[33m"
#define C_FG_B "\x1b[34m"
#define C_FG_M "\x1b[35m"
#define C_FG_C "\x1b[36m"
#define C_FG_W "\x1B[38m"

#define C_BG_K "\x1b[40m"
#define C_BG_R "\x1b[41m"
#define C_BG_G "\x1b[42m"
#define C_BG_Y "\x1b[43m"
#define C_BG_B "\x1b[44m"
#define C_BG_M "\x1b[45m"
#define C_BG_C "\x1b[46m"
#define C_BG_W "\x1B[47m"

#define C_RESET      "\x1b[0m"
#define C_BOLD       "\x1b[1m"
#define C_BLINK      "\x1b[5m"
#define C_REVERSE    "\x1b[7m"

using namespace std;

static bool volatile keep_running = true;

void signal_handler(int dummy) {
	keep_running = false;
}

string ps_decode_from_mic(ps_decoder_t* ps, ad_rec_t* ad);

int
main(int argc, char *argv[]) 
{
	/* create audio rec (ALSA) struct */
	ad_rec_t     *ad;

	/* create ps decoder and config structs for PT_BR */
	ps_decoder_t *ps_PT_BR;
	cmd_ln_t     *config_PT_BR;

	/* create ps decoder and config structs for EN_US */
	ps_decoder_t *ps_EN_US;
	cmd_ln_t     *config_EN_US;

	int PT_BR = 1;
	signal(SIGINT, signal_handler);

	/* load configuration structure */
	#if DEGUB
		cerr << "[PT_BR]\tAlocando recursos" << endl;
	#endif
	config_PT_BR = cmd_ln_init(NULL, ps_args(), TRUE,         // ps_args() passes the default values
			"-hmm",  MODELDIR_PT_BR "/res",                   // acoustic model dir
			"-dict", MODELDIR_PT_BR "/dic_sphinx_pt_BR.dict", // phonetic dictionaty (lexicon)
			"-logfn", "/dev/null",                            // suppress log info from being sent to screen
			NULL);

	#if DEGUB
		cerr << "[EN_US]\tAllocating resources" << endl;
	#endif
	config_EN_US = cmd_ln_init(NULL, ps_args(), TRUE,         // ps_args() passes the default values
			"-hmm",  MODELDIR_EN_US "/res",                   // acoustic model dir
			"-dict", MODELDIR_EN_US "/dic_sphinx_en_US.dict", // phonetic dictionaty (lexicon)
			"-logfn", "/dev/null",                            // suppress log info from being sent to screen
			NULL);

	/* init ps decoder */
	#if DEGUB
		cerr << "[PS]\tInitializing PocketSphinx decoder for both languages" << endl;
	#endif
	ps_PT_BR = ps_init(config_PT_BR);
	ps_EN_US = ps_init(config_EN_US);

	/* open default mic at default sample rate */
	#if DEGUB
		cerr << "[ADMIC]\tOpening mic device" << endl;
	#endif
	ad = ad_open_dev("default", (int) cmd_ln_float32_r(config_PT_BR, "-samprate"));

	/* set keyword to spot */
	#if DEGUB
		cerr << "[PS]\tSetting keywords (keyphrases) to spot in both languages" << endl;
	#endif
	ps_set_keyphrase(ps_PT_BR, "kws_PT_BR", "acordar sistema");
	ps_set_keyphrase(ps_EN_US, "kws_EN_US", "wake up system");

	/* set FSG grammar from JSGF file */
	#if DEGUB
		cerr << "[PT_BR]\tDefinindo gramática [de estados finitos] livre-de-contexto (autômato)" << endl;
	#endif
	ps_set_jsgf_file(ps_PT_BR, "jsgf_PT_BR", MODELDIR_PT_BR "/gram_pt_BR.jsgf");
	ps_set_fsg(ps_PT_BR, "fsg_PT_BR", ps_get_fsg(ps_PT_BR, "jsgf_PT_BR"));

	#if DEGUB
		cerr << "[EN_US]\tDefining context-free [finite state] grammar (automata)" << endl;
	#endif
	ps_set_jsgf_file(ps_EN_US, "jsgf_EN_US", MODELDIR_EN_US "/gram_en_US.jsgf");
	ps_set_fsg(ps_EN_US, "fsg_EN_US", ps_get_fsg(ps_EN_US, "jsgf_EN_US"));

	/* export: tell kernel I'm gonna use GPIO pins */
	#if DEBUG
		cerr << "[GPIO]\texporting" << endl;
	#endif
	gpio_export(LED_RED);
	gpio_export(LED_AMBER);
	gpio_export(LED_GREEN);

	/* direction: define them as output pins */
	#if DEBUG
		cerr << "[GPIO]\tset direction as output" << endl;
	#endif
	gpio_set_dir(LED_RED,   OUTPUT_PIN);
	gpio_set_dir(LED_AMBER, OUTPUT_PIN);
	gpio_set_dir(LED_GREEN, OUTPUT_PIN);

	/* make sure all LEDs start off */
	#if DEBUG
		cerr << "[GPIO]\tstart all LEDs off" << endl;
	#endif
	gpio_set_value(LED_RED,   HIGH);
	gpio_set_value(LED_AMBER, HIGH);
	gpio_set_value(LED_GREEN, HIGH);

	string sent;
	while(keep_running) {

		sent = "";

		#if DEBUG
			cerr << "turning red LED on" << endl;
		#endif

		/* Turn red light on */
		gpio_set_value(LED_RED, LOW);
		gpio_set_value(LED_AMBER, HIGH);
		gpio_set_value(LED_GREEN, HIGH);

		/* switch to keyword spotting mode */
		#if DEGUB
			cerr << "[PS]\tSwitching to keyword spotting mode";
		#endif
		ps_set_search(ps_PT_BR, "kws_PT_BR");
		ps_set_search(ps_EN_US, "kws_EN_US");

		do {
			if(PT_BR) {
				cerr << C_BG_G << C_FG_W << endl << "Por favor, fale a palavra-chave (\"acordar sistema\"):";
				sent = ps_decode_from_mic(ps_PT_BR, ad);
				cerr << C_RESET << C_FG_Y ;
			} else {
				cerr << C_BG_R << C_FG_W << endl << "Please, speak the keyword (\"wake up system\"):";
				sent = ps_decode_from_mic(ps_EN_US, ad);
				cerr << C_RESET << C_FG_C << "\t";
			}
		} while(sent == "" && keep_running);
		cerr << "\t" << sent << C_RESET;

		/* handling */
		if(!keep_running)
			break;

		/* switch to grammar mode */
		#if DEGUB
			cerr << endl << "[PS]\tSwitching to grammar mode";
		#endif
		ps_set_search(ps_PT_BR, "fsg_PT_BR");
		ps_set_search(ps_EN_US, "fsg_EN_US");

		do {
			if(PT_BR) {
				cerr << endl << C_FG_G << "Agora, fale o comando de controle: ";
				sent = ps_decode_from_mic(ps_PT_BR, ad);
				if(sent != "") {
					if(sent[0] == 'm') {      // [m]udar pro inglês
						PT_BR = 0;
					} else if(sent[0]=='l') { // [l]igar televisão
						
					} else if(sent[1]=='e') { // d[e]sligar televisão
						
					} else if(sent[0]=='a') { // [a]umentar volume
						
					} else if(sent[1]=='i') { // d[i]minuir volume
					
					} else if(sent[0]=='p') { // [p]róximo canal
						
					} else if(sent[0]=='c') { // [c]anal anterior
					
					} else { /* default */
						/* sentença desconhecida */
					}
				}
			} else {
				cerr << endl << C_FG_R << "Now, speak the command of control: ";
				sent = ps_decode_from_mic(ps_EN_US, ad);
				if(sent != "") {
					if(sent[0] == 's') {                      // [s]witch to portuguese
						PT_BR = 1;
					} else if(sent[0]=='t' && sent[9]=='n') { // [t]urn tv o[n]
						
					} else if(sent[0]=='t' && sent[9]=='f') { // [t]urn tv o[f]f
						
					} else if(sent[0]=='i') {                 // [i]ncrease volume
						
					} else if(sent[0]=='d') {                 // [d]ecrease volume
						
					} else if(sent[0]=='n') {                 // [n]ext channel
						
					} else if(sent[0]=='p') {                 // [p]revious channel
						
					} else { /* default */
						/* unknown sentence */
					}
				}
			}
		} while(sent == "" && keep_running);
		cerr << C_RESET << "\t" << sent << endl;
	} // whilc keep running

	/* close mic */
	#if DEGUB
		cerr << "[ADMIC]\tClosing mic device" << endl;
	#endif
	ad_close(ad); 

	/* free resources PT_BR */
	#if DEGUB
		cerr << "[PT_BR]\tLiberando recursos de gramática" << endl;
	#endif
	ps_unset_search(ps_PT_BR, "kws_PT_BR");
	ps_unset_search(ps_PT_BR, "fsg_PT_BR");

	/* free resources EN_US */
	#if DEGUB
		cerr << "[EN_US]\tFreeing grammar resources" << endl;
	#endif
	ps_unset_search(ps_EN_US, "kws_EN_US");
	ps_unset_search(ps_EN_US, "fsg_EN_US");

	/* Turn all LEDs off */
	#if DEBUG
		cerr << "[GPIO]\tturn all LEDs off" << endl;
	#endif
	gpio_set_value(LED_RED,   HIGH);
	gpio_set_value(LED_AMBER, HIGH);
	gpio_set_value(LED_GREEN, HIGH);

	/* free GPIO pins */
	#if DEBUG
		cerr << "[GPIO]\tunexport/free pins" << endl;
	#endif
	gpio_unexport(LED_RED);
	gpio_unexport(LED_AMBER);
	gpio_unexport(LED_GREEN);

	/* cleaning up: destroy PT_BR structures */
	#if DEGUB
		cerr << "[PT_BR]\tDestruindo estruturas de dados do PocketSphinx" << endl;
	#endif
	ps_free(ps_PT_BR);
	cmd_ln_free_r(config_PT_BR);

	/* cleaning up: destroy EN_US structures */
	#if DEGUB
		cerr << "[PT_BR]\tDestroying PocketSphinx data structures" << endl;
	#endif
	ps_free(ps_EN_US);
	cmd_ln_free_r(config_EN_US);

	return 0;
}

string
ps_decode_from_mic(ps_decoder_t* ps, ad_rec_t* ad)
{
	ad_start_rec(ad); // start recording
	ps_start_utt(ps); // mark the start of the utterance

	int16 adbuf[4096]; // buffer array to hold audio data
	int32 k;           // holds the number of frames in the audio buffer

	uint8 utt_started; // track active speech: has speech started? 
	uint8 in_speech;   // track active speech: is speech currently happening? 

	char const *hyp;   // pointer to "hypothesis" (best guess at the decoded result)

	utt_started = FALSE;                             // clear the utt_started flag
	for(;;) {
		k = ad_read(ad, adbuf, 4096);                // capture the number of frames in the audio buffer
		ps_process_raw(ps, adbuf, k, FALSE, FALSE);  // send the audio buffer to the pocketsphinx decoder

		in_speech = ps_get_in_speech(ps);            // test to see if speech is being detected

		if (in_speech && !utt_started) {             // if speech has started and utt_started flag is false
			utt_started = TRUE;                      // then set the flag
		}

		if (!in_speech && utt_started) {             // if speech has ended and the utt_started flag is true 
			ps_end_utt(ps);                          // then mark the end of the utterance
			ad_stop_rec(ad);                         // stop recording

			hyp = ps_get_hyp(ps, NULL);              // query pocketsphinx for "hypothesis" of decoded statement
			if(hyp != 0)
				return hyp;                          // the function returns the hypothesis

			break;                                   // exit the while loop and return to main
		}
	}

	return "";
}

/*** EOF ***/
