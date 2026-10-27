plugins {
    id("com.android.application") version "8.13.2"
}

android {
    namespace = "io.github.mh4u.shell"
    compileSdk = 36

    defaultConfig {
        applicationId = "io.github.mh4u.shell"
        minSdk = 26
        targetSdk = 35
        versionCode = 1
        versionName = "0.1.0-shell"
    }
}
