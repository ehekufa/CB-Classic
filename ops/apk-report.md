# Отчёт сборки APK

- дата: 2026-11-23T12:13:29Z
- SDL: release-2.30.9
- NDK: 26.1.10909125
- Gradle: 8.7

## APK

```
-rw-r--r-- 1 runner runner 31928029 Nov 23 12:13 dist/cb-classic-debug.apk
3929aeeff2042f8322773f44324ffce1f0c3fba676a9c82a70043af2b5210311  dist/cb-classic-debug.apk
     1056  1981-01-01 01:01   classes.dex
  1309008  1981-01-01 01:01   lib/arm64-v8a/libSDL2.so
  1330832  1981-01-01 01:01   lib/arm64-v8a/libc++_shared.so
   482712  1981-01-01 01:01   lib/arm64-v8a/libmain.so
   918044  1981-01-01 01:01   lib/armeabi-v7a/libSDL2.so
  1109732  1981-01-01 01:01   lib/armeabi-v7a/libc++_shared.so
   304920  1981-01-01 01:01   lib/armeabi-v7a/libmain.so
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
    41907  1981-01-01 01:01   META-INF/MANIFEST.MF
---------                     -------
 35330736                     333 files
```

всего записей: 333

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
> Task :app:generateDebugResValues
> Task :app:checkDebugAarMetadata
> Task :app:javaPreCompileDebug
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
> Task :app:mergeDebugJavaResource
> Task :app:mergeDebugStartupProfile
> Task :app:preReleaseBuild UP-TO-DATE
> Task :app:checkDebugDuplicateClasses
> Task :app:processDebugResources
> Task :app:configureCMakeRelease[arm64-v8a]

> Task :app:compileDebugJavaWithJavac
Note: Some input files use or override a deprecated API.
Note: Recompile with -Xlint:deprecation for details.

> Task :app:dexBuilderDebug
> Task :app:mergeDebugGlobalSynthetics
> Task :app:mergeExtDexDebug
> Task :app:mergeLibDexDebug
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

BUILD SUCCESSFUL in 54s
39 actionable tasks: 39 executed
код возврата gradle: 0
```
