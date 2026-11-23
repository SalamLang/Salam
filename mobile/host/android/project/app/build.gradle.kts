plugins {
    alias(libs.plugins.android.application)
}

val appId = project.findProperty("appId")?.toString() ?: "app.salam.demo"
val appName = project.findProperty("appName")?.toString() ?: "Salam App"
val appVersion = project.findProperty("appVersion")?.toString() ?: "1.0"
val appVersionCode = project.findProperty("appVersionCode")?.toString()?.toIntOrNull() ?: 1

android {
    namespace = "ir.salamlang.host"
    compileSdk = 36

    defaultConfig {
        applicationId = appId
        minSdk = 21
        targetSdk = 35
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
    packaging {
        jniLibs {
            useLegacyPackaging = true
        }
    }
}
