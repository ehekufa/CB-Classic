# Отчёт сборки APK

- дата: 2026-11-23T16:30:16Z
- SDL: release-2.30.9
- NDK: 26.1.10909125
- Gradle: 8.7
- запуск: https://github.com/ehekufa/CB-Classic/actions/runs/45216703920
- APK в запуске: артефакт `cb-classic-debug-apk`

## APK

```
-rw-r--r-- 1 runner runner 33224362 Nov 23 16:30 dist/cb-classic-debug.apk
0f700e995b8f0fc68d8c30bbb99a5eb9488b242caefa5051535d4f47b91bf1c6  dist/cb-classic-debug.apk
     1056  1981-01-01 01:01   classes.dex
  1309008  1981-01-01 01:01   lib/arm64-v8a/libSDL2.so
  1330832  1981-01-01 01:01   lib/arm64-v8a/libc++_shared.so
   732640  1981-01-01 01:01   lib/arm64-v8a/libmain.so
   918044  1981-01-01 01:01   lib/armeabi-v7a/libSDL2.so
  1109732  1981-01-01 01:01   lib/armeabi-v7a/libc++_shared.so
   492572  1981-01-01 01:01   lib/armeabi-v7a/libmain.so
    43863  1981-01-01 01:01   assets/assets/project.lua
  4086673  1981-01-01 01:01   assets/assets/scenes/26_igra/016_fon/images/0222_fon.png
  4293837  1981-01-01 01:01   assets/assets/scenes/26_igra/016_fon/images/0604_fon_so_snegom.png
      274  1981-01-01 01:01   assets/assets/scenes/26_igra/017_ten_igroka/images/0015_ten_igroka.png
      274  1981-01-01 01:01   assets/assets/scenes/26_igra/018_ten_vraga/images/0016_ten_vraga.png
     4677  1981-01-01 01:01   assets/assets/scenes/26_igra/019_krov/images/0605_krov.png
      383  1981-01-01 01:01   assets/assets/scenes/26_igra/020_pricel/images/0018_pricel.png
      274  1981-01-01 01:01   assets/assets/scenes/26_igra/021_igrok/images/0019_igrok_1.png
     5505  1981-01-01 01:01   assets/assets/scenes/26_igra/021_igrok/images/0021_igrok_3.png
      196  1981-01-01 01:01   assets/assets/scenes/26_igra/021_igrok/images/0022_igrok_4.png
     1011  1981-01-01 01:01   assets/assets/scenes/26_igra/021_igrok/images/0023_igrok_5.png
      334  1981-01-01 01:01   assets/assets/scenes/26_igra/021_igrok/images/0024_igrok_6.png
     1037  1981-01-01 01:01   assets/assets/scenes/26_igra/021_igrok/images/0026_igrok_8.png
...
    42537  1981-01-01 01:01   META-INF/MANIFEST.MF
---------                     -------
 37774344                     339 files
```

всего записей: 339


## Проверка содержимого APK

```
### dex
classes.dex: 1056 байт
  ссылок на org/cbclassic: 3
  ссылок на org/libsdl:    0
  MainActivity:            0
classes2.dex: 7680 байт
  ссылок на org/cbclassic: 3
  ссылок на org/libsdl:    1
  MainActivity:            3
classes3.dex: 97980 байт
  ссылок на org/cbclassic: 0
  ссылок на org/libsdl:    54
  MainActivity:            0

### ресурсы
всего записей: 339
assets/:        324
  png:          167
  mp3:          8
  generated/lua:136
  src/lua:      6
  project.lua:  1
  index.json:   1
  layout.lua:   1
  font Roboto:  1
  font DejaVu:  1

### манифест
package: name='org.cbclassic' versionCode='1' versionName='2.8' platformBuildVersionName='14' platformBuildVersionCode='34' compileSdkVersion='34' compileSdkVersionCodename='14'
targetSdkVersion:'34'
application-label:'Cubic battle'
launchable-activity: name='org.cbclassic.MainActivity'  label='' icon=''
native-code: 'arm64-v8a' 'armeabi-v7a'

activity/action в манифесте:
        A: http://schemas.android.com/apk/res/android:name(0x01010003)="android.permission.INTERNET" (Raw: "android.permission.INTERNET")
          E: activity (line=24)
            A: http://schemas.android.com/apk/res/android:name(0x01010003)="org.cbclassic.MainActivity" (Raw: "org.cbclassic.MainActivity")
                    A: http://schemas.android.com/apk/res/android:name(0x01010003)="android.intent.action.MAIN" (Raw: "android.intent.action.MAIN")
                    A: http://schemas.android.com/apk/res/android:name(0x01010003)="android.intent.category.LAUNCHER" (Raw: "android.intent.category.LAUNCHER")

подпись:
V2 Signer: certificate DN: C=US, O=Android, CN=Android Debug
V2 Signer: certificate SHA-256 digest: 46f956ed61e6bd3a9922281cee61e9046a611a0f18e6f69015b96414e69736b9
V2 Signer: certificate SHA-1 digest: 09bf5fa0401ef747f52767a58e1c3804fe9b6cee
V2 Signer: certificate MD5 digest: 6b8481a39a3c9756c3b1fdd2e8c2abde
WARNING: META-INF/com/android/build/gradle/app-metadata.properties not protected by signature. Unauthorized modifications to this JAR entry will not be detected. Delete or move the entry outside of META-INF/.

### основные lua-модули
assets/generated/lua/scenes/01/index.lua: 909 байт
assets/generated/lua/scenes/01/obj_003.lua: 751 байт
assets/generated/lua/scenes/01/obj_004.lua: 459 байт
```

## Хвост лога сборки

```

Welcome to Gradle 8.7!

Here are the highlights of this release:
 - Compiling and testing with Java 22
 - Cacheable Groovy script compilation
 - New methods in lazy collection properties

For more details see https://docs.gradle.org/8.7/release-notes.html

To honour the JVM settings for this build a single-use Daemon process will be forked. For more on this, please refer to https://docs.gradle.org/8.7/userguide/gradle_daemon.html#sec:disabling_the_daemon in the Gradle documentation.
Daemon will be stopped at the end of the build 
> Task :app:preBuild UP-TO-DATE
> Task :app:preDebugBuild UP-TO-DATE
> Task :app:mergeDebugNativeDebugMetadata NO-SOURCE
> Task :app:javaPreCompileDebug
> Task :app:checkDebugAarMetadata
> Task :app:generateDebugResValues
> Task :app:mapDebugSourceSetPaths
> Task :app:generateDebugResources
> Task :app:packageDebugResources
> Task :app:mergeDebugResources
> Task :app:createDebugCompatibleScreenManifests
> Task :app:extractDeepLinksDebug
> Task :app:parseDebugLocalResources
> Task :app:processDebugMainManifest
> Task :app:processDebugManifest
> Task :app:mergeDebugShaders
> Task :app:compileDebugShaders NO-SOURCE
> Task :app:generateDebugAssets UP-TO-DATE
> Task :app:mergeDebugAssets
> Task :app:compressDebugAssets
> Task :app:desugarDebugFileDependencies
> Task :app:processDebugJavaRes NO-SOURCE
> Task :app:processDebugManifestForPackage
> Task :app:checkDebugDuplicateClasses
> Task :app:mergeDebugJavaResource
> Task :app:mergeDebugStartupProfile
> Task :app:preReleaseBuild UP-TO-DATE
> Task :app:mergeExtDexDebug
> Task :app:mergeLibDexDebug
> Task :app:processDebugResources
> Task :app:configureCMakeRelease[arm64-v8a]

> Task :app:compileDebugJavaWithJavac
Note: Some input files use or override a deprecated API.
Note: Recompile with -Xlint:deprecation for details.

> Task :app:dexBuilderDebug
> Task :app:mergeDebugGlobalSynthetics
> Task :app:mergeProjectDexDebug
> Task :app:buildCMakeRelease[arm64-v8a]
> Task :app:configureCMakeRelease[armeabi-v7a]
> Task :app:buildCMakeRelease[armeabi-v7a]
> Task :app:mergeDebugJniLibFolders
> Task :app:mergeDebugNativeLibs
> Task :app:validateSigningDebug
> Task :app:writeDebugAppMetadata
> Task :app:writeDebugSigningConfigVersions
> Task :app:stripDebugDebugSymbols
> Task :app:packageDebug
> Task :app:createDebugApkListingFileRedirect
> Task :app:assembleDebug

BUILD SUCCESSFUL in 1m 44s
39 actionable tasks: 39 executed
код возврата gradle: 0
```
