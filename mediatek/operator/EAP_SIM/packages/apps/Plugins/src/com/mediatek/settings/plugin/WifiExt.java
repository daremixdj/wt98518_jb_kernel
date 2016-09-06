package com.mediatek.settings.plugin;
import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.preference.PreferenceScreen;
import android.provider.Settings.System;
import android.view.View;
import android.widget.Switch;
import android.widget.TextView;

import com.mediatek.settings.ext.DefaultWifiExt;
import com.mediatek.xlog.Xlog;
import android.os.SystemProperties;

public class WifiExt extends DefaultWifiExt {
    private static final String TAG = "WifiExt";
    private Context mContext;
	
	static final String SSID_1="CHT Wi-Fi Auto";
	static final String SSID_2="FET Wi-Fi Auto";
	static final String SSID_3="TWM WiFi Auto";
	static final String SSID_4="SingTel WIFI";
	static final String SSID_5="Wireless@SGx";
	static final int SECURITY_EAP = 5;
	private boolean ishtcCid="HTC__621".equals(SystemProperties.get("ro.cid")) ||"HTC__622".equals(SystemProperties.get("ro.cid"))
					||"HTC__044".equals(SystemProperties.get("ro.cid"));
    public WifiExt(Context context) {
		super(context);
        mContext = context;
        Xlog.d(TAG,"WifiExt");
    }
    //wifi enabler
  /*  public void registerAirplaneModeObserver(Switch tempSwitch) {
    }
    public void unRegisterAirplaneObserver() {
    }
    public boolean getSwitchState() {
        Xlog.d(TAG,"getSwitchState(), return true");
        return true;
    }
    public void initSwitchState(Switch tempSwitch) {
    }
    //wifi access point enabler
    public String getWifiApSsid() {
        return mContext.getString(
            com.android.internal.R.string.wifi_tether_configure_ssid_default);
    }*/
    //wifi config controller
    public boolean shouldAddForgetButton(String ssid, int security) {
    	if(ishtcCid && SSID_4.equals(ssid) && (security == SECURITY_EAP)) {	
              return false;	
        }
        return true;
    }
   /* public void setAPNetworkId(int apNetworkId) {
    }
    public void setAPPriority(int apPriority) {
    }
    public View getPriorityView() {
        //view.findViewById(priorityId).setVisibility(View.GONE);
        return null;
    }
    public void setSecurityText(TextView view) {
    }
    public String getSecurityText(int security) {
        return mContext.getString(security);
    }

    public boolean shouldSetDisconnectButton() {
        return false;
    }
    public int getPriority() {
        return -1;
    }
    public void closeSpinnerDialog() {
    }
    public void setProxyText(TextView view) {
    }
//advanced wifi settings
    public void initConnectView(Activity activity,PreferenceScreen screen) {
    }

    public void initNetworkInfoView(PreferenceScreen screen) {
    }
    public void refreshNetworkInfoView() {
    }
    public void initPreference(ContentResolver contentResolver) {
    }
    public int getSleepPolicy(ContentResolver contentResolver) {
        return System.getInt(contentResolver, System.WIFI_SLEEP_POLICY, System.WIFI_SLEEP_POLICY_NEVER);
    }

    //access point
    public int getApOrder(String currentSsid, int currentSecurity, String otherSsid, int otherSecurity) {
        //Xlog.d(TAG,"getApOrder(),return 0");
        return 0;
    }*/
}
