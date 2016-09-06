package com.mediatek.settings.plugin;

import android.preference.Preference;
import android.preference.PreferenceGroup;
import android.preference.PreferenceScreen;

import com.mediatek.settings.ext.DefaultApnSettingsExt;
import com.mediatek.xlog.Xlog;

public class ApnSettingsExt extends DefaultApnSettingsExt {
    
    private static final String TAG = "OP02ApnSettingsExt";
    private static final String CU_3GNET_NAME = "3gnet";
    
    public boolean isAllowEditPresetApn(String type, String apn, String numeric, int sourcetype) {
        /// M: HTC D616 CUC. @ {
        Xlog.d(TAG,"isAllowEditPresetApn");
        if (numeric.equals("46001")) {
            if (sourcetype != 0) {
                return true;
            } else {
                return false;
            }
        } else if (numeric.equals("45407")) {
            if (sourcetype != 0) {
                return true;
            } else {
                return false;
            }
        } else {
            return true;
        }
        /// @ }
    }

    public Preference getApnPref(PreferenceGroup apnList, int count, int[] array) {
        Preference apnPref = apnList.getPreference(0);
        for (int i = 0; i < count; i++) {
            Preference preference =  apnList.getPreference(i);
            CharSequence strApn = preference.getSummary();
            if (CU_3GNET_NAME.equals(strApn) && (array[i] == 0)) {
                apnPref = preference;
            }                        
        }
        Xlog.d(TAG,"Get apn: " + apnPref.getSummary());
        return apnPref;

    }
    
}
