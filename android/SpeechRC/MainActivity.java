package com.example.cassio.speechrc;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.graphics.Color;
import android.os.Build;
import android.speech.RecognitionListener;
import android.speech.RecognizerIntent;
import android.speech.SpeechRecognizer;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Toast;

import java.io.IOException;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

	public static final String TAG = "BT";

	private SpeechRecognizer sr = null;

	RadioGroup rg;
	RadioButton en_us;
	RadioButton pt_br;

	EditText text_ip;
	Button button_google;

	/*BT*/
	private BluetoothAdapter adapter = null; //pointer to my android BT dev
	private BluetoothSocket skt = null; //socket to manage connection

	public Arduino arduino; //thread to communicate android and arduino
	public OutputStream out = null; //stream to store and send string

	/* SPP UUID service (Not for peer-peer) */
	public static final UUID uu_id = UUID.
		fromString("00001101-0000-1000-8000-00805F9B34FB");

	/* MAC-address of Bluetooth module HC-05 "Coruja Remote" */
	public static final String address = "20:13:06:03:02:00";

	@Override
		protected void onCreate(Bundle savedInstanceState) {
			super.onCreate(savedInstanceState);
			setContentView(R.layout.activity_main);

			rg = (RadioGroup) findViewById(R.id.radiogroup);
			en_us = (RadioButton) findViewById(R.id.en_us);
			pt_br = (RadioButton) findViewById(R.id.pt_br);

			text_ip = (EditText) findViewById(R.id.editText);
			text_ip.setFocusable(false);

			button_google = (Button) findViewById(R.id.button_google);
			button_google.setBackgroundColor(Color.GREEN);

			adapter = BluetoothAdapter.getDefaultAdapter(); //get BT adapter from SO
		}

	/* Create Insecure RFCOMM channel based on UUID and a pointer to Mac-Addr BT */
	private BluetoothSocket createBluetoothSocket(BluetoothDevice device) throws IOException {
		if(Build.VERSION.SDK_INT >= 10) {
			try {
				final Method m = device.getClass().getMethod(
						"createInsecureRfcommSocketToServiceRecord", new Class[] { UUID.class });
				return (BluetoothSocket) m.invoke(device, uu_id); //return here
			} catch (Exception e) {
				Log.e(TAG, "Can't create Insecure RFCOMM Connection" + e.getMessage());
			}
		}
		return device.createRfcommSocketToServiceRecord(uu_id);
	}

	private void errorExit(String title, String msg) {
		Toast.makeText(getBaseContext(),
				title + ":" + msg, Toast.LENGTH_LONG).show();
		finish();
	}

	@Override
		public void onResume() {
			super.onResume();

			/* Check if Bluetooth is on ... */
			if(!adapter.isEnabled())
				startActivityForResult(new Intent(
							BluetoothAdapter.ACTION_REQUEST_ENABLE), 1);

			/* ... and wait until bluetooth is enabled (Gambiarra) */
			while(!adapter.isEnabled());

			/* Set up a pointer to arduino using its BT module address */
			BluetoothDevice device = adapter.getRemoteDevice(address);
			try {
				skt = createBluetoothSocket(device); //open RFCOMM channel based on UUID
			} catch (IOException e) {
				Log.e(TAG, e.getMessage());
				errorExit("onResume()",
						"socket create failed: " + e.getMessage());
			}

			adapter.cancelDiscovery(); //saves battery and prevents another connection

			Log.i(TAG, "ok i'm here");

			if (sr == null) {
				sr = SpeechRecognizer.createSpeechRecognizer(getApplicationContext());
				sr.setRecognitionListener(new listener());
			}

			/* Establish the connection. This will block until it connects */
			try {
				skt.connect();
			} catch (IOException e) {
				try {
					skt.close();
				} catch (IOException e2) {
					errorExit("onResume()",
							"at closing skt in conn failure" + e2.getMessage());
				}
			}
			arduino = new Arduino(skt); //pass socket to thread
			arduino.start(); //start thread
		}//close onResume

	@Override
		public void onPause() {//to back button
			if (sr != null) {
				sr.destroy();
			}

			sr = null;

			super.onPause();

			try {
				skt.close();
			} catch (IOException e) {
				errorExit("onPause()",
						"failed to close socket." + e.getMessage());
			}
		}//close onPause()

	public void onClick(View v) {

		button_google.setEnabled(false);
		rg.setEnabled(false);

		Intent intent = new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH);
		intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL,
				RecognizerIntent.LANGUAGE_MODEL_FREE_FORM);
		intent.putExtra(RecognizerIntent.EXTRA_CALLING_PACKAGE, this.getPackageName());
		intent.putExtra(RecognizerIntent.EXTRA_MAX_RESULTS, 1);

		if(pt_br.isChecked())
			intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE, "pt-BR");
		else
			intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE, "en-US");

		sr.startListening(intent);
	}

	class listener implements RecognitionListener {
		public void onReadyForSpeech(Bundle params) {
			Log.i("Google", "I'm ready! Speak...");
			Toast.makeText(getApplicationContext(),
					"I'm Ready. Speak!",  Toast.LENGTH_LONG).show();
		}

		public void onBeginningOfSpeech() {
			Log.d(TAG, "Comecei a falar");
		}

		public void onRmsChanged(float rmsdB) {
			//Log.d(TAG, "RMS changed");
		}

		public void onBufferReceived(byte[] buffer) {
			Log.d(TAG, "buffer received");
		}

		public void onEndOfSpeech() {
			Log.d(TAG, "Terminei de falar");
		}


		public void onError(int error) {
			Toast.makeText(getApplicationContext(),
					"Error", Toast.LENGTH_LONG).show();
			Log.e("Google",  "Error code " +  error);
			//sr.cancel();
			button_google.setEnabled(true);
			rg.setEnabled(true);
		}

		public void onResults(Bundle results) {
			String result = results.getStringArrayList(
					SpeechRecognizer.RESULTS_RECOGNITION).get(0).toLowerCase();
			text_ip.setText(result);

			button_google.setEnabled(true);
			rg.setEnabled(true);

			Log.i("RESULT", "Result: " + result);

			if(en_us.isChecked()) {
				/* ROLL */
				if(result.contains("turn")) {
					if(result.contains("on")) {
						arduino.write("RR"); // turn on tv 
					} else if(result.contains("off")) {
						arduino.write("RL"); // turn off tv 
					} else {
						Log.e("ON/OFF ERROR", result);
					}
					/* YAW */
				} else if(result.contains("channel")) {
					if(result.contains("next")) {
						arduino.write("YR"); // next channel 
					} else if(result.contains("previous")) {
						arduino.write("YL"); // previous channel 
					} else {
						Log.e("CHANNEL ERROR", result);
					}
					/* PITCH */
				} else if(result.contains("volume")) {
					if(result.contains("increase")) {
						arduino.write("PU"); // increase volume 
					} else if(result.contains("decrease")) {
						arduino.write("PD"); // decrease volume 
					} else {
						Log.e("VOLUME ERROR", result);
					}
				} else {
					Log.e("RESULT", "ERROR EN " + result);
				}
			} else {
				if (result.contains("televisão")) {
					if (result.contains("ligar")) {
						arduino.write("RR"); // ligar televisão
					} else if (result.contains("desligar")) {
						arduino.write("RL"); // desligar televisão
					} else {
						Log.e("ON/OFF ERROR", result);
					}
				} else if (result.contains("canal")) {
					if (result.contains("mais")) {
						arduino.write("YR"); // canal mais
					} else if (result.contains("menos")) {
						arduino.write("YL"); // canal menos
					} else {
						Log.e("CANAL ERROR", result);
					}
				} else if (result.contains("volume")) {
					if (result.contains("aumentar")) {
						arduino.write("PU"); // aumentar volume 
					} else if (result.contains("diminuir")) {
						arduino.write("PD"); // diminuir volume 
					} else {
						Log.e("VOLUME ERROR", result);
					}
				} else {
					Log.e("RESULT", "ERRO PT " + result);
				}
			}//close if lang
			// sr.cancel();
		}//close onResults

		public void onPartialResults(Bundle partialResults) {
			Log.d(TAG, "partial result");
		}

		public void onEvent(int eventType, Bundle params) {
			Log.d(TAG, "evento");
		}
	}//close subclass listener

	/** Thread which communicates devices */
	private class Arduino extends Thread {
		/* Constructor: Receive BluetoothSocket object and set the output stream */
		public Arduino(BluetoothSocket socket) {
			try {
				out = socket.getOutputStream();
			} catch (IOException e) {
				Log.e("th1", e.getMessage());
			}
		}//close constructor

		/* Call this from the main activity to send data to arduino */
		public void write(String message) {
			byte[] data = message.getBytes();
			try {
				out.write(data);
			} catch (IOException e) {
				Log.e("th2", "Error data send: " + e.getMessage());
			}
		}//write
	}//close class ConnectThread

}//close activity
