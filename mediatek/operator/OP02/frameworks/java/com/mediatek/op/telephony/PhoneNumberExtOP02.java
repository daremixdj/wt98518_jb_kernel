package com.mediatek.op.telephony;
import com.mediatek.common.featureoption.FeatureOption;
import android.util.Log;

public class PhoneNumberExtOP02 extends PhoneNumberExt {
    static final String TAG = "PhoneNumberExt_SEA";

    public boolean isCustomizedEmergencyNumber(String number, String plusNumber, String numberPlus) {
        // Customized ecc number
        if(FeatureOption.WT_HTC_ECC_NUMBER_CALL){
		        Log.d(TAG, "isCustomizedEmergencyNumber, number: " + number);
		        String []emergencyNumList = {"112", "911", "191", "192", "199"};
		
		        for (String emergencyNum : emergencyNumList) {
		            numberPlus = emergencyNum + "+";
		            if (emergencyNum.equals(number)
		                 || numberPlus.equals(number)) {
		                return true;
		            }
		        }
	      }else{
        	//add by wangwentao
						Log.d(TAG, "isCustomizedEmergencyNumber | number = " + number);
						String []emergencyNumList = {"112", "911", "110", "119"};
				        for (String emergencyNum : emergencyNumList) {
				            numberPlus = emergencyNum + "+";
				            if (emergencyNum.equals(number)
				                 || numberPlus.equals(number)) {
				                return true;
				            }
				        }
						//add by wangwentao
				}
        return false;
    }

    public boolean isSpecialEmergencyNumber(String dialString) {
        /* These special emergency number will show ecc in MMI but sent to nw as normal call */
        if(FeatureOption.WT_HTC_ECC_NUMBER_CALL){
		       	Log.d(TAG, "isSpecialEmergencyNumber, dialString: " + dialString);
		        String []emergencyNumList = {"191", "192", "199"};
		
		        for (String emergencyNum : emergencyNumList) {
		            if (emergencyNum.equals(dialString)) {
		                return true;
		            }
		        }
        }else{
	        if (FeatureOption.MTK_CTA_SET) {
	            String [] emergencyCTAList = {"112", "911", "110", "119"};
	            String numberPlus = null;
	            for (String emergencyNum : emergencyCTAList) {
	                numberPlus = emergencyNum + "+";
	                if (emergencyNum.equals(dialString)
	                    || numberPlus.equals(dialString)) {
	                    Log.d(TAG, "[isSpecialEmergencyNumber] CTA list: " + "return true");
	                    return true;
	                }
	            }
	        }
	      }
        
        return false;

    }

    public boolean isCustomizedEmergencyNumberExt(String number, String plusNumber, String numberPlus) {
        // Customized ecc number when SIM card is not inserted
        //add by wangwentao
		Log.d(TAG, "isCustomizedEmergencyNumberExt | number = " + number);
        String []emergencyNumList = {"112", "911", "000", "08", "110", "118", "119", "999", "120", "122"};
        for (String emergencyNum : emergencyNumList) {
            numberPlus = emergencyNum + "+";
            if (emergencyNum.equals(number)
                 || numberPlus.equals(number)) {
                return true;
            }
        }
		//add by wangwentao
        return false;
    }
}
