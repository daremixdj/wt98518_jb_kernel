package com.mediatek.FMRadio;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.util.Log;
import android.view.KeyEvent;
import android.content.Context;


public class HeadsetHookReceiver extends BroadcastReceiver {

    private FMRadioActivity mFmRadioApp=null;
    private final String TAG = "Fm HeadsetHookReceiver";

    private void toogleFmPower() {
        //mFmRadioApp.toogleFmPower();
        if(mFmRadioApp != null){//baal modify for B233605 20130529
            /// yubosong 20130904 modified for bug 246251 @{
            //mFmRadioApp.seekStation(mFmRadioApp.mCurrentStation, true);
            //if (mFmRadioApp.isPlaying()) {
            //    mFmRadioApp.seekStation(mFmRadioApp.mCurrentStation, true);
           // }
            //else {
                mFmRadioApp.toogleFmPower();
            //}
            /// @}
        }
    }

    public HeadsetHookReceiver() {
        mFmRadioApp= (FMRadioActivity)FMRadioActivity.getInstance();
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if(intent.getAction() == null){
            return;
        }
        if (intent.getAction().equals(Intent.ACTION_MEDIA_BUTTON)) {
            KeyEvent event = (KeyEvent) intent.getParcelableExtra(Intent.EXTRA_KEY_EVENT);

            if (event == null) {
                return;
            }

            int keycode = event.getKeyCode();
            int action = event.getAction();
            long eventTime = event.getEventTime();

            LogUtils.d(TAG, "fm HeadsetHookReceiver action:" + action + ",keycode:" + keycode);
            if (action == KeyEvent.ACTION_DOWN) {
                switch (keycode) {
                    case KeyEvent.KEYCODE_HEADSETHOOK:
                    case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
                        //toogle fm power
                        toogleFmPower();
                        break;
                    case KeyEvent.KEYCODE_MEDIA_STOP:
                    case KeyEvent.KEYCODE_MEDIA_NEXT:
                    case KeyEvent.KEYCODE_MEDIA_PREVIOUS:
                    case KeyEvent.KEYCODE_MEDIA_PAUSE:
                    case KeyEvent.KEYCODE_MEDIA_PLAY:
                        break;
                }
            } else {

            }
            if (isOrderedBroadcast()) {
                abortBroadcast();
            }
        }
    }
}

