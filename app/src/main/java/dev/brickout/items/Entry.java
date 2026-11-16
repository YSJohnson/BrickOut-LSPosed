package dev.brickout.items;
import android.content.pm.PackageManager;
import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.IXposedHookZygoteInit;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.callbacks.XC_LoadPackage;
import java.io.File;
public class Entry implements IXposedHookLoadPackage, IXposedHookZygoteInit {
    private static String libraryPath;
    public void initZygote(StartupParam param) {
        libraryPath = new File(param.modulePath).getParent() + "/lib/arm64/libbrickout.so";
    }
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam p) {
        if (!"com.puzzle1studio.go.brickoutshoottheball".equals(p.packageName) || !p.packageName.equals(p.processName)) return;
        XposedBridge.hookAllMethods(android.app.Application.class, "attach", new XC_MethodHook() {
            protected void afterHookedMethod(MethodHookParam param) {
                android.content.Context context = (android.content.Context) param.args[0];
                try {
                    PackageManager pm = context.getPackageManager();
                    if (!"2623.0.0".equals(pm.getPackageInfo(context.getPackageName(), 0).versionName)) return;
                    System.load(libraryPath);
                    start();
                } catch (Throwable e) { XposedBridge.log(e); }
            }
        });
    }
    private static native void start();
}
