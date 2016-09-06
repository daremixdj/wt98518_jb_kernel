package com.mediatek.settings.plugin;

import android.content.ContentResolver;
import android.content.Context;
import android.net.wifi.WifiConfiguration;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;

import com.mediatek.settings.ext.DefaultWifiSettingsExt;
import com.mediatek.xlog.Xlog;
import android.os.SystemProperties;
/* Dummy implmentation , do nothing */
public class WifiSettingsExt extends DefaultWifiSettingsExt {
    private static final String TAG = "DefaultWifiSettingsExt";

	static final String SSID_1="CHT Wi-Fi Auto";
	static final String SSID_2="FET Wi-Fi Auto";
	static final String SSID_3="TWM WiFi Auto";
	static final String SSID_4="SingTel WIFI";
	static final String SSID_5="Wireless@SGx";
	static final int SECURITY_EAP = 5;
	private boolean ishtcCid="HTC__621".equals(SystemProperties.get("ro.cid")) ||"HTC__622".equals(SystemProperties.get("ro.cid"))
					||"HTC__044".equals(SystemProperties.get("ro.cid"));
    public boolean shouldAddForgetMenu(String ssid, int security) {
		if(ishtcCid && SSID_4.equals(ssid) && (security == SECURITY_EAP)) {	
              return false;	
        }
        Xlog.d(TAG,"WifiSettingsExt, shouldAddMenuForget(),return true");
        return true;
    }
    /*public void registerPriorityObserver(ContentResolver contentResolver, Context context) {
    }
    public void unregisterPriorityObserver(ContentResolver contentResolver) {
    }
    public void setLastConnectedConfig(WifiConfiguration config) {
    }
    public void setLastPriority(int priority) {
    }
    public void updatePriority() {
    }
    public boolean shouldAddDisconnectMenu() {
        return false;
    }
    public boolean isCatogoryExist() {
        return false;
    }
    public void setCategory(PreferenceCategory trustPref, PreferenceCategory configedPref, 
            PreferenceCategory newPref) {
    }
    public void emptyCategory(PreferenceScreen screen) {
        screen.removeAll();
    }
    public void emptyScreen(PreferenceScreen screen) {
        screen.removeAll();
    }*/
    public boolean isTustAP(String ssid, int security) {
		if(ishtcCid){
		if(SSID_1.equals(ssid) && (security == SECURITY_EAP)) {	
              return true;	
        }else if(SSID_2.equals(ssid) && (security == SECURITY_EAP)) {	
              return true;	
        }else if(SSID_3.equals(ssid) && (security == SECURITY_EAP)) {	
              return true;	
        }else if(SSID_4.equals(ssid) && (security == SECURITY_EAP)) {	
              return true;	
        }else if(SSID_5.equals(ssid) && (security == SECURITY_EAP)) {	
              return true;	
        }
        }
        return false;
    }
    /*public void refreshCategory(PreferenceScreen screen) {
    }
    public int getAccessPointsCount(PreferenceScreen screen) {
        return screen.getPreferenceCount();
    }
    public void adjustPriority() {
    }
    public void recordPriority(int selectPriority) { 
    }
    public void setNewPriority(WifiConfiguration config) {
    }
    public void updatePriorityAfterSubmit(WifiConfiguration config) {
    }
    public void disconnect(int networkId) {
    }
    public void updatePriorityAfterConnect(int networkId) {
    }*/
}
