/* Build
 * ./mk -o=HAVE_CMMB_FEATURE=yes mt6589_phone_qhdv2 mm ./mediatek/frameworks/base/tests/JavaLibraryProGuard/
 * Execute
 * adb shell am instrument -w -e class com.mediatek.javalibrary.proguard.test.ProGuardTest com.mediatek.javalibrary.proguard.test/android.test.InstrumentationTestRunner
 */

package com.mediatek.javalibrary.proguard.test;

import android.app.Instrumentation;
import android.test.AndroidTestCase;
import android.util.Log;
import java.lang.reflect.Method;
import junit.framework.Assert;

//Target Package pluginmanager
import com.mediatek.pluginmanager.PluginManager;
import android.content.pm.Signature;

public class ProGuardTest extends AndroidTestCase {
    private static final String TAG = "JavaLibraryProGuardTest";
    private boolean CMMBObjNameObfuscated = true;
    private boolean PushParserObjNameObfuscated = true;
    private boolean GeminiObjNameObfuscated = true;
    private boolean PluginManagerObjNameObfuscated = true;
    private boolean ANRManagerObjNameObfuscated = true;

    private boolean CMMBAMethodName = false;
    private boolean PushParserAMethodName = false;
    private boolean GeminiAMethodName = false;
    private boolean PluginManagerAMethodName = false;
    private boolean ANRManagerAMethodName = false;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Log.i(TAG, " Setup JavaLibrary ProGuard Test Case ");

    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        Log.i(TAG, " TearDown JavaLibrary ProGuard Test Case ");
    }

    public void testPluginManagerProGuard() throws Throwable {

        Log.i(TAG, " testPluginManagerProGuard ");
        Signature [] mSignature = null;

        PluginManager<String> checkClass = PluginManager.create(getContext(),"testPluginManagerProGuard", mSignature);
        Method[] methods = checkClass.getClass().getDeclaredMethods();

        for ( Method checkMethodName : methods) {
            if (PluginManagerObjNameObfuscated && checkMethodName.getName().equals("checkPermission")){
              PluginManagerObjNameObfuscated = false;
            }
            if (!PluginManagerAMethodName && checkMethodName.getName().equals("a")){
              PluginManagerAMethodName = true;
            }
        }

        Log.i(TAG,"PluginManager checkMethodName = checkPermission is obfusted : " + PluginManagerObjNameObfuscated);
        Log.i(TAG,"PluginManager checkMethodName = a is                        : " + PluginManagerAMethodName);
        assertTrue("PluginManager object name is not obfuscated",(PluginManagerObjNameObfuscated & PluginManagerAMethodName));

    }

}

