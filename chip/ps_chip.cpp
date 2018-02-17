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
 *
 */

#include "ps_cpp.hpp"

using namespace std;

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
	#if DEGUB
		cerr << "[GPIO]\texporting" << endl;
	#endif
	gpio_export(LED_RED);
	gpio_export(LED_AMBER);
	gpio_export(LED_GREEN);

	/* direction: define them as output pins */
	#if DEGUB
		cerr << "[GPIO]\tset direction as output" << endl;
	#endif
	gpio_set_dir(LED_RED,   OUTPUT_PIN);
	gpio_set_dir(LED_AMBER, OUTPUT_PIN);
	gpio_set_dir(LED_GREEN, OUTPUT_PIN);

	/* make sure all LEDs start off */
	#if DEGUB
		cerr << "[GPIO]\tstart all LEDs off" << endl;
	#endif
	gpio_set_value(LED_RED,   HIGH);
	gpio_set_value(LED_AMBER, HIGH);
	gpio_set_value(LED_GREEN, HIGH);

	string sent;
	while(keep_running) {

		sent = "";

		#if DEGUB
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
				cerr << C_BG_G << C_FG_W << endl;
				cerr << "Por favor, fale a palavra-chave (\"acordar sistema\"):";
				sent = ps_decode_from_mic(ps_PT_BR, ad);
				cerr << C_RESET << C_FG_Y ;
			} else {
				cerr << C_BG_R << C_FG_W << endl;
				cerr << "Please, speak the keyword (\"wake up system\"):";
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
					if(sent[0] == 'm') {              /* [m]udar pro inglês */
						PT_BR = 0;
					} else if(sent[0]=='l') {         /* [l]igar televisão */
						samsung_send(SAMSUNG_ON_OFF);
					} else if(sent[1]=='e') {         /* d[e]sligar televisão */
						samsung_send(SAMSUNG_ON_OFF);
					} else if(sent[0]=='a') {         /* [a]umentar volume */
						samsung_send(SAMSUNG_INC_VOL);
					} else if(sent[1]=='i') {         /* d[i]minuir volume */
						samsung_send(SAMSUNG_DEC_VOL);
					} else if(sent[0]=='p') {         /* [p]róximo canal */
						samsung_send(SAMSUNG_NEXT_CH);
					} else if(sent[0]=='c') {         /* [c]anal anterior */
						samsung_send(SAMSUNG_PREV_CH);
					} else { /* default */
						/* sentença desconhecida */
					}
				}
			} else {
				cerr << endl << C_FG_R << "Now, speak the command of control: ";
				sent = ps_decode_from_mic(ps_EN_US, ad);
				if(sent != "") {
					if(sent[0] == 's') {                      /* [s]witch to portuguese */
						PT_BR = 1;
					} else if(sent[0]=='t' && sent[9]=='n') { /* [t]urn tv o[n] */
						samsung_send(SAMSUNG_ON_OFF);
					} else if(sent[0]=='t' && sent[9]=='f') { /* [t]urn tv o[f]f */
						samsung_send(SAMSUNG_ON_OFF);
					} else if(sent[0]=='i') {                 /* [i]ncrease volume */
						samsung_send(SAMSUNG_INC_VOL);
					} else if(sent[0]=='d') {                 /* [d]ecrease volume */
						samsung_send(SAMSUNG_DEC_VOL);
					} else if(sent[0]=='n') {                 /* [n]ext channel */
						samsung_send(SAMSUNG_NEXT_CH);
					} else if(sent[0]=='p') {                 /* [p]revious channel */
						samsung_send(SAMSUNG_PREV_CH);
					} else { /* default */
						/* unknown sentence */
					}
				}
			}
		} while(sent == "" && keep_running);
		cerr << C_RESET << "\t" << sent << endl;
	} /* while keep running */

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
	#if DEGUB
		cerr << "[GPIO]\tturn all LEDs off" << endl;
	#endif
	gpio_set_value(LED_RED,   HIGH);
	gpio_set_value(LED_AMBER, HIGH);
	gpio_set_value(LED_GREEN, HIGH);

	/* free GPIO pins */
	#if DEGUB
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
} /* close main */

string
ps_decode_from_mic(ps_decoder_t* ps, ad_rec_t* ad)
{
	ad_start_rec(ad); /* start recording */
	ps_start_utt(ps); /* mark the start of the utterance (initial silence <s>?) */

	int16 adbuf[4096]; /* buffer array to hold audio data */
	int32 k;           /* holds the number of frames in the audio buffer */

	uint8 utt_started; /* track active speech: has speech started? */
	uint8 in_speech;   /* track active speech: is speech currently happening? */

	char const *hyp;   /* pointer to "hypothesis" */

	utt_started = FALSE;                             // clear the utt_started flag
	for(;;) {
		k = ad_read(ad, adbuf, 4096);                // capture the number of frames in the audio buffer
		ps_process_raw(ps, adbuf, k, FALSE, FALSE);  // send the audio buffer to the pocketsphinx decoder

		/* test to see if speech is being detected 
		 * (voice activity detection?) */
		in_speech = ps_get_in_speech(ps);

		/* if speech has been already detected
		 * but the flag says it hasn't been started yet 
		 * then we set the flag, saying that speech has already started */
		if (in_speech && !utt_started)
			utt_started = TRUE; 

		/* if speech is not being detected anymore 
		 * but the flag says it has already started 
		 * then we've detected the end point (silence </s>) */
		if (!in_speech && utt_started) {
			ps_end_utt(ps);  /* so we mark the end of the utterance */
			ad_stop_rec(ad); /* and stop recording. */

			/* query pocketsphinx for "hypothesis" of decoded statement */
			if((hyp = ps_get_hyp(ps, NULL)) != 0)
				return hyp;

			break;
		}
	}

	/* if the threshold score has not been achieved, we return nothing */
	return "";
}

void
signal_handler(int dummy) 
{
	keep_running = false;
}

/*** EOF ***/
