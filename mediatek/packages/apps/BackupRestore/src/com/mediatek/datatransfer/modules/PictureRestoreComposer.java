/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.datatransfer.modules;

import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore.Images;
import android.provider.MediaStore;

import com.mediatek.datatransfer.utils.BackupZip;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.SDCardUtils;
import com.mediatek.datatransfer.utils.Constants.ModulePath;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

/**
 * Describe class <code>PictureRestoreComposer</code> here.
 *
 * @author 
 * @version 1.0
 */
public class PictureRestoreComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/PictureRestoreComposer";
    //private static final String mStoragepath = "/mnt/sdcard2/.backup/Data/picture";
    private int mIndex;
    private File[] mFileList;
    private ArrayList<String> mExistFileList = null;
    private static final String[] mProjection = new String[] { MediaStore.Images.Media._ID,
                                                               MediaStore.Images.Media.DATA };

    /**
     * Creates a new <code>PictureRestoreComposer</code> instance.
     *
     * @param context a <code>Context</code> value
     */
    public PictureRestoreComposer(Context context) {
        super(context);
    }

    public int getModuleType() {
        return ModuleType.TYPE_PICTURE;
    }

    public int getCount() {
        int count = 0;
        if (mFileList != null) {
            count = mFileList.length;
        }

        MyLogger.logD(CLASS_TAG, "getCount():" + count);
        return count;
    }

    public boolean init() {
        boolean result = false;
        String path = mParentFolderPath + File.separator + ModulePath.FOLDER_PICTURE;
        File folder = new File(path);
        if(folder.exists() && folder.isDirectory()) {
            mFileList = folder.listFiles();
            mExistFileList = getExistFileList(path);
            result = true;
        }

        MyLogger.logD(CLASS_TAG, "init():" + result + ",count:" + getCount());
        return result;
    }

    public boolean isAfterLast() {
        boolean result = true;
        if (mFileList != null) {
            result = (mIndex >= mFileList.length) ? true : false;
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    public boolean implementComposeOneEntity() {
        boolean result = false;
        if (mFileList != null) {
            File file = mFileList[mIndex++];
            String tmpName = File.separator + file.getName();
            String destName = SDCardUtils.getExternalStoragePath() + File.separator + "Pictures" + tmpName;
            
            if(!mExistFileList.contains(tmpName)) {
            	try {
                    copyFile(file.getAbsolutePath(), destName);
                } catch (IOException e) {
                    MyLogger.logD(CLASS_TAG, "copy file fail");
                }
            } else {
                MyLogger.logD(CLASS_TAG, "already exist");
            }
            
            Uri data = Uri.parse("file://" + destName);
            mContext.sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, data));
            
            result = true;
        }

        return result;
    }

    private ArrayList<String> getExistFileList(String path) {
        ArrayList<String> fileList = new ArrayList<String>();
        
        String sPath = SDCardUtils.getStoragePath();
        String externalSDPath = "%" + sPath.subSequence(0, sPath.lastIndexOf(File.separator)) + "%";
        Cursor cur = mContext.getContentResolver().query(Images.Media.EXTERNAL_CONTENT_URI,
                mProjection, MediaStore.Images.Media.DATA + " not like ?",
                new String[] { externalSDPath }, null);
        
        if(cur != null ) {
            if(cur.moveToFirst()) {
                while(!cur.isAfterLast()) {
                    int dataColumn = cur.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
                    String data = cur.getString(dataColumn);
                    if(data != null) {
                    	try {
                            String tmpName = data.subSequence(data.lastIndexOf(File.separator), data.length()).toString();
                            fileList.add(tmpName);
                    	} catch (StringIndexOutOfBoundsException e) {
                        	e.printStackTrace();
                        } catch(Exception e) {
                            e.printStackTrace();
                        }
                    }
                    cur.moveToNext();
                }
            }

            cur.close();
        }

        return fileList;
    }

    // private void deleteFolder(File file) {
    //     if (file.exists()) {
    //         if (file.isFile()) {
    //             int count = mContext.getContentResolver().delete(
    //                     MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
    //                     MediaStore.Images.Media.DATA + " like ?",
    //                     new String[] { file.getAbsolutePath() });
    //             MyLogger.logD(CLASS_TAG, "deleteFolder():" + count + ":" + file.getAbsolutePath());
    //             file.delete();
    //         } else if (file.isDirectory()) {
    //             File files[] = file.listFiles();
    //             for (int i = 0; i < files.length; ++i) {
    //                 this.deleteFolder(files[i]);
    //             }
    //         }

    //         file.delete();
    //     }
    // }

//     private void deleteRecord(String path) {
//         if(mFileList != null) {
//             int count = 0;
//             for(File file : mFileList) {
//                 count += mContext.getContentResolver().delete(MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
//                                                               MediaStore.Images.Media.DATA + " like ?",
//                                                               new String[] { file.getAbsolutePath() });
//                 MyLogger.logD(CLASS_TAG, "deleteRecord():" + count + ":" + file.getAbsolutePath());
//             }
//
//             MyLogger.logD(CLASS_TAG, "deleteRecord():" + count + ":" + path);
//         }
//     }

	private void copyFile(String srcFile, String destFile) throws IOException {
		try {
			File f1 = new File(srcFile);
			if (f1.exists() && f1.isFile()) {
				InputStream inStream = new FileInputStream(srcFile);
				FileOutputStream outStream = new FileOutputStream(destFile);
				byte[] buf = new byte[1024];
                int byteRead = 0;
				while ((byteRead = inStream.read(buf)) != -1) {
					outStream.write(buf, 0, byteRead);
				}
				outStream.flush();
				outStream.close();
				inStream.close();
			}
        } catch(IOException e) {
            throw e;
		} catch (Exception e) {
            e.printStackTrace();
		}
	}
	
    public void onStart() {
        super.onStart();

        String path = SDCardUtils.getExternalStoragePath() + File.separator + "Pictures";
        File folder = new File(path);
        if (!folder.exists()) {
        	folder.mkdirs();
        }

    }
}
