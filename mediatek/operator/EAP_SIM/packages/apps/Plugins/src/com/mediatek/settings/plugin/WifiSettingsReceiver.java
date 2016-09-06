package com.mediatek.op.plugin;


import android.content.Context;
import android.content.Intent;
import android.util.Log;
import com.android.internal.os.AtomicFile;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.IccCardConstants;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyProperties;


import android.telephony.TelephonyManager;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiConfiguration;
import android.os.SystemProperties;


import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.os.Bundle;
import android.content.ComponentName;
import android.os.ServiceManager;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.TelephonyManagerEx;

import java.util.ArrayList;
import java.util.List;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.SystemProperties;


public class WifiSettingsReceiver extends BroadcastReceiver{
	
	private final static String TAG = "SKY";
	private final static String DEFAULT_PATH = "default_path";
	static final String CMCC_AUTO_SSID = "CMCC-AUTO";
	static final String SSID_1="CHT Wi-Fi Auto";
	static final String SSID_2="FET Wi-Fi Auto";
	static final String SSID_3="TWM WiFi Auto";
	static final String SSID_4="SingTel WIFI";
	static final String SSID_5="Wireless@SGx";
	private boolean ishtcCid="HTC__621".equals(SystemProperties.get("ro.cid")) ||"HTC__622".equals(SystemProperties.get("ro.cid"))
					||"HTC__044".equals(SystemProperties.get("ro.cid"));

	private static final int SIM_CARD_1 = PhoneConstants.GEMINI_SIM_1;
    private static final int SIM_CARD_2 = PhoneConstants.GEMINI_SIM_2;
    private static final int SIM_CARD_UNDEFINED = -1;

	private static final int SSID_MATCH_SIM_1= 1;
	private static final int SSID_MATCH_SIM_2= 2;
	private static final int SSID_MATCH_SIM_NONE= 0;

	
    private static final int BUFFER_LENGTH = 40;
    private static final int MNC_SUB_BEG = 3;
    private static final int MNC_SUB_END = 5;
    private static final int MCC_SUB_BEG = 0;

	//private WifiManager.Channel mChannel;
	private static String new_imsi;
	/*to mark if the tcard is insert, set to true only the time SD inserted*/
    private WifiManager mWifiManager;
	private TelephonyManager mTm;
	private TelephonyManagerEx mTelephonyManagerEx;
	private ConnectivityManager mCm;

	protected int mSimId;
	

	@Override
	public void onReceive(Context context,Intent intent){

        String stateExtra;
		//boolean mIsSIMExist = false;
		//boolean isCmccCard = false;
		//String mNumeric;

		String action = intent.getAction();
		ITelephony mITelephony = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
		
		mWifiManager = (WifiManager)context.getSystemService(Context.WIFI_SERVICE);
		mTm = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
		mTelephonyManagerEx = TelephonyManagerEx.getDefault();
		mCm = (ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE);
		NetworkInfo info = mCm.getActiveNetworkInfo();
		//ITelephony mITelephony = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
		//mChannel = mWifiManager.initialize(context, context.getMainLooper(), null);
		Log.i(TAG, "WifiSettingsReceiver onReceive action = " + action);
		if(!ishtcCid){
			Log.i(TAG, "WifiSettingsReceiver onReceive is not htcCid "+ SystemProperties.get("ro.cid"));
			return;
		}
		
	    /*if it is because wifi enabled it wifi update the CMCC-AUTO config
		    about SIM slot AND IMSI*/
		if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(action))
		{
		    
		    final int wifiState = mWifiManager.getWifiState();
			Log.i(TAG, "WifiSettingsReceiver onReceive WIFI_STATE_CHANGED_ACTION wifiState = " + wifiState);
		    if (wifiState == WifiManager.WIFI_STATE_ENABLED)
		    {	    
		        final List<WifiConfiguration> configs = mWifiManager.getConfiguredNetworks();
				Log.i(TAG, "WifiSettingsReceiver onReceive WIFI_STATE_CHANGED_ACTION WIFI_STATE_ENABLED AND configs = " + configs);
				if (configs != null && FeatureOption.MTK_EAP_SIM_AKA)
		        {
		            for (WifiConfiguration config : configs)
                    {
                        Log.i(TAG, "WifiSettingsReceiver onReceive WIFI_STATE_CHANGED_ACTION WIFI_STATE_ENABLED config.SSID = " + config.SSID);
						Log.i(TAG, "WifiSettingsReceiver onReceive WIFI_STATE_CHANGED_ACTION WIFI_STATE_ENABLED config.simSlot = " + config.simSlot);
						Log.i(TAG, "WifiSettingsReceiver onReceive WIFI_STATE_CHANGED_ACTION WIFI_STATE_ENABLED config = " + config.toString());
						int matched_sim =SSID_MATCH_SIM_NONE;
                        if (SSID_1.equals(removeDoubleQuotes(config.SSID)) 
							&& (config.toString().contains("eap: SIM") || config.toString().contains("eap: AKA")))
                        {
                        	matched_sim = receviceWifiOn(config,mITelephony);
							if(matched_sim !=SSID_MATCH_SIM_NONE){
								if(info == null || (info.getType() != ConnectivityManager.TYPE_WIFI)){
								Log.i(TAG,"enable network "+config.networkId);
								mWifiManager.enableNetwork(config.networkId,false);
								//mWifiManager.startScan();
								}
							}

                        }
						else if (SSID_2.equals(removeDoubleQuotes(config.SSID))
							&& (config.toString().contains("eap: SIM") || config.toString().contains("eap: AKA")))
						{
							matched_sim = receviceWifiOn(config,mITelephony);
							if(matched_sim !=SSID_MATCH_SIM_NONE){
								if(info == null || (info.getType() != ConnectivityManager.TYPE_WIFI)){
								Log.i(TAG,"enable network "+config.networkId);
								mWifiManager.enableNetwork(config.networkId,false);
								//mWifiManager.startScan();
								}
							}
						}
						else if (SSID_3.equals(removeDoubleQuotes(config.SSID))
							&& (config.toString().contains("eap: SIM") || config.toString().contains("eap: AKA")))
						{
							matched_sim = receviceWifiOn(config,mITelephony);
							if(matched_sim !=SSID_MATCH_SIM_NONE){
								if(info == null || (info.getType() != ConnectivityManager.TYPE_WIFI)){
								Log.i(TAG,"enable network "+config.networkId);
								mWifiManager.enableNetwork(config.networkId,false);
								//mWifiManager.startScan();
								}
							}
						}
						else if (SSID_4.equals(removeDoubleQuotes(config.SSID))
							&& (config.toString().contains("eap: SIM") || config.toString().contains("eap: AKA")))
						{
							matched_sim = receviceWifiOn(config,mITelephony);
							if(matched_sim !=SSID_MATCH_SIM_NONE){
								if(info == null || (info.getType() != ConnectivityManager.TYPE_WIFI)){
								Log.i(TAG,"enable network "+config.networkId);
								mWifiManager.enableNetwork(config.networkId,false);
								//mWifiManager.startScan();
								}
							}
						}
						else if (SSID_5.equals(removeDoubleQuotes(config.SSID))
							&& (config.toString().contains("eap: SIM") || config.toString().contains("eap: AKA")))
						{
							matched_sim = receviceWifiOn(config,mITelephony);
							if(matched_sim !=SSID_MATCH_SIM_NONE){
								if(info == null || (info.getType() != ConnectivityManager.TYPE_WIFI)){
								Log.i(TAG,"enable network "+config.networkId);
								mWifiManager.enableNetwork(config.networkId,false);
								//mWifiManager.startScan();
								}
							}
						}
		            }
		        }
		    }
			return;
		}

		
		/*if the sim card insert or plug out, and the wifi is enabled, it wifi update the CMCC-AUTO config
		    about SIM slot AND IMSI*/
		if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED) 
			    && (mWifiManager.getWifiState() == WifiManager.WIFI_STATE_ENABLED) )
		{
		  
		    stateExtra = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
			mSimId = intent.getIntExtra("simId", SIM_CARD_UNDEFINED);


		    /*if it is in the state of bootup done or sim card inserted*/
			final List<WifiConfiguration> configs = mWifiManager.getConfiguredNetworks();
			Log.i(TAG, "WifiSettingsReceiver onReceive stateExtra = " + stateExtra + ";simid = " + mSimId);
		    if (IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(stateExtra))
		    {
		        Log.i(TAG, "WifiSettingsReceiver onReceive INTENT_VALUE_ICC_LOADED");
		        if (configs != null && FeatureOption.MTK_EAP_SIM_AKA)
		        {
                    for (WifiConfiguration config : configs)
                    {
                        Log.i(TAG, "WifiSettingsReceiver onReceive INTENT_VALUE_ICC_LOADED config.SSID = " + config.SSID);
						Log.i(TAG, "WifiSettingsReceiver onReceive INTENT_VALUE_ICC_LOADED config.simSlot = " + config.simSlot);
						int matched_sim =SSID_MATCH_SIM_NONE;
                        if (SSID_1.equals(removeDoubleQuotes(config.SSID))
							&& (config.toString().contains("eap: SIM") || config.toString().contains("eap: AKA")))
                        {
                        	matched_sim = receiveSimLoaded(mSimId,config,mITelephony);
							if(matched_sim == (mSimId+1)){
								if(info == null || (info.getType() != ConnectivityManager.TYPE_WIFI)){
								Log.i(TAG,"enable network "+config.networkId);
								mWifiManager.enableNetwork(config.networkId,false);
								//mWifiManager.startScan();
								}
							}
                        }
						else if (SSID_2.equals(removeDoubleQuotes(config.SSID))
							&& (config.toString().contains("eap: SIM") || config.toString().contains("eap: AKA")))
						{
							matched_sim = receiveSimLoaded(mSimId,config,mITelephony);
							if(matched_sim == (mSimId+1)){
								if(info == null || (info.getType() != ConnectivityManager.TYPE_WIFI)){
								Log.i(TAG,"enable network "+config.networkId);
								mWifiManager.enableNetwork(config.networkId,false);
								//mWifiManager.startScan();
								}
							}
						}
						else if (SSID_3.equals(removeDoubleQuotes(config.SSID))
							&& (config.toString().contains("eap: SIM") || config.toString().contains("eap: AKA")))
						{
							matched_sim = receiveSimLoaded(mSimId,config,mITelephony);
							if(matched_sim == (mSimId+1)){
								if(info == null || (info.getType() != ConnectivityManager.TYPE_WIFI)){
								Log.i(TAG,"enable network "+config.networkId);
								mWifiManager.enableNetwork(config.networkId,false);
								//mWifiManager.startScan();
								}
							}
						}
						else if (SSID_4.equals(removeDoubleQuotes(config.SSID))
							&& (config.toString().contains("eap: SIM") || config.toString().contains("eap: AKA")))
						{
							matched_sim = receiveSimLoaded(mSimId,config,mITelephony);
							if(matched_sim == (mSimId+1)){
								if(info == null || (info.getType() != ConnectivityManager.TYPE_WIFI)){
								Log.i(TAG,"enable network "+config.networkId);
								mWifiManager.enableNetwork(config.networkId,false);
								//mWifiManager.startScan();
								}
							}
						}
						else if (SSID_5.equals(removeDoubleQuotes(config.SSID))
							&& (config.toString().contains("eap: SIM") || config.toString().contains("eap: AKA")))
						{
							matched_sim = receiveSimLoaded(mSimId,config,mITelephony);
							if(matched_sim == (mSimId+1)){
								if(info == null || (info.getType() != ConnectivityManager.TYPE_WIFI)){
								Log.i(TAG,"enable network "+config.networkId);
								mWifiManager.enableNetwork(config.networkId,false);
								//mWifiManager.startScan();
								}
							}
						}
                    }
		        }
		        return;
		    }
			/*if it is in the state of sim card plugout*/
			if (IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(stateExtra))
			{
			    Log.i(TAG, "WifiSettingsReceiver onReceive [sim plugout] configs = " + configs);
			    if (configs != null && FeatureOption.MTK_EAP_SIM_AKA)
		        {
		            for (WifiConfiguration config : configs)
                    {
                        Log.i(TAG, "WifiSettingsReceiver onReceive [sim plugout] SSID = " + config.SSID);
						int matched_sim =SSID_MATCH_SIM_NONE;

					    if (SSID_1.equals(removeDoubleQuotes(config.SSID))
							&& (config.toString().contains("eap: SIM") || config.toString().contains("eap: AKA")))
					    {
					    	matched_sim = receiveSimAbsent(mSimId,config,mITelephony);
					    }
						else if (SSID_2.equals(removeDoubleQuotes(config.SSID))
							&& (config.toString().contains("eap: SIM") || config.toString().contains("eap: AKA")))
						{
							matched_sim = receiveSimAbsent(mSimId,config,mITelephony);
						}
						else if (SSID_3.equals(removeDoubleQuotes(config.SSID))
							&& (config.toString().contains("eap: SIM") || config.toString().contains("eap: AKA")))
						{
							matched_sim = receiveSimAbsent(mSimId,config,mITelephony);
						}
						else if (SSID_4.equals(removeDoubleQuotes(config.SSID))
							&& (config.toString().contains("eap: SIM") || config.toString().contains("eap: AKA")))
						{
							matched_sim = receiveSimAbsent(mSimId,config,mITelephony);
						}
						else if (SSID_5.equals(removeDoubleQuotes(config.SSID))
							&& (config.toString().contains("eap: SIM") || config.toString().contains("eap: AKA")))
						{
							matched_sim = receiveSimAbsent(mSimId,config,mITelephony);
						}
                        
		            }
			    }
			    return;
			} 

		}		        		
	}
	/**
       * W: 
       * @param 
       * @return 0 not match; 1 match sim1, 2 match sim2
       */
	private int receviceWifiOn(WifiConfiguration config, ITelephony ITelephony){
		ITelephony mITelephony=ITelephony;
		boolean mIsSIMExist = false;
		boolean isCmccCard = false;
		int simid = SSID_MATCH_SIM_NONE;
		String mNumeric;
		{
		    /*the first time it try to set the CMCC-AUTO config*/
			Log.i(TAG, "WifiSettingsReceiver onReceive WIFI_STATE_CHANGED_ACTION WIFI_STATE_ENABLED  eap: SIM  or eap: AKA");
		    if (config.simSlot == null)
		    {
		        Log.i(TAG, "WifiSettingsReceiver onReceive first time update");
		        if (FeatureOption.MTK_GEMINI_SUPPORT)
			    {
					Log.i(TAG, "WifiSettingsReceiver onReceive dual sim load");
		            try{
						mIsSIMExist = false;
						mIsSIMExist = mITelephony.isSimInsert(0);
					}
					catch (Exception e) {
		                Log.i(TAG, "WifiSettingsReceiver onReceive  wifi enabled first time get sim 0 excep");
		            }
					/*check the sim 1 first, if sim 1 has card*/
					if(mIsSIMExist)
					{
					    Log.i(TAG, "WifiSettingsReceiver onReceive sim1 insert");
					    mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
					    isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric); 
		                /*if sim1 is CMCC card*/
						if (isCmccCard)
					    {
					        Log.i(TAG, "WifiSettingsReceiver onReceive sim1 IS CMCC CARD, UPDATE");
					        updateCmccapgemini(0, config, mITelephony,mWifiManager);
							simid = SSID_MATCH_SIM_1;
					    }
						else /* check the sim 2*/
						{
						    Log.i(TAG, "WifiSettingsReceiver onReceive sim1 IS NOT CMCC CARD, try sim2");
						    try{
								mIsSIMExist = false;
								mIsSIMExist = mITelephony.isSimInsert(1);
							}
							catch (Exception e) {
			                    Log.i(TAG, "WifiSettingsReceiver onReceive  wifi enabled first time get sim 0 excep");
			                }
							/*sim2 is inserted*/
						    if (mIsSIMExist)
						    {
						        Log.i(TAG, "WifiSettingsReceiver onReceive sim2 IS insert");
						        mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2, "-1");
							    isCmccCard =  getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric);
							    if (isCmccCard)
							    {
							        Log.i(TAG, "WifiSettingsReceiver onReceive sim2 IS CMCC CARD, UPDATE");
							        updateCmccapgemini(1, config, mITelephony,mWifiManager);
									simid = SSID_MATCH_SIM_2;
							    }
								else/*sim1 and sim2 are not ok, set it to default(sim 1)*/
				    			{
								    Log.i(TAG, "WifiSettingsReceiver onReceive sim2 IS NOT CMCC CARD, SET 1");
								    updateNoMatchapgemini(0,config,mITelephony,mWifiManager);
								}
				    		}
							else
							{
							    Log.i(TAG, "WifiSettingsReceiver onReceive sim2 IS NOT INSERT, SET 1");
							    updateNoMatchapgemini(0,config,mITelephony,mWifiManager);
							}
						}
					}
					else/*the check the sim 2*/
					{
					    try{
							mIsSIMExist = false;
							mIsSIMExist = mITelephony.isSimInsert(1);
					    }
						catch (Exception e) {
		                    Log.i(TAG, "WifiSettingsReceiver onReceive  wifi enabled first time get sim 1 excep");
		                }
						/*sim2 is inserted*/
					    if (mIsSIMExist)
					    {
					        Log.i(TAG, "WifiSettingsReceiver onReceive sim2 IS INSERT");
					        mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2, "-1");
						    isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric); 
						    if (isCmccCard)
							{
						        Log.i(TAG, "WifiSettingsReceiver onReceive sim2 IS CMCC CARD, SET 2");
						        updateCmccapgemini(1, config, mITelephony,mWifiManager);
								simid = SSID_MATCH_SIM_2;
						    }
							else/*sim1 and sim2 are not ok, set it to default(sim 1)*/
				    		{
							    Log.i(TAG, "WifiSettingsReceiver onReceive sim2 IS NOT CMCC CARD, SET 1");
							    updateNoMatchapgemini(0,config,mITelephony,mWifiManager);
							}
					    }
						else
						{
						    Log.i(TAG, "WifiSettingsReceiver onReceive sim2 IS NOT INSERT, SET 1");
						    updateNoMatchapgemini(0,config,mITelephony,mWifiManager);
						}
					}
				}
				else
				{
				    Log.i(TAG, "WifiSettingsReceiver onReceive single sim load");
					mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
					isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric);
				    updateCmccap(config, mITelephony, mWifiManager);
					if(isCmccCard){
						simid = SSID_MATCH_SIM_1;
						}
					/*if(isCmccCard){
						if(info == null || (info.getType() != ConnectivityManager.TYPE_WIFI)){
							Log.i(TAG,"enable network "+config.networkId);
							mWifiManager.enableNetwork(config.networkId,false);
							//mWifiManager.startScan();
							}
						break;
						}	
					else
						continue;*/
				}
		    }
			else
			{
			    /*only to update the right simslot*/
			    if (FeatureOption.MTK_GEMINI_SUPPORT)
			    {
					Log.i(TAG, "WifiSettingsReceiver onReceive dual sim load");
			        if(config.simSlot.equals(addQuote("0")))
			        {
			            Log.i(TAG, "WifiSettingsReceiver onReceive  default is sim1");
			            try{
							mIsSIMExist = false;
			            	mIsSIMExist = mITelephony.isSimInsert(0);
						}
						catch (Exception e) {
		                    Log.i(TAG, "WifiSettingsReceiver onReceive  wifi enabled first time get sim 0 excep");
		                }
						/*sim 1 inserted*/

						if (mIsSIMExist)
						{
						    Log.i(TAG, "WifiSettingsReceiver onReceive  sim1 is insert");
						    if (!(config.imsi.equals(makeNAI(mTm.getSubscriberIdGemini(0), "SIM")) || config.imsi.equals(makeNAI(mTm.getSubscriberIdGemini(0), "AKA"))))
						    {
						        Log.i(TAG, "WifiSettingsReceiver onReceive  sim1 have change");
						        mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
						        isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric); 
						        if (isCmccCard)
						        {
						            Log.i(TAG, "WifiSettingsReceiver onReceive  sim1 is CMCC CARD");
						            updateCmccapgemini(0, config, mITelephony,mWifiManager);
									simid=SSID_MATCH_SIM_1;
						        }
								else
								{
								    Log.i(TAG, "WifiSettingsReceiver onReceive  sim1 is NOT CMCC CARD");
								    try{
										mIsSIMExist = false;
										mIsSIMExist = mITelephony.isSimInsert(1);									    
									}
									catch (Exception e) {
					                    Log.i(TAG, "WifiSettingsReceiver onReceive  wifi enabled first time get sim 0 excep");
					                }
								    if (mIsSIMExist)
								    {
								        Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 is INSERT");
								        mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
								        isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric); 
								        if (isCmccCard)
								        {
								            Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 is CMCC");
								            updateCmccapgemini(1, config, mITelephony,mWifiManager);
											simid=SSID_MATCH_SIM_2;
								        }
										else
										{
										    Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 is NOT CMCC");
										    updateNoMatchapgemini(0,config,mITelephony,mWifiManager);
										}
								    }
									else
									{
									    Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 is NOT INSERT");
									    updateNoMatchapgemini(0,config,mITelephony,mWifiManager);
									}
								}
						    }
							else{
								simid=SSID_MATCH_SIM_1;
							}
						}
						else/*sim 1 is not inserted,check sim2*/
						{
						    Log.i(TAG, "WifiSettingsReceiver onReceive  sim1 is NOT INSERT");
						    try{
								mIsSIMExist = false;
								mIsSIMExist = mITelephony.isSimInsert(1);    
							}
							catch (Exception e) {
			                    Log.i(TAG, "WifiSettingsReceiver onReceive  wifi enabled first time get sim 1 excep");
			                }
						    if (mIsSIMExist)
						    {
						        Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 is insert");
						        mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2, "-1");
								isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric);
						        if (isCmccCard)
						        {
						            Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 is cmcc");
						            updateCmccapgemini(1, config, mITelephony,mWifiManager);
									simid=SSID_MATCH_SIM_2;
						        }
								else
								{
								    Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 is not cmcc");
								    updateNoMatchapgemini(0,config,mITelephony,mWifiManager);
								}
						    }
							else
							{
							    Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 is not insert");
							    updateNoMatchapgemini(0,config,mITelephony,mWifiManager);
							}
						}
			            
			        }
					else if(config.simSlot.equals(addQuote("1")))
					{
					    Log.i(TAG, "WifiSettingsReceiver onReceive  default is sim2");
					    try{
							mIsSIMExist = false;
					    	mIsSIMExist = mITelephony.isSimInsert(1);
							Log.i(TAG, "WifiSettingsReceiver onReceive  default is sim2 and insert = " + mIsSIMExist);
						}
						catch (Exception e) {
		                    Log.i(TAG, "WifiSettingsReceiver onReceive  wifi enabled first time get sim 1 excep");
		                }
		                /*sim 2 insert*/
						if (mIsSIMExist)
						{
						    /*not the same card*/
							Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 is insert");
						    if (!(config.imsi.equals(makeNAI(mTm.getSubscriberIdGemini(1), "SIM")) || config.imsi.equals(makeNAI(mTm.getSubscriberIdGemini(1), "AKA"))))
						    {
						        Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 is change");
						        mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2, "-1");
						        isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric);
						        if (isCmccCard)
						        {
						            Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 is CMCC");
						            updateCmccapgemini(1, config, mITelephony,mWifiManager);
									simid=SSID_MATCH_SIM_2;
						        }
								else
								{
								    Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 is NOT CMCC");
								    try{
										mIsSIMExist = false;
										mIsSIMExist = mITelephony.isSimInsert(0);
									}
									catch (Exception e) {
					                    Log.i(TAG, "WifiSettingsReceiver onReceive  wifi enabled first time get sim 1 excep");
									}
									if(mIsSIMExist)
									{
								        Log.i(TAG, "WifiSettingsReceiver onReceive  sim1 is INSERT");
								        mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
						                isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric); 
								        if(isCmccCard)
								        {
								            Log.i(TAG, "WifiSettingsReceiver onReceive  sim1 is CMCC");
								            updateCmccapgemini(0, config, mITelephony,mWifiManager);
											simid=SSID_MATCH_SIM_1;
								        }
										else
									    {
										    Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 is NOT CMCC");
										    updateNoMatchapgemini(1,config,mITelephony,mWifiManager);
										}
								    }
									else
									{
									    Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 is INSERT");
									    updateNoMatchapgemini(1,config,mITelephony,mWifiManager);
									}
								}
							}else{
								simid=SSID_MATCH_SIM_2;
							}
					    }
						else
						{
						    Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 is NOT INSERT");
						    try{
								mIsSIMExist =false;
								mIsSIMExist = mITelephony.isSimInsert(0);
							}
							catch (Exception e) {
			                    Log.i(TAG, "WifiSettingsReceiver onReceive  wifi enabled first time get sim 1 excep");
			                }
						    if (mIsSIMExist)
						    {
						        Log.i(TAG, "WifiSettingsReceiver onReceive  sim1 is INSERT");
						        mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
						        isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric); 
						        if(isCmccCard)
						        {
						            Log.i(TAG, "WifiSettingsReceiver onReceive  sim1 is CMCC");
						            updateCmccapgemini(0, config, mITelephony,mWifiManager);
									simid=SSID_MATCH_SIM_1;
						        }
								else
							    {
								    Log.i(TAG, "WifiSettingsReceiver onReceive  sim1 is NOT CMCC");
								    updateNoMatchapgemini(1,config,mITelephony,mWifiManager);
								}
							    
						    }
							else
							{
							    Log.i(TAG, "WifiSettingsReceiver onReceive  sim1 is NOT INSERT");
							    updateNoMatchapgemini(1,config,mITelephony,mWifiManager);
							}
						}
					}
					}
				else
				{
				    Log.i(TAG, "WifiSettingsReceiver onReceive single sim load");
					mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
					isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric); 
				    updateCmccap(config, mITelephony,mWifiManager);
					if(isCmccCard){
						simid=SSID_MATCH_SIM_1;
					}
					/*if(isCmccCard){
						if(info == null || (info.getType() != ConnectivityManager.TYPE_WIFI)){
							Log.i(TAG,"enable network "+config.networkId);
							mWifiManager.enableNetwork(config.networkId,false);
							//mWifiManager.startScan();
							}
						break;
						}
					else
						continue;*/
				}
			}
		}
		return simid;
	}

	private int receiveSimLoaded(int mSimId,WifiConfiguration config, ITelephony mITelephony){
		boolean mIsSIMExist = false;
		boolean isCmccCard = false;
		int simid = SSID_MATCH_SIM_NONE;
		String mNumeric;
		{
	    
	    /*if it is the first time to update CMCC-AUTO config,
	                       set the IMSI and simslot to the CMCC-AUTO config*/
	    
	    Log.i(TAG, "WifiSettingsReceiver onReceive SSID_1 [sim plugin]  siminserted [simid] = " + mSimId + ";[simSlot] = " + config.simSlot);
	    Log.i(TAG, "WifiSettingsReceiver onReceive SSID_1 config.imsi = " + config.imsi);
		
		if(config.simSlot == null)
		{
		    Log.i(TAG, "WifiSettingsReceiver onReceive  first time for sim plugin");
		    if (FeatureOption.MTK_GEMINI_SUPPORT)
		    {
		        isCmccCard = false;
		        if (mSimId == 0)
				{
				    Log.i(TAG, "WifiSettingsReceiver onReceive  first time for sim plugin sim 1 plugin");
				    mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
			        isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric);
					if(isCmccCard) simid=SSID_MATCH_SIM_1;
				}
				else if(mSimId == 1)
				{
				    Log.i(TAG, "WifiSettingsReceiver onReceive  first time for sim plugin sim 2 plugin");
				    mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2, "-1");
			        isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric);
					if(isCmccCard) simid=SSID_MATCH_SIM_2;
				}
		        /*cmcc card inserted, update*/
				if (isCmccCard)	
				{
				    Log.i(TAG, "WifiSettingsReceiver onReceive  first time for sim plugin sim = " + mSimId + "; is CMCC CARD");
				    updateCmccapgemini(mSimId,config,mITelephony,mWifiManager);
				}
				//continue; //next config
		    }
			else
			{
				mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
			    isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric);
			    updateCmccap(config,mITelephony,mWifiManager);
				if(isCmccCard) simid=SSID_MATCH_SIM_1;
				/*if(isCmccCard){
					if(info == null || (info.getType() != ConnectivityManager.TYPE_WIFI)){
						Log.i(TAG,"enable network "+config.networkId);
						mWifiManager.enableNetwork(config.networkId,false);
						//mWifiManager.startScan();
						}
					break;
					}
				else
					continue; */
			}    
		}
		else
		{
		    if (FeatureOption.MTK_GEMINI_SUPPORT)
			{
			    /*insert the same card*/
	            if (((mSimId == SIM_CARD_1) && (addQuote("0").equals(config.simSlot))) || ((mSimId == SIM_CARD_2) && (addQuote("1").equals(config.simSlot))))
	            {
	                isCmccCard = false;
					if (mSimId == 0)
					{
					    mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
				        isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric); 
						if(isCmccCard) simid=SSID_MATCH_SIM_1;
					}
					else if (mSimId == 1)
					{
					    mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2, "-1");
				        isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric); 
						if(isCmccCard) simid=SSID_MATCH_SIM_2;
					}

					Log.i(TAG, "WifiSettingsReceiver onReceive sim plugin insert the same SIM card [sim] = " + mSimId + "; isCMCCcard = " + isCmccCard);
					if (isCmccCard)
					{
					    updateCmccapgemini(mSimId,config,mITelephony,mWifiManager);
					}
	            } 
				else
				{
				    Log.i(TAG, "WifiSettingsReceiver onReceive sim plugin insert the other SIM card [sim] = " + mSimId);
				    if (mSimId == 0)
				    {
				        try{
							mIsSIMExist =false;
							mIsSIMExist = mITelephony.isSimInsert(1);
						}
						catch (Exception e) {
		                    Log.i(TAG, "WifiSettingsReceiver onReceive  wifi enabled first time get sim 1 excep");
		                }

						if(mIsSIMExist)
						{
						    isCmccCard = false;
						    mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2, "-1");
				            isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric);
							Log.i(TAG, "WifiSettingsReceiver onReceive sim plugin insert the other SIM card, sim 1 inserting , and sim2 inserted and sim2 isCMCCCARD = " + isCmccCard);
							if (!isCmccCard)
							{
							    isCmccCard = false;
						        mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
				                isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric); 
								if(isCmccCard)
								{
								    Log.i(TAG, "WifiSettingsReceiver onReceive  sim1 insert, deafault sim2, sim2 insert but not CMCC, SIM1 cmcc");
								    updateCmccapgemini(0,config,mITelephony,mWifiManager);
									simid=SSID_MATCH_SIM_1;
								}
							}
							else{
								simid=SSID_MATCH_SIM_2;
							}
						}
						else
						{
						    isCmccCard = false;
					        mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
			                isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric); 
							Log.i(TAG, "WifiSettingsReceiver onReceive  sim1 insert, deafault sim2, sim2 not insert");
							if(isCmccCard)
							{
							    Log.i(TAG, "WifiSettingsReceiver onReceive  sim1 insert, deafault sim2, sim2 not insert, SIM1 cmcc");
							    updateCmccapgemini(0,config,mITelephony,mWifiManager);
								simid=SSID_MATCH_SIM_1;
							}
						}
				    }
					else if (mSimId == 1)
					{
					    Log.i(TAG, "WifiSettingsReceiver onReceive sim plugin insert the other SIM card [sim] = " + mSimId);
					    try{
							mIsSIMExist =false;
							mIsSIMExist = mITelephony.isSimInsert(0);
						}
						catch (Exception e) {
		                    Log.i(TAG, "WifiSettingsReceiver onReceive  wifi enabled first time get sim 1 excep");
		                }

						if(mIsSIMExist)
						{
						    Log.i(TAG, "WifiSettingsReceiver onReceive sim plugin insert the other sim,  default sim 1, sim1 inserted");
						    isCmccCard = false;
						    mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
				            isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric); 
							if (!isCmccCard)
							{
							    Log.i(TAG, "WifiSettingsReceiver onReceive sim plugin insert the other sim,  default sim 1, sim1 inserted but not CMCC");
							    isCmccCard = false;
						        mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2, "-1");
				                isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric); 
								if(isCmccCard)
								{
								    Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 insert, deafault sim1, sim1 insert but not CMCC, SIM2 cmcc");
								    updateCmccapgemini(1,config,mITelephony,mWifiManager);
									simid=SSID_MATCH_SIM_2;
								}
							}else{
								simid=SSID_MATCH_SIM_1;
							}
						}
						else
						{
						    Log.i(TAG, "WifiSettingsReceiver onReceive sim plugin insert the other sim,  default sim 1, sim1 NOT inserted");
						    isCmccCard = false;
					        mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2, "-1");
			                isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric); 
							if(isCmccCard)
							{
							    Log.i(TAG, "WifiSettingsReceiver onReceive  sim2 insert, deafault sim1, sim1 not insert, SIM2 cmcc");
							    updateCmccapgemini(1,config,mITelephony,mWifiManager);
								simid=SSID_MATCH_SIM_2;
							}
						}
					}
				}
					//continue;
			}
			else{
			    //set the info for CMCC-AUTO config for single sim project
				mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
			    isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric);
				updateCmccap(config,mITelephony,mWifiManager);
				if(isCmccCard) simid=SSID_MATCH_SIM_1;
				/*if(isCmccCard){
					if(info == null || (info.getType() != ConnectivityManager.TYPE_WIFI)){
						Log.i(TAG,"enable network "+config.networkId);
						mWifiManager.enableNetwork(config.networkId,false);
						//mWifiManager.startScan();
						}
					break;
					}
				else
					continue;*/
			}
		}
	}
	return simid;
	}
	private int receiveSimAbsent(int mSimId,WifiConfiguration config, ITelephony mITelephony){
		boolean mIsSIMExist = false;
		boolean isCmccCard = false;
		int simid = SSID_MATCH_SIM_NONE;
		String mNumeric;
		{
		    if (config.simSlot != null)
		    {
			    Log.i(TAG, "WifiSettingsReceiver onReceive [sim plugout] CMCC_AUTO_SSID configs = " + config.toString());
		        if (FeatureOption.MTK_GEMINI_SUPPORT)
				{
				    /*if the sim is beening used, it should update the config*/
					if ((mSimId == 0 && config.simSlot.equals(addQuote("0"))) || (mSimId == 1 && config.simSlot.equals(addQuote("1"))))
					{
					    Log.i(TAG, "WifiSettingsReceiver onReceive [sim plugout] the sim using is plugout");
					    if (mSimId == 0)
					    {
					        Log.i(TAG, "WifiSettingsReceiver onReceive [sim plugout] SIM 1 plugout");
					        isCmccCard = false;
					        mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2, "-1");
			                isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric);  
							if(isCmccCard)
							{
							    Log.i(TAG, "WifiSettingsReceiver onReceive  deafault sim1, sim1 not insert, SIM2 is cmcc,");
							    updateCmccapgemini(1,config,mITelephony,mWifiManager);
								simid=SSID_MATCH_SIM_2;
							}else{
								updateNoMatchapgemini(0,config,mITelephony,mWifiManager);
							}
					    }

						if (mSimId == 1)
						{
						    Log.i(TAG, "WifiSettingsReceiver onReceive [sim plugout] SIM 2 plugout");
						    isCmccCard = false;
					        mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
			                isCmccCard = getIsMatchedCard(removeDoubleQuotes(config.SSID),mNumeric);  
							if(isCmccCard)
							{
							    Log.i(TAG, "WifiSettingsReceiver onReceive  deafault sim2, sim2 not insert, SIM1 is cmcc");
							    updateCmccapgemini(0,config,mITelephony,mWifiManager);
								simid=SSID_MATCH_SIM_1;
							}else{
								updateNoMatchapgemini(1,config,mITelephony,mWifiManager);
							}
						}
					}  
				}
				else
				{
				    //set the info for CMCC-AUTO config for single sim project
					if (config.toString().contains("eap: SIM"))
					{
					    String oldImsi = config.imsi;
					    config.imsi = makeNAI(mTm.getSubscriberId(), "SIM");
		                config.simSlot = addQuote("0");
		                config.pcsc = addQuote("rild");
					    Log.i(TAG, "WifiSettingsReceiver onReceive [sim plugout] save config.imsi = " + config.imsi + ";config.simSlot = " + config.simSlot);
					    updateConfig(config, oldImsi);
					}

					if (config.toString().contains("eap: AKA"))
					{
					    String oldImsi = config.imsi;
					    config.imsi = makeNAI(mTm.getSubscriberId(), "AKA");
		                config.simSlot = addQuote("0");
		                config.pcsc = addQuote("rild");
						Log.i(TAG, "WifiSettingsReceiver onReceive [sim plugout] save config.imsi = " + config.imsi + ";config.simSlot = " + config.simSlot);
						updateConfig(config, oldImsi);
					}
				    
					
				} 
		    }                             
		}
		return simid;
	}
	 /**
       * M: make NAI
       * @param imsi eapMethod
       * @return the string of NAI
       */
      private static String makeNAI(String imsi, String eapMethod) {
      		Log.i(TAG, "makeNAI,imsi="+imsi);

            // airplane mode & select wrong sim slot
            if (imsi == null) {
                  return addQuote("error");
            }

            StringBuffer NAI = new StringBuffer(BUFFER_LENGTH);
            // s = sb.append("a = ").append(a).append("!").toString();
            System.out.println("".length());

            if (eapMethod.equals("SIM")) {
                  NAI.append("1");
            } else if (eapMethod.equals("AKA")) {
                  NAI.append("0");
            }

            // add imsi
            NAI.append(imsi);
            NAI.append("@wlan.mnc");
            // add mnc
            NAI.append("0");
            NAI.append(imsi.substring(MNC_SUB_BEG, MNC_SUB_END));
            NAI.append(".mcc");
            // add mcc
            NAI.append(imsi.substring(MCC_SUB_BEG, MNC_SUB_BEG));

            // NAI.append(imsi.substring(5));
            NAI.append(".3gppnetwork.org");
            Log.i(TAG, NAI.toString());
            Log.i(TAG, "\"" + NAI.toString() + "\"");
            return addQuote(NAI.toString());
      }

      /**
       * M: add quote for strings
       * @param string
       * @return add quote to the string
       */
      private static String addQuote(String s) {
            return "\"" + s + "\"";
      }

      private String removeDoubleQuotes(String string) {
        int length;
        if(string == null){
            return "";
        }  
        length = string.length();
        if ((length > 1) && (string.charAt(0) == '"')
                && (string.charAt(length - 1) == '"')) {
            return string.substring(1, length - 1);
        }
        return string;
    }
    private void updateCmccapgemini(int simslot , WifiConfiguration config , ITelephony mITelephony, WifiManager mWifiManager) {
		String simtype = null;
		Log.i(TAG," updateCmccapgemini simslot="+simslot);
		try{
			simtype = mITelephony.getIccCardTypeGemini(simslot);
			Log.i(TAG," updateCmccapgemini simtype="+simtype);
		}
		catch (Exception e) {
		    Log.i(TAG, "WifiSettingsReceiver [updateCmccap] excep simslot = " + simslot);
		}
		String oldImsi = config.imsi;
		if (simtype != null){
		    if (simtype.equals("USIM"))
		    {
		        config.imsi = makeNAI(mTelephonyManagerEx.getSubscriberId(simslot),"AKA");
				if (simslot == 0){
				    config.simSlot = addQuote("0");
				}
				if (simslot == 1){
				    config.simSlot = addQuote("1");
				}
                config.pcsc = addQuote("rild");
			    Log.i(TAG, "WifiSettingsReceiver onReceive [updateCmccapgemini] AKA config.imsi = " + config.imsi + ";config.simSlot = " + config.simSlot);
			    updateConfig(config,oldImsi);
		    }else{
				config.imsi = makeNAI(mTelephonyManagerEx.getSubscriberId(simslot),"SIM");
				if (simslot == 0){
					config.simSlot = addQuote("0");
				}
				if (simslot == 1){
					config.simSlot = addQuote("1");
				}
                config.pcsc = addQuote("rild");
			    Log.i(TAG, "WifiSettingsReceiver [updateCmccapgemini] SIM config.imsi = " + config.imsi + ";config.simSlot = " + config.simSlot);
			    updateConfig(config, oldImsi);
		    }
		}
		else{
			config.imsi = makeNAI(mTelephonyManagerEx.getSubscriberId(simslot),"SIM");
			if (simslot == 0){
				config.simSlot = addQuote("0");
			}
			if (simslot == 1){
				config.simSlot = addQuote("1");
			}
            config.pcsc = addQuote("rild");
		    Log.i(TAG, "WifiSettingsReceiver [updateCmccapgemini] SIM config.imsi = " + config.imsi + ";config.simSlot = " + config.simSlot);
		    updateConfig(config, oldImsi);
		}
	}
	
	private void updateNoMatchapgemini(int simslot , WifiConfiguration config , ITelephony mITelephony, WifiManager mWifiManager) {
			String oldImsi = config.imsi;
			config.imsi = makeNAI(null,"SIM");
			if (simslot == 0){
				config.simSlot = addQuote("0");
			}
			if (simslot == 1){
				config.simSlot = addQuote("1");
			}
            config.pcsc = addQuote("rild");
		    Log.i(TAG, "WifiSettingsReceiver [updateCmccapgemini] SIM config.imsi = " + config.imsi + ";config.simSlot = " + config.simSlot);
		    updateConfig(config, oldImsi);
	}
	
	private void updateCmccap(WifiConfiguration config , ITelephony mITelephony, WifiManager mWifiManager) {
		String simtype = null;
		try{
			simtype = mITelephony.getIccCardType();
			Log.i(TAG," updateCmccapge simtype="+simtype);
		}
		catch (Exception e) {
		    Log.i(TAG, "WifiSettingsReceiver [updateCmccap] excep simslot " );
		}
		String oldImsi = config.imsi;
		if (simtype != null)
		{
		    if (simtype.equals("USIM"))
		    {
		        config.imsi = makeNAI(mTm.getSubscriberId(),"AKA");
				config.simSlot = addQuote("0");
                config.pcsc = addQuote("rild");
			    Log.i(TAG, "WifiSettingsReceiver onReceive [updateCmccap] AKA config.imsi = " + config.imsi + ";config.simSlot = " + config.simSlot);
			    updateConfig(config,oldImsi);
		    }
			else{
			    config.imsi = makeNAI(mTm.getSubscriberId(),"SIM");
				config.simSlot = addQuote("0");
	            config.pcsc = addQuote("rild");
			    Log.i(TAG, "WifiSettingsReceiver [updateCmccap] SIM config.imsi = " + config.imsi + ";config.simSlot = " + config.simSlot);
			    updateConfig(config, oldImsi);
			}
		}
		else {
			config.imsi = makeNAI(null,"SIM");
			config.simSlot = addQuote("0");
            config.pcsc = addQuote("rild");
		    Log.i(TAG, "WifiSettingsReceiver [updateCmccap] SIM config.imsi = " + config.imsi + ";config.simSlot = " + config.simSlot);
		    updateConfig(config, oldImsi);
			}

		
		}
		
	private void updateConfig(WifiConfiguration config, String oldImsi) {
        if (config == null) {
            return;
        }
        Log.d(TAG, "updateConfig, config.imsi:" + config.imsi + ", oldImsi:" + oldImsi);
        WifiConfiguration newConfig = new WifiConfiguration();
        newConfig.networkId = config.networkId;
        newConfig.priority = -1;
		newConfig.imsi = config.imsi;
		newConfig.simSlot = config.simSlot;
		newConfig.pcsc = config.pcsc;
		if (oldImsi != null && !oldImsi.equals(config.imsi)) {
		    newConfig.anonymous_identity.setValue("");
		}
        //mWifiManager.updateNetwork(newConfig);
        mWifiManager.save(newConfig, null);
	}
	private boolean getIsMatchedCard(String ssid_name,String mNumeric){
		Log.d(TAG,"mNumeric= "+mNumeric);
		boolean isMatch=false;
		if(CMCC_AUTO_SSID.equals(ssid_name)){		
			isMatch=mNumeric.equals("46000") || mNumeric.equals("46002") || mNumeric.equals("46007");
		}
		else if(SSID_1.equals(ssid_name)){
			isMatch=mNumeric.equals("46692");
		}
		else if(SSID_2.equals(ssid_name)){
			isMatch=mNumeric.equals("46601");	
		}else if(SSID_3.equals(ssid_name)){
			isMatch=mNumeric.equals("46697");	
		}else if(SSID_4.equals(ssid_name)){
			isMatch=mNumeric.equals("52501")|| mNumeric.equals("52502");	
		}else if(SSID_5.equals(ssid_name)){
			isMatch=mNumeric.equals("52501")|| mNumeric.equals("52502");	
		}
		Log.i(TAG,"simcard is needed card? "+isMatch);
		return isMatch;
	}

/*	
	private void updateApgemini(int simslot , WifiConfiguration config, WifiManager mWifiManager) {
		if(config.toString().contains("eap: SIM"))
	    {
	        config.imsi = makeNAI(mTelephonyManagerEx.getSubscriberId(simslot),"SIM");
	    }
		else
		{
		    config.imsi = makeNAI(mTelephonyManagerEx.getSubscriberId(simslot),"AKA");
		}
		if (simslot == 0){
			config.simSlot = addQuote("0");
		}
		if (simslot == 1){
			config.simSlot = addQuote("1");
		}
        config.pcsc = addQuote("rild");
		Log.i(TAG, "WifiSettingsReceiver onReceive [updateApgemini] wifi enabled config.imsi = " + config.imsi + ";config.simSlot = " + config.simSlot);
		mWifiManager.save(config, null);
	}

	private void updateAp(WifiConfiguration config, WifiManager mWifiManager) {
		if(config.toString().contains("eap: SIM"))
	    {
	        config.imsi = makeNAI(mTm.getSubscriberId(),"SIM");
	    }
		else
		{
		    config.imsi = makeNAI(mTm.getSubscriberId(),"AKA");
		}
		config.simSlot = addQuote("0");
        config.pcsc = addQuote("rild");
		Log.i(TAG, "WifiSettingsReceiver onReceive [updateAp] wifi enabled config.imsi = " + config.imsi + ";config.simSlot = " + config.simSlot);
		mWifiManager.save(config, null);
	}
	*/
}

