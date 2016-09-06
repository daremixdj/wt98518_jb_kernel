package com.mediatek.backuprestore;

import com.mediatek.backuprestore.utils.Constants;
import com.mediatek.backuprestore.utils.Constants.ContactType;

public class ContactItemData {

    private int mSimId;
    private boolean mIsChecked;
    private String mContactName;
    private int mSimColor;

    public ContactItemData(int simId, boolean isChecked, String contactName, int simColor) {
        mSimId = simId;
        mIsChecked = isChecked;
        mContactName = contactName;
        mSimColor = simColor;
    }

    public int getSimId() {
        return mSimId;
    }
    
    public String getmContactName() {
        return mContactName;
    }

    public int getIconId() {
        int ret = ContactType.DEFAULT;
        switch (mSimColor) {
        case ContactType.SIMID_PHONE:
            ret = R.drawable.contact_phone_storage;
            break;
        case ContactType.SIMID_SIM1:
            ret = R.drawable.ic_contact_account_sim_blue;
            break;
        case ContactType.SIMID_SIM2:
            ret = R.drawable.ic_contact_account_sim_orange;
            break;
        default:
            break;
        }
        return ret;
    }

    public boolean isChecked() {
        return mIsChecked;
    }

    public void setChecked(boolean checked) {
        mIsChecked = checked;
    }
}