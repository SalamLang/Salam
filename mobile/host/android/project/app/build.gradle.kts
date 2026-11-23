plugins {
    alias(libs.plugins.android.application)
}

val appId = project.findProperty("appId")?.toString() ?: "app.salam.demo"
val appName = project.findProperty("appName")?.toString() ?: "Salam App"
val appVersion = project.findProperty("appVersion")?.toString() ?: "1.0"
val appVersionCode = project.findProperty("appVersionCode")?.toString()?.toIntOrNull() ?: 1
val minSdkVersion = project.findProperty("minSdkVersion")?.toString()?.toIntOrNull() ?: 21
val targetSdkVersion = project.findProperty("targetSdkVersion")?.toString()?.toIntOrNull() ?: 35

android {
    namespace = "ir.salamlang.host"
    compileSdk = 36

    defaultConfig {
        applicationId = appId
        minSdk = minSdkVersion
        targetSdk = targetSdkVersion
        versionCode = appVersionCode
        versionName = appVersion
        resValue("string", "app_name", appName)
    }

    val storeFilePath = project.findProperty("storeFile")?.toString()
    if (!storeFilePath.isNullOrBlank()) {
        signingConfigs {
            create("release") {
                storeFile = file(storeFilePath)
                storePassword = project.findProperty("storePassword")?.toString()
                keyAlias = project.findProperty("keyAlias")?.toString()
                keyPassword = project.findProperty("keyPassword")?.toString()
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            signingConfig =
                if (storeFilePath.isNullOrBlank()) {
                    signingConfigs.getByName("debug")
                } else {
                    signingConfigs.getByName("release")
                }
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    buildFeatures {
        resValues = true
    }
    val splitAbis = project.findProperty("splitAbis")?.toString()
    if (!splitAbis.isNullOrBlank()) {
        splits {
            abi {
                isEnable = true
                reset()
                include(*splitAbis.split(",").toTypedArray())
                isUniversalApk = false
            }
        }
    }
    packaging {
        jniLibs {
            useLegacyPackaging = true
        }
    }
}
